/* $Id: shape.h,v 1.5 1997/03/20 15:49:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef shape_h
#define shape_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif 

EXTERN(void, ut_compare_loops,(model_loop *loop_data,int outer,
				       int inner));
EXTERN(void, ut_check_shape,(model_loop *loop_data,int loop));

#endif
