/* $Id: map.C,v 1.2 1998/04/29 13:00:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 * FIXES:
 *
 *	need to ensure that any variable of type TYPE_CHARACTER gets assigned
 *	a storage_class of SC_STACK or SC_STATIC depending on SAVE status.
 *	(This can arise in an undeclared variable, in conjunction with an
 *	IMPLICIT statement, so it takes some care.  Suggests a function that
 *	returns storage class values for undeclared variables.)
 *
 * Added code for double word alignment.
 *						kdc 5/24/90
 *
 */

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>

#include <libs/ipAnalysis/interface/IPQuery.h>

/* forward declarations */
static void AssignOffsetsForVarsInCommon(SymDescriptor,fst_index_t,Generic);
static void AssignLengthsToEachNonParamVar(SymDescriptor, fst_index_t, Generic);
static void AssignStorageClasses (SymDescriptor,fst_index_t,Generic);
static void AssignStaticSpace (SymDescriptor,fst_index_t,Generic);
static void AndEquivalencedNames(SymDescriptor,fst_index_t,Generic);

extern C_CallGraph   ProgramCallGraph;



/* MapStorage():
 *
 *	This routine takes the completed (nearly) Symbol Table
 *	and assigns a storage offset to each variable.  The 
 *	offset can be interpreted only in conjunction with the
 *	storage_class and common_name fields.
 *
 *	SC	 CN	offset
 *	global	 j	bytes from start of common_table[j]
 *	static	 - 	bytes from start of static pool
 *	stack	 -	bytes from base of procedure's stack frame
 *	constant -	bytes from start of constant pool
 *	no mem	 -	stack location if used as an actual
 *
 */

void MapStorage(AST_INDEX StmtList)
  //AST_INDEX StmtList;
{
  register int i;

  /* The Strategy:
   *
   *    (1) Run the PrePass
   *
   *	(2) Assign lengths to each non-parameter, non-character 
   *	     variable
   *
   *	(3) Walk the common table and put in the offsets for all
   *	    variables declared to be in common
   *
   *    (4) Process all the stacked-up equivalence statements
   *        (This is already done by the TypeChecker)
   *
   *	(5a) Make storage class decisions for actual parameters
   *		- this requires looking at ALIAS and LOCAL mod
   * 		  information
   *
   *    (5b) If we've seen a SAVE for all variables 
   *	    (SymHaveSeenASave == 1), then we mark the appropriate
   *	    things as Static.
   *
   *	(5c) Walk through the symbol table, 
   *
   *		(a) decide which variables need stack or static space
   *		(b) "enregister" variables
   *
   *	(6) set offsets for all equivalenced variables
   *
   *	(7) Run the PostMap pass
   *
   */

  /* Step 1 -- the PrePass() 		*/
  /* finds undeclared variables, constants	*/

  aiPrePass(StmtList);

  /* Step 2 - assign lengths to all solidly dimensioned objects 	*/
  /*	      except Character strings which already have a length      */
  fst_ForAll (ft_SymTable, AssignLengthsToEachNonParamVar, 0);

  if (aiDebug > 0)
     (void) fprintf(stdout, "MapStorage: processing the common table.\n");

  /* Step 3, the common table	*/

  fst_ForAll (ft_SymTable, AssignOffsetsForVarsInCommon, 0);

  /* Step 4 - process equivalences */
  /* already done */

  /* Steps 5a, 5b, and 5c -- all run together
   *
   * and now, we assign storage classes according to the following rules
   *
   *		  GLOBALS &		     Non-GLOBALS	
   *		 Parameters		   Stack	  Static
   *
   *		Alias	Not		Eqv	Not	Eqv	Not
   *		------------------------------------------------------
   * Scalar	GLOB	GLOB|R		STK	R	STC	STC|R
   *		
   *		(?R)			(?R)		(?R)
   *
   *		------------------------------------------------------
   * Non-	GLOB	GLOB		STK	STK	STC	STC
   * Scalar
   *
   *
   */

  if (aiDebug > 0)
     (void) fprintf(stdout, "\tand non-EQUIVALENCEd names.\n");

  i = aiFunctionValueIndex();
  if (i != -1)
     fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_NO_MEMORY);

  fst_ForAll (ft_SymTable, AssignStorageClasses, 0);

  fst_ForAll (ft_SymTable, AssignStaticSpace, 0);

  /* Step 6 - can in the offsets for EQUIVALENCEd names
   * 	      checking all the while for inconsistency 
   */
  if (aiDebug > 0)
     (void) fprintf(stdout, "\tand offsets for EQUIVALENCEd names.\n");

  fst_ForAll (ft_SymTable, AndEquivalencedNames, 0);
  
  /* 7 - run the PostPass */
  /* 	 check for modified loop index variables ... needs to run after	*/
  /*	 the mapping so that we can check the interprocedural info.	*/
  aiPostMap(StmtList);
} /* MapStorage */




