/* $Id: tarjan.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- tarjan.c
 *
 *        Tarjan interval finder (nested strongly connected regions).
 *        Algorithm from Tarjan, J. of Computer & System Sciences 9, 355-365 
 *        (1974).
 *
 *        Extensions to handle reducible SCR's surrounded by or containing
 *        irreducible ones.
 *
 *        Result is returned as a tree of SCR's, where a parent SCR contains 
 *        its children.  A leaf SCR is a single node, which is not really 
 *        a loop unless the isCyclic flag is set.
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>

#define ROOT_DFNUM 0

typedef struct {
    int wk_vertex;		/* map from DFS# of vertex to CfgNodeId */
    int wk_dfnum;		/* map from CfgNodeId of vertex to DFS# */
    int wk_last;		/* DFS # of vertex's last descendant */
    int wk_header;		/* header of the vertex's interval -- HIGHPT */
    int wk_nextP;		/* next member of P set == reachunder */
    int wk_nextQ;		/* next member of Q set == worklist */
    /*
     *  For UNION-FIND
     */
    int wk_Parent;		/* parent in UNION-FIND forest */
    int wk_Name;		/* name of root in UNION-FIND forest */
    int wk_Count;		/* count of tree in UNION-FIND forest */
    int wk_Root;		/* root for name in UNION-FIND forest */

    Boolean wk_inP;		/* test for membership in P == reachunder */
    Boolean wk_isCyclic;	/* has backedges -- to id singles */
    Boolean wk_reducible;	/* true if cyclic scr is reducible */
} WkInfo;

static WkInfo *wk;	/* temporary array for algorithm */
#define vertex(x)	(wk[x].wk_vertex)
#define dfnum(x)	(wk[x].wk_dfnum)
#define Last(x)		(wk[x].wk_last)
#define header(x)	(wk[x].wk_header)
#define nextP(x)	(wk[x].wk_nextP)
#define nextQ(x)	(wk[x].wk_nextQ)
#define Parent(x)	(wk[x].wk_Parent)
#define Name(x)		(wk[x].wk_Name)
#define Count(x)	(wk[x].wk_Count)
#define Root(x)		(wk[x].wk_Root)
#define inP(x)		(wk[x].wk_inP)
#define isCyclic(x)	(wk[x].wk_isCyclic)
#define reducible(x)	(wk[x].wk_reducible)


STATIC(void, init,(CfgInstance cfg));
STATIC(void, init_arrays,(CfgInstance cfg));
STATIC(void, DFS,(CfgInstance cfg, CfgNodeId v));
STATIC(void, get_tarjans,(CfgInstance cfg));
STATIC(void, build_tarj_tree,(CfgNodeId root));
STATIC(void, prenumber,(int v));
STATIC(void, free_all,(void));
STATIC(int, FIND,(int v));
STATIC(int, do_FIND,(int v));
STATIC(void, UNION,(int i, int j, int k));
STATIC(void, print_tarjans,());

/*
 *  These static variables can only be referenced in local routines,
 *  not in routines to be called from outside.
 */
static TarjTree tarj;		/* temporary pointer to interval tree array */

static int n;		/* next DFS preorder number */
static int last_id;	/* CfgNodeId whose DFS preorder number is n*/
static int g_size;	/* max number of nodes in cfgNodes */

/*
 *  is_backedge(a,b) returns true if a is a descendant of b in DFST
 *
 *  Note that this is in the local structures; corresponds somewhat
 *  to TARJ_contains(tarj,b,a) for the public structures.
 */
#define is_backedge(a,b) ((dfnum(b) <= dfnum(a)) & (dfnum(a) <= Last(b)))

/* 
 *  Return tree of Tarjan intervals for CfgInstance 'cfg' 
 */
void tarj_build(CfgInstance cfg)
{
    CfgNodeId root = cfg->start;

    init(cfg);

    DFS(cfg, root);
    get_tarjans(cfg);
    build_tarj_tree(root);
    free_all();

    cfg->intervals = tarj;

    tarj = (TarjTree) 0;

    tarj_sort(cfg);       /* 3/16/93 RvH: Now always sort intervals */
}





/*
 * -- tarj_sort
 *
 *       Sort the Tarjan Tree in topological ordering
 */
