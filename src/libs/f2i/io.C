/* $Id: io.C,v 1.3 1998/02/21 17:28:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#define IO 1
/*
 *
 * we don't need anything in the case for FORMAT ... it gets handled
 * by the GenerateFormat call in the READ, WRITE, and PRINT cases.
 * I don't think OPEN, CLODE, ENDFILE, REWIND, ... need GenerateFormat
 *
 */

/* io.c -- generate the code for IO statements 
 *
 *
 * The plan:
 *
 *	General Case:
 *
 *	1) write a procedure that does the required IO onto the 
 *	   end of the IO source file.
 *
 *	2) generate a template to call the IO procedure and pass
 *	   it to generate_call()
 *
 *	Initializations:
 *
 *	1) open the IO source file
 *
 * Details:
 *
 *	PrePass:  all of this activity must be coordinated with the 
 *	code over in aiPrePass, since the calls generated to io routines
 *	will push actual parameters on the stack.  To accomplish this
 *	in a relatively robust way, the routine BuildIOL() is called
 *	from the PrePass.  For each IO statement, it builds a data 
 *	structure that allows (1) easy determination of the number of
 *	stack slots required for parameter passing, and (2) easy 
 *	contruction of the call template (at later time).
 *
 *	Thus, for every IO statement, there is some code over in the
 *	PrePass to coordinate this activity.
 *
 *	Statement Generation time: as IO statements are encountered in
 *	aiStmtList(), they invoke DoIO().  If the IO statement requires 
 *	code in the executable (FORMAT does not), then DoIO() generates
 *	a procedure header, a copy of the IO statement, a copy of any
 *	referenced FORMAT statement, and an END statement.  This is all
 *	written to a file named "<procedure name>.io.f".
 *
 *	Next, a call template is generated and passed to GenerateCall().
 *	That takes care of the iloc side of things.
 *
 *	Tricky stuff: the call template needs an identifier node for each 
 *	actual parameter.  This is easy for variables named in the IO 
 *	statement.  If, however, the statement names an array of variable
 *	dimension, the iloc code must pass the appropriate dimensioning
 *	variables.  Finding these is easy; finding identifier nodes for 
 *	them is not.  The code plays a three card Monte game to get the
 *	necessary node.  Over in routine.c, formal parameters get their
 *	IDENTIFIER node AST_INDEXes stuffed into the data_value field of
 *	their ai_SymTable entries.  The code in BuildIOL() and its children
 *	can in a recognizably illegal value during the PrePass.  Finally,
 *	GenerateHeader() sees the illegal value and replaces it with the 
 *	right AST_INDEX.   -- The PrePass runs before the code in routine.c,
 *	so that we have to resort to this intricate dance.
 *
 *	Note:  if all the nodes for IO statments were built consistently,
 *	this code would be much shorter.
 *
 *			--- kdc 06/02/1991
 *
 */
#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/mnemonics.h>
#include <libs/f2i/call.h>

/* some globals shared with label.c, stmt.c, and ai.c */
extern   FortTextTree ftt;
extern   FortTree     ft;

static int counter   = 0;
static int initial   = 1;
static int open_file = 1;
static FILE *fd;

static struct CallTemplate Template;
static AST_INDEX space_for_ACTUALS[MAXPARMS];
static int	 space_for_ACTUALTYPES[MAXPARMS];
static int	 space_for_ACTUALMODS[MAXPARMS];
static int	 space_for_ACTUALUSES[MAXPARMS];
static int	 space_for_ACTUALREG[MAXPARMS];

static char name[256];

/* a couple of tables to allow us to iterate over the set of valid ILOC types */

static int ITypes[] = 
  {TYPE_CHARACTER, TYPE_LOGICAL, TYPE_INTEGER, TYPE_REAL, 
   TYPE_COMPLEX, TYPE_DOUBLE_PRECISION};

static char *FNames[] =
  {"character *(*) ", "logical ", "integer ", "real ", "complex ", 
   "double precision "};

#define NTYPES 6

