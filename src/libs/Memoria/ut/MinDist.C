#include <iostream.h>
#include <assert.h>
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/ut/MinDist.h>
#include <libs/Memoria/ut/DDG.h>
#include <libs/support/Lambda/Lambda.h>

// access a one-d array with two subscripts. 


MinDist::MinDist(DDG *ddg,
		 int MinII)

{

  Graph = ddg;
  II = MinII - 1;
  Size = Graph->NumberOfNodes();
  Dist = la_matNew(Size,Size); 

  do
  {
    II++;
    BuildMatrix();
    AllPairsShortestPath();
  } while (NOT(IsValid()));

  Negate();
  
}

void MinDist::BuildMatrix()

{
  int i,j;
  DDGNode *ddgNode;
  DDGEdge *ddgEdge;
  int edgeDist;

  // Negate all values when building matrix

  for (i = 0; i < Size; i++)
    for (j = 0; j < Size; j++)
      if (i == j)
	Dist[i][j] = 0;
      else
	Dist[i][j] = INFINITY;

  for (DDGNodeIterator NodeIter(Graph);
       ddgNode = NodeIter.Current();
       ++NodeIter)
    for (DDGEdgeIterator EdgeIter(ddgNode,DirectedEdgeOut);
	 ddgEdge = EdgeIter.Current();
	 ++EdgeIter)
    {
      edgeDist = -(ddgEdge->GetLatency() - ddgEdge->GetOmega() * II);

      if (edgeDist <
	  Dist[ddgNode->GetId()][ddgEdge->GetSink()->GetId()])
	if (ddgNode == ddgEdge->GetSink() && edgeDist >= 0)
	  Dist[ddgNode->GetId()][ddgEdge->GetSink()->GetId()] = 0;
	else
	  Dist[ddgNode->GetId()][ddgEdge->GetSink()->GetId()] = edgeDist;
    }

}


void MinDist::AllPairsShortestPath()
{  
  for (int k=0; k < Size; k++)
    for (int i = 0; i < Size; i++)
      for (int j = 0; j < Size; j++)
	if (Dist[i][k] != INFINITY &&
	    Dist[k][j] != INFINITY)
	  Dist[i][j] = MIN(Dist[i][j],Dist[i][k] + Dist[k][j]);
}
  
      
Boolean MinDist::IsValid()
{
  Boolean Valid = true;

  for (int i = 0; i < Size && Valid; i++)
    Valid = (Dist[i][i] >= 0);
     
  return Valid;
}

void MinDist::Negate()
{
  for (int i = 0; i < Size; i++)
    for (int j = 0; j < Size; j++)
      Dist[i][j] = -Dist[i][j];
}


void MinDist::Dump()
{
  for (int i = 0; i < Size; i++)
  {
    for (int j = 0; j < Size;j++)
    {
      cout.width(5);
      cout.setf(ios::left,ios::adjustfield);
      if (Dist[i][j] == -INFINITY)
	cout << "-inf";
      else if (Dist[i][j] == INFINITY)
	cout << "inf";
      else
	cout << Dist[i][j];
    }
    cout << endl;
  }
}
