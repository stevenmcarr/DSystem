/* $Id: MemoryOrder.h,v 1.3 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef MemoryOrder_h
#define MemoryOrder_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>

EXTERN(void,li_ComputeMemoryOrder,(model_loop    *loop_data,
				   SymDescriptor symtab,
				   PedInfo       ped,
				   arena_type    *ar));

#endif 