struct IOL
{
  struct IOL *next;
  AST_INDEX  stmt;
  AST_INDEX  node[MAXPARMS];
  int	list[MAXPARMS];
  int	mod[MAXPARMS];
  int   ref[MAXPARMS];
  int	num;		/* number of parameters			*/
};

static struct IOL *IOLHead = (struct IOL*) 0;

/* forward declarations */

static void GenerateStmt(AST_INDEX);
static void GenerateHeader(AST_INDEX, int);
static void WalkList(struct IOL *,AST_INDEX,int,int);
static void AddName(struct IOL *,AST_INDEX,int, int, int);



/* determine type of I/O statement and generate appropriate code */
void DoIO(int type, AST_INDEX node)
//   int       type;
//   AST_INDEX node;
{
  char      fname[100];

#ifdef IO
  if (open_file)
  {
    (void) sprintf(fname, "%s.io.f", proc_text);
    fd = fopen(fname, "w");
    open_file = 0;
  }

  if (initial)
  {
    initial = 0;
    Template.Actuals	= space_for_ACTUALS;
    Template.ActualTypes= space_for_ACTUALTYPES;
    Template.ActualMods	= space_for_ACTUALMODS;
    Template.ActualUses = space_for_ACTUALUSES;
    Template.ActualReg  = space_for_ACTUALREG;
  }


  switch(type)
  { 
	case GEN_READ_SHORT:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		GenerateFormat(gen_READ_SHORT_get_format_identifier(node));
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_READ_LONG:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		GenerateFormat(gen_READ_LONG_get_kwd_LIST(node));
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_WRITE:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		GenerateFormat(gen_WRITE_get_kwd_LIST(node));
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_PRINT:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		GenerateFormat(gen_PRINT_get_format_identifier(node));
		(void) fprintf(fd,"        end\n");
		break;
/* additions start here */
	case GEN_OPEN:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_CLOSE:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_INQUIRE:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_BACKSPACE_SHORT:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_BACKSPACE_LONG:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_ENDFILE_SHORT:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_ENDFILE_LONG:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_REWIND_SHORT:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_REWIND_LONG:
		GenerateHeader(node, counter++);
		GenerateStmt(node);
		(void) fprintf(fd,"        end\n");
		break;

	case GEN_FORMAT:
		break;

/* end of additions */
	default:		/* may contain other statements */
		break;
  }
#endif
} /* DoIO */




/* generate FORTRAN statement from AST node */
static void GenerateStmt(AST_INDEX node)
  // AST_INDEX node;
{
  char *line;
  int  l1, l2, c1, c2;
  int i;

  ftt_NodeToText(ftt, node, &l1, &c1, &l2, &c2);
  line = (char *) ftt_GetTextLine(ftt, l1);
  if (c1 == 0)
     (void) fprintf(fd, "%.60s\n", &line[c1]);
  else 
     (void) fprintf(fd, "        %.60s\n", &line[c1]);
  i = c1 + 60;
  while (i<strlen(line)) {
    (void) fprintf(fd, "     *  %.60s\n",&line[i]);
    i += 60;
  }
} /* GenerateStmt */


/* generate FORTRAN declarations for variables in I/O subroutine */
static char array[100];

