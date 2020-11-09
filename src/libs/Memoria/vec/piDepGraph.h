#ifndef pi_dep_graph_h
#define pi_dep_graph_h

#include <iostream> 
#include <list> 
#include <stack> 
#include <map>

#include <libs/frontEnd/ast/ast.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

#include <regionNode.h>

using namespace std; 

class PiDependenceGraph 
{ 
    PedInfo ped;
	int V;
    int nodeNum = 0; // index of the last node inserted
	map< RegionNode*,std::list<RegionNode*>* > adjList;
	map< RegionNode*,std::list<RegionNode*>* > revAdjList;

	// A Recursive DFS based function used by SCC() 
	void addEdge(std::map<RegionNode*,std::list<RegionNode*>*>& adjList,RegionNode *v, RegionNode *w); // function to add an edge to graph 
public: 
	PiDependenceGraph(int V,PedInfo ped); // Constructor 
	void addRegionNode(RegionNode *);
	int size() { return V; }
	void buildGraph(int k);
	std::list<RegionNode*> *topSort();
}; 

#endif