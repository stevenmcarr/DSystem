/* $Id: stats.h,v 1.2 1992/10/03 15:15:58 rn Exp $ */
#ifndef stats_h
#define stats_h

#include <Arena.h>
#include <global.h>

typedef struct {
  PedInfo       ped;
  DG_Edge       *dg;
  arena_type    *ar;
  int           num_loops;
  UtilList      *GroupList;
  int           level;
  model_loop    *loop_data;
 } RefInfoType;

typedef struct {
  int          number;
  Boolean      Invariant,
               Spatial,
               OtherSpatial,
               Temporal;
  UtilList     *RefList;
 }RefGroupType;

typedef struct {
  PedInfo      ped;
  UtilList     *EdgeList;
 }EdgeInfoType;

#define INDEX  "mh: index"
#define PRE_VAL  1
#define POST_VAL 2

typedef enum {MISS, IHIT, AHIT,REGISTER} RefType;

#endif