char *GenerateArrayDecl(int index)
  // int index;
{
  char *p, *q;
  int  i, bound;
  ArrayBound *bounds;	

  array[0] = '\0';
  (void) strcat(array, (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
  (void) strcat(array, "(");
  
  bounds = (ArrayBound *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_DIM_BOUNDS);

  for (i=0;i<fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS);i++)
  {
    if (i > 0)
       (void) strcat(array,",");

    bound = getIndexForlb (bounds, i, index);
    if (fst_my_GetFieldByIndex(ft_SymTable, bound, SYMTAB_STORAGE_CLASS) == SC_CONSTANT)
    {
      p = (char *) fst_my_GetFieldByIndex( ft_SymTable, bound, SYMTAB_NAME);
      q = array;
      while(*q!='\0')
	q++;
      while(*p != '[' && *p != '\0')
	*q++ = *p++;
      *q++ = ':';
      *q++ = '\0';
    }
    else 
    {
      p = (char *) fst_my_GetFieldByIndex( ft_SymTable, bound, SYMTAB_NAME);
      (void) strcat(array, p);
      (void) strcat(array,":");
    }

    bound = getIndexForub (bounds, i, index);
    if (fst_my_GetFieldByIndex(ft_SymTable, bound, SYMTAB_STORAGE_CLASS) == SC_CONSTANT)
    {
      p = (char *) fst_my_GetFieldByIndex(ft_SymTable, bound, SYMTAB_NAME);
      q = array;
      while(*q!='\0')  q++;
      while(*p != '[' && *p != '\0')  *q++ = *p++;
      *q++ = '\0';
    }
    else 
      {
	p = (char *) fst_my_GetFieldByIndex( ft_SymTable, bound, SYMTAB_NAME);
	(void) strcat(array, p);
      }
  }
  (void) strcat(array, ")");
  return array;   
} /* GenerateArrayDecl */



/* generate subroutine header */
static void GenerateHeader(AST_INDEX stmt, int cntr)
//   AST_INDEX stmt;
//   int cntr;
{
  int  first, i, j, column;
  char *parm;
  struct IOL *p;

  /* find the IOL for this statement */
  p = IOLHead;
  while (p->stmt != stmt)
  {
    if (p->next == (struct IOL *) 0)
	ERROR("GenerateHeader", "Logic error in io.c", FATAL);
    p = p->next;
  }

  /* print out the header and declarations...
     this part is REALLY ugly!!! */

  if (aiRocket)
    (void) sprintf(name, "IO_%s%03d", proc_text, cntr);
  else
    (void) sprintf(name, "%s%03d", proc_text, cntr);
  (void) fprintf(fd, "\tsubroutine %s(", name);
  column = 11 + strlen(name);

  for (i=0;i<p->num-1;i++)
  {
    parm = (char *) fst_my_GetFieldByIndex(ft_SymTable, p->list[i], SYMTAB_NAME);
    column += strlen(parm) + 1;
    if (column >= 60)	
    {
      fprintf(fd, "\n     *  ");
      column = 0;
    }
    (void) fprintf(fd, "%s,",parm);
  }
  if (p->num > 0) /* last one gets no comma! */
     (void) fprintf(fd, "%s",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, p->list[p->num-1], SYMTAB_NAME)); 
 
  (void) fprintf(fd, ")\n");

  for (j=0;j<NTYPES;j++)
  {
    first = 1;
    for (i=0;i<p->num;i++)
    {
      if (fst_my_GetFieldByIndex(ft_SymTable, p->list[i], SYMTAB_TYPE) == ITypes[j])
      {
        if (first)
        {
          (void) fprintf(fd, "\t%s", FNames[j]);
          column = strlen(FNames[j]);
          first = 0;
        }
	else
	  (void) fprintf(fd, ",");

        if (fst_my_GetFieldByIndex(ft_SymTable, p->list[i], SYMTAB_NUM_DIMS) !=0)
	  parm = GenerateArrayDecl(p->list[i]);
        else
          parm = (char *) fst_my_GetFieldByIndex(ft_SymTable, p->list[i], SYMTAB_NAME);

	if (column + strlen(parm) >= 60)
	{
	  fprintf(fd, "\n     *  ");
	  column = 0;
	}  
        column += strlen(parm) + 1; /* 1 is for comma */
	(void) fprintf(fd, "%s", parm);
		
      }
    }
    if (first == 0)
      (void) fprintf(fd, "\n");
  }
  /* fill in the call template */

  for (i=1;i<=p->num;i++)
  {
    if (p->node[i-1] == 0)	/* see note "Tricky stuff" above */
       Template.Actuals[i]  =
	      (AST_INDEX) fst_my_GetFieldByIndex(ft_SymTable, p->list[i-1], SYMTAB_EXPR);
    else
       Template.Actuals[i]  = p->node[i-1];
    Template.ActualTypes[i] =
			fst_my_GetFieldByIndex(ft_SymTable, p->list[i-1], SYMTAB_TYPE);
    Template.ActualMods[i]  = p->mod[i-1];
    Template.ActualUses[i]  = p->ref[i-1];
    Template.ActualReg[i]   = 0;
  }

  Template.name      = name;
  Template.CallSite   = AST_NIL;
  Template.InLibrary = 0;       /* not user generated code */
  Template.NumParms  = p->num;
  Template.ReturnReg = 0;	/* a subroutine, not a function */

  GenerateCall(&Template);
} /* GenerateHeader */




