/* $Id: Reversal.h,v 1.2 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef Reversal_h
#define Reversal_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>

EXTERN(Boolean, li_LoopReversal, (model_loop  *loop_data,
				  int         loop,
				  UtilList    *EdgeList,
				  PedInfo     ped));

#endif 
