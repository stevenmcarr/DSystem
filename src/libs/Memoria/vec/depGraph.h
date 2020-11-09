#ifndef dep_graph_h
#define dep_graph_h

#include<iostream> 
#include <list> 
#include <stack> 
#include <map>

#include <libs/frontEnd/ast/ast.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

#include <regionNode.h>

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
	std::list< AST_INDEX > *sccs; 
	std::map<AST_INDEX,int> stmtNums;

	// A Recursive DFS based function used by SCC() 
	void SCCUtil(AST_INDEX u, map<AST_INDEX,int> disc, map<AST_INDEX,int> low, 
				stack<AST_INDEX> *st, map<AST_INDEX,bool> stackMember); 
	void addEdge(AST_INDEX v, AST_INDEX w,DG_Edge *edge); // function to add an edge to graph 
public: 
	DependenceGraph(int V,PedInfo ped); // Constructor 
	void SCC(); // build strongly connected components 
    int addNodeToRegion(AST_INDEX v, int level);
	void buildGraph(int k);
	std::list<AST_INDEX> *getSCCS() {return sccs;}
	int size() { return V; }
	bool isSCCCyclic(int sccNum);
}; 

#endif