#ifndef UniformlyGeneratedSets_h
#define UniformlyGeneratedSets_h

#include <string.h>
#include <general.h>
#include <mh_ast.h>
#ifndef sllist_h
#include <misc/sllist.h>
#endif
#include <fort/treeutil.h>

#include <IntegerList.h>
#include <fort/AstIter.h>
#include <Lambda/Lambda.h>

#define MINVAL -1.0E+99
#define EPSILON 1.0E-10
#ifndef MININT
#define MININT	-2147483648
#endif


class UniformlyGeneratedSetsEntry : public IntegerList {

  int NestingLevel,Subscripts;
  char name[80];
  la_matrix H;
  la_vect ZeroSpace, LocalizedIterationSpace;
  Boolean Uniform;
  int Solve(la_matrix B, la_vect c, int m, int n, la_matrix *S, int *num);
  Boolean DetermineIfReuseExists(la_matrix A,la_vect C1, la_vect C2);
  Boolean SolutionIsValid(la_matrix A,la_vect x,la_vect b);
  Boolean CheckIntersect(la_vect X1,la_vect X2,int n);
  Boolean IsSolutionInLIS(la_matrix X,la_vect r_p,int row,int column,
			  Boolean TestAll);
  Boolean NullSpaceIsZero(la_matrix X,int row,int column);

public:

  UniformlyGeneratedSetsEntry(char *EntryName,la_matrix nodeH, int level,int subs,
			      la_vect LIS, Boolean uniform) : IntegerList()
  {
   int i,j;

     (void)strcpy(name,EntryName);
     NestingLevel = level;
     Subscripts = subs;
     Uniform = uniform;
     H = la_matNew(Subscripts,NestingLevel);
     la_matCopy(nodeH,H,Subscripts,NestingLevel);
     ZeroSpace = la_vecNew(Subscripts);
     la_vecClear(ZeroSpace,Subscripts);
     LocalizedIterationSpace = la_vecNew(NestingLevel);
     if (LIS == NULL)
       {
	la_vecClear(LocalizedIterationSpace, NestingLevel-1);
	LocalizedIterationSpace[NestingLevel-1] = 1;
       }
     else
       la_vecCopy(LIS,LocalizedIterationSpace,NestingLevel);
  };

  ~UniformlyGeneratedSetsEntry()
    {
     int i;

       la_matFree(H,Subscripts,NestingLevel);
       la_vecFree(ZeroSpace);
       la_vecFree(LocalizedIterationSpace);
    }

  void GetConstants(AST_INDEX node1,la_vect C);

  void SetLocalizedIterationSpace(la_vect LIS)
    {
     int i;

       la_vecCopy(LIS,LocalizedIterationSpace,NestingLevel);
    }

  void PrintH()
  {
    int i,j;
    
      for (i = 0; i < Subscripts; i++)
	{
	  for (j = 0; j < NestingLevel; j++)
	    printf("\t%d",H[i][j]);
	  printf("\n");
	}
  }

  void PrintLIS()
  {
    int i;
    
      for (i = 0; i< NestingLevel; i++)
	printf("\t%d",LocalizedIterationSpace[i]);
      printf("\n");
  }

  Boolean IsVectorSpaceInLIS(la_vect NS,la_vect Sol);
  Boolean SingleNodeHasGroupSpatialReuse(AST_INDEX node1);
  Boolean SingleNodeHasGroupTemporalReuse(AST_INDEX node1);
  Boolean NodesHaveGroupSpatialReuse(AST_INDEX node1, AST_INDEX node2);
  Boolean NodesHaveGroupTemporalReuse(AST_INDEX node1, AST_INDEX node2);
  Boolean SingleNodeHasSelfTemporalReuse();
  Boolean SingleNodeHasSelfSpatialReuse();
  Boolean SameUniformlyGeneratedSet(AST_INDEX node,la_matrix nodeH);
  void InterchangeRows(int *IV);
  la_vect getLocal() { return LocalizedIterationSpace;};
  int getNestl() { return NestingLevel;};
  int getSubs() { return Subscripts;};
  char* getName() { return name; };
  la_matrix getH() { return H;};
  void PrintOut();
 };


class UGSEntryIterator : public IntegerListIter {

  public:
    UGSEntryIterator(UniformlyGeneratedSetsEntry& UGSEntry) : 
      IntegerListIter(UGSEntry)
      {
      }; 
    UGSEntryIterator(UniformlyGeneratedSetsEntry* UGSEntry) : 
      IntegerListIter(UGSEntry)
      {
      };

   AST_INDEX operator() ();

};



class UniformlyGeneratedSets : public IntegerList {
  int NestingLevel;
  char **IndexVars;
  int *LocalizedIterationSpace;

  UniformlyGeneratedSetsEntry *GetUniformlyGeneratedSet(AST_INDEX node);
  UniformlyGeneratedSetsEntry *GetUniformlyGeneratedSet(AST_INDEX node, la_matrix NodeH);
  void Append(la_matrix nodeH, AST_INDEX node, int NumSubs, Boolean uniform );
  int GetIndex(char *ivar);
  void ComputeH(AST_INDEX node,la_matrix nodeH,Boolean *uniform,AST_INDEX expr,
		int SubPos);
  void GetH(AST_INDEX node,la_matrix nodeH,Boolean *uniform);


public:
  UniformlyGeneratedSets(AST_INDEX loop,int NL,char **IV,la_vect LIS = NULL) ;
  ~UniformlyGeneratedSets()
    { delete IndexVars;
      la_vecFree(LocalizedIterationSpace);
    };

  void AddNode(AST_INDEX node);
  Boolean NodeHasGroupSpatialReuse(AST_INDEX node);
  Boolean NodeHasGroupTemporalReuse(AST_INDEX node);
  Boolean NodeHasSelfTemporalReuse(AST_INDEX node);
  Boolean NodeHasSelfSpatialReuse(AST_INDEX node);

  void SetLocalizedIterationSpace(la_vect LIS)
    {
     int i;
     UniformlyGeneratedSetsEntry *UGSEntry;

       la_vecCopy(LIS,LocalizedIterationSpace,NestingLevel);
       for (IntegerListIter UGSIter(*this);
	    UGSEntry = (UniformlyGeneratedSetsEntry*)UGSIter();)
         UGSEntry->SetLocalizedIterationSpace(LIS);
    }
 };

class UGSIterator : public IntegerListIter {

  public:
    UGSIterator(UniformlyGeneratedSets& UGS) : IntegerListIter(UGS)
      {
      };
    UGSIterator(UniformlyGeneratedSets* UGS) : IntegerListIter(UGS)
      {
      };

   UniformlyGeneratedSetsEntry* operator() ();


};
#endif
