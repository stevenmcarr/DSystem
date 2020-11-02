#ifndef dep_graph_h
#define dep_graph_h

#include<iostream> 
#include <list> 
#include <stack> 
#include <map>

#include <libs/frontEnd/ast/ast.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

#define NIL -2 
using namespace std; 

// A class that represents an directed graph 
class DependenceGraph 
{ 
    PedInfo ped;
	int V; // No. of vertices 
    int nodeNum = 0; // index of the last node inserted
    int sccNum = 0;
	std::list<int> *adj; // A dynamic array of adjacency lists 
	std::list<DG_Edge*> *depEdges;
	std::map<AST_INDEX,int> R;
    std::map<AST_INDEX,int> stmt_level;
	std::list<int> *sccs; 
	std::map<int,AST_INDEX> *nodes;

	// A Recursive DFS based function used by SCC() 
	void SCCUtil(int u, int disc[], int low[], 
				stack<int> *st, bool stackMember[]); 
	void addEdge(AST_INDEX v, AST_INDEX w,DG_Edge *edge); // function to add an edge to graph 
public: 
	DependenceGraph(int V,PedInfo ped); // Constructor 
	void SCC(); // build strongly connected components 
    int addNodeToRegion(AST_INDEX v, int level);
	void buildGraph();
}; 

#endif