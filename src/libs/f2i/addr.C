/* $Id: addr.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>
#include <libs/f2i/mnemonics.h>




/* forward declarations */

static int parameterArray(AST_INDEX  , int  , int  , AST_INDEX *);
static int constantBounds(AST_INDEX  , int  , int  , AST_INDEX *);


/* return the address of an array location */
int getSubscriptLValue( AST_INDEX node )
  // AST_INDEX 	node;
{
  AST_INDEX	ptr;
  AST_INDEX	exprs[7]; /* standard limits us to seven dimensions */

  int 		index, count, result;

  if (aiDebug > 0)
     (void) fprintf(stdout, "getSubscriptLValue( %d ).\n", node);

  ptr	   = list_first(gen_SUBSCRIPT_get_rvalue_LIST(node));

  count	   = 0;

  while (ptr != ast_null_node)
  {
    exprs[count++] = ptr;
    ptr = list_next(ptr);

    if (count > 6 && ptr != ast_null_node)
       ERROR("Subscript", "too many indices (>7).", FATAL);
  }

  ptr	= gen_SUBSCRIPT_get_name(node);
  index = fst_QueryIndex (ft_SymTable, (char *) gen_get_text(ptr));

  /* check the "reasonableness" of the address expression */
  if (count != fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS))
  {
    (void) sprintf(error_buffer,
	"variable '%s'(%d) - dimension clash - %d vs. %d",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
	index, fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS),
	count );
    ERROR("getSubscriptLValue", error_buffer, WARNING);
    ERROR("getSubscriptLValue", "code may give wrong results", WARNING);
  }

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) & 
                                                             OC_IS_FORMAL_PAR)
  { 
    result = parameterArray( ptr, index, count, exprs );
  }
  else 
  { /* the array bounds must be fixed at compile time and be of type integer*/
    result = constantBounds( ptr, index, count, exprs );
  }
  return result;
} /* getSubscriptLValue */


static char name_buffer[64];