void tarj_sort(CfgInstance cfg)
{
    CfgNodeId* map;   /* topologically sorted CFG map */
    CfgNodeId cfgId, parent; /* cfgId and  Tarj parent (loop header) */
    int i;

    tarj = cfg->intervals;

    if (!tarj)
    {
	tarj_build(cfg);
	tarj = cfg->intervals;
    }

    map = cfg->topMap;

    if (!map) /* need to topological sort CFG */
    {
	/*
	 * Need Tarjan tree fro cfg_top_sort()
	 */
	map = cfg_top_sort(cfg);
    }

    /*
     * Now disconnect all "next" field for all tarj nodes
     */
    for (cfgId = cfg_get_first_node(cfg); cfgId != CFG_NIL;
	 cfgId = cfg_get_next_node(cfg, cfgId))
    {
	TARJ_inners(tarj, cfgId) = CFG_NIL;
	TARJ_next(tarj, cfgId) = CFG_NIL;
    }


    for (i = cfg_get_last_node(cfg); i != CFG_NIL;
	 i = cfg_get_prev_node(cfg, i))
    {
	cfgId = map[i];
	if ( cfgId != CFG_NIL ) 
	{
	    /*
	     * Add new kid
	     */
	    parent = TARJ_outer(tarj, cfgId);
	    if (parent != CFG_NIL)
	    {
		TARJ_next(tarj, cfgId) = TARJ_inners(tarj, parent);
		TARJ_inners(tarj, parent) = cfgId;
	    }
	}
    }
    tarj_renumber(cfg);

} /* end of tarj_sort() */


/*********************************************************************
 *  Returns next cfg node in specified order on the 
 *  Tarjan interval tree.  No recursion and no saved state.
 */
CfgNodeId
cfg_tarj_next(CfgInstance cfg, CfgNodeId cn, TraversalOrder order)
     /*CfgNodeId      cn;         Node last visited */
     /*TraversalOrder order;      from general.h    */
{
  CfgNodeId next_level;
  CfgNodeId next = CFG_NIL;
  Boolean   done = false;

  if (cn == CFG_NIL)            /* Beginning of tree walk ? */
  {
    /* if (!(cfg->topMap))   Assume that intervals are sorted already
     *   tarj_sort(cfg);
     */
    next = cfg->start;          /* Start at root of interval tree */
    done = BOOL(order == PreOrder);       /* PreOrder ==> return root */
  }

  if (!done)
  {
    switch (order)
    {
    case PreOrder:
      if (tarj_is_header(cfg->intervals, cn)) /* Is header ? */
      {
	next = TARJ_inners(cfg->intervals, cn);  /* Return first son */
      }
      else                         /* Find successor */
      {
	next_level = cn;
	do {                       /* Go up until successor found */
	  next  = TARJ_next(cfg->intervals, next_level);
	  if (next == CFG_NIL)
	    next_level = TARJ_outer(cfg->intervals, next_level);
	} while ((next == CFG_NIL) && (next_level != CFG_NIL));
      }
      break;

    case PostOrder:
      if (cn != CFG_NIL)           /* Not beginning of tree walk ? */ 
      {
	next = TARJ_next(cfg->intervals, cn);
	if (next == CFG_NIL) {     /* Last in interval ? */
	  next = TARJ_outer(cfg->intervals, cn);  /* Return header */
	  done = true;
	}
      }
      if (!done) {                 /* Find non-header successor */
	next_level = next;
	do {                       /* Go down until non-header found */
	  next  = next_level;
	  next_level = TARJ_inners(cfg->intervals, next_level);
	} while (next_level != CFG_NIL);
      }
      break;

    default:
      printf("WARNING: cfg_tarj_next(): order %d not supported !",
	     order);
    }
  }

  return next;
} /* end of cfg_tarj_next() */


/* 
 *  Allocate and initialize structures for algorithm, and UNION-FIND
 */
