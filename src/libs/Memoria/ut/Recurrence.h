#ifndef Recurrence_h
#define Recurrence_h

#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/ASTToIntMap.h>

#define INFINITY 100000000

class Recurrence {

  int NumberOfNodes;
  AST_INDEX *Nodes;
  AST_INDEX loop;
  PedInfo ped;
  int *T;
  int level;
  ASTToIntMap *ASTMap;

  void ComputeT();

public:
  
  Recurrence (AST_INDEX Loop,PedInfo ped,int level);
  ~Recurrence()
    {
      int i;
      
      delete ASTMap;
      delete Nodes;
      delete T;
    }

  Boolean IsReferenceOnRecurrence(AST_INDEX node);
  void PutNode(AST_INDEX node) {Nodes[NumberOfNodes] = node;}
  int GetNumberOfNodes() {return NumberOfNodes;}
  void IncrementNumberOfNodes() {++NumberOfNodes;}
  ASTToIntMap *GetASTMap() {return ASTMap;}
   
};

STATIC(int, CountNodes,(AST_INDEX node, int *size));
STATIC(int, InitializeNodeMapping,(AST_INDEX node, Recurrence *R));

#endif