/* generate FORTRAN FORMAT statement from AST node */
void GenerateFormat(AST_INDEX node)
  // AST_INDEX node;
{
  AST_INDEX list;
  int type;

  if (is_list(node))
  {
    list = list_first(node);

    while (list != AST_NIL && gen_get_node_type(list) != GEN_FMT_SPECIFY)
       list = list_next(list);
    if (list != AST_NIL) /* a FMT SPECIFY */
       node = gen_FMT_SPECIFY_get_format_identifier(list);
    else
       node = AST_NIL;
  }

  type = gen_get_node_type(node);

  switch(type)
  {
    case GEN_STAR: /* no format statement */
	break;

    case GEN_LABEL_REF:
	node = LabelGetNode(gen_get_text(node));
	GenerateStmt(node);
	break;

    default:
	ERROR("DoIO", "Bad format specifier", WARNING);
	break;
  }
} /* GenerateFormat */




/* routine called by PrePass to build a data structure that allows */
/* easy determination of the number of stack slots required for    */
/* parameter passing and easy construction of the call template    */
void BuildIOL( AST_INDEX stmt, AST_INDEX kwds, AST_INDEX vars, int mod, int ref )
//   AST_INDEX stmt;
//   AST_INDEX kwds;
//   AST_INDEX vars;
//   int mod;
//   int ref;
{
  struct IOL *p, *q;

  p 		 = (struct IOL *) get_mem(sizeof(struct IOL), "IO list");
  p->next 	 = (struct IOL *) 0;
  p->stmt 	 = stmt;
  p->num  	 = 0;

  WalkList(p, kwds,/*mod*/0,/*ref*/1);	/* build the actual record */
  WalkList(p, vars, mod, ref);

  if (IOLHead == (struct IOL *) 0)	/* link it into the list */
     IOLHead = p;
  else
  {
    q = IOLHead;
    while(q->next != (struct IOL *) 0)
      q = q->next;
    q->next = p;
  }  
} /* BuildIOL */




/* called by BuildIOL to walk the AST and fill in the IOL structure */
static void WalkList(struct IOL *p, AST_INDEX node, int mod, int ref)
//   struct IOL *p;
//   AST_INDEX  node;
//   int	     mod;
//   int	     ref;
{
  int i, j, ub, dim;
  ArrayBound *bounds;

  if (is_list(node))		/* look at every element */
  {
    node = list_first(node);
    while(node != AST_NIL)
    {
      WalkList(p, node, mod, ref);
      node = list_next(node);
    }
  }
  else switch(gen_get_node_type(node))
  {
    case GEN_IDENTIFIER:	/* add it to the list of formals */
	i = getIndex(node);
	if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
	{
	  AddName(p, node, i, mod, ref);

	  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NUM_DIMS) > 0 &&
	      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) &
	      OC_IS_FORMAL_PAR)
	  {
	     bounds =
		(ArrayBound *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_DIM_BOUNDS);

	    ub = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NUM_DIMS);

	    for (j=0;j<ub;j++)
	    {
	      dim = getIndexForlb(bounds, j, i);
	      if (!(fst_my_GetFieldByIndex(ft_SymTable, dim, SYMTAB_STORAGE_CLASS) &
		  SC_CONSTANT))
	         AddName(p, /*nyk*/0, dim, /*mod*/0, /*ref*/1);
	      dim = getIndexForub(bounds, j, i);
	      if (!(fst_my_GetFieldByIndex(ft_SymTable, dim, SYMTAB_STORAGE_CLASS) &
		  SC_CONSTANT))
	         AddName(p, /*nyk*/0, dim, /*mod*/0, /*ref*/1);

	    }
	  }
	}
	break;

    case GEN_CONSTANT:	/* do nothing! */
	break;

    default:		/* recurse down into the tree */
	j = gen_how_many_sons(gen_get_node_type(node));
	for (i=1;i<=j;i++)
	    WalkList(p, ast_get_son_n(node,i), mod, ref);
	break;
   }
} /* WalkList */




