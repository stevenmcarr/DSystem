/* $Id: la.h,v 1.8 1998/08/05 19:31:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


//  Linear Algebra Data Reuse Model
//      Header File
//
//         includes
//	 ---- DataReuseModel
//	 ---- DataReuseModelEntry
//       ---- GroupSpatialEntry
//       ---- GroupSpatialSet
//       ---- GroupTemporalEntry
//       ---- GroupTemporalSet
//	 ---- VectListEntry
//	 ---- VectList
// 

#ifndef la_h
#define la_h

#include <libs/support/Lambda/Lambda.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/ast/AstIter.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>
#include <libs/Memoria/uj/compute_uj.h>
#include <libs/Memoria/include/GenericList.h>
#include <libs/Memoria/include/mem_util.h>


#define True  1
#define False 0

class ComputeBoard;

extern int BlockSize;
class VectListEntry:
	 public SinglyLinkedListEntry
	{
	 private:
		la_vect const_vect;
		AST_INDEX node;
	 public:
		VectListEntry(AST_INDEX, la_vect, int);
		la_vect GetVect() { return const_vect; }
		void PrintOut();
	};

class VectList:
	 public SinglyLinkedList
	{
	 private:
	        int Subs;	
		friend class VectListIter;
	 public:
                VectList(int sub); 
		void AddVect(AST_INDEX, la_vect);
		void PrintOut();  
		
	};	

class VectListIter:
	 public SinglyLinkedListIterator
	{
	 public:
		VectListIter(VectList *l):
		  SinglyLinkedListIterator(l)
		{ }
		
		VectListIter(VectList &l):
                  SinglyLinkedListIterator(l)
                { }
		
		VectListEntry* operator () ();
	};

class GroupTemporalEntry:
	 public SinglyLinkedListEntry
	{
	 private:
		int size;	 // # of GT references 
		int distance;    // the distance between this GT set to the GS set leader
		la_matrix H;
		la_vect LocIterSpace;
		la_vect ZeroSpace;
                int Nestl, Subs;
                la_vect leader_v;
		AST_INDEX leader_n;
		VectList *vectlst;
		GenericList *NodeList;
	 public:
		GroupTemporalEntry(la_vect, int, int, la_matrix, AST_INDEX, la_vect );
		~GroupTemporalEntry(){};
		int take(AST_INDEX, la_vect);
		int NodeshasGT(la_vect, la_vect);
		int CheckIntersect(la_vect, la_vect, int);
		int SolutionIsValid(la_matrix, la_vect, la_vect);
                int IsSolutionInLIS(la_matrix, la_vect, int, int, int);
		int Distanceof() { return distance; };
		int SizeofGT() { return size; };
		la_vect getleader() { return leader_v; };
		AST_INDEX LeaderNode() { return leader_n; };
		void PrintOut();
		void computedist();  // compute distance
		Boolean Member(AST_INDEX node);
		GenericList *GetNodeList() {return NodeList;};
	};

class GroupTemporalSet:
	 public SinglyLinkedList
	{
	 private:
	 	int size; // # of GT sets
		la_vect LocIterSpace;
		int Nestl, Subs;
		la_matrix H;
	        
		friend class GTSetIter;
	 public:
		GroupTemporalSet(la_vect, int, int, la_matrix );
		~GroupTemporalSet(){};
		void PutintoGTEntry(AST_INDEX, la_vect);
		void PrintOut();
		void FillArray(int*, int);
		void DoAnalysis(int *, int *, int *&);
		int SizeofGTSet(){ return size; }
		void computedist(); 
	};


class GTSetIter:
	  public SinglyLinkedListIterator
         {
          public:
                 GTSetIter(GroupTemporalSet *l):
                   SinglyLinkedListIterator(l)
                     { }
                 GTSetIter(GroupTemporalSet &l):        
		   SinglyLinkedListIterator(l) 
                     { }
                 GroupTemporalEntry* operator () ();      
          };

class GroupSpatialEntry: 
	public SinglyLinkedListEntry
        {
         private:
		GroupTemporalSet *gts; 
                int *consts; // array of the 1st dem. const of each GT leader.
		int *range;   // array of cache line range. 
		int NumGap;
		int TotalBlock;  // # of big blocks accessed by this GS
		int *Gap;
                la_vect leader_v;   // The leader of this spatial group
		AST_INDEX leader_n;
                la_vect trailer_v;   // The trailer of this spatial group
		AST_INDEX trailer_n;
                la_vect load_leader_v;   // The load_leader of this spatial group
		AST_INDEX load_leader_n;
		la_vect LocIterSpace;
		la_vect ZeroSpace;
		la_matrix H;
                int Nestl, Subs;
		GenericList *NodeList;

	 public:
		GroupSpatialEntry(la_vect loc, 
				  int nestl, int sub,
				  la_matrix h,
				  AST_INDEX,
				  la_vect vect);
		~GroupSpatialEntry(){};
		int Marked;   // 1 is marked, 0 is unmarked 
		int ComputePrefetch( la_vect unroll_vect );
		int take( AST_INDEX, la_vect );
		int NodeshasGS(la_vect, la_vect );
		int CheckIntersect(la_vect, la_vect, int);
		int SolutionIsValid(la_matrix, la_vect, la_vect);
		int IsSolutionInLIS(la_matrix, la_vect, int, int, int);
		int SizeofGTSet() { return gts->SizeofGTSet(); }
		void ComputeDistance();
		void FillArray(int* a,int s) {gts->FillArray(a, s);}
		void DoAnalysis();
		la_vect Leader() { return leader_v; };
		la_vect Trailer() { return trailer_v; };
		la_vect LoadLeader() { return load_leader_v; };
		AST_INDEX LeaderNode() { return leader_n; };
		AST_INDEX TrailerNode() { return trailer_n; };
		AST_INDEX LoadLeaderNode() { return load_leader_n; };
		void PrintOut();
		void AddSpatialDependences(AST_INDEX node);
		Boolean Member(AST_INDEX node);
		Boolean HasGroupTemporalReuse(AST_INDEX node);
		GenericList *GetNodeList() {return NodeList;};
	};

class GroupSpatialSet :
	 public SinglyLinkedList
	{
	 private:
		friend class GSSetIter;
		int size;  // # of GS Entry 
		la_vect LocIterSpace;
                char Name[80];
		int Nestl;
		int Subs;
		int IsSelfTemporal;
		int IsSelfSpatial;
		Boolean NegativeStep;
		ComputeBoard *computeboard;
	 	la_matrix H;	
	 public:
		GroupSpatialSet(la_vect, int, int, char*, la_matrix);
		~GroupSpatialSet() {};
		void PutintoGSEntry( AST_INDEX, la_vect );
		void PrintGSSet();
		void PrintH();
		void PrintLIS();
		void FindSelfReuse(); 
		int IsSolutionInLIS(la_matrix, int);
                void ComputeDistance();
		int Strideof(int);   // return stride of any dimension in array
		int SizeofGSSet() { return size; };
		int NullSpaceIsZero(la_matrix, int, int);
		void DoAnalysis();   
		float ComputePrefetch(int, int, int, int);
		float ComputePrefetch(int, int);
		Boolean HasSelfSpatial() {return BOOL(IsSelfSpatial);};
		Boolean HasSelfTemporal() {return BOOL(IsSelfTemporal);};
	};

class GSSetIter:
	 public SinglyLinkedListIterator
	{
	 public:
		GSSetIter(GroupSpatialSet *l):
		 SinglyLinkedListIterator(l)
		 { };
		GSSetIter(GroupSpatialSet &l):
		SinglyLinkedListIterator(l)
		{ };
		GroupSpatialEntry* operator () ();
	};


class DataReuseModelEntry:
	 public SinglyLinkedListEntry
	{
	 private:
		GroupSpatialSet *gsset;
	 public:
		DataReuseModelEntry(UniformlyGeneratedSetsEntry *UGSentry);
		~DataReuseModelEntry(){};
		void PrintOut() { gsset->PrintGSSet(); };
		void DoAnalysis() {gsset->DoAnalysis(); };
		float ComputePrefetch(int, int, int, int);
		float ComputePrefetch(int, int);
		Boolean IsGroupSpatialLeader(AST_INDEX node);
		Boolean IsGroupSpatialLoadLeader(AST_INDEX node);
		Boolean AddSpatialDependences(AST_INDEX node);
		Boolean HasGroupSpatialReuse(AST_INDEX node);
		Boolean HasSelfSpatialReuse(AST_INDEX node);
		Boolean HasGroupTemporalReuse(AST_INDEX node);
		Boolean HasSelfTemporalReuse(AST_INDEX node);
		GroupSpatialSet *GetGSSet() { return gsset; }
	};

class DataReuseModel:
         public SinglyLinkedList
	{
	 private:
		 int size; // # of UGSets
		 friend class DRIter;
	 public:
		DataReuseModel(UniformlyGeneratedSets *UGS);
		~DataReuseModel(){};
		void PrintOut();
		void DoAnalysis(int);
		float ComputePrefetch(int, int, int, int);
		float ComputePrefetch(int, int);
		Boolean IsGroupSpatialLeader(AST_INDEX node);
		Boolean IsGroupSpatialLoadLeader(AST_INDEX node);
		void AddSpatialDependences(AST_INDEX node);
		LocalityType GetNodeReuseType(AST_INDEX node);
		Boolean HasGroupSpatialReuse(AST_INDEX node);
		Boolean HasSelfSpatialReuse(AST_INDEX node);
		Boolean HasGroupTemporalReuse(AST_INDEX node);
		Boolean HasSelfTemporalReuse(AST_INDEX node);
	};

class DRIter:
	 public SinglyLinkedListIterator
	{
	 public:
		DRIter(DataReuseModel *l):
		  SinglyLinkedListIterator(l)
		    { };
		DRIter(DataReuseModel &l):
		  SinglyLinkedListIterator(l)
		    { };
   	        DataReuseModelEntry* operator () ();
 	};	

typedef struct StmtOrderType {
    AST_INDEX Old;
    AST_INDEX New;
    int Found; } StmtOrderInfoType;

#endif
