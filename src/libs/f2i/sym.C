/* $Id: sym.C,v 1.2 1999/03/31 21:56:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/support/strings/rn_string.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/char.h>

struct common_table_elt *common_table;
int CommonNextSlot;
int CommonNumberOfSlots; 
struct leader_table_elt *leader_table;
int    leader_table_free_list;
int SymHaveSeenASave;
int                      SymDebug;
SymDescriptor ft_SymTable;
char                     *Sym2ndReg;
int                      SymHaveSeenAnEquivalence;




/* forward declarations */
static char *StorageClassName(int, int);

static char *ObjectClassNames[] = {	"unk", "exe", "dat", "com", "spe",
					"unk*", "exe*", "dat*", "com*","spe*",
					"ill" };

static char *StorageClassNames[] = {	"fun", "sub", "prg", "ext", "int",
					"blk", "ent", "sfn", "slb", 
					"con", "stk", "glo", "sav", "reg", 
					"ill" };

static char *TypeNames[] = { 	"unk", "char", "logi", "int", "lbl",
				"real","cplx","doub", "dcpx" };
static char char_name[32];

static char nameBuffer[32];




/* return the name of the object class given the index */
char *decodeObjectClass(int c)
  // register int c;
{
  register int i;

  c = fst_GetFieldByIndex(ft_SymTable, c, SYMTAB_OBJECT_CLASS);
  switch(c)
  {
    case OC_UNDEFINED:				i=0;	break;
    case OC_IS_EXECUTABLE:			i=1;	break;
    case OC_IS_DATA:				i=2;	break;
    case OC_IS_COMMON_NAME:			i=3;	break;
    case OC_IS_SPECIAL:				i=4;	break;
    case OC_IS_FORMAL_PAR | OC_UNDEFINED:	i=5;	break;
    case OC_IS_FORMAL_PAR | OC_IS_EXECUTABLE:	i=6;	break;
    case OC_IS_FORMAL_PAR | OC_IS_DATA:		i=7;	break;
    case OC_IS_FORMAL_PAR | OC_IS_COMMON_NAME:	i=8;	break;
    case OC_IS_FORMAL_PAR | OC_IS_SPECIAL:	i=9;	break;
    default:					i=10;	break;
  }
  return ObjectClassNames[i];
} /* decodeObjectClass */




/* return the name of the storage class, given the index */
char *decodeStorageClass(int sc)
//   register int sc;
{
  register int i, oc;
  register char *p;

  oc = fst_my_GetFieldByIndex(ft_SymTable, sc, SYMTAB_OBJECT_CLASS);
  sc = fst_my_GetFieldByIndex(ft_SymTable, sc, SYMTAB_STORAGE_CLASS);

  if (oc & OC_IS_EXECUTABLE)
  {
    switch(sc)
    {
        case SC_FUNCTION:                       i=0;    break;
        case SC_SUBROUTINE:                     i=1;    break;
        case SC_PROGRAM:                        i=2;    break;
        case SC_EXTERNAL:                       i=3;    break;
        case SC_INTRINSIC:                      i=4;    break;
        case SC_BLOCK_DATA:                     i=5;    break;
        case SC_ENTRY:                  	i=6;    break;
        case SC_STMT_FUNC:                      i=7;    break;
        case SC_STMT_LABEL:                     i=8;    break;
        default:                                i=14;   break;
    }
    p = StorageClassNames[i];
  }
  else if (oc & OC_IS_DATA)
  {
    p = nameBuffer;     /* use the buffer       */
    *p = '\0';          /* initialize it        */

    i = 14;

    if (sc & SC_CONSTANT)               i = 9;  /* can't use a case statement,  */
    else if (sc & SC_STACK)             i = 10; /* because it's a multi-valued  */
    else if (sc & SC_GLOBAL)            i = 11; /* field ... so we need to be   */
    else if (sc & SC_STATIC)            i = 12; /* ever so slightly clever ...  */
    else if (sc & SC_NO_MEMORY) i = 13;

    (void) strcat(p, StorageClassNames[i]);

    if (sc & SC_NO_MEMORY && sc != SC_NO_MEMORY)
       (void) strcat(p, "*");

  }
  else
     p = StorageClassNames[12];

  return p;
} /* decodeStorageClass */




/* return the name of the type given the index */
char *decodeType(int t)
//   register int t;
{
  register int i;

  i = fst_my_GetFieldByIndex(ft_SymTable, t, SYMTAB_TYPE);

  switch(i)
  {
    case TYPE_CHARACTER:        i=1;    break;
    case TYPE_LOGICAL:          i=2;    break;
    case TYPE_INTEGER:          i=3;    break;
    case TYPE_LABEL:            i=4;    break;
    case TYPE_REAL:             i=5;    break;
    case TYPE_COMPLEX:          i=6;    break;
    case TYPE_DOUBLE_PRECISION: i=7;    break;
    case TYPE_DOUBLE_COMPLEX:   i=8;    break;
    default:                    i=0;    break;
  }
  if (i!=1)
     return TypeNames[i];
  else
  {
    if (fst_my_GetFieldByIndex(ft_SymTable, t, SYMTAB_CHAR_LENGTH) == STAR_LEN)
      (void) sprintf(char_name,"ch*(*)");
    else
      (void) sprintf(char_name, "ch*%d",
                     fst_my_GetFieldByIndex(ft_SymTable, t, SYMTAB_CHAR_LENGTH));
    return char_name;
  }
} /* decodeType */




/* return the name of the storage class     */
/* given the object class and storage class */
static char *StorageClassName(int sc, int oc)
//   register int sc, oc;
{
  register int i;


  if (oc & OC_IS_EXECUTABLE)
     switch(sc)
     {
        case SC_FUNCTION:
        case (SC_FUNCTION | SC_EXTERNAL):       i=0;    break;
        case SC_SUBROUTINE:
        case (SC_SUBROUTINE | SC_EXTERNAL):     i=1;    break;
        case SC_PROGRAM:                        i=2;    break;
        case SC_EXTERNAL:                       i=3;    break;
        case SC_INTRINSIC:
        case (SC_INTRINSIC | SC_FUNCTION):      i=4;    break;
        case SC_BLOCK_DATA:
        case (SC_BLOCK_DATA | SC_EXTERNAL):     i=5;    break;
        case SC_ENTRY:
        case (SC_ENTRY | SC_EXTERNAL):  	i=6;    break;
        case SC_STMT_FUNC:                      i=7;    break;
        case SC_STMT_LABEL:                     i=8;    break;
        default:                                i=14;   break;
     }

  else if (oc & OC_IS_DATA)
     switch(sc)
     {
        case SC_CONSTANT:                       i=9;    break;
        case SC_STACK:                  	i=10;   break;
        case SC_GLOBAL:                 	i=11;   break;
        case SC_STATIC:                 	i=12;   break;
        case SC_NO_MEMORY:                      i=13;   break;
        default:                                i=14;   break;
     }
  else
     i = 13;

  return StorageClassNames[i];
} /* StorageClassName */




