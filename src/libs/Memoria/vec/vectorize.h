/* $Id: interchange.h,v 1.8 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef vectorize_h
#define vectorize_h

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#include <libs/Memoria/vec/depGraph.h>

 typedef struct vecinfotype {
   PedInfo ped;
   DependenceGraph *vecDepGraph;
 } vec_info_type;

#endif
