// A C++ program to find strongly connected components in a given
// directed graph using Tarjan's algorithm (single DFS)

#include <libs/Memoria/vec/depGraph.h>
#include <libs/Memoria/vec/scc.h>

#include <libs/Memoria/include/mh.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

DependenceGraph::DependenceGraph(int V, PedInfo ped)
{
	this->V = V;
	this->ped = ped;
	R = new RegionNode();
}

std::list<SCC*>& DependenceGraph::getSCCS() {
	return sccs;
}

void DependenceGraph::addNodeToRegion(AST_INDEX v)
{
	R->addStmt(v);
	get_stmt_info_ptr(v)->R = R;
}

void DependenceGraph::addNode(AST_INDEX v) 
{
	adj.insert(pair<AST_INDEX,std::list<AST_INDEX>*>(v,new std::list<AST_INDEX>));
}

void DependenceGraph::addEdge(AST_INDEX v, AST_INDEX w)
{
	std::list<AST_INDEX> *adjList = adj.find(v)->second;
	adjList->push_back(w);
}

void DependenceGraph::buildGraph(int k)
{
	DG_Edge *dg = dg_get_edge_structure(PED_DG(ped));

	for (std::list<AST_INDEX>::iterator it = R->getStmts().begin();
		 it != R->getStmts().end();
		 it++)
		addNode(*it);

	for (std::list<AST_INDEX>::iterator it = R->getStmts().begin();
		 it != R->getStmts().end();
		 it++)
	{
		AST_INDEX stmt = *it;
		int vector = get_info(ped, stmt, type_levelv);
		int level = get_stmt_info_ptr(stmt)->level;

		for (int i = k; i < level; i++)
		{

			for (EDGE_INDEX edge = dg_first_src_stmt(PED_DG(ped), vector, i);
				 edge != END_OF_LIST;
				 edge = dg_next_src_stmt(PED_DG(ped), edge))
			{
				AST_INDEX sink_stmt = ut_get_stmt(dg[edge].sink);
				if (get_stmt_info_ptr(sink_stmt)->R == get_stmt_info_ptr(stmt)->R)
					addEdge(stmt, sink_stmt);
			}

			
		}
		for (EDGE_INDEX edge = dg_first_src_stmt(PED_DG(ped), vector, LOOP_INDEPENDENT);
			 edge != END_OF_LIST;
			 edge = dg_next_src_stmt(PED_DG(ped), edge)) 
			{
				AST_INDEX sink_stmt = ut_get_stmt(dg[edge].sink);
				if (get_stmt_info_ptr(sink_stmt)->R == get_stmt_info_ptr(stmt)->R)
					addEdge(stmt, sink_stmt);
			}
	}
}

// A recursive function that finds and prints strongly connected
// components using DFS traversal
// u --> The vertex to be visited next
// disc[] --> Stores discovery times of visited vertices
// low[] -- >> earliest visited vertex (the vertex with minimum
//			 discovery time) that can be reached from subtree
//			 rooted with current vertex
// *st -- >> To store all the connected ancestors (could be part
//		 of SCC)
// stackMember[] --> bit/index array for faster check whether
//				 a node is in stack
void DependenceGraph::SCCUtil(AST_INDEX u, map<AST_INDEX,int>& disc, map<AST_INDEX,int>& low, stack<AST_INDEX> *st,
							  map<AST_INDEX,bool>& stackMember)
{
	// A static variable is used for simplicity, we can avoid use
	// of static variable by passing a pointer.
	static int time = 0;

	// Initialize discovery time and low value
	disc[u] = ++time;
	low[u] = time;
	st->push(u);
	stackMember[u] = true;

	// Go through all vertices adjacent to this
	std::list<AST_INDEX> *adjList = adj[u];
	for (std::list<AST_INDEX>::iterator i = adjList->begin(); 
		 i != adjList->end(); 
		 ++i)
	{
		AST_INDEX v = *i; // v is current adjacent of 'u'

		// If v is not visited yet, then recur for it
		if (disc[v] == -1)
		{
			SCCUtil(v, disc, low, st, stackMember);

			// Check if the subtree rooted with 'v' has a
			// connection to one of the ancestors of 'u'
			// Case 1 (per above discussion on Disc and Low value)
			low[u] = min(low[u], low[v]);
		}

		// Update low value of 'u' only of 'v' is still in stack
		// (i.e. it's a back edge, not cross edge).
		// Case 2 (per above discussion on Disc and Low value)
		else if (stackMember[v] == true)
			low[u] = min(low[u], disc[v]);
	}

	// head node found, pop the stack and print an SCC
	AST_INDEX w = 0; // To store stack extracted vertices
	if (low[u] == disc[u])
	{
		cout << "SCC: ";
		SCC* scc = new SCC();
		while (st->top() != u)
		{
			w = (AST_INDEX)st->top();
			scc->addNode(w);
			cout << w << " ";
			stackMember[w] = false;
			st->pop();
		}
		w = (AST_INDEX)st->top();
		scc->addNode(w);
		sccs.push_back(scc);
		sccNum++;
		cout << w << "\n";
		stackMember[w] = false;
		st->pop();
	}
}

// The function to do DFS traversal. It uses SCCUtil()
void DependenceGraph::computeSCC()
{
	map<AST_INDEX,int> disc;
	map<AST_INDEX,int> low;
	map<AST_INDEX,bool> stackMember;
	stack<AST_INDEX> *st = new stack<AST_INDEX>();

	// Initialize disc and low, and stackMember arrays
	for (map<AST_INDEX,std::list<AST_INDEX>*>::iterator it = adj.begin();
		 it != adj.end();
		 it++)
	{
		AST_INDEX i = it->first;
		disc[i] = NIL;
		low[i] = NIL;
		stackMember[i] = false;
	}

	// Call the recursive helper function to find strongly
	// connected components in DFS tree with vertex 'i'
	for (map<AST_INDEX,std::list<AST_INDEX>*>::iterator it = adj.begin();
		 it != adj.end();
		 it++) 
	{
        AST_INDEX i = it->first;
		if (disc[i] == NIL)
			SCCUtil(i, disc, low, st, stackMember);
	}
}

bool DependenceGraph::isRegionCyclic(RegionNode& R) {
	if (R.getNumStmts() > 1)
		return true;
	else
	{
		bool cyclic = false;
		AST_INDEX v = R.getStmts().front();
		std::list<AST_INDEX>* adjList = adj[v];
		for (std::list<AST_INDEX>::iterator it = adjList->begin();
			 cyclic && it != adjList->end();
			 it++)
			 if (v == *it)
			 	cyclic = true;
		return cyclic;
	}
	
}

void DependenceGraph::dumpGraph() 
{
	cout << "Dependence Graph\n________________\n\n";
	cout << "\tRegion " << R->getRegionNum();
	for (map<AST_INDEX,std::list<AST_INDEX> *>::iterator it = adj.begin();
		 it != adj.end();
		 it++) 
	{
		cout << "\n\tStatement " << get_stmt_info_ptr(it->first)->stmt_num;
		cout << "\n\tAdjacent To = ";
		for (std::list<AST_INDEX>::iterator it2 = it->second->begin();
			 it2 != it->second->end();
			 it2++)
		{
			cout << get_stmt_info_ptr(*it2)->stmt_num << " ";
		}
		cout << "\n\n";
	}
}
