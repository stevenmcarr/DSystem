/* $Id: Memoria_label.h,v 1.1 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef label_h
#define label_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

EXTERN(char *, ut_symtab_get_label_str,(SymDescriptor symtab));
EXTERN(void, ut_update_labels,(AST_INDEX stmt,SymDescriptor symtab));

#endif