/* if the requested change in storage class  */
/* is valid, make change in the symbol table */
void SymPromoteStorageClass( int Index, int Class )
//   int Index, Class;
{
  register int oc, sc, nsc;

  sc  = fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS);

  if (sc & Class)
     return;

  oc  = fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS);
  nsc = DONT_KNOW;

  if (oc & OC_IS_EXECUTABLE)
  {
     if (sc & SC_NO_MEMORY)
     {  /* initialization case, any EXEC goes   */
        if (Class == SC_FUNCTION || Class == SC_SUBROUTINE ||
            Class == SC_PROGRAM  || Class == SC_EXTERNAL   ||
            Class == SC_INTRINSIC|| Class == SC_BLOCK_DATA ||
            Class == SC_ENTRY    || Class == SC_STMT_LABEL ||
            Class == SC_STMT_FUNC)
        nsc = Class;
     }
     else if (sc & SC_EXTERNAL)
     {  /* allow other executables              */
        if (Class == SC_FUNCTION || Class == SC_SUBROUTINE ||
            Class == SC_ENTRY    || Class == SC_BLOCK_DATA)
           nsc = Class | sc;
     }
     else if (sc & SC_INTRINSIC)
     {
        if (Class == SC_FUNCTION)
           nsc = Class | sc;
     }
     else if (sc & SC_FUNCTION)
     {
        if (Class == SC_INTRINSIC || Class == SC_EXTERNAL)
           nsc = Class | sc;
     }
     else if (sc & (SC_SUBROUTINE | SC_ENTRY | SC_BLOCK_DATA))
     {
        if (Class == SC_EXTERNAL)
           nsc = Class | sc;
     }
     else if (sc & (SC_PROGRAM | SC_STMT_LABEL | SC_STMT_FUNC ))
     {
        if (Class & sc) /* new Class is a subset of old Class */
           nsc = Class;
     }
  }
  else if (oc & OC_IS_DATA)
  {
     if (sc & DONT_KNOW || sc & SC_NO_MEMORY)
     {
        if ((fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS) &
             (OC_IS_DATA | OC_IS_FORMAL_PAR) ) &&
            (Class == SC_EXTERNAL) )
        { /* Object is function valued parameter, but its Class
           * was set to data when the parameter list was walked.
           * We need to set the record straight -
           *    it's storage Class becomes EXTERNAL
           *    it's object Class becomes EXECUTABLE
           */
          fst_PutFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS,
                              OC_IS_EXECUTABLE | OC_IS_FORMAL_PAR);
        }
        nsc = Class;
     }
     else if (sc & SC_STACK)
     {
        if (Class == SC_GLOBAL || Class == SC_STATIC)
           nsc = Class;
     }
     else if (sc & SC_STATIC)
     {
        if (Class == SC_GLOBAL)
           nsc = Class;
     }
  }
  else if (oc == OC_UNDEFINED)
  {
     if (sc == SC_NO_MEMORY) /* initial condition */
        switch(Class)
        {
                case SC_FUNCTION:
                case SC_SUBROUTINE:
                case SC_PROGRAM:
                case SC_EXTERNAL:
                case SC_INTRINSIC:
                case SC_BLOCK_DATA:
                case SC_ENTRY:
                case SC_STMT_FUNC:
                case SC_STMT_LABEL:
                        nsc = Class;
                        fst_PutFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS,
                                            OC_IS_EXECUTABLE);
                        break;
                case SC_CONSTANT:
                case SC_STACK:
                case SC_GLOBAL:
                case SC_STATIC:
                case SC_NO_MEMORY:
                        nsc = Class;
                        fst_PutFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS,
                                            OC_IS_DATA);
                        break;
                default:
                        break;
        }
  }
  else if (oc & OC_IS_SPECIAL)
  {
    if (Class == SC_STMT_LABEL)
        nsc = Class;
  }

  if (nsc != DONT_KNOW)
    fst_PutFieldByIndex(ft_SymTable, Index, SYMTAB_OBJECT_CLASS, nsc);
  else
  {
    (void) sprintf(error_buffer,
	"Attempt to promote name '%s' from '%s'(%d) to '%s'(%d)",
        (char *) fst_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME),
	StorageClassName(sc,oc), sc, StorageClassName(Class,oc),Class);
    ERROR("SymPromoteStorageClass", error_buffer, WARNING);
  }
} /* SymPromoteStorageClass */




/* insert information into symbol table for existing symbol */
void SymInsertData(int index, int type, int obj_class, int dims, int Class, int alias)
//   int   index;
//   int   type;
//   int   obj_class;
//   int   dims;
//   int   Class;
//   int   alias;

{

  if (type != DONT_KNOW)
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_TYPE, type);
  else
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_TYPE, TYPE_UNKNOWN);

  if (obj_class != DONT_KNOW)
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS, obj_class | 
	   (OC_IS_FORMAL_PAR &
	    fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS)));
  else
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS, OC_UNDEFINED);

  if (dims != DONT_KNOW)
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS, dims);
  
  if (Class != DONT_KNOW)
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS, Class);
  else
    fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS, SC_NO_MEMORY);

  fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_alias, alias);

} /* SymInsertData */




/*ARGSUSED*/
/* initialize the symbol table fields given the index */
void InitFieldsInSymTab (SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{
int type;

/* change the TC's default to SC_NO_MEMORY */
if (fst_GetFieldByIndex (ft_SymTable, index, SYMTAB_STORAGE_CLASS) == SC_STACK)
  fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_STORAGE_CLASS, SC_NO_MEMORY);

fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_alias, NO_ALIAS);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_addressReg, NO_REGISTER);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_scratch, 0);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_offset, DK);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_high, 0);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_low, 0);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_used, 0);
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_fortran_parameter, 0);

/* this temp field is used in data.c - previously, the SYMTAB_PARENT	     */
/* was being used but I don't know if the NEW TC symbol table would approve! */
fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_temp, 0);

fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_REG, aiNextReg);

type = fst_GetFieldByIndex (ft_SymTable, index, SYMTAB_TYPE);
if (type == TYPE_DOUBLE_PRECISION)
    aiNextReg += 2;
else if (type == TYPE_DOUBLE_COMPLEX)
  aiNextReg += 4;
else
  aiNextReg += 1;
} /* InitFieldsInSymTab */