/* compute array address when the array bounds are constant */
static int constantBounds( AST_INDEX var, int index, int dims, AST_INDEX *args )
//   AST_INDEX 	var;	/* ptr to IDENTIFIER		*/
//   int		index;	/* "var"'s symbol table index	*/
//   int		dims;   /* number of dimensions		*/
//   AST_INDEX	*args;	/* ptrs to index expressions	*/
{
  int temp, size, i, ubound, lbound;
  int lbr, minusLb;
  int past, present;
  char buffer[100];
  ArrayBound *bounds;

  if (aiDebug > 0)
     (void) fprintf(stdout, "\tConstantBounds( %d, %d, %d, %d ).\n",
	     var, index, dims, args );

  dims--; 	/* used to index zero-addressed arrays, so decrement it */

  /* generate the outermost dimension */
  temp = getExprInReg(args[dims]);

  bounds = (ArrayBound *) fst_my_GetFieldByIndex
		(ft_SymTable, index, SYMTAB_DIM_BOUNDS);
  lbound = bounds[dims].lb.value.const_val;

  if (lbound != 0)
  {
    lbr = getConstantInRegFromInt (lbound); 
    minusLb = TempReg(temp, lbr, iSUB, TYPE_INTEGER);
    generate(0, iSUB, temp, lbr, minusLb, "index - lb");
    past = minusLb;
  }
  else
     past = temp;

  /* while the inner ones are regular */  

  for (i=dims-1; i >= 0; i--)
  { 
    /* first, running total gets multiplied by size of current dimension */
    /* for non-parameter arrays, this is a compile time constant	 */

    ubound = bounds[i].ub.value.const_val;
    lbound = bounds[i].lb.value.const_val;

    size = ubound - lbound + 1;

    (void) sprintf(name_buffer, "%d", size);
    temp = getIntConstantInRegister(name_buffer);
    present = TempReg(past, temp, iMUL, TYPE_INTEGER);
    generate(0, iMUL, past, temp, present, "total *= dimension");
    past = present;

    /* then, temp gets index through current dimension */
    temp = getExprInReg( args[i] );

    if (lbound != 0)
    {
      lbr = getConstantInRegFromInt (lbound); 
      minusLb = TempReg(temp, lbr, iSUB, TYPE_INTEGER);
      generate(0, iSUB, temp, lbr, minusLb, "index - lb");
    }
    else
       minusLb = temp;

    /* and it gets accumulated into the running total */
    present = TempReg(past, minusLb, iADD, TYPE_INTEGER);
    generate(0, iADD, past, minusLb, present, "total += index-1");
    past = present;
  }

  /* now, running total gets multiplied by bytes/element */
  switch(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE))
  {
    case TYPE_CHARACTER:
        if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH) ==
		STAR_LEN)
	  {
	    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS)
		 & OC_IS_FORMAL_PAR)
	      {
		temp = StrTempReg("s-len",index,TYPE_INTEGER);
	      }
	    else {
	      (void) sprintf(buffer,"character %s is not a parameter", (char *)
		          fst_my_GetFieldByIndex
			  (ft_SymTable, index, SYMTAB_NAME));
	      ERROR("constantBounds",buffer,SERIOUS);
	    }
	  }
	else 
	  {
	    /* we know the length */
	    temp = getConstantInRegFromInt(fst_my_GetFieldByIndex
			(ft_SymTable, index, SYMTAB_CHAR_LENGTH));
	    present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	  }
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is variable");
	past = present;
	break;

    case TYPE_LABEL:
    case TYPE_INTEGER:
        if (aiLongIntegers)
	  temp = getIntConstantInRegister("8");
	else
	  temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is Integer");
	past = present;
	break;
    case TYPE_LOGICAL:
	temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 4");
	past = present;
	break;
    case TYPE_REAL:	 /* length is four or 8 */
        if (aiDoubleReals)
	  temp = getIntConstantInRegister("8");
	else
	  temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 4");
	past = present;
	break;

    case TYPE_DOUBLE_PRECISION:
    case TYPE_COMPLEX:	 /* length is eight */
        temp = getIntConstantInRegister("8");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 8");
	past = present;
	break;

    case TYPE_DOUBLE_COMPLEX:
        temp = getIntConstantInRegister("16");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 16");
	past = present;
	break;

	
    default:
	(void) sprintf (error_buffer,
	    "IDENTIFIER(%s)'S type(%s) is unexpected.", 
  	    (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME), 
            TypeName(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)));
	ERROR("Subscript(c)", error_buffer, WARNING);
	break;
  }

  /* finally, add in the address of the base */
  temp = getAddressInRegister(index);
  present = TempReg(past, temp, iADD, TYPE_INTEGER);
  generate(0, iADD, past, temp, present, "EA is address+subscript");
    
  return present;
} /* constantBounds */




/* for arrays passed as parameters, we use the variable names from
 * the DIMENSIONing declaration to get all of the values for the 
 * address computation.
 */
