/* $Id: stats.h,v 1.8 1995/03/13 15:10:23 carr Exp $ */
#ifndef stats_h
#define stats_h

#ifndef general_h
#include <general.h>
#endif

#ifndef Arena_h
#include <misc/Arena.h>
#endif

#ifndef dp_h
#include <dp.h>
#endif

#ifndef dg_h
#include <dg.h>
#endif

#ifndef list_h
#include <misc/list.h>
#endif

#ifndef mh_h
#include <mh.h>
#endif

#include <LoopStats.h>
#include <UniformlyGeneratedSets.h>

/* #define STATSDEBUG  */

typedef struct edgeinfotype {
  PedInfo      ped;
  UtilList     *EdgeList;
 }EdgeInfoType;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

#endif