/*ARGSUSED*/
/* assign lengths to each non-parameter variable */
static void AssignLengthsToEachNonParamVar(SymDescriptor SymTab, 
					   fst_index_t index, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t index;
// Generic dummy;
{

  if ((fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) & OC_IS_DATA) &&
      !(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) &
	OC_IS_FORMAL_PAR))
    {
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_SIZE,  VarSize(index));
    }
} /*AssignLengthsToEachNonParamVar */




/*ARGSUSED*/
/* assign offsets for variables in common */
static void AssignOffsetsForVarsInCommon(SymDescriptor SymTab,fst_index_t i, 
					 Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;
{
register int j, bytes;

  if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)
    {
      bytes = 0;
      j = fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_FIRST_NAME);
      while ( j != -1 )
	{
	  if (fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_offset) != DONT_KNOW) 
	    /* internal consistency check */
	    {
	      (void) sprintf(error_buffer, "'%s' was prematurely assigned an offset",
			     (char *) fst_my_GetFieldByIndex
					(ft_SymTable, j, SYMTAB_NAME));
	      ERROR("MapStorage", error_buffer, WARNING);
	    }

	  fst_my_PutFieldByIndex(ft_SymTable, j, SYMTAB_offset,  bytes);
	  fst_my_PutFieldByIndex(ft_SymTable, j, SYMTAB_STORAGE_CLASS,  SC_GLOBAL);
	  
	  bytes		 += fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_SIZE);
	  j 		  = fst_my_GetFieldByIndex(ft_SymTable, j, SYMTAB_NEXT_COMMON);
	}
      fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_SIZE, bytes);
    }
} /* AssignOffsetsForVarsInCommon */




/*ARGSUSED*/
/* assign storage classes according to the specified rules */
static void AssignStorageClasses (SymDescriptor SymTab, fst_index_t i, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;
{
int CTi, CToffset;      /* used in fst_Symbol_To_EquivLeader_Offset_Size for */
unsigned int CTsize;    /* determining if a variable is a common variable    */
EquivalenceClass *equiv_class_of_i;

/*  if (i == 0) return; */
/* should get storage class for a function */

  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_IS_DATA &&
      /* a var*/
      IsValidName((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)) == 1)
    {
      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NUM_DIMS) > 0 ||
	  /* Non-Scalar */
	  fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) == TYPE_CHARACTER)
	{
	  /* get index of common block name by getting the leader */
	  /* (that is if it exists) 				  */
	  fst_Symbol_To_EquivLeader_Offset_Size
			(ft_SymTable, i, &CTi, &CToffset, &CTsize);

	  if ((CTi != SYM_INVALID_INDEX) &&            /* global */
	      (fst_GetFieldByIndex(ft_SymTable, CTi, SYMTAB_OBJECT_CLASS) &
	      OC_IS_COMMON_NAME))
	    { /* consistency check */
	      if (!(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_GLOBAL))
		{
		 (void) sprintf(error_buffer, 
		 "Storage class set inconsistently for '%s' - '%d' should be SC_GLOBAL",
		     (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),
     	             fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS));
		 ERROR("MapStorage", error_buffer, SERIOUS);
		 fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
		     SC_GLOBAL);
		}
	    }
	  else if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) &
		                                          OC_IS_FORMAL_PAR)
	    {						/* parameter 	*/
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_GLOBAL);	
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  DONT_KNOW);
	    }
	  else if (SymHaveSeenASave != 0 ||		/* static 	*/
		 fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) &
		 SC_STATIC)
	    {
	      if (aiAlignDoubles > 0 &&
		  is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
	     aiNextStatic = PadToAlignment(aiNextStatic,8);

	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStatic);
	      aiNextStatic   += Align(fst_my_GetFieldByIndex
						(ft_SymTable, i, SYMTAB_SIZE));
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STATIC);
	    }
	  else						/* stack 	*/
	    {
	      if (aiAlignDoubles > 0 && 
		  is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
		aiNextStack = PadToAlignment(aiNextStack,8);

	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStack);
	      aiNextStack  += Align(fst_my_GetFieldByIndex
					(ft_SymTable, i, SYMTAB_SIZE));
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STACK);

	    }
	} /* end of Non-Scalar */
      else						/* A Scalar */
	{

	  /* get index of common block name by getting the leader */
	  /* (that is if it exists) 				  */
	  fst_Symbol_To_EquivLeader_Offset_Size
			(ft_SymTable, i, &CTi, &CToffset, &CTsize);

	  if ((CTi != SYM_INVALID_INDEX) &&            /* global */
	      (fst_GetFieldByIndex(ft_SymTable, CTi, SYMTAB_OBJECT_CLASS) &
	      OC_IS_COMMON_NAME))
	    {
	      if (aiEnregGlobals > 0 && IsAliased(i,true) == 0)
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
	        fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) |
		SC_NO_MEMORY);
	    }
	  
	  else if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) &
		SC_CONSTANT)
	    {						/* constant 	*/
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
		SC_CONSTANT);
	      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) == TYPE_INTEGER)
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
	          fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) |
		SC_NO_MEMORY);

	  /* allocate some space */
	  if ((fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_INTEGER)  &&
	      (strncmp((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),
		       "2nd reg",7) != 0) )
	    {
	      if (aiAlignDoubles > 0 &&
		  is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
		aiNextStatic = PadToAlignment(aiNextStatic,8);

	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStatic);
	      aiNextStatic      += Align(fst_my_GetFieldByIndex
					(ft_SymTable, i, SYMTAB_SIZE));
	    }
	    }
	  
	  else if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & 
		   OC_IS_FORMAL_PAR)
	    {						/* parameter	*/
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  DONT_KNOW);
	      if (IsAliased(i,false) == 0)
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
				       SC_GLOBAL | SC_NO_MEMORY);
	      else
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
			SC_GLOBAL);
	    }
	  
	  else if (SymHaveSeenASave != 0 ||		/* static	*/
		   fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) &
		   SC_STATIC)
	    {
	      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_low) ==
		  fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_high))
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
				       SC_STATIC | SC_NO_MEMORY);
	      else
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
			SC_STATIC);
	      /* allocate some space */
	      if (aiAlignDoubles > 0 && 
		  is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
		aiNextStatic = PadToAlignment(aiNextStatic,8);
	      
	      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStatic);
	      aiNextStatic  += Align(fst_my_GetFieldByIndex
					(ft_SymTable, i, SYMTAB_SIZE));
	    }
	  
	  else 						/* stack	*/
	    {
	      equiv_class_of_i = fst_OverlappingSymbols(ft_SymTable, i);
	      if (equiv_class_of_i == 0) /* then no symbols in the equivalence class */
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
			SC_NO_MEMORY);
	      else if (equiv_class_of_i->members == 1)
		fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
			SC_NO_MEMORY);
	      else
		{
		  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,
			SC_STACK);
		  /* allocate some space */
		  if (aiAlignDoubles > 0 && 
		      is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
		    aiNextStack = PadToAlignment(aiNextStack,8);

		  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStack);
		  aiNextStack += Align(fst_my_GetFieldByIndex
					(ft_SymTable, i, SYMTAB_SIZE));
		}
	    }
	} 
      
    }
} /* AssignStorageClasses */