/* The data in the symbol data is not all usable.		  */
/* So, some of it will be wiped out and filled in later as we hit */
/* the symbols (from prepass) or as we go through map.c.	  */
   
/* For common block names, we need to do some extra stuff.	  */
/* As for the vars in the common block, map.c will take care	  */
/* of assigning the common block name, as well as making the	  */
/* storage class global, and assigning the appropriate offset.    */





/* initialize the symbol table fields, given the index, and     */
/* create the address register if the name of a common is found */
void InitSymTab (SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{

InitFieldsInSymTab(SymTab, index, dummy);

if (fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) == OC_IS_COMMON_NAME)
  {

    fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_addressReg, 
			 StrTempReg("Common",index,TYPE_INTEGER));
    fst_PutFieldByIndex (ft_SymTable, index, SYMTAB_SIZE, 0);  /* maybe */
  }
} /* InitSymTab */




/* insert symbol and appropriate data into table */
int SymInsertSymbol( char *name, int type, int obj_class, int dims, int Class,int alias )
//   char  *name;
//   int   type;
//   int   obj_class;
//   int   dims;
//   int   class;
//   int   alias;
{
int index;

if (fst_QueryIndex(ft_SymTable, name) == SYM_INVALID_INDEX)
  {
  index = fst_Index (ft_SymTable, name);
  fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS, OC_UNDEFINED);
  fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS, 0);
  fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_TYPE, type); /* InitFields needs this */
  InitFieldsInSymTab (ft_SymTable, index, 0);
  SymInsertData (index, type, obj_class, dims, Class, alias);
  if (type != TYPE_CHARACTER)
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_SIZE, SizeOfType(type));
  else
    fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_SIZE, StringLength(index));
  }
else
  {
    index = fst_QueryIndex(ft_SymTable, name);
    SymInsertData (index, type, obj_class, dims, Class, alias);
  }

return index;
} /* SymInsertSymbol */

static char s[64];




/* return symbol table index for the lower bound */
int getIndexForlb (ArrayBound *bounds, int dim, fst_index_t index)
// ArrayBound *bounds;
// int dim; 
// fst_index_t index;

{
  int   temp;
  int   result;

  if (bounds[dim].lb.type == constant) 
    {  /* then it must be an integer */
      return getIntConstantIndex (bounds[dim].lb.value.const_val);
    }
  else if (bounds[dim].lb.type == symbolic_expn_ast_index)
    { /* This assumes that the type checker has insured that it is an integer expression.*/
      /* Note that we turn off code generation so that getExprInReg produces no code.    */
      temp = aiGenerate;
      aiGenerate = 1;
      result = getExprInReg((AST_INDEX) bounds[dim].lb.value.ast);
      aiGenerate = temp;
      return result;
    }
  else if (bounds[dim].lb.type == star)
    {
      return fst_QueryIndex(ft_SymTable, "DIM(*)");
    }
  else
    {
      return SYM_INVALID_INDEX;
    }
} /* getIndexForlb*/




/* return symbol table index for the upper bound */
int getIndexForub (ArrayBound *bounds, int dim, fst_index_t index)
// ArrayBound *bounds;
// int dim; 
// fst_index_t index;

{
  int   temp;
  int   result;

  if (bounds[dim].ub.type == constant) 
    {  /* then it must be an integer */
      return getIntConstantIndex (bounds[dim].ub.value.const_val);
    }
  else if (bounds[dim].ub.type == symbolic_expn_ast_index)
    { /* This assumes that the type checker has insured that it is an integer expression. */
      /* Note that we turn off code generation so that getExprInReg produces no code.     */
      temp = aiGenerate;
      aiGenerate = 1;
      result = getExprInReg((AST_INDEX) bounds[dim].ub.value.ast);
      aiGenerate = temp;
      return result;
    }
  else if (bounds[dim].ub.type == star)
    {
      return fst_QueryIndex(ft_SymTable, "DIM(*)");
    }
  else
    {
      return SYM_INVALID_INDEX;
    }
} /* getIndexForub */




