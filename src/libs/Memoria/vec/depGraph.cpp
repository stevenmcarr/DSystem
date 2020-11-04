// A C++ program to find strongly connected components in a given
// directed graph using Tarjan's algorithm (single DFS)

#include <depGraph.h>

#include <libs/Memoria/include/mh.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

DependenceGraph::DependenceGraph(int V, PedInfo ped)
{
	this->V = V;
	this->ped = ped;
	adj = new std::list<int>[V];
	depEdges = new std::list<DG_Edge *>[V];
	sccs = new std::list<AST_INDEX>[V];
	R = new RegionNode();
}

int DependenceGraph::addNodeToRegion(AST_INDEX v, int level)
{
	R->addStmt(v,level);
	int v_index = get_stmt_info_ptr(v)->stmt_num;
	nodes.insert(pair<int,AST_INDEX>(v_index,v));
	get_stmt_info_ptr(v)->R = R;
}

void DependenceGraph::addEdge(AST_INDEX v, AST_INDEX w, DG_Edge *edge)
{
	int v_index = get_stmt_info_ptr(v)->stmt_num;
	int w_index = get_stmt_info_ptr(w)->stmt_num;

	adj[v_index].push_back(w_index);
	depEdges[v_index].push_back(edge);
}

void DependenceGraph::buildGraph(int k)
{
	DG_Edge *dg = dg_get_edge_structure(PED_DG(ped));

	for (std::list< pair<AST_INDEX, int> >::iterator it = R->getStmts()->begin();
		 it != R->getStmts()->end();
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
				if (get_stmt_info_ptr(dg[edge].sink)->R == this)
					addEdge(stmt, dg[edge].sink, &dg[edge]);
				else
					interRegionEdges.push_back(&dg[edge]);
			}
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
void DependenceGraph::SCCUtil(int u, int disc[], int low[], stack<int> *st,
							  bool stackMember[])
{
	// A static variable is used for simplicity, we can avoid use
	// of static variable by passing a pointer.
	static int time = 0;

	// Initialize discovery time and low value
	disc[u] = low[u] = ++time;
	st->push(u);
	stackMember[u] = true;

	// Go through all vertices adjacent to this
	std::list<int>::iterator i;
	for (i = adj[u].begin(); i != adj[u].end(); ++i)
	{
		int v = *i; // v is current adjacent of 'u'

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
	int w = 0; // To store stack extracted vertices
	if (low[u] == disc[u])
	{
		while (st->top() != u)
		{
			w = (int)st->top();
			sccs[sccNum].push_back(nodes.find(w)->second);
			//cout << w << " ";
			stackMember[w] = false;
			st->pop();
		}
		w = (int)st->top();
		sccs[sccNum++].push_back(nodes.find(w)->second);
		//cout << w << "\n";
		stackMember[w] = false;
		st->pop();
	}
}

// The function to do DFS traversal. It uses SCCUtil()
void DependenceGraph::SCC()
{
	int *disc = new int[V];
	int *low = new int[V];
	bool *stackMember = new bool[V];
	stack<int> *st = new stack<int>();

	// Initialize disc and low, and stackMember arrays
	for (int i = 0; i < V; i++)
	{
		disc[i] = NIL;
		low[i] = NIL;
		stackMember[i] = false;
	}

	// Call the recursive helper function to find strongly
	// connected components in DFS tree with vertex 'i'
	for (int i = 0; i < V; i++)
		if (disc[i] == NIL)
			SCCUtil(i, disc, low, st, stackMember);
}
