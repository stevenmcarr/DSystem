/* $Id: label.h,v 1.4 1992/12/11 11:19:47 carr Exp $ */

#ifndef label_h
#define label_h

#ifndef general_h
#include <general.h>       /* for EXTERN */
#endif
#ifndef fortsym_h
#include <fortsym.h>       /* for SymDescriptor */
#endif
#ifndef ast_h
#include <fort/ast.h>      /* for AST_INDEX */
#endif

EXTERN(char *, ut_symtab_get_label_str,(SymDescriptor symtab));
EXTERN(void, ut_update_labels,(AST_INDEX stmt,SymDescriptor symtab));

#endif
