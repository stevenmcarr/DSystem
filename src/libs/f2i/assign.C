/* $Id: assign.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
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
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>

static char name_buffer[32];




/* generate code for Fortran ASSIGN statement */
void HandleAssign(AST_INDEX node)
  // AST_INDEX node;
{
   AST_INDEX		lhs;		/* integer identifier		   */
   AST_INDEX		rhs;		/* label			   */
   register int		t_lhs;		/* type (integer identifier)	   */
   register int		t_rhs;		/* type (label)			   */
   register int		address;	/* address register for identifier */
   int 			label_value;	/* iloc value for the label	   */
   int 			label_reg;	/* register for the label	   */
   int			target;		/* register for the identifier     */
   register STR_TEXT	AST_label;	/* ast value for the label	   */


   /* retrieve AST node and type for variable and label reference */
   lhs = gen_ASSIGN_get_name(node);
   rhs = gen_ASSIGN_get_lbl_ref(node);

   t_lhs = gen_get_node_type(lhs);
   t_rhs = gen_get_node_type(rhs);

   /* retrieve the value of the label */
   if (t_rhs != GEN_LABEL_REF)
      ERROR("Assign", "Missing target statement label", FATAL);
   else
   { /* get the label's value */
     AST_label = gen_get_text(rhs);
     LabelGet( AST_label, &label_value );
     (void) sprintf(name_buffer, "LL%4.4d", label_value);

     label_reg = SymInsertSymbol(name_buffer, TYPE_LABEL, OC_IS_EXECUTABLE,
			0, SC_STMT_LABEL, NO_ALIAS);
   }

   /* retrieve identifier */
   if (t_lhs == GEN_IDENTIFIER)
   {
      if (gen_get_real_type(lhs) == TYPE_INTEGER)
      {
	target = getIndex(lhs);
	generate(0, iLDI, label_value, label_reg, GEN_LABEL, NOCOMMENT);
	generate_move(target, label_reg, TYPE_INTEGER);

	if (!(fst_my_GetFieldByIndex(ft_SymTable, target, SYMTAB_STORAGE_CLASS)
		& SC_NO_MEMORY))
	{
	  address = getAddressInRegister(target);
	  generate_store(address, target, TYPE_INTEGER, target, "&unknown");
	}
      }
      else
	ERROR("Assign", "Target variable is not of type integer", FATAL);
   }
   else 
      ERROR("Assign", "Target variable is not an identifier", FATAL);
} /* HandleAssign */

