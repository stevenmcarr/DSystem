/* $Id: stats.h,v 1.3 1992/12/07 10:15:36 carr Exp $ */
#ifndef stats_h
#define stats_h

#include <Arena.h>
#include <global.h>

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

typedef enum {MISS, IHIT, AHIT,REGISTER} RefType;

#endif