static void init(CfgInstance cfg)
{
    g_size = f_curr_size((Generic) cfg->cfgNodes);
    n = ROOT_DFNUM;

    /*
     *  Local work space
     */
    wk	= (WkInfo *) get_mem(g_size * sizeof(WkInfo),
			     "Tarjan interval work array");

    /*
     *  initialization of interval tree array
     */
    if (cfg->intervals)
	free_mem((void*) cfg->intervals);

    tarj = (TarjTree) get_mem(g_size * sizeof(TarjEntry),
			      "Tarjan interval tree");

    /* 
     *  skip any nodes not in current instance cfg
     */
    init_arrays(cfg);
}

/* 
 *  initialize only nodes in current instance cfg
 */
static void init_arrays(CfgInstance cfg)
{
    int v;
    static TarjEntry t = {
	0,
	TARJ_ACYCLIC,
	CFG_NIL, CFG_NIL, CFG_NIL,
	CFG_NIL, CFG_NIL
    };
    static WkInfo w = {
	CFG_NIL, CFG_NIL, CFG_NIL, /* header = */ ROOT_DFNUM,
	CFG_NIL, CFG_NIL, CFG_NIL, CFG_NIL,
	/* Count = */ 1, CFG_NIL,
	false, false, /* reducible= */ true
    };

    for (v = 0; v < g_size; v++) {
	wk[v] = w;

        if (!(CFG_node(cfg, v)->freed)) {
	    Last(v)	= v;

	    Name(v)	= v;
	    Root(v)	= v;
        }

	tarj[v] = t;
    }
}

/*
 *  Do depth first search on control flow graph to 
 *  initialize vertex[], dfnum[], last[]
 */
static void DFS(CfgInstance cfg, CfgNodeId v)
{
    CfgEdgeId succ;
    int son;

    /* 
     *  skip over numbers corresponding to cfg nodes that have been freed
     */
    while (CFG_node(cfg, n)->freed)
        n++;

    vertex(n) = v;
    dfnum(v)  = n++;

    /*
     *  succ is a CfgEdgeId
     */
    for (succ = CFG_node(cfg, v)->outs; succ != CFG_NIL;
	 succ = CFG_edge(cfg, succ)->outNext)
    {
        son = CFG_edge(cfg, succ)->dest;

        if (dfnum(son) == CFG_NIL) DFS(cfg, son);
    }
    /*
     *  Equivalent to # of descendants -- number of last descendant
     */
    Last(v) = n-1;
}


/* 
 *  In bottom-up traversal of depth-first spanning tree, 
 *  determine nested strongly connected regions and flag irreducible regions.
 *                                                     phh, 4 Mar 91
 */
static void get_tarjans(CfgInstance cfg)
{
    int i;		/* loop */
    CfgNodeId w;	/* vertex names */
    CfgEdgeId pred;	/* loop */
    int firstP, firstQ;	/* set and worklist */

    /*
     *  Following loop should skip root (prenumbered as 0)
     */
    for (i = n - 1; i != ROOT_DFNUM; i--) /* loop c */
        /*
	 *  skip any nodes freed or not reachable
	 */
        if ((vertex(i) != CFG_NIL) && !(CFG_node(cfg, i)->freed)) {
            w = vertex(i);

	    firstP = firstQ = CFG_NIL;

            /*
	     *  Add sources of cycle arcs to P -- and to worklist Q
	     */
            for (pred = CFG_node(cfg, w)->ins; pred != CFG_NIL; 
		 pred = CFG_edge(cfg, pred)->inNext) /* loop d */
	    {
		CfgNodeId u,v;            /* vertex names */
                v = CFG_edge(cfg, pred)->src;

                /*
		 *  ignore predecessors not reachable
		 */
                if ((dfnum(v) != CFG_NIL) && (is_backedge(v,w)))
		{
		    if (v == w)
			/*
			 *  Don't add w to its own P and Q sets
			 */
			isCyclic(w) = true;
		    else
		    {
			/*
			 *  Add FIND(v) to P and Q
			 */
			u = FIND(v);

			if (!inP(u)) {
			    nextP(u) = nextQ(u) = firstP;
			    firstP   = firstQ   = u;
			    inP(u)   = true;
			}
		    }
		}
            }
	    /*
	     *  P nonempty -> w is header of a loop
	     */
	    if (firstP != CFG_NIL) isCyclic(w) = true;


	    while (firstQ != CFG_NIL) {
		int x, y, yy;             /* vertex names */

		x = firstQ;
		firstQ = nextQ(x);      /* remove x from worklist */

		/*
		 *  Now look at non-cycle arcs
		 */
		for (pred = CFG_node(cfg, x)->ins; pred != CFG_NIL;
		     pred = CFG_edge(cfg, pred)->inNext) /* loop e */
		{
		    y = CFG_edge(cfg, pred)->src;

		    /*
		     *  ignore predecessors not reachable
		     */
		    if ((dfnum(y) != CFG_NIL) && (!is_backedge(y,x)))
		    {
			/*
			 *  Add FIND(y) to P and Q
			 */
			yy = FIND(y);

			if (is_backedge(yy,w)) {
			    if ((!inP(yy)) & (yy != w)) {
				nextP(yy) = firstP;
				nextQ(yy) = firstQ;
				firstP = firstQ = yy;
				inP(yy) = true;
			    }
			    /*
			     *  Slight change to published alg'm...
			     *  moved setting of header (HIGHPT)
			     *  from here to after the union.
			     */
			}
			else {
			    /*
			     *  Irreducible region!
			     */
			    reducible(w) = false;
			}
		    }
		}
	    }
	    /*
	     *  now P = P(w) as in Tarjan's paper
	     */
	    while (firstP != CFG_NIL) {
		/*
		 *  First line is a change to published algorithm;
		 *  Want sources of cycle edges to have header w
		 *  and w itself not to have header w.
		 */
		if ((header(firstP) == ROOT_DFNUM) & (firstP != w))
		    header(firstP) = dfnum(w);
		UNION(firstP, w, w);
		inP(firstP) = false;
		firstP = nextP(firstP);
	    }
	}
}