/*ARGSUSED*/
/* assign static space according to the specified rules */
static void AssignStaticSpace (SymDescriptor SymTab, fst_index_t i, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;
{
  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_CONSTANT  &&
      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_INTEGER 	  &&
      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_LOGICAL 	  &&
      fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset) == DONT_KNOW 	  &&
      strncmp((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),
			"2nd reg",7) != 0)
    {
      if (aiAlignDoubles > 0 && 
	  is_double(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE)))
	aiNextStatic = PadToAlignment(aiNextStatic, 8);
      
      fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_offset,  aiNextStatic);
      aiNextStatic	+= Align(fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_SIZE));
    }
} /* AssignStaticSpace */




/*ARGSUSED*/
/* assign offsets to the variables in an equivalence class */
static void AndEquivalencedNames(SymDescriptor SymTab, fst_index_t i, Generic dummy)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic dummy;

{
EquivalenceClass *equiv_class_of_i;

int leader_index, leader_offset;  /* used in fst_Symbol_To_EquivLeader_Offset_Size for   */
unsigned int leader_size;	  /* determining whether a variable is a common variable */

int j, index;

if ((i == 0) || ((fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_PARENT) == -1)))
  return;

equiv_class_of_i = fst_OverlappingSymbols(ft_SymTable, i);
if ((equiv_class_of_i != 0) && (equiv_class_of_i->members != 1))
						/* then the variable was equivalenced */
  {
  if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset) == DONT_KNOW)
       { /* assign offsets to the variables in the equivalence class */


       for (j=0; j<equiv_class_of_i->members; j++)
	 {           
	   index = equiv_class_of_i->member[j];

	   fst_Symbol_To_EquivLeader_Offset_Size (ft_SymTable, index, &leader_index,
						  &leader_offset, &leader_size);
	   
           fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_offset, leader_offset);
       
	   /* Now, check to make sure that we haven't seen a backward
	    * extension to a COMMON block.  If we really trusted the 
	    * type checker, we could eliminate this code.
	    */

	   /* get index of common block name by lookin at the leader */
	   /* (that is if it exists) 				     */

	  if ((leader_index != SYM_INVALID_INDEX) &&            /* global */
	      (fst_GetFieldByIndex(ft_SymTable, leader_index, SYMTAB_OBJECT_CLASS) &
			OC_IS_COMMON_NAME) &&
	      (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset) < 0))
	    { 
	      (void) sprintf(error_buffer, 
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME), 
		   (char *) fst_GetFieldByIndex(ft_SymTable, leader_index, SYMTAB_NAME),
		      -fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_offset));
	      ERROR("Case1", error_buffer, WARNING);
	      ERROR("Case1", "This violates the standard", SERIOUS);
	    } 
	 }
     }
  }
} /* AndEquivalencedNames */




