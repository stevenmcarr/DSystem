/* $Id: stats.h,v 1.9 1997/03/27 20:25:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef stats_h
#define stats_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef Arena_h
#include <libs/support/memMgmt/Arena.h>
#endif

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef list_h
#include <libs/support/lists/list.h>
#endif

#ifndef mh_h
#include <libs/Memoria/include/mh.h>
#endif

#include <libs/Memoria/include/LoopStats.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>

/* #define STATSDEBUG  */

typedef struct edgeinfotype {
  PedInfo      ped;
  UtilList     *EdgeList;
 }EdgeInfoType;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

#endif
