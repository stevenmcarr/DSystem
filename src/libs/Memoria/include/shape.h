/* $Id: shape.h,v 1.4 1992/12/11 11:20:02 carr Exp $ */

#ifndef shape_h
#define shape_h

#ifndef general_h
#include <general.h>                 /* for Boolean, EXTERN */
#endif
#ifndef mh_h
#include <mh.h>                      /* for model_loop */
#endif 

EXTERN(void, ut_compare_loops,(model_loop *loop_data,int outer,
				       int inner));
EXTERN(void, ut_check_shape,(model_loop *loop_data,int loop));

#endif