/* 
 *  Traverse df spanning tree, building tree of Tarjan intervals
 */
static void build_tarj_tree(CfgNodeId root)
    /* CfgNodeId root;	entry node of cfg */
{
    int i;           /* loop */
    int w;           /* vertex */
    int outer;       /* header of surrounding loop */

    /*
     *  Let the root of the tree be the root of the instance...
     *
     *  Following loop can skip the root (prenumbered 0)
     */

    for (i = ROOT_DFNUM +1; i < n; i++) 
	/*
	 *  skip any nodes not in current instance cfg
	 */
	if (vertex(i) != CFG_NIL) {
	    w = vertex(i);

	    outer = vertex(header(w));
	    tarj[w].outer  = outer;
	    /*
	     *  tarj[w].inners = CFG_NIL;  % done in init_arrays
	     */
	    if (isCyclic(w)) {
		/*
		 *  Level is deeper than outer if this is a loop
		 *
		 */
		tarj[w].level = tarj[outer].level + 1;

		if (reducible(w)) tarj[w].type = TARJ_INTERVAL;
		else              tarj[w].type = TARJ_IRREDUCIBLE;
	    }
	    else {
		/*
		 *  tarj[w].type  = TARJ_ACYCLIC;	% done in init_arrays
		 */
		tarj[w].level = tarj[outer].level;		
	    }
	    tarj[w].next = tarj[outer].inners;
	    tarj[outer].inners = w;
	     
	}
    n = 0;
    prenumber(root);
}

    
    



static void prenumber(int v)
{
    int inner;

    tarj[v].prenum = ++n;
    last_id = v;
    
    for (inner = tarj[v].inners; inner != CFG_NIL; inner = tarj[inner].next) {
      prenumber(inner);
    }

    /* tarj[v].last = n; // 3/18/93 RvH: switch to CfgNodeId last_id */
    tarj[v].last_id = last_id;
}

void tarj_renumber(CfgInstance cfg)
{
    tarj = cfg->intervals;
    n = 0;
    prenumber(cfg->start);
}

/* 
 *  Free all space used in computation of 
 *  Tarjan interval tree (but not tree itself)
 */
static void free_all()
{
    free_mem((void*) wk);
    wk = (WkInfo *) 0;
}

void tarj_free(CfgInstance cfg)
{
    free_mem((void*)cfg->intervals);
    cfg->intervals = (TarjTree) 0;
}

static char pt_str[75];

