#ifndef Recurrence_h
#define Recurrence_h

#include <libs/Memoria/include/mh.h>

#define INFINITY 100000000

class Recurrence {

  int NumberOfStmts;
  AST_INDEX *Stmts;
  AST_INDEX loop;
  PedInfo ped;
  int *T;
  int level;

  void ComputeT();

public:
  
  Recurrence (AST_INDEX Loop,PedInfo ped,int level);
  ~Recurrence()
    {
      int i;
      
      delete Stmts;
      delete T;
    }

  Boolean IsReferenceOnRecurrence(AST_INDEX ref);
  void PutStmt(AST_INDEX stmt) {Stmts[NumberOfStmts] = stmt;}
  int GetNumberOfStmts() {return NumberOfStmts;}
  void IncrementNumberOfStmts() {++NumberOfStmts;}
   
};

STATIC(int, CountStatements,(AST_INDEX stmt,int level, int *size));
STATIC(int, InitializeStmtMapping,(AST_INDEX stmt,int level, Recurrence *R));

#endif
