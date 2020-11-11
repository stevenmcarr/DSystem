#ifndef dep_graph_h
#define dep_graph_h

#include<iostream> 
#include <list> 
#include <stack> 
#include <map>

#include <libs/Memoria/vec/regionNode.h>
#include <libs/Memoria/vec/scc.h>

#include <libs/frontEnd/ast/ast.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

using namespace std; 

// A class that represents an directed graph 
class DependenceGraph 
{ 
    PedInfo ped;
	int V; // No. of vertices 
    int sNum = 0; // index of the last node inserted
    int sccNum = 0;
	map<AST_INDEX, std::list<AST_INDEX>*> adj; // A dynamic array of adjacency lists 
	RegionNode *R;
	std::list<SCC*> sccs; 

	// A Recursive DFS based function used by SCC() 
	void SCCUtil(AST_INDEX u, map<AST_INDEX,int>& disc, map<AST_INDEX,int>& low, 
				stack<AST_INDEX> *st, map<AST_INDEX,bool>& stackMember); 
	void addEdge(AST_INDEX v, AST_INDEX w); // function to add an edge to graph 
	void addNode(AST_INDEX v);
public: 
	DependenceGraph(int V,PedInfo ped); // Constructor 
	void computeSCC(); // build strongly connected components 
    void addNodeToRegion(AST_INDEX v);
	void buildGraph(int k);
	std::list<SCC*>& getSCCS(); 
	int size() { return V; }
	bool isRegionCyclic(RegionNode& R);
	void dumpGraph();
	int getNumSCCs() { return sccNum; }
}; 

#endif
