/* $Id: label.h,v 1.2 1992/10/03 15:51:01 rn Exp $ */
#ifndef label_h
#define label_h

EXTERN(char *, ut_symtab_get_label_str,(SymDescriptor symtab));
EXTERN(void, ut_update_labels,(AST_INDEX stmt,SymDescriptor symtab));

#endif
