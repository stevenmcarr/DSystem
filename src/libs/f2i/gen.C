/* $Id: gen.C,v 1.11 2000/04/09 20:21:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <ctype.h>
#include <include/frontEnd/astnode.h>
#include <libs/frontEnd/include/gi.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>

#include <libs/f2i/mnemonics.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>

#include <assert.h>
#include <libs/support/lists/list.h>

/* some representation lookup routines */

/* forward declarations */
static char *InsertPeriod(char *);
extern int compares_by_type[];

/* and a macro to shorten the exposition */
#define MNEM	iloc_mnemonic

static char	complex_buffer[128];




/* generate a single iloc instruction with no more than three arguments */
void generate(int label, int op, Generic r1, Generic r2, Generic r3, char *comment)
//   char *comment;
//   int label, op, r1, r2, r3;
{
    /* return if no output is to be generated */
    if (aiGenerate != 0)
     return;

    /* print debugging output, as necessary */
    if (aiDebug > 0)
     (void) fprintf(stdout, "Generate(%d, %s(%d), %d, %d, %d, '%s');\n",
	     label, iloc_mnemonic(op), op, r1, r2, r3, comment);

    /* labels are optional, statement numbers are required on executable statements */
    if (label > 0)
      {
	if (op >= ILOC_EXECUTABLE)                      /* opcode is executable */
	 (void) fprintf(stdout, "LL%-4.4d:\t%d", label, aiStmtCount);
	else                                            /* opcode is not executable */
         (void) fprintf(stdout, "LL%-4.4d:\t", label);
      }
    else
      {
        if (op >= ILOC_EXECUTABLE)                      /* opcode is executable */
         (void) fprintf(stdout, "\t%d", aiStmtCount);
        else                                            /* opcode is not executable */
         (void) fprintf(stdout, "\t");
      }

    /* output instructions by format */
    switch(iloc_format(op))
    {
      case RR2R:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	r3 = fst_my_GetFieldByIndex(ft_SymTable, r3, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\tr%d\t=>\tr%d", MNEM(op), r1, r2, r3);
	break;
	
      case R2R:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\t=>\tr%d\t", MNEM(op), r1, r2);
	break;

      case RR:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\tr%d\t\t", MNEM(op), r1, r2);
	break;

      case RL:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\t[%s]\t\t", MNEM(op), r1, (char *) r2);
	break;

      case R:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\t\t\t", MNEM(op), r1);
	break;
	
      case CR2R:
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	r3 = fst_my_GetFieldByIndex(ft_SymTable, r3, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\t%d\tr%d\t=>\tr%d", MNEM(op), r1, r2, r3);
	break;
	
      case C2R:
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	 	
	if (op != iLDI)
	  (void) fprintf(stdout, "\t%-8.8s\t%d\t=>\tr%d\t", MNEM(op), r1, r2);
	else
	  switch(r3)
		{
		  case GEN_LABEL:
			(void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\t=>\tr%d\t",MNEM(op),
				r1, r2);
			break;
		  case GEN_NUMBER:
			(void) fprintf(stdout, "\t%-8.8s\t%d\t=>\tr%d\t",MNEM(op), r1, r2);
			break;
		  case GEN_STRING:
			(void) fprintf(stdout, "\t%-8.8s\t%s\t=>\tr%d\t",MNEM(op),
			  (char *) r1, r2);
			break;
		  default:
			(void) fprintf(stdout, "\tMISSING CASE\t");
  			(void) sprintf(error_buffer, 
			  "Missing case - format %s, op %s(%d), val %d",
			  format_name(iloc_format(op)), MNEM(op), op, r3);
			ERROR("Generate", error_buffer, WARNING);
			break;
		}
	break;

      case C2L:
	(void) fprintf(stdout, "\t%-8.8s\t%d\t=>\t%s\t", MNEM(op), r1, (char *) r2);
	break;
	
      case LLR:
	/* note that this assumes that the label is of "GEN_LABEL" format */
	r3 = fst_my_GetFieldByIndex(ft_SymTable, r3, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\tLL%-4.4d\tr%d\t", MNEM(op), r1, r2, r3);
	break;
	
      case C:
	switch (op)
	  {
            case BYTES:	/* print constant as an integer */
	      (void) fprintf(stdout, "\t%-8.8s\t%d\t\t\t", MNEM(op), r1);
	      break;

            case NAME:	/* print constant as a string */
	      (void) fprintf(stdout, "\t%-8.8s\t%s\t\t\t", MNEM(op), (char *) r1);
	      break;
	      
	    case JMPl:	/* print constant as a label */
	      (void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\t\t\t", MNEM(op), r1);
	      break;
	      
	    default:
		(void) sprintf(error_buffer,
			"Format %s for opcode %s(%d) is not handled by generate.",
	  		format_name(iloc_format(op)), MNEM(op), op);
		ERROR("Generate", error_buffer, FATAL);	
	      break;
	  }
	break;

      case CC:		/* call PrintData to print DATA statements */
	PrintData(op, r1, r2, r3);
        break;
        
      case L:
	(void) fprintf(stdout, "\t%-8.8s\t[%s]\t\t\t", MNEM(op), (char *) r1);
	break;

      case NA:
	(void) fprintf(stdout, "\t%-8.8s\t\t\t\t", MNEM(op));
	break;

	  case NF:
      default:
	(void) sprintf(error_buffer, "Format %s for opcode %s(%d) is not handled by generate.",
	  format_name(iloc_format(op)), MNEM(op), op);
	ERROR("Generate", error_buffer, FATAL);
	break;
    }

    /* and handle the optional comment */
    if (aiAnnotate > 0)
    {
       /* print comments on several lines if necessary */
//      int i = 60;
      if (*comment == '\0')
  	 (void) fprintf(stdout, "\n");
      else 
	 (void) fprintf(stdout, "\t# %s\n", comment );
//      while (i<strlen(comment)) {
//	(void) fprintf(stdout,"\t%d\t%-8.8s\t\t\t\t\t# %.60s\n",
//		       aiStmtCount,MNEM(NOP),&comment[i]);
//	i += 60;
//      }
    }
    else (void) fprintf(stdout, "\n");

  aiNumInstructions++;
} /* generate */




/* generate a single iloc instruction with no more than three arguments */
void generate_long(int label, int op, Generic r1, Generic r2, Generic r3, 
		   Generic r4, Generic r5, Generic r6, Generic r7, char *comment)
//   char *comment;
//   int label, op, r1, r2, r3, r4, r5, r6, r7;
{
  /* return if no output is to be generated */
  if (aiGenerate != 0)
     return;

  /* print debugging output, as necessary */
  if (aiDebug > 0)
     (void) fprintf(stdout, "Generate(%d, %s(%d), %d, %d, %d, '%s');\n",
	     label, iloc_mnemonic(op), op, r1, r2, r3, comment);

  /* labels are optional, statement numbers are required */
  if (label > 0)
       (void) fprintf(stdout, "LL%-4.4d:\t%d", label, aiStmtCount);
  else
       (void) fprintf(stdout, "\t%d", aiStmtCount);
 
  /* output instructions by format */
  switch(op)
    {
      case JSRr:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\tr%d\t%s\t[%s]\t[%s]",
		MNEM(op), r1, r2, (char *) r3, (char *) r4, (char *) r5);
	break;
	
      case iJSRr:
      case fJSRr:
      case dJSRr:
      case cJSRr:
      case qJSRr:
	r1 = fst_my_GetFieldByIndex(ft_SymTable, r1, SYMTAB_REG);
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\tr%d\tr%d\t%s\t=>\tr%d\t[%s]\t[%s]",
		MNEM(op), r1, r2, (char *) r3, r4, (char *) r5, (char *) r6);
	break;
	
      case JSRl:
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	
	if (r7 == GEN_LABEL)	/* the label is an iloc label */
	  (void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\tr%d\t%s\t[%s]\t[%s]",
		MNEM(op), r1, r2, (char *) r3, (char *) r4, (char *) r5);
		
	else if (r7 == GEN_STRING)	/* the label is a character string */
	  (void) fprintf(stdout, "\t%-8.8s\t%s\tr%d\t%s\t[%s]\t[%s]",
		MNEM(op), (char *) r1, r2, (char *) r3, (char *) r4, (char *) r5);
		
	else	/* invalid flag detected */
	  {
	  	(void) fprintf (stdout, "\tMISSING CASE\t");
	  	(void) sprintf (error_buffer, "Missing case - op %s(%d), val %d",
			MNEM(op), op, r7);
		ERROR("GenerateLong", error_buffer, WARNING); 
	  }
	break;
	
      case iJSRl:
      case fJSRl:
      case dJSRl:
      case cJSRl:
      case qJSRl:
	r2 = fst_my_GetFieldByIndex(ft_SymTable, r2, SYMTAB_REG);
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	
	if (r7 == GEN_LABEL)	/* the label is an iloc label */
	  (void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\tr%d\t%s\t=>\tr%d\t[%s]\t[%s]",
		MNEM(op), r1, r2, (char *) r3, r4, (char *) r5, (char *) r6);
		
	else if (r7 == GEN_STRING)	/* the label is a character string */
	  (void) fprintf(stdout, "\t%-8.8s\t%s\tr%d\t%s\t=>\tr%d\t[%s]\t[%s]",
		MNEM(op), (char *) r1, r2, (char *) r3, r4, (char *) r5, (char *) r6);
		
	else	/* invalid flag detected */
	  {
	  	(void) fprintf (stdout, "\tMISSING CASE\t");
	  	(void) sprintf (error_buffer, "Missing case - op %s(%d), val %d",
			MNEM(op), op, r7);
		ERROR("GenerateLong", error_buffer, WARNING); 
	  }
	break;
	
      case bLDor:
      case iLDor:
      case fLDor:
      case dLDor:
      case cLDor:
      case qLDor:
      case bSLDor:
      case iSLDor:
      case fSLDor:
      case dSLDor:
      case cSLDor:
      case qSLDor:
      case bCONor:
      case iCONor:
      case fCONor:
      case dCONor:
      case cCONor:
      case qCONor:
      case dPFLDI:
      case iPFLDI:
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	r5 = fst_my_GetFieldByIndex(ft_SymTable, r5, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t%d\tr%d\t=>\tr%d",
		MNEM(op), (char *) r1, r2, r3, r4, r5);
	break;
	
    // Added for dPFLD  MJB
        case dPFLD:
        case iPFLD:

	r3 = fst_my_GetFieldByIndex(ft_SymTable, r3, SYMTAB_REG);
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	r5 = fst_my_GetFieldByIndex(ft_SymTable, r5, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\tr%d\tr%d\t=>\tr%d",
		MNEM(op), (char *) r1, r2, r3, r4, r5);
    break;

      case bSTor:
      case iSTor:
      case fSTor:
      case dSTor:
      case cSTor:
      case qSTor:
      case bSSTor:
      case iSSTor:
      case fSSTor:
      case dSSTor:
      case cSSTor:
      case qSSTor:
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	r5 = fst_my_GetFieldByIndex(ft_SymTable, r5, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t%d\tr%d\tr%d",
		MNEM(op), (char *) r1, r2, r3, r4, r5);
	break;

      case FETCHor:
      case FLUSHor:
	r4 = fst_my_GetFieldByIndex(ft_SymTable, r4, SYMTAB_REG);
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t%d\tr%d\t",
		MNEM(op), (char *) r1, r2, r3, r4);
	break;

      default:
	(void) sprintf(error_buffer, "Opcode %s(%d) is not handled by generate_long.",
	  MNEM(op), op);
	ERROR("GenerateLong", error_buffer, FATAL);
	break;
  }

  /* and handle the optional comment */
  if (aiAnnotate > 0)
  {
    /* print comments on several lines if necessary */
    //int i = 60;
    if (*comment == '\0')
	(void) fprintf(stdout, "\n");
    else 
	(void) fprintf(stdout, "\t# %s\n", comment );
   // while (i<strlen(comment)) {
	//(void) fprintf(stdout,"\t%d\t%-8.8s\t\t\t\t\t# %.60s\n",
	//	       aiStmtCount,MNEM(NOP),&comment[i]);
	//i += 60;
    //}
  }
  else (void) fprintf(stdout, "\n");
  
  aiNumInstructions++;
} /* generate_long */




/* determines the appropriate opcode to use for conversion */
int convert_opcode( int type1, int type2 )
//   int type1, type2;
{
  register int result;

  switch(type1)
  {
    case TYPE_INTEGER:
	result = i2i;
	break;
    case TYPE_REAL:
	result = f2i;
	break;
    case TYPE_DOUBLE_PRECISION:
	result = d2i;
	break;
    case TYPE_COMPLEX:
	result = c2i;
	break;
    case TYPE_DOUBLE_COMPLEX:
	result = q2i;
	break;
    default:
	result = 0;
	break;
  }
  switch(type2)
  {
    case TYPE_INTEGER:
	break;
    case TYPE_REAL:
	result += 1;
	break;
    case TYPE_DOUBLE_PRECISION:
	result += 2;
	break;
    case TYPE_COMPLEX:
	result += 3;
	break;
    case TYPE_DOUBLE_COMPLEX:
	result += 4;
	break;
    default:
	result = 0;
	break;
  }
  if (aiDebug > 0)
     (void) fprintf(stdout, "Convert_flag( %d <%s>, %d <%s> ) => %s (%d).\n",
	     type1, TypeName(type1), 
	     type2, TypeName(type2),
	     iloc_mnemonic(result), result); 

  return result;
} /* convert_opcode */




/* generate_string() is called in situations where the label on */
/* the instruction is a character string and, as a result,      */
/* cannot easily be produced by generate().  This function is   */
/* rarely required.                                             */
void generate_string(char *label, int op, Generic r1, Generic r2, Generic r3, char *comment)
//   char *comment;
//   char *label;
//   int op, r1, r2, r3;
{
  if (aiDebug >1)
     (void) fprintf(stdout, "GenerateString(%s, %d, %d, %d, %d, %s).\n",
	     label, op, r1, r2, r3, comment);

  if (aiGenerate == 0)
  {
    (void) fprintf(stdout, "%s:", label);
    generate(0, op, r1, r2, r3, comment);
  }
} /* generate_string */



/*ARGSUSED*/
/* determine the alignment for a variable */
int getAlignment(int Index)
  // int	Index;
{
  /* this routine needs to be written by KDC */
  return 4;
  
} /* getAlignment */


static char tag_space[256];
/*ARGSUSED*/
/* get the tag associated with a variable */
char *getTag(int Index)
  // int Index;
{
  char *ptr, *tag, name[32];
  int  oddName, storeClass, offset, commonIndex, size;

  if (Index == -1)
     return "@-9090";

  oddName = 0;

  tag = tag_space;   /* copy name field into the tag */
  *tag++ = '@';
  if (fst_GetFieldByIndex(ft_SymTable,Index,SYMTAB_NUM_DIMS) != 0)
     *tag++ = '*';

  ptr = (char *) fst_GetFieldByIndex(ft_SymTable,Index,SYMTAB_NAME);
  while (*ptr != '\0')
  {
    if (isalnum(*ptr))  
       *tag++ = *ptr++;
    else    /* non-parsing char turns into a dot */
    {
       *tag++ = '.';
       ptr++;
       oddName = 1;
    }
  }
  *tag = '\0';  

  if (oddName == 1)
  {
    storeClass  = fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS);
    offset      = fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_offset);

    if (storeClass & SC_STACK)
       sprintf(tag_space, "@STACK(%d)", offset);
 
    else if (storeClass & (SC_STATIC | SC_CONSTANT))
       sprintf(tag_space, "@STATIC(%d)", offset);

    else if (storeClass & SC_GLOBAL)
    {
      fst_Symbol_To_EquivLeader_Offset_Size(ft_SymTable, Index, 
					    &commonIndex, &offset,(unsigned int *)&size);
      sprintf(tag_space, "@%s(%d)", aiGetCommonBlockName(commonIndex, name),offset);
    }
  }

  if (aiDebug > 1)
     fprintf(stdout, "getTag(%d) => '%s'\n", Index, tag_space);

  return tag_space;
  
} /* getTag */