static int parameterArray( AST_INDEX var, int index, int dims, AST_INDEX *args )
//   AST_INDEX 	var;	/* ptr to IDENTIFIER		*/
//   int		index;	/* "var"'s symbol table index	*/
//   int		dims;   /* number of dimensions		*/
//   AST_INDEX	*args;	/* ptrs to index expressions	*/
{
  int temp, size, i, ubound, lbound, one;
  int lb, lbr, minusLb;
  int past, present;
  ArrayBound *bounds;

  if (aiDebug > 0)
     (void) fprintf(stdout, "\tParameterArray( %d, %d, %d, %d ).\n",
	     var, index, dims, args );

  dims--; 	/* used to index zero-addressed arrays, so decrement it */

  /* generate the index of outermost dimension */
  temp = getExprInReg(args[dims]);

  bounds = (ArrayBound *) fst_my_GetFieldByIndex
		(ft_SymTable, index, SYMTAB_DIM_BOUNDS);

  if ((bounds[dims].lb.type == constant) &&
	(bounds[dims].lb.value.const_val == 0))
    {
      past = temp;
    }
  else 
    {
      lb = getIndexForlb(bounds, dims, index);
      lbr = getValueInReg(lb);
      minusLb = TempReg(temp, lbr, iSUB, TYPE_INTEGER);
      generate(0, iSUB, temp, lbr, minusLb, "index - lb");
      past = minusLb;
    }

  one = getIntConstantInRegister("1");

  /* while the inner ones are regular */  
  for (i=dims-1; i >= 0; i--)
  { 
    /* first, running total gets multiplied by size of current dimension */

    ubound = getIndexForub (bounds, i, index);
    ubound = getValueInReg(ubound);

    if (fst_my_GetFieldByIndex(ft_SymTable, ubound, SYMTAB_TYPE) !=
		TYPE_INTEGER)
	ubound = getConversion(ubound, TYPE_INTEGER);

    lbound = getIndexForlb (bounds, i, index);
    lb = lbound;

    if (lbound != one)
    {
      lbound = getValueInReg(lbound);
      if (fst_my_GetFieldByIndex(ft_SymTable, lbound, SYMTAB_TYPE) !=
		TYPE_INTEGER)
	 lbound = getConversion(lbound, TYPE_INTEGER);

      temp = TempReg(ubound, lbound, iSUB, TYPE_INTEGER);
      generate(0, iSUB, ubound, lbound, temp, NOCOMMENT);
      size = TempReg(temp, one, iADD, TYPE_INTEGER);
      generate(0, iADD, temp, one, size, NOCOMMENT);
    }
    else 
       size = ubound; 

    present = TempReg(past, size, iMUL, TYPE_INTEGER);
    generate(0, iMUL, past, size, present, "total *= dimension");
    past = present;

    /* then, temp gets index through current dimension */
    temp = getExprInReg( args[i] );

    lbr     = getValueInReg(lb);
    minusLb = TempReg(temp, lbr, iSUB, TYPE_INTEGER);
    generate(0, iSUB, temp, lbr, minusLb, "index - lb");

    /* and it gets accumulated into the running total */
    present = TempReg(past, minusLb, iADD, TYPE_INTEGER);
    generate(0, iADD, past, minusLb, present, "total += index-1");
    past = present;
  }

  /* now, running total gets multiplied by bytes/element */
  switch(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE))
  {
    case TYPE_CHARACTER: /* length is one! */
	break;

    case TYPE_LABEL:
    case TYPE_INTEGER:
        if (aiLongIntegers)
	  temp = getIntConstantInRegister("8");
	else
	  temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is Integer");
	past = present;
	break;
    case TYPE_LOGICAL:
	temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 4");
	past = present;
	break;
    case TYPE_REAL:	 /* length is four or eight*/
        if (aiDoubleReals)
	  temp = getIntConstantInRegister("8");
	else
	  temp = getIntConstantInRegister("4");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 4");
	past = present;
	break;

    case TYPE_DOUBLE_PRECISION:
    case TYPE_COMPLEX:	 /* length is eight */
        temp = getIntConstantInRegister("8");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 8");
	past = present;
	break;

    case TYPE_DOUBLE_COMPLEX:
        temp = getIntConstantInRegister("16");
	present = TempReg(past, temp, iMUL, TYPE_INTEGER);
	generate(0, iMUL, past, temp, present, "elt length is 16");
	past = present;
	break;

    default:
	(void) sprintf (error_buffer,
	 "IDENTIFIER(%s)'S type(%s) is unexpected.", 
	 (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
	 TypeName(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)));
	ERROR("Subscript(p)", error_buffer, WARNING);
	break;
  }

  /* finally, we add in the base position */
  temp = getAddressRegister(index); 
  present = TempReg(past, temp, iADD, TYPE_INTEGER);
  generate(0, iADD, past, temp, present, "EA is address+subscript");
  
  return present;
} /* parameterArray */




/* generate load to get a value in a register */
int getValueInReg( int index )
  // int index;
{
  register int reg, type, AReg, DReg, SC;

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_OBJECT_CLASS) &
		OC_IS_DATA)
  {
    SC = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS);
    if (SC & SC_NO_MEMORY)
       reg = index;
    else if (SC & SC_CONSTANT)
       reg = getConstantInRegFromIndex(index,
		  fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE));
    else if (SC & (SC_GLOBAL | SC_STACK | SC_STATIC))
    {
      AReg = getAddressInRegister(index);
      type = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE);
      DReg = StrTempReg("!", AReg, type);
      generate_load(DReg, AReg, type, index,"&unknown");
      generate_move(index, DReg, type);
      reg = index;
    }
    else
    {
      (void) sprintf(error_buffer, "'%s'(%d) has an invalid storage class", 
		     (char *) fst_my_GetFieldByIndex
			      (ft_SymTable, index, SYMTAB_NAME), index);
      ERROR("getValueInReg", error_buffer, FATAL);
      reg = -1;
    }

  }
  else
  {
    (void) sprintf(error_buffer, "'%s'(%d) is not a data item",
	    (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
	    index);
     ERROR("getValueInReg", error_buffer, FATAL);
  }
  return reg;
} /* getValueInReg */