/*ARGSUSED*/
/* print out dimension information in the symbol table */
void Sym_Dim_Dump(SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;

{
  ArrayBound *bounds;
  int dim;

  if (fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS) > 0)
    {

      bounds = (ArrayBound *) fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_DIM_BOUNDS);
      (void) fprintf (stdout, "%4d\t%-7.7s %d: <%s:%s", index, 
		fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS),
		(char *) fst_GetFieldByIndex(ft_SymTable, getIndexForlb(bounds,0,index),
						 SYMTAB_NAME), 
		(char *) fst_GetFieldByIndex(ft_SymTable, getIndexForub(bounds,0,index),
						 SYMTAB_NAME));
      
      for (dim = 1; dim<fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS); dim++)
	{
	  
	  (void) fprintf (stdout, ", %s:%s",
			(char *) fst_GetFieldByIndex(ft_SymTable,
				getIndexForlb(bounds,dim,index), SYMTAB_NAME), 
			(char *) fst_GetFieldByIndex(ft_SymTable,
				getIndexForub(bounds,dim,index), SYMTAB_NAME));
	}
      (void) fprintf (stdout, ">\n");
    }
} /* Sym_Dim_Dump */




/*ARGSUSED*/
/* print out basic symbol table information given an index */
void Short_Dump(SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{
int SC;

  (void) fprintf(stdout,"%5d\t%.7s\t%s\t%s\t%s\t%d\t",
		 index, (char *) fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		 decodeStorageClass(index), decodeType(index), decodeObjectClass(index),
		 fst_GetFieldByIndex(ft_SymTable,index,SYMTAB_NUM_DIMS));
  if (fst_GetFieldByIndex(ft_SymTable,index,SYMTAB_offset) == DONT_KNOW)
    (void) fprintf(stdout, "*\t");
  else
    (void) fprintf(stdout, "%d\t",
		fst_GetFieldByIndex(ft_SymTable,index, SYMTAB_offset));
  if (fst_GetFieldByIndex(ft_SymTable,index, SYMTAB_OBJECT_CLASS) & OC_IS_DATA)
    {
      SC = fst_GetFieldByIndex(ft_SymTable,index, SYMTAB_STORAGE_CLASS);
      if (SC & SC_GLOBAL)
	(void) fprintf(stdout, "/%s", 
		       (char *) fst_GetFieldByIndex(ft_SymTable,
			fst_GetFieldByIndex(ft_SymTable,index, SYMTAB_PARENT),
			SYMTAB_NAME));
      else if (SC & SC_STACK)
	(void) fprintf(stdout, "STACK");
      else if (SC & SC_STATIC)
	(void) fprintf(stdout, "STATIC");
      else if (SC & SC_CONSTANT)
	(void) fprintf(stdout, "CONSTANT");
      
      if (SC & SC_NO_MEMORY)
	(void) fprintf(stdout,"(r%d)",index);
    }
  
  (void) fprintf(stdout, "\n");
} /* Short_Dump */




/*ARGSUSED*/
/* print out equivalence information */
void Sym_Dump_Equivalences(SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{
  (void) fprintf(stdout,"%.7s\t%s\t%s\t%s\t%d\t%d\t%d\n",
		 (char *) fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		 decodeStorageClass(index), decodeType(index), decodeObjectClass(index),
		 fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS), 
		 fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_EQ_OFFSET),
		 fst_GetFieldByIndex(ft_SymTable, index, SYMTAB_PARENT));
  
} /* Sym_Dump_Equivalences */




/* print out symbol table information for all elements in the symbol table */
void fst_Short_Dump()
{

(void) fprintf(stdout, "\nSymbol Table:\n");
	(void) fprintf(stdout, "\n\t\tStorage\t\tObject\t\t\n");
        (void) fprintf(stdout, "Index\tName\tClass\tType\tClass\tDim's\tOffset");
	(void) fprintf(stdout, "\tBase\n");
fst_ForAll (ft_SymTable, Short_Dump, 0);
  
/*  if any dimension'ed objects have been found, talk about it 	*/
(void) fprintf( stdout, "\nDimension Table:\n\n%s\t%s\t%s\n", 
	       "index", "name", "dim's");
fst_ForAll(ft_SymTable, Sym_Dim_Dump, 0);
(void) fprintf(stdout, "\n");

if (aiSymDump < 3) return;
     
/* print out equivalence related information	*/
 (void) fprintf(stdout, 
		"\nEquivalence information:\n\n%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
		"name", "storage", "type", "class", "dims", "offset", "parent");
fst_ForAll(ft_SymTable, Sym_Dump_Equivalences, 0);
} /* fst_Short_Dump */




/*ARGSUSED*/
/* zero the scratch field in the symbol table */
void SymZeroScratchField(SymDescriptor SymTab, fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{

fst_PutFieldByIndex(ft_SymTable, index, SYMTAB_scratch, 0);
} /* SymZeroScratchField */
