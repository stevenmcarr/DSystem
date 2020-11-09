// A C++ program to find strongly connected components in a given
// directed graph using Tarjan's algorithm (single DFS)

#include <piDepGraph.h>

#include <libs/Memoria/include/mh.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

PiDependenceGraph::PiDependenceGraph(int V, PedInfo ped)
{
	this->V = V;
	this->ped = ped;
}

void PiDependenceGraph::addRegionNode(RegionNode *R)
{
	std::list<RegionNode*> *rlist = new std::list<RegionNode*>();
	adjList.insert(pair<RegionNode*,std::list<RegionNode*>*>(R,rlist));
	revAdjList.insert(pair<RegionNode*,std::list<RegionNode*>*>(R,rlist));
}

void PiDependenceGraph::addEdge(map<RegionNode*,std::list<RegionNode*>*>& adjList,RegionNode *src, RegionNode *sink)
{
	map< RegionNode*,std::list<RegionNode *>* >::iterator it = adjList.find(src);
	std::list<RegionNode*> *rlist = it->second;
	rlist->push_back(sink);
}

void PiDependenceGraph::buildGraph(int k)
{
	DG_Edge *dg = dg_get_edge_structure(PED_DG(ped));

	for (std::map<RegionNode*, std::list<RegionNode*>* >::iterator it = adjList.begin();
		 it != adjList.end();
		 it++)
	{
		RegionNode *R = it->first;
		std::list<pair < AST_INDEX,int > > *slist = R->getStmts();
		for (std::list< pair<AST_INDEX,int> >::iterator it = slist->begin();
			 it != slist->end();
			 it++) 
		 {
			AST_INDEX stmt = it->first;
			int vector = get_info(ped, stmt, type_levelv);
			int level = it->second;

			for (int i = k; i <= level; i++)
			{

				EDGE_INDEX next_edge;
				for (EDGE_INDEX edge = dg_first_src_stmt(PED_DG(ped), vector, i);
				 	 edge != END_OF_LIST;
				 	 edge = next_edge)
				{
					next_edge = dg_next_src_stmt(PED_DG(ped), edge);
					AST_INDEX sink_stmt = ut_get_stmt(dg[edge].sink);
					if (get_stmt_info_ptr(sink_stmt)->R != get_stmt_info_ptr(stmt)->R)
					{
						addEdge(adjList,get_stmt_info_ptr(stmt)->R, get_stmt_info_ptr(sink_stmt)->R);
						addEdge(revAdjList,get_stmt_info_ptr(sink_stmt)->R, get_stmt_info_ptr(stmt)->R);
					}
				}
			}
		}
	}
}

std::list<RegionNode *> *PiDependenceGraph::topSort() {
	map<RegionNode*,std::list<RegionNode*>*> revAdjCopy(revAdjList);
	std::list<RegionNode*> *topOrder = new std::list<RegionNode*>();

	bool hasUnMarkedNodes = true;
	while (hasUnMarkedNodes) 
	{
		hasUnMarkedNodes = false; 
		for (map<RegionNode*,std::list<RegionNode*>*>::iterator it = revAdjCopy.begin();
			 it != revAdjCopy.end();
			 it++) 
	    {
			RegionNode *R = it->first;
			if (!R->isVisited())
			{
				if (it->second->empty()) /* no incoming edges */
		    	{
					R->markVisited();
					topOrder->push_back(R);
					std::list<RegionNode*> *succs = adjList.find(R)->second;
					for (std::list<RegionNode*>::iterator it2 = succs->begin();
					     it2 != succs->end();
						 it2++)
						revAdjCopy.find(*it2)->second->remove(R);
				} else
				{
					hasUnMarkedNodes = true;
				}
			}
		}
	}

	return topOrder;
}

void PiDependenceGraph::dumpGraph() 
{
	cout << "D Pi\n____\n\n";
	for (map<RegionNode*,std::list<RegionNode *> *>::iterator it = adjList.begin();
		 it != adjList.end();
		 it++) 
	{
		it->first->dumpRegion();
		cout << "\n\tAdjacent To = ";
		for (std::list<RegionNode *>::iterator it2 = it->second->begin();
			 it2 != it->second->end();
			 it2++)
		{
			cout << (*it2)->getRegionNum() << " ";
		}
		cout << "\n\n";
	}
}

void PiDependenceGraph::dumpRevGraph() 
{
	cout << "Reverse D Pi\n____________\n\n";
	for (map<RegionNode*,std::list<RegionNode *> *>::iterator it = revAdjList.begin();
		 it != revAdjList.end();
		 it++) 
	{
		it->first->dumpRegion();
		cout << "\n\tAdjacent From = ";
		for (std::list<RegionNode *>::iterator it2 = it->second->begin();
			 it2 != it->second->end();
			 it2++)
		{
			cout << (*it2)->getRegionNum() << " ";
		}
		cout << "\n\n";
	}
}