#ifndef MemoryOrder_h
#define MemoryOrder_h

#include <general.h>
#include <mh.h>

EXTERN(void,li_ComputeMemoryOrder,(model_loop    *loop_data,
				   SymDescriptor symtab,
				   PedInfo       ped,
				   arena_type    *ar));

#endif 
