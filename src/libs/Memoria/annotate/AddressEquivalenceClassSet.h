#ifndef AddressEquivalenceClassSet_h
#define AddressEquivalenceClassSet_h


#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/UniformlyGeneratedSets.h>
#include <libs/Memoria/include/la.h>
#include <libs/Memoria/include/ASTToIntMap.h>
#include <libs/Memoria/include/GenericList.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>

class AddressEquivalenceClass;

class AddressEquivalenceClassSet : public GenericList
{
  ASTToIntMap *Offsets;
  int NestingLevel;
  char **IndexVars;
  int Size;

  void GetH(AST_INDEX node,
	    la_matrix nodeH,
	    Boolean *uniform);
  void ComputeH(AST_INDEX node,la_matrix nodeH,
		Boolean *uniform, AST_INDEX expr,
		int SubPos);
  AddressEquivalenceClass *Append(la_matrix nodeH, AST_INDEX node,
	      int NumSubs, Boolean uniform);
  AddressEquivalenceClass *GetAddressEquivalenceClass(AST_INDEX node,
						       la_matrix nodeH);
  void ComputeAddressOffsets(void);
  int GetIndex(char *ivar);

public:

  AddressEquivalenceClassSet(AST_INDEX loop,int NL,char **IV);

  ~AddressEquivalenceClassSet(void)
    { 
      Offsets->Destroy();
      delete Offsets;
      delete IndexVars;
    }

  AST_INDEX GetLeader(AST_INDEX node);
  AST_INDEX GetFirstInLoop(AST_INDEX node);
  int GetOffset(AST_INDEX n);
  void AddNode(AST_INDEX node);
  void AddNode(Directive *Dir);
  int GetSize() {return Size;};

};


class AddressEquivSetIterator : public GenericListIter {

  public:
    AddressEquivSetIterator(AddressEquivalenceClassSet& AECS) : GenericListIter(AECS)
      {
      };
    AddressEquivSetIterator(AddressEquivalenceClassSet* AECS) : GenericListIter(AECS)
      {
      };

    AddressEquivalenceClass* operator() ();


};


class AddressEquivalenceClass : public GenericList
{
  int NestingLevel,Subscripts;
  char name[80];
  la_matrix H;
  Boolean Uniform;
  AST_INDEX Leader;
  AST_INDEX FirstInLoop;
  la_vect C_L;
  Boolean LeaderIsADirective;
  Directive *LeaderDirective;
  Boolean FirstInLoopIsADirective;
  Directive *FirstInLoopDirective;


  Boolean NewReferenceIsEarlier(AST_INDEX Old,Directive *Dir,Boolean);
  Boolean NewReferenceIsEarlier(Directive *Dir,AST_INDEX New,Boolean);
  Boolean NewReferenceIsEarlier(Directive *LeaderDir,Directive *Dir,Boolean);
  Boolean NewReferenceIsEarlier(AST_INDEX Old,AST_INDEX New,Boolean);
  int CompareVectors(la_vect LeaderVect,la_vect NewVect);

public:

  AddressEquivalenceClass(char *EntryName,la_matrix nodeH, int level,int subs,
			  Boolean uniform);
  ~AddressEquivalenceClass()
   { 
      la_matFree(H,Subscripts,NestingLevel);
      la_vecFree(C_L);
   }
  
  void AddEntry(AST_INDEX n) 
    { (*this) += (Generic)n; }

  AST_INDEX GetLeader() {return Leader;}
  AST_INDEX GetFirstInLoop() {return FirstInLoop;}

  void CheckLeader(AST_INDEX n);
  void CheckLeader(Directive *Dir);

  void CheckFirstInLoopBody(AST_INDEX n);
  void CheckFirstInLoopBody(Directive *Dir);

  Boolean SameEquivalenceClass(AST_INDEX node,
			      la_matrix nodeH);
  void GetConstants(AST_INDEX node1,la_vect C);
  int GetSubscripts() {return Subscripts;}
};

class AddressClassIterator : public GenericListIter {

  public:
    AddressClassIterator(AddressEquivalenceClass& AEC) : 
      GenericListIter(AEC)
      {
      }; 
    AddressClassIterator(AddressEquivalenceClass* AEC) : 
      GenericListIter(AEC)
      {
      };

   AST_INDEX operator() ();

};

#endif
