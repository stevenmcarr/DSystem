/* $Id: stats.h,v 1.7 1994/07/11 13:40:21 carr Exp $ */
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

/* #define STATSDEBUG  */

typedef struct refinfotype {
  PedInfo       ped;
  DG_Edge       *dg;
  arena_type    *ar;
  int           num_loops;
  UtilList      *GroupList;
  int           level;
  model_loop    *loop_data;
  Boolean       VisitedMark;
  float         InvariantCost,
                SpatialCost,
                OtherSpatialCost,
                NoneCost,
                TemporalCost;
 } RefInfoType;

typedef struct refgrouptype {
  int          number;
  Boolean      Invariant,
               Spatial,
               OtherSpatial,
               Temporal;
  UtilList     *RefList;
 }RefGroupType;

typedef struct edgeinfotype {
  PedInfo      ped;
  UtilList     *EdgeList;
 }EdgeInfoType;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

#endif
