/* $Id: strings.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/classes.h>
#include <libs/f2i/mnemonics.h>

#include <stdio.h>

/* forward declarations */




/* Compute the length of a string.  If the string is */
/* a constant, store the length in the symbol table. */
int StringLength(int index)
  // int index;

{
  int ubound, lbound, elements, lb, ub, i;
  char *p;
  ArrayBound *bounds;

  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) != TYPE_CHARACTER)
  {
    (void) sprintf(error_buffer, 
	"Attempt to take the string length of '%s' which has type '%s'",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		TypeName(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)));
    ERROR("StringLength", error_buffer, FATAL);
  }

  /* Constant? */
  if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) & SC_CONSTANT)
  {
    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH) > 0)
	elements = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH);
    else /* compute it */
    {
      elements = 0;
      p = (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME);
      if (*p == '\'')
	p++;
      while (*p != '\'' && *p != '\0')
      {
	p++; elements++;
      }
      fst_my_PutFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH,  elements);
    }
  }

  /* Retrieve the outermost dimension and compute it's size ! */
  else if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS) == 0)
     elements = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH);
  else 
  {
    i = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NUM_DIMS) - 1 /*0-based index*/;
    bounds = (ArrayBound *)
		fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_DIM_BOUNDS);

    lb = getIndexForlb(bounds, i, index);

    if (lb == SYM_INVALID_INDEX)
    {
	(void) sprintf(error_buffer, "%d, lb dimension is not in Symbol Table", i);
	ERROR("StringLength", error_buffer, FATAL);
    }
    else if (!(fst_my_GetFieldByIndex(ft_SymTable, lb, SYMTAB_STORAGE_CLASS)
		& SC_CONSTANT))
    {
	ERROR("StringLength", 
	      "String has non-constant dimension lower bound - 1 assumed", 
	      WARNING);
	lbound = 1;

     }
     else
       lbound = bounds[i].lb.value.const_val;

     ub = getIndexForub(bounds, i, index);

     if (ub == SYM_INVALID_INDEX)
     {
	(void) sprintf(error_buffer, "%d, ub dimension is not in Symbol Table", i);

	ERROR("StringLength", error_buffer, FATAL);
     }
     else if (!(fst_my_GetFieldByIndex(ft_SymTable, ub, SYMTAB_STORAGE_CLASS)
		& SC_CONSTANT))
     {  
	ERROR("StringLength", 
	      "String has non-constant dimension upper bound - 1 assumed", 
	      WARNING);
	ubound = 1;
     }
     else
        ubound = bounds[i].ub.value.const_val;

	elements = ubound - lbound + 1;
     }

  return elements;
} /* StringLength */




/* Return number of characters (excluding quotes) in a string. */
int QuotedLength( char *str )
  // char *str;
{
  register int len;
  register char *p;

  p = str;
  if (*p == '\'')
     p++;

  len = 0;
  while(*p != '\'' && *p != '\0')
  {
    p++; len++;
  }

  return len;
} /* QuotedLength */