static void print_tarjans(int node)
{
    static char *Tarj_Type[] = {"NOTHING", "Acyclic",
				  "Interval", "Irreducible"};
    int kid;

    /*
     *  Indent by three
     */
    if (strlen(pt_str) < (size_t)72) strcat(pt_str, "   ");

    printf("%s%d(%d,%s)\n", 
	   pt_str, node, tarj[node].level,
	   Tarj_Type[(int) (tarj[node].type)]);

    for (kid = tarj[node].inners; (kid != CFG_NIL);
	 kid = tarj[kid].next)
	print_tarjans(kid);

    /*
     *  Unindent
     */
    if (strlen(pt_str) >= (size_t)3) pt_str[strlen(pt_str) -3] = '\0';
}

void tarj_print(TarjTree tarjans, CfgNodeId root)
{
    tarj = tarjans;
    printf("Tarjan interval tree: <node id>(level,type)\n");
    pt_str[0] = '\0';
    print_tarjans(root);
}

/*
 *  UNION-FIND copied from Aho, Hopcroft & Ullman,
 *  _The Design and Analysis of Computer Algorithms_,
 *  Addison Wesley '74
 */
static int FIND(int v)
{
    if (Parent(v) == CFG_NIL)
	return Name(v);

    Parent(v) = do_FIND(Parent(v));

    return Name(Parent(v));
}

static int do_FIND(int v)
{
    if (Parent(v) == CFG_NIL)
	return v;

    Parent(v) = do_FIND(Parent(v));

    return Parent(v);
}

static void UNION(int i, int j, int k)
{
    int large, small;

    if ((i == j) && (j == k)) return;

    if (Count(Root(i)) > Count(Root(j))) {
	large = Root(i);
	small = Root(j);
    }
    else {
	large = Root(j);
	small = Root(i);
    }
    Parent(small) = large;
    Count(large) += Count(small);
    Name(large)   = k;
    Root(k)       = large;
}


/*
 *  Return number of loops exited in traversing cfg edge from src to dest
 *  (0 is normal).
 */
int tarj_exits(TarjTree tarjans, CfgNodeId src, CfgNodeId dest)
{
    CfgNodeId lca = tarj_lca(tarjans, src, dest);
    if (lca == CFG_NIL) return 0;
    return max(0, tarjans[src].level - tarjans[lca].level);
}

/*
 *  Return outermost loop exited in traversing cfg edge from src to dest
 *  (CFG_NIL if no loops exited).
 */
CfgNodeId tarj_loop_exited(TarjTree tarjans, CfgNodeId src,
                           CfgNodeId dest)
{
    CfgNodeId lca = tarj_lca(tarjans, src, dest);
    if (lca == CFG_NIL) return CFG_NIL;

    if (lca == src) return CFG_NIL;

    while (!TARJ_contains(tarjans,tarjans[src].outer,dest))
	src = tarjans[src].outer;
    
    if (tarjans[src].type == TARJ_INTERVAL ||
	tarjans[src].type == TARJ_IRREDUCIBLE) return src;

    return CFG_NIL;
}

/*
 *  Return type of cfg edge from src to dest, one of
 *          TARJ_NORMAL
 *          TARJ_LOOP_ENTRY
 *          TARJ_IRRED_ENTRY
 *          TARJ_ITERATE (back edge)
 */
TarjEdgeType tarj_edge_type(TarjTree tarjans, CfgNodeId src,
                            CfgNodeId dest)
{
    CfgNodeId anc;

    anc = tarj_lca(tarjans,src, dest);
    if (anc == dest) return TARJ_ITERATE;
    else if ((tarjans[tarjans[dest].outer].type == TARJ_IRREDUCIBLE) &
	     (anc != tarjans[dest].outer))
	/*
	 *  Entering irreducible region not through the "header"
	 */
	return TARJ_IRRED_ENTRY;
    else switch (tarjans[dest].type) {
    case TARJ_INTERVAL:
	return TARJ_LOOP_ENTRY;
    case TARJ_IRREDUCIBLE:
	/*
	 *  Entering irreducible region through the "header"
	 */
	return TARJ_IRRED_ENTRY;
    case TARJ_ACYCLIC:
    case TARJ_NOTHING:
    default:
	return TARJ_NORMAL;
    }
}

