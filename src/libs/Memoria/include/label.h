#ifndef label_h
#define label_h

EXTERN_FUNCTION(char *ut_symtab_get_label_str,(SymDescriptor symtab));
EXTERN_FUNCTION(void ut_update_labels,(AST_INDEX stmt,SymDescriptor symtab));

#endif
