/* $Id: analyze.h,v 1.3 1992/12/07 10:17:02 carr Exp $ */

#ifndef analyze_h
#define analyze_h

typedef struct buildinfotype {
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

EXTERN(void, ut_analyze_loop,(AST_INDEX root,model_loop *loop_data,
				      int level,PedInfo ped,
				      SymDescriptor symtab));

#endif