/* return the size of a variable (or array) */
int VarSize(int index)
  //  int index;
{
  int ubound[7], lbound[7], size, elements, lb, ub, i;
  ArrayBound *bounds;

  elements = 1;

  int ftype = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE);
  switch(ftype)
  {
    case TYPE_CHARACTER:	size = fst_my_GetFieldByIndex(ft_SymTable, index, 
						SYMTAB_CHAR_LENGTH);	break;
    case TYPE_LOGICAL:
    case TYPE_INTEGER:		
    case TYPE_LABEL:	
    case TYPE_REAL:	
    case TYPE_DOUBLE_PRECISION:
    case TYPE_COMPLEX:	
    case TYPE_DOUBLE_COMPLEX:
	size = GetDataSize(ftype);
	break;
    default:
	(void) sprintf(error_buffer, 
		"The name '%s' has an unexpected data type", 
		(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	ERROR("VarSize", error_buffer, WARNING);
	size = 1;
	break;
  }

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS) != 0)
     for (i = 0; i<fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS); i++)
     { /* get the raw information about array's dimensions */
       bounds = (ArrayBound *) fst_my_GetFieldByIndex
					(ft_SymTable, index, SYMTAB_DIM_BOUNDS);
	lb = getIndexForlb (bounds,i,index);
	if (lb == SYM_INVALID_INDEX)
	{
	  (void) sprintf(error_buffer, "'%d lb dim' not in Symbol Table", i);
	  ERROR("VarSize", error_buffer, FATAL);
	}
	else if (!(fst_my_GetFieldByIndex(ft_SymTable, lb, SYMTAB_STORAGE_CLASS) & 
		                                                       SC_CONSTANT))
	{
	  (void) sprintf(error_buffer, 
		 "Array '%s' has non-constant lower bound - '1' assumed",
		 (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	  ERROR("VarSize", error_buffer, WARNING);
	  lbound[i] = 1;

	}
	else
	  lbound[i] = bounds[i].lb.value.const_val;

	ub = getIndexForub (bounds,i, index);
	if (ub == SYM_INVALID_INDEX)
	{
	  (void) sprintf(error_buffer, "'%d ub dimension' not in Symbol Table", i);
	  ERROR("VarSize", error_buffer, FATAL);
	}
	else if (!(fst_my_GetFieldByIndex(ft_SymTable, ub, SYMTAB_STORAGE_CLASS) &
                                                                       SC_CONSTANT))
	{
	  (void) sprintf(error_buffer, 
		  "Array '%s' has non-constant upper bound - '1' assumed",
		  (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME));
	  ERROR("VarSize", error_buffer, WARNING);
	  ubound[i] = 1;
	}
	else
          ubound[i] = bounds[i].ub.value.const_val;

	elements = elements * (ubound[i] - lbound[i] + 1);
     }
  else
     elements = 1;

  return elements * size;
} /* VarSize */




/* determine if the string contains a valid FORTRAN variable name */
int IsValidName( char *p )
  // char *p;
{
  if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')))
     return 0;

  p++;
  while (*p != '\0')
  {
    if (!(((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) ||
          (*p >= '0' && *p <= '9')) )
       return 0;
    p++;
  }
  return 1;
} /* IsValidName */




/* determine if the type is double precision or double-precision complex */
int is_double(int type) {
  return (type == TYPE_DOUBLE_PRECISION || type == TYPE_DOUBLE_COMPLEX);
} /* is_double */




/* change size to ensure word alignment */
int Align(int size)
  // int size;
{
  if (size % 4 != 0)
     size += 4 - (size % 4);

  return size;
} /* Align */




/* alter the base to account for the alignment specified by boundary */
int PadToAlignment(int base, int boundary)
  //int base;
  //int boundary;
{
  if (base % boundary != 0)
     base = base + (boundary - (base % boundary));

  return base;
} /* PadToAlignment */