CfgNodeId tarj_lca(TarjTree tarjans, CfgNodeId a, CfgNodeId b)
{
    if ((a == CFG_NIL) || (b == CFG_NIL)) return CFG_NIL;

    if (TARJ_contains(tarjans,a,b)) return a;

    while ((b != CFG_NIL) && !TARJ_contains(tarjans,b,a))
	b = tarjans[b].outer;

    return b;
}


/*
 *  -- cfg_is_backedge
 *
 *        Return true if the CFG edge passed in is an backedge
 *
 *        Precondition: Tarjan tree must be built already if not
 *                      one will be created
 */
Boolean cfg_is_backedge(CfgInstance cfg, CfgEdgeId edgeId)
{
    TarjEdgeType edgeType;

    if (!(cfg->intervals)) tarj_build(cfg);

    edgeType = tarj_edge_type(cfg->intervals,
			      CFG_edge(cfg, edgeId)->src,
			      CFG_edge(cfg, edgeId)->dest);

    return BOOL(edgeType == TARJ_ITERATE);

} /* end of cfg_is_backedge() */





/*
 * -- cfg_get_forward_fanout
 *
 *         This function returns Number of forward fan out edges
 *
 *         Precondition: Tarjan tree must have been built already
 */
int cfg_get_forward_fanout(CfgInstance cfg, CfgNodeId cfgId)
{
    int fFanOut;  /* fanout for forward edges */
    CfgEdgeId outEdge;

    fFanOut = 0;
    outEdge = CFG_node(cfg, cfgId)->outs;
    
    while( outEdge != CFG_NIL )
    {
	if ( !cfg_is_backedge(cfg, outEdge) )
	    fFanOut++;
	outEdge = CFG_edge(cfg, outEdge)->outNext;
    }

    return( fFanOut );

} /* end of cfg_get_forward_fanout() */






/*
 * -- cfg_get_forward_fanin
 *
 *         This function returns Number of forward fan in edges
 *
 *         Precondition: Tarjan tree must have been built already
 */
int cfg_get_forward_fanin(CfgInstance cfg, CfgNodeId cfgId)
{
    int fFanIn;  /* fanin for forward edges */
    CfgEdgeId inEdge;

    fFanIn = 0;
    inEdge = CFG_node(cfg, cfgId)->ins;
    
    while( inEdge != CFG_NIL )
    {
	if ( !cfg_is_backedge(cfg, inEdge) )
	    fFanIn++;
	inEdge = CFG_edge(cfg, inEdge)->inNext;
    }

    return( fFanIn );

} /* end of cfg_get_forward_fanin() */



TarjTree
cfg_get_intervals(CfgInstance cfg)
{
    if (!(cfg->intervals)) tarj_build(cfg);

    return cfg->intervals;
}

CfgNodeId
tarj_outer(TarjTree tarjans, CfgNodeId id)
{
    return TARJ_outer(tarjans, id);
}

CfgNodeId
tarj_inners(TarjTree tarjans, CfgNodeId id)
{
    return TARJ_inners(tarjans, id);
}

CfgNodeId
tarj_next(TarjTree tarjans, CfgNodeId id)
{
    return TARJ_next(tarjans, id);
}

CfgNodeId
tarj_inners_last(TarjTree tarjans, CfgNodeId id)
{
    return TARJ_last(tarjans, id);
}


Boolean
tarj_is_header(TarjTree tarjans, CfgNodeId id)
{
    return BOOL(tarj_inners(tarjans, id) != CFG_NIL);
}


Boolean
tarj_is_first(TarjTree tarjans, CfgNodeId id)
{
    if (tarj_outer(tarjans, id) == CFG_NIL) return true;

    return BOOL(tarj_inners(tarjans, tarj_outer(tarjans, id)) == id);
}


Boolean
tarj_is_last(TarjTree tarjans, CfgNodeId id)
{
    return BOOL(tarj_next(tarjans, id) == CFG_NIL);
}


int
tarj_level(TarjTree tarjans, CfgNodeId id)
{
    return (int) TARJ_level(tarjans, id);
}

TarjType
tarj_type(TarjTree tarjans, CfgNodeId id)
{
    return TARJ_type(tarjans, id);
}

Boolean
tarj_contains(TarjTree tarjans, CfgNodeId a, CfgNodeId b)
{
    return BOOL(TARJ_contains(tarjans, a, b));
}

