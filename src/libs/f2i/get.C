/* $Id: get.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <sys/file.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/mnemonics.h>


/* global variable - shared with stmts.c */
extern int NotMapped;

/* static variables */
static char name_buffer[128];

/* forward declarations */




/* return the symbol table index for an       */
/* identifier or a constant given an AST node */
int getIndex( AST_INDEX node )
  // AST_INDEX	node;
{
  register int nodeType, realType, i, ct;
  register char *name;

  nodeType = gen_get_node_type(node);

  switch(nodeType)
  {
    case GEN_IDENTIFIER:
	name = gen_get_text(node);
	i = fst_my_QueryIndex(ft_SymTable, name);
        if (i == SYM_INVALID_INDEX)
	{
	  (void) sprintf(error_buffer, 
		       "Identifier '%s' not found in Symbol Table", name);
	  ERROR("getIndex", error_buffer, FATAL);
	}
	else if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) ==
		OC_UNDEFINED)
	{  /* need to insert into symbol table */
	  realType = gen_get_real_type(node);
	  i = SymInsertSymbol( name, realType, DK, DK, DK, DK );
          fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS, DK);
	}

	if ((NotMapped == FALSE) && 	/* prevent premature typing */
	    (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS) & OC_UNDEFINED))
	{ /* is an undeclared variable => local and scalar */
	  if (fst_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) ==
	      TYPE_CHARACTER && SymHaveSeenASave == 0)
	     fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_STACK);
	  else if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) ==
			TYPE_CHARACTER)
	     fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_TYPE,  SC_STATIC);
	  else if (SymHaveSeenASave != 0)
	     fst_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  
				 SC_NO_MEMORY | SC_STATIC);
	  else 
	     fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS,  SC_NO_MEMORY);

	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_OBJECT_CLASS,  OC_IS_DATA);
	  fst_my_PutFieldByIndex(ft_SymTable, i, SYMTAB_SIZE,  VarSize(i));
	}
	break;

    case GEN_CONSTANT:
	ct   = gen_get_real_type(node);
	name = ConstantName(gen_get_text(node), ct, name_buffer);
	i = fst_my_QueryIndex(ft_SymTable, name);
	if (i == SYM_INVALID_INDEX)
	{
	  i = SymInsertSymbol( name, ct, OC_IS_DATA, 0, 
			       SC_CONSTANT, NO_ALIAS );
	}
	break;

    default:
	(void) sprintf(error_buffer, "%s - type %s (%d)", 
		"invoked on non-IDENTIFIER node", 
		ast_get_node_type_name(node), node);
	ERROR("GetIndex", "invoked on non-ID node", FATAL);
	i = -1; /* return a bogus value if we're running "-F" 	*/
	break;	
  }
  return i;
} /* getIndex */




/* return the symbol table index for an integer constant */
int getIntConstantIndex(int num)
  // int num;
{
  register int i;
  char name[64];

  (void) sprintf(name, "%d[%d]", num, TYPE_INTEGER);
  i = fst_QueryIndex(ft_SymTable, name);
  if (i == SYM_INVALID_INDEX)
    {
      i = SymInsertSymbol( name, TYPE_INTEGER, OC_IS_DATA, 0,
                          SC_CONSTANT, NO_ALIAS );
    }
  return i;
} /* getIntConstantIndex */


char *getLocality(AST_INDEX node)
  // AST_INDEX node;

  {
   if (DepInfoPtr(node) == NULL)
     return("&unknown");
   switch (DepInfoPtr(node)->Locality)
     {
      case UNDEFINED:
        return("&unknown");
      case NONE:
	return("&none");
      case SELF_SPATIAL:   
        return("&self-spatial");
      case GROUP_SPATIAL:  
	return("&group-spatial");
      case SELF_TEMPORAL:         
      case SELF_TEMPORAL_CACHE:
	return("&self-temporal");
      case GROUP_TEMPORAL_CACHE:
      case GROUP_TEMPORAL: 
	return("&group-temporal");
      default:
        return("&unknown");
     }
  }
