#ifndef MinDist_h
#define MinDist_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/support/Lambda/Lambda.h>

#define INFINITY 100000000

class DDG;

class MinDist {

  la_matrix Dist;
  int II;
  int Size;
  DDG *Graph;

  void BuildMatrix();
  void AllPairsShortestPath();
  void Negate();

public:
  
  MinDist(DDG *, int);

  ~MinDist()
    {
      la_matFree(Dist,Size,Size);
    }

  la_vect operator[](int i)
    {
      return (la_vect)Dist[i];
    }


  int GetFinalII()
    {
      return II;
    };

  Boolean IsValid();
  
  void Dump();

};

STATIC(int, CountNodes,(AST_INDEX node, int *size));
STATIC(int, InitializeNodeMapping,(AST_INDEX node, MinDist *D));

#endif
