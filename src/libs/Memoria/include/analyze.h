/* $Id: analyze.h,v 1.4 1992/12/11 11:19:41 carr Exp $ */

#ifndef analyze_h
#define analyze_h

#ifndef general_h
#include <general.h>       /* for EXTERN */
#endif
#ifndef mh_h
#include <mh.h>               /* for loop_data */
#endif
#ifndef dp_h
#include <dp.h>              /* for PedInfo */
#endif
#ifndef fortsym_h
#include <fort/fortsym.h>     /* for SymDescriptor */
#endif
#ifndef ast_h
#include <fort/ast_h>         /* for AST_INDEX */
#endif

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
