#ifndef RefGroups_h
#define RefGroups_h

#include <general.h>
#include <mh.h>
#include <fort/AstIter.h>
#include <IntegerList.h>
#include <UniformlyGeneratedSets.h>

typedef struct refinfotype {
  PedInfo       ped;
  DG_Edge       *dg;
  arena_type    *ar;
  int           num_loops;
  int           level;
  int           loop;
  model_loop    *loop_data;
  Boolean       VisitedMark;
  float         InvariantCost,
                SpatialCost,
                GroupSpatialCost,
                NoneCost,
                TemporalCost;
  char          **IVar;
  UniformlyGeneratedSets *UGS;
 } RefInfoType;

class RefGroupCore {

public:
  Boolean CanMoveToInnermost(DG_Edge *edge);
  Boolean OnlyInInnermostPosition(model_loop *loop_data, AST_INDEX node,int level);
  char*   FindInductionVar(model_loop *loop_data,AST_INDEX node,int level);
  Boolean NotInOtherPositions(AST_INDEX node,char *var);
  void    UnsetVisitedMark(AST_INDEX node);
 };
  



class RefGroupMember : public IntegerList, RefGroupCore {

  Boolean SelfTemporal,
          SelfSpatial,
          GroupTemporal,
          GroupSpatial;

public:

  RefGroupMember()
    {
     SelfTemporal = false;
     SelfSpatial = false;
     GroupTemporal = false;
     GroupSpatial = false;
    }
  
public:

  void SetSelfTemporal() {SelfTemporal = true;};
  void SetSelfSpatial() {SelfSpatial = true;};
  void SetGroupTemporal() {GroupTemporal = true;};
  void SetGroupSpatial() {GroupSpatial = true;};
  
  Boolean HasSelfTemporal() {return SelfTemporal;};
  Boolean HasSelfSpatial() {return SelfSpatial;};
  Boolean HasGroupTemporal() {return GroupTemporal;};
  Boolean HasGroupSpatial() {return GroupSpatial;};

  void CheckSelfSpatial(AST_INDEX  sublist,char *var,int words,float& SpatialCost,
			UniformlyGeneratedSets *UGS);
  void CheckSelfTemporal(AST_INDEX node, PedInfo ped,model_loop *loop_data,int loop,
			 float& InvariantCost,UniformlyGeneratedSets *UGS);

  void CheckTemporal(AST_INDEX node,PedInfo ped,model_loop *loop_data,int loop,
		     float& TemporalCost,UniformlyGeneratedSets *UGS);
  void CheckGroupSpatial(AST_INDEX node,PedInfo ped,model_loop *loop_data,
			 int loop,float& GroupSpatialCost,int words,
			 UniformlyGeneratedSets *UGS);
  AST_INDEX FindOldestValue(PedInfo ped);


 };



class RefGroupMemberIter : public IntegerListIter {

  public:
    RefGroupMemberIter(RefGroupMember& RGS) : IntegerListIter(RGS)
      {
      }
    RefGroupMemberIter(RefGroupMember* RGS) : IntegerListIter(RGS)
      {
      } 

   AST_INDEX operator() ()
    {
     return (AST_INDEX) 
            (IntegerListIter::operator()())->GetValue();
    } 

};




class RefGroupSet : public IntegerList, RefGroupCore {

  UniformlyGeneratedSets *UGS;
  Boolean UseUniformlyGeneratedSets;

  void PartitionNames(AST_INDEX node, RefInfoType& RefInfo);
  void DoPartition(AST_INDEX name,RefGroupMember *RG,DG_Edge *dg,PedInfo ped,
	   int level,int MinLevel,Boolean VisitedMark,model_loop *loop_data);
  void BuildRefGroupsWithUGS();

public:
  RefGroupSet(AST_INDEX loop, int NL,RefInfoType& RefInfo,
	      Boolean UseUGS = false, int *LIS = NULL);
  ~RefGroupSet() {if (UseUniformlyGeneratedSets) delete UGS;}

};





class RefGroupSetIter : public IntegerListIter {

  public:
    RefGroupSetIter(RefGroupSet& RGS) : IntegerListIter(RGS)
      {
      }
    RefGroupSetIter(RefGroupSet* RGS) : IntegerListIter(RGS)
      {
      } 

   RefGroupMember *operator() ()
    {
     return (RefGroupMember *) 
            (IntegerListIter::operator()())->GetValue();
    } 

};

#endif