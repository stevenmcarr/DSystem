/* $Id: label.h,v 1.3 1992/12/07 10:17:25 carr Exp $ */

#ifndef label_h
#define label_h

EXTERN(char *, ut_symtab_get_label_str,(SymDescriptor symtab));
EXTERN(void, ut_update_labels,(AST_INDEX stmt,SymDescriptor symtab));

#endif
