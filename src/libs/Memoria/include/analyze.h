#ifndef analyze_h
#define analyze_h

typedef struct {
  int        parent,
             last_loop,
             last_stack[MAX_LEVEL],
             loop_num,
             level,
             stack_top;
  model_loop *loop_data;
  PedInfo    ped;
  SymDescriptor symtab;
 } build_info_type;

EXTERN_FUNCTION(void ut_analyze_loop,(AST_INDEX root,model_loop *loop_data,
				      int level,PedInfo ped,
				      SymDescriptor symtab));

#endif