/* add variable names with corresponding information to IOL structure */
static void AddName(struct IOL *p, AST_INDEX node, int index, int mod, int ref)
//   struct IOL *p;
//   AST_INDEX  node;
//   int index;
//   int mod;
//   int ref;
{
  register int i, found;

  found = 0;
  i = 0;
  while(i < p->num && found == 0)	/* check for duplication	*/
  {
    if (index == p->list[i])		/* it's already there 		*/
    {
      p->mod[i] |= mod;			/* fix the interprocedural info	*/
      p->ref[i] |= ref;
      found = 1;
    }
    i++;
  }

  if (found == 0)
  {
    i = p->num++;
    p->node[i] = node;
    p->list[i] = index;
    p->mod[i]  = mod;
    p->ref[i]  = ref;
  }
} /* AddName */




/* called from PrePass to retrieve actual parameters from IOL structure */
int getIOLActuals()
{
  struct IOL *p;
  register int total, count, i,j;

  total = 0;
  p	= IOLHead;
  while(p != (struct IOL *) 0)		/* for each IO-based call site */
  {
    count = 0;				/* count it's actuals */
    for (i=0;i<p->num;i++)
    {
      count++;
      j = p->list[i];
      if (fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_TYPE) == TYPE_CHARACTER)
	 count++;	/* implicit length field */

      if (fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_OBJECT_CLASS) & OC_IS_DATA &&
		/* not a formal! */
	  fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS) & SC_NO_MEMORY &&
		/* in register   */
	  fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NUM_DIMS) == 0 &&
		/* scalar	 */
	  fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_addressReg) == NO_REGISTER)
		/* no spill loc	 */
      {
	(void) getAddressRegister(j); /* mark it as having a spill loc	 */
	aiExpressionStackSpace +=
		SizeOfType(fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_TYPE));
      }
    }
    if (count > total)			/* now, find the maximum value */
       total = count;
    p = p->next;
  }
  return total * SizeOfType(TYPE_INTEGER);
} /* getIOLActuals */




/* generate the code for a FORTRAN PAUSE statement using the same technique employed for I/O  */
void HandlePause(AST_INDEX node)

  // AST_INDEX   node;

{
  char    fname[100];

  if (open_file)
  {
    (void) sprintf(fname, "%s.io.f", proc_text);
    fd = fopen(fname, "w");
    open_file = 0;
  }

  GenerateHeader(node, counter++);
  GenerateStmt(node);
  (void) fprintf(fd, "        end\n");

} /* HandlePause */




/* generate the code for a FORTRAN STOP statement using the same technique employed for I/O */
void HandleStop(AST_INDEX node)

  // AST_INDEX   node;

{
/*
  For ROCKET, we don't care if stop is handled like in FORTRAN.  We'll just
  halt the program

  char   fname[100];

  if (open_file)
  {
    (void) sprintf(fname, "%s.io.f", proc_text);
    fd = fopen(fname, "w");
    open_file = 0;
  }

  GenerateHeader(node, counter++);
  GenerateStmt(node);
  (void) fprintf(fd, "        end\n");
*/

  /*  let the flow analyzer know that this is a stop statement */
  generate(0, HALT, 0, 0, 0, "This is a stop statement");

} /* HandleStop */