/* generate a load instruction */
void generate_load(int sink, int addr, int type, int Index, char *Locality)
//   int sink, addr, type, Index;
//   char *Locality;
{
  int	alignment, storeClass;
  char	*tag;
    
  /* determine the alignment */
  alignment = getAlignment(Index);
	
  /* get the name of the variable */
  tag = getTag(Index);

  /* check for constant load form */
  storeClass  = fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS);

  if (storeClass & SC_CONSTANT)	/* constant? */
    switch(type)
    {
      case TYPE_CHARACTER:
	generate_long(0, bCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate_long(0, iCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_REAL:
	generate_long(0, fCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_PRECISION:
	generate_long(0, dCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_COMPLEX:
	generate_long(0, cCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_COMPLEX:
	generate_long(0, qCONor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      default:
	(void) fprintf(stdout, "\tMISSING CASE\n");
	(void) sprintf(error_buffer, "Missing case on type %d", type);
	ERROR("GenerateLoad", error_buffer, WARNING);
	break;
    }
  else if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NUM_DIMS) == 0) /* scalar? */
    switch(type)
    {
      case TYPE_CHARACTER:
	generate_long(0, bSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate_long(0, iSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_REAL:
	generate_long(0, fSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_PRECISION:
	generate_long(0, dSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_COMPLEX:
	generate_long(0, cSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_COMPLEX:
	generate_long(0, qSLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      default:
	(void) fprintf(stdout, "\tMISSING CASE\n");
	(void) sprintf(error_buffer, "Missing case on type %d", type);
	ERROR("GenerateLoad", error_buffer, WARNING);
	break;
    }
  else
    switch(type)
    {
      case TYPE_CHARACTER:
	generate_long(0, bLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate_long(0, iLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_REAL:
	generate_long(0, fLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_PRECISION:
	generate_long(0, dLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_COMPLEX:
	generate_long(0, cLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      case TYPE_DOUBLE_COMPLEX:
	generate_long(0, qLDor, (Generic) tag, alignment, 0, addr, sink, 0, 0, Locality);
	break;
      default:
	(void) fprintf(stdout, "\tMISSING CASE\n");
	(void) sprintf(error_buffer, "Missing case on type %d", type);
	ERROR("GenerateLoad", error_buffer, WARNING);
	break;
    }

} /* generate_load */




/* generate a store instruction */
void generate_store( int addr, int source, int type, int Index, char *Locality)
		     
//   int addr, source, type, Index;
//   char *Locality;
{
   int	alignment;
   char	*tag;
   
   /* determine the alignment */
   alignment = getAlignment(Index);
   
   /* get the name of the variable */
   tag = getTag(Index);

   if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NUM_DIMS) == 0) /* scalar? */
     switch(type)
      {
        case TYPE_CHARACTER:
	  generate_long(0, bSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_LOGICAL:
        case TYPE_INTEGER:
	  generate_long(0, iSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_REAL:
	  generate_long(0, fSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_DOUBLE_PRECISION:
	  generate_long(0, dSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_COMPLEX:
	  generate_long(0, cSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_DOUBLE_COMPLEX:
	  generate_long(0, qSSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        default:
	  (void) fprintf(stdout, "\tMISSING CASE\n");
	  (void) sprintf(error_buffer, "Missing case on type %d", type);
	  ERROR("GenerateStore", error_buffer, WARNING);
	  break;
      }
   else
     switch(type)
      {
        case TYPE_CHARACTER:
	  generate_long(0, bSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_LOGICAL:
        case TYPE_INTEGER:
	  generate_long(0, iSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_REAL:
	  generate_long(0, fSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_DOUBLE_PRECISION:
	  generate_long(0, dSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_COMPLEX:
	  generate_long(0, cSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        case TYPE_DOUBLE_COMPLEX:
	  generate_long(0, qSTor, (Generic) tag, alignment, 0, addr, source, 0, 0, Locality);
	  break;
        default:
	  (void) fprintf(stdout, "\tMISSING CASE\n");
	  (void) sprintf(error_buffer, "Missing case on type %d", type);
	  ERROR("GenerateStore", error_buffer, WARNING);
	  break;
      }
} /* generate_store */




/* generate a move of the appropriate type */
void generate_move( int target, int source, int type)
//   int target, source, type;
{
    switch(type)
    {
      case TYPE_CHARACTER:
      case TYPE_LOGICAL:
      case TYPE_INTEGER:
	generate(0, i2i, source, target, 0, NOCOMMENT);
	break;
      case TYPE_REAL:
	generate(0, f2f, source, target, 0, NOCOMMENT);
	break;
      case TYPE_DOUBLE_PRECISION:
	generate(0, d2d, source, target, 0, NOCOMMENT);
	break;
      case TYPE_COMPLEX:
	generate(0, c2c, source, target, 0, NOCOMMENT);
	break;
      case TYPE_DOUBLE_COMPLEX:
	generate(0, q2q, source, target, 0, NOCOMMENT);
	break;
      default:
	(void) fprintf(stdout, "\tMISSING CASE\n");
	(void) sprintf(error_buffer, "Missing case on type %d", type);
	ERROR("GenerateMove", error_buffer, WARNING);
	break;
    }
} /* generate_move */




/* generate a conditional branch */
void generate_branch(int stmt_label, int cmp_op, int arg1, int arg2, int type, 
		     int true_label, int false_label, char *comment)
//   int	stmt_label;	/* label on the first statement of the branch */
//   int	cmp_op;		/* equality or inequality opcode */
//   int	arg1;		/* register containing first argument */
//   int	arg2;		/* register containing second argument */
//   int	type;		/* type of arguments */
//   int	true_label;	/* label for the .true. branch */
//   int	false_label;	/* label for the .false. branch */
//   char	*comment;	/* comment on first statement of the branch */
{
  int	cc_reg;		/* register containing cc variable */
  int	cc_log;		/* register containing logical version of cc variable */
  int	gen_nop = 0;	/* generate a target for the false branch */

  /* generate a target for the false branch, if necessary */
  if (false_label == NO_TARGET)
  {
    false_label = LABEL_NEW;
    gen_nop = TRUE;
  }
  
  /* create temporary registers */
  cc_reg = TempReg(arg1, arg2, iCMP, type);
  cc_log = TempReg(cc_reg, 0, cmp_op, TYPE_LOGICAL);

  /* generate compare instructions */
  generate(stmt_label, compares_by_type[index_by_type(type)], 
		arg1, arg2, cc_reg,comment);
  generate(0, cmp_op, cc_reg, cc_log, 0, NOCOMMENT);

  /* generate branch */
  generate(0, BR, true_label, false_label, cc_log, NOCOMMENT);

  /* generate the target of the false branch, if necessary */
  if (gen_nop)
    generate(false_label, NOP, 0, 0, 0, "target of false branch");
    
} /* generate branch */


void generate_pfload(int sink, int addr, int PrefetchDistance,
		     AST_INDEX OffsetAST,int type,int Index, char *Locality)
{
  int	alignment, reg;
  char	*tag;
    
  /* get the name of the variable */
  tag = getTag(Index);

  if (OffsetAST == AST_NIL) // Offset is a compile-time constant
    if (type == TYPE_DOUBLE_PRECISION)
      generate_long(0, dPFLDI, (Generic) tag, PrefetchDistance, 0, addr, sink, 0,
		    0, Locality);
    else
      generate_long(0, iPFLDI, (Generic) tag, PrefetchDistance, 0, addr, sink, 0,
		    0, Locality);
  else
    {
        // Change to generate the offset iloc. MJB
        aiPrePass(OffsetAST);
        reg = getExprInReg(OffsetAST);
	if (type == TYPE_DOUBLE_PRECISION)
	  generate_long(0, dPFLD, (Generic) tag, 8, reg, addr, sink, 0,
			0, Locality);        
	else
	  generate_long(0, iPFLD, (Generic) tag, 4, reg, addr, sink, 0,
			0, Locality);        

    }
}


/* generates sequence of DATA statements for a character string */
void PrintString( char *str, int repeat )
//   char *str;
//   int	repeat;
{
  char *p;

  p = str;
  if (*p == '\'')
     p++;                                 /* skip any leading quote       */
     
  while (repeat--)			  /* handle multiple copies 	  */
  {
    while (*p != '\'' && *p != '\0')      /* peel off a character at a time */
    {
     (void) fprintf(stdout, "\t%-8.8s\t0x%hx\t1\t\t# '%c'",
                     MNEM(bDATA), (short)*p, *p);
     p++;
     if (*p != '\'' && *p != '\0')        /* no newline on last one so that   */
        (void) fputc('\n', stdout);              /* generate can put out the comment */
    }
  }
} /* PrintString */




/* generates an fDATA statement for a floating point number   */
/* and carries out machine specific conversions, as necessary */
void PrintFloat ( char *str, int repeat)
//   char *str;
//   int	repeat;
{
  int tmp;
  
  if (aiRt <= 0)
    {
      (void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(fDATA), str, repeat);
    }
  else  /* RT/PC requires hex format */
    {
      (void) sscanf(str, "%f", &tmp);
      (void) fprintf(stdout, "\t%-8.8s\t0x%x\t%d\t\t", MNEM(iDATA), tmp, repeat);
    }
} /* PrintFloat */




static char num_buffer2[128];

/* convert a double-precision floating-point number into a standard format */
static char *InsertPeriod( char *numstr )
  // char *numstr;
{
  char *p, *q;
  int  PeriodFound;

  PeriodFound = FALSE;
  p = numstr;
  q = num_buffer2;

  while (*p != '\0' && *p != 'd' && *p != 'D')
  {
    if (*p == '.')
       PeriodFound = TRUE;

    *q++ = *p++;
  }

  if (PeriodFound == FALSE)
  {
    *q++ = '.';
  }

  while (*p != '\0')
    *q++ = *p++;

  *q = '\0';

  return num_buffer2;
} /* InsertPeriod */




/* generates an dDATA statement for a double-precision floating-point */
/* number and carries out machine specific conversions, as necessary  */
void PrintDouble(char *str, int repeat)
//   char *str;
//   int	repeat;
{
  char *converted, *p;
  
  converted = InsertPeriod(str);

  if (aiRt <= 0)
    {
      if (aiSparc == 0)		/* C has no 'D' representation */
	{
	  p = converted;
          while (*p)
	    {
	      if (*p == 'd' || *p == 'D')
          	  *p = 'e';
	      p++;
	    }
	}
       (void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(dDATA), 
		     converted, repeat);
    }
  else   /* RT/PC requires a special format */
    {
      (void) fprintf(stdout, "\t%-8.8s\t0D%s\t%d\t\t", MNEM(dDATA),
		     converted, repeat);
    }
} /* PrintDouble */



	  
/* generates DATA statements by type */
void PrintData(int opcode, int data, int repeat, int type)
  // int opcode, data, repeat, type;
{
  char *kludge_p1;
  char *kludge_p2;
  char *p;
  char *imag_part;

  p = (char *) data;

  switch(type)
  {
    case DATA_LABEL:
	(void) fprintf(stdout, "\t%-8.8s\tLL%-4.4d\t%d\t\t", MNEM(iDATA), data, repeat);
	break;

    case DATA_STRING_LABEL:
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(iDATA), p, repeat);
	break;

    case DATA_CHARACTER:
	PrintString(p, repeat);
	break;

    case DATA_BYTE:
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(bDATA), p, repeat);
	break;

    case DATA_LOGICAL:
	if (strcmp(p, ".true.") == 0)
	   (void) fprintf(stdout, "\t%-8.8s\t1\t%d\t\t", MNEM(iDATA), repeat);
	else
	   (void) fprintf(stdout, "\t%-8.8s\t0\t%d\t\t", MNEM(iDATA), repeat);
	break;

    case DATA_INTEGER:
	(void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(iDATA), p, repeat);
	break;

    case DATA_FLOAT:
	PrintFloat(p, repeat);
	break;

    case DATA_DOUBLE:
	PrintDouble(p, repeat);
	break;

    case DATA_COMPLEX:
	/* some gross, disgusting pointer chasing to*/
	/* pick apart the complex constant string   */
	kludge_p1 = p;
	kludge_p2 = complex_buffer; /* get the real part */
	kludge_p1 ++; /* skip the '(' */
	while (*kludge_p1 != ',')
	   *kludge_p2++ = *kludge_p1++;
	*kludge_p2 = '\0';

	/* and the imag part */
	imag_part = ++kludge_p2;
	kludge_p1 ++; /* skip the ',' */
	while (*kludge_p1 != ')')
	   *kludge_p2++ = *kludge_p1++;
	*kludge_p2 = '\0';

	while (repeat--)
	  /* note that both components of the complex constant have been */
	  /* converted to single precision floating point numbers        */
	  (void) fprintf(stdout, "\t%-8.8s\t%s\t1\n\t\t%-8.8s\t%s\t1\t\t",
			MNEM(fDATA), complex_buffer, MNEM(fDATA), imag_part);
	break;

	case DATA_DOUBLE_COMPLEX:
	  (void) fprintf(stderr,
		"Double precision complex DATA is not yet supported in a2i\n");
	break;

	case DATA_UNTYPED:
	switch(opcode)
	{
	  case bDATA:
	    (void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(bDATA), p, repeat);
	  break;
	  
	  case iDATA:
	    (void) fprintf(stdout, "\t%-8.8s\t%s\t%d\t\t", MNEM(iDATA), p, repeat);
	  break;
	  
	  case fDATA:
	    PrintFloat(p, repeat);
	  break;

	  case dDATA:
	    PrintDouble(p, repeat);
	  break;
	  
	  default:
            (void) fprintf(stdout, "\tMISSING CASE\t");
            (void) sprintf(error_buffer,
	      "Missing case - format %s, op %s(%d), val %d, repeat %d",
              format_name(iloc_format(opcode)), MNEM(opcode), opcode, data, repeat);
            ERROR("PrintData", error_buffer, WARNING);
	  break;
	}
	break;
	
    default:
	(void) fprintf(stdout, "\tMISSING CASE\t");
  	(void) sprintf(error_buffer,
		"Missing case - format %s, op %s(%d), val %d, repeat %d",
		format_name(iloc_format(opcode)), MNEM(opcode), opcode, data, repeat);
	ERROR("PrintData", error_buffer, WARNING);
	break;
  }

} /* PrintData */




static char num_buffer[128];

/* return the prefix of a string up to the square brackets */
char *Prefix( char *str )
  // char *str;
{
  char *p, *q;

  p = num_buffer;
  q = str;

  while (*q != '[' && *q != '\0')
    *p++ = *q++;

  *p = '\0';

  return num_buffer;
} /* Prefix */




void generate_cache_op(int addr, int Index, Directive *Dir)
//   int addr, Index;
//   Directive *Dir;
{
  int	alignment;
  char	*tag;
    
  /* determine the alignment */
  alignment = getAlignment(Index);
	
  /* get the name of the variable */
  tag = getTag(Index);

  switch(Dir->Instr) {
    case PrefetchInstruction:
         generate_long(0, FETCHor, (Generic) tag, alignment, 0, addr, 0, 0, 0, 
		       GenDepCommentForStmt(Dir));
	 break;
    case FlushInstruction:
	 generate_long(0, FLUSHor, (Generic) tag, alignment, 0, addr, 0, 0, 0,
		       GenDepCommentForStmt(Dir));
	 break;
	}

} /* generate_prefetch */


char *GenDepComment(AST_INDEX node)

//   AST_INDEX node;

  {
   char *comment;
   char number[80],name[80];
   UtilNode *LNode;
   DepStruct *Dep;
   
     comment = (char *)malloc(1024*sizeof(char));
     if (aiCache)
	sprintf(comment,"%s &",getLocality(node));
     else
	sprintf(comment,"&unknown &");
     ut_GetSubscriptText(node,name);
     (void)strcat(comment,name);
     if (DepInfoPtr(node) == NULL)
       {
	(void)strcat(comment," -1");
	return(comment);
       }
     else
       {
	sprintf(number," ref %d dep",DepInfoPtr(node)->ReferenceNumber);
	(void)strcat(comment,number);
	for (LNode = UTIL_HEAD(DepInfoPtr(node)->DependenceList);
	     LNode != NULLNODE;
	     LNode = UTIL_NEXT(LNode))
	  {
	   assert(strlen(comment) < 1000);
	   Dep = (DepStruct *)UTIL_NODE_ATOM(LNode);
	   sprintf(number," %d%c%d",Dep->ReferenceNumber,Dep->DType,Dep->Distance);
	   (void)strcat(comment,number);
	  }
	util_free_nodes(DepInfoPtr(node)->DependenceList);
	util_list_free(DepInfoPtr(node)->DependenceList);
	return(comment);
       }
  }

char *GenDepCommentForStmt(Directive *Dir)

  //  Directive *Dir; 

  {
   char *comment;
   char number[80],name[80];
   UtilNode *LNode;
   DepStruct *Dep;
   
     comment = (char *)malloc(1024*sizeof(char));
     sprintf(comment,"&directive &");
     ut_GetSubscriptText(Dir->Subscript,name);
     (void)strcat(comment,name);
     sprintf(number," ref %d",Dir->DirectiveNumber);
     (void)strcat(comment,number);
     if (Dir->DependenceList != NULLLIST)
       {
	(void)strcat(comment," dep ");
	for (LNode = UTIL_HEAD(Dir->DependenceList);
	     LNode != NULLNODE;
	     LNode = UTIL_NEXT(LNode))
	  {
	   assert(strlen(comment) < 1000);
	   Dep = (DepStruct *)UTIL_NODE_ATOM(LNode);
	   sprintf(number," %d%c%d",Dep->ReferenceNumber,Dep->DType,Dep->Distance);
	   (void)strcat(comment,number);
	  }
	util_free_nodes(Dir->DependenceList);
	util_list_free(Dir->DependenceList);
	return(comment);
       }
  }