/* get the address of a value given the AST node */
int AddressFromNode(AST_INDEX n)
  // AST_INDEX n;
{
  int addr, type, i;

  type = gen_get_node_type(n);
  if (type == GEN_IDENTIFIER || type == GEN_CONSTANT) {
    i = getIndex(n);
    if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) &
		SC_NO_MEMORY)
      {
	ERROR("AddressFromNode", 
	      "AI logic error - AddressFromNode (addresses.c)", SERIOUS);
	ERROR("AddressFromNode",
	      "Attempt to take address of register value (multivalued)",
	      SERIOUS);
	(void) sprintf(error_buffer, "Name '%s', Storage Class '%s', Type '%s'",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME),
		decodeStorageClass(i), decodeType(i));
	ERROR("AddressFromNode", error_buffer, SERIOUS);
      }
     addr = getAddressInRegister(i);
   }

  else if (type == GEN_SUBSCRIPT)
     addr = getSubscriptLValue(n);

  else if (type == GEN_SUBSTRING)
     addr = getSubstringAddress(n);

  else
     ERROR("AddressFromNode", "Unexpected node type encountered", FATAL);

  return addr;
} /* AddressFromNode */


static char BaseAddr[64];



/* return the name of the base address (COMMON block name) */
char *BaseAddressName(int i)
  // int i;
{
  register int SC;
  int CTi, CToffset;
  unsigned int CTsize;
  char *p;

  SC = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);

  if (SC & SC_STATIC || SC & SC_CONSTANT)
  {
    p = (char *) fst_GetFieldByIndex(ft_SymTable, aiStaticLabel(), SYMTAB_NAME);
  }
  else if (SC & SC_GLOBAL)
  {
    /* get index of common block name by getting the leader */
    fst_Symbol_To_EquivLeader_Offset_Size
	(ft_SymTable, i, &CTi, &CToffset, &CTsize);

    if ((CTi == SYM_INVALID_INDEX) ||
	(!(fst_GetFieldByIndex(ft_SymTable, CTi, SYMTAB_OBJECT_CLASS) &
	OC_IS_COMMON_NAME)))
    {
      (void) sprintf(error_buffer, 
		"Variable '%s' is GLOBAL but has no COMMON name",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
      ERROR("BaseAddressName", error_buffer, FATAL);
    }
    (void) sprintf(BaseAddr, "_%s_", (char *)
		   fst_GetFieldByIndex(ft_SymTable, CTi, SYMTAB_NAME));
    p = BaseAddr;
  }
  else 
  {
    p = BaseAddr;
    *p = '\0';
    (void) sprintf(error_buffer, 
		"Request for base address for variable '%s' which has none",
		(char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
    ERROR("BaseAddressName", "AI logic error - BaseAddressName()", SERIOUS);
    ERROR("BaseAddressName", error_buffer, SERIOUS);
  }
  return p;
} /* BaseAddressName */




/* return symbol table index of the COMMON block name */
int BaseIndex(int i)
  // int i;
{
  register int SC, p;
  int CTi, CToffset;
  unsigned int CTsize;

  SC = fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS);

  if (SC & SC_STATIC || SC & SC_CONSTANT)
  {
    p = -1;
  }
  else if (SC & SC_GLOBAL)
  {
    /* get index of common block name by getting the leader */
    fst_Symbol_To_EquivLeader_Offset_Size
	(ft_SymTable, i, &CTi, &CToffset, &CTsize);

    if ((CTi == SYM_INVALID_INDEX) ||
	(!(fst_GetFieldByIndex(ft_SymTable, CTi, SYMTAB_OBJECT_CLASS) &
	OC_IS_COMMON_NAME)))
    {
      (void) sprintf(error_buffer, 
		     "Variable '%s' is GLOBAL but has no COMMON name",
		     (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
      ERROR("BaseIndex", error_buffer, FATAL);
    }
    p = CTi;
  }
  else 
  {
    p = DONT_KNOW;
    (void) sprintf(error_buffer, 
		   "Request for base address for variable '%s' which has none",
		   (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME));
    ERROR("BaseIndex", "AI logic error - BaseIndex()", SERIOUS);
    ERROR("BaseIndex", error_buffer, SERIOUS);
  }
  return p;
} /* BaseIndex */

