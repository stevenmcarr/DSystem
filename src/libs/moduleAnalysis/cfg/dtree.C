/* $Id: dtree.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- dtree.c
 *
 *          "A Fast Algorithm for Finding Dominators in a Flowgraph"
 *           Lengauer and Tarjan, July 1979
 *
 *           This code relies on CFG_NIL being -1.
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/dtree.h>
#include <libs/moduleAnalysis/cfg/cfg_utils.h>

STATIC(void, init,(CfgInstance cfg));
STATIC(void, init_arrays,(CfgInstance cfg));
STATIC(void, DFS,(CfgInstance cfg, CfgNodeId v));
STATIC(void, get_sdoms,(CfgInstance cfg));
STATIC(void, build_dt,(CfgNodeId root));
STATIC(void, LINK,(int v, int w));
STATIC(int, EVAL,(int v));
STATIC(void, COMPRESS,(int v));
STATIC(void, add_to_bucket,(int elem, int bckt));
STATIC(int, Delete,(int bckt));
STATIC(void, free_all,(void));
STATIC(void, prenumber,(int v));
STATIC(void, add_dt_elem,(int elem));

static int	*parent		= (int *) 0;
static int	*semi		= (int *) 0;
static int	*vertex		= (int *) 0;
static int	*dom		= (int *) 0;
static int	*bucket		= (int *) 0;
static int	*ancestor	= (int *) 0;
static int	*label		= (int *) 0;
static int	*size		= (int *) 0;
static int	*child		= (int *) 0;

typedef struct {
    int	node;
    int next;
} BucketType;

static BucketType *buckets	= (BucketType *) 0;

/*
 *  These static variables can only be referenced in local routines,
 *  not in routines to be called from outside.
 */
static DomTree dt = (DomTree)0;	/* temporary pointer to dominator tree array */

static int n = 0;		/* next DFS preorder number */
static int g_size = 0;		/* number of nodes in graph */

static Boolean go_forward;	/* if true, build predom tree, else postdom */

/* 
 *  Return dominator tree for CfgInstance 'cfg'
 */
void dom_build(CfgInstance cfg, Boolean forward_direction)
{
    CfgNodeId root;

    go_forward = forward_direction;

    init(cfg);

    root = go_forward ? cfg->start : cfg->end;

    DFS(cfg, root);

    get_sdoms(cfg);
    build_dt(root);

    free_all();
    /*
     *  Safe to return this value because next call to build_domntr_tree
     *  will (via init) allocate new array and clobber this pointer
     */
    if (go_forward)
	cfg->predom = dt;
    else
	cfg->postdom = dt;
}

/* allocate and initialize structures for algorithm, and LINK and EVAL */
static void init(CfgInstance cfg)
{
    /*
     *  Names in range 0..(#nodes -1)
     *  This way -1 can be null (0 in original paper -- phh)
     */
    g_size = f_curr_size((Generic) cfg->cfgNodes);
    n = 0;

    /*
     *  The array will run from 0..(g_size -1) with (g_size) elements.
     */
    if (parent || semi || vertex || dom ||
	bucket || ancestor || label || size || child || buckets)
    {
	fprintf(stderr, "Storage leak in dtree.c\n");
    }
    parent	= (int *)get_mem((g_size +1) * sizeof(int), "dom parent[]");
    semi	= (int *)get_mem((g_size +1) * sizeof(int), "dom semi[]");
    vertex	= (int *)get_mem((g_size +1) * sizeof(int), "dom vertex[]");
    dom		= (int *)get_mem((g_size +1) * sizeof(int), "dom dom[]");
    bucket	= (int *)get_mem((g_size +1) * sizeof(int), "dom bucket[]");
    ancestor	= (int *)get_mem((g_size +1) * sizeof(int), "dom ancestor[]");
    label	= (int *)get_mem((g_size +1) * sizeof(int), "dom label[]");
    size	= (int *)get_mem((g_size +1) * sizeof(int), "dom size[]");
    child	= (int *)get_mem((g_size +1) * sizeof(int), "dom child[]");

    /*
     *  Bucket space -- size is flexible
     */
    buckets = (BucketType *) f_alloc(g_size/2, sizeof(BucketType),
				     "dom_build buckets",
				     (f_init_callback) 0);

    /* 
     *  Initialization for dominator tree... indices from 0..n-1
     */
    dt = (DomTree) get_mem(g_size * sizeof(DomEntry), "Dominator tree");

    if (go_forward)
    {
	if (cfg->predom) free_mem((void*) cfg->predom);
	cfg->predom = dt;
    }
    else
    {
	if (cfg->postdom) free_mem((void*) cfg->postdom);
	cfg->postdom = dt;
    }

    /* skip any nodes not in current CfgInstance cfg */
    init_arrays(cfg);
}

/* initialize only nodes in current CfgInstance cfg */
static void init_arrays(CfgInstance cfg)
{
    int v;
    static DomEntry d = {CFG_NIL,CFG_NIL,CFG_NIL,CFG_NIL,CFG_NIL};

    int_set(parent,	(g_size +1), CFG_NIL); parent++;
    int_set(semi,	(g_size +1), CFG_NIL); semi++;
    int_set(vertex,	(g_size +1), CFG_NIL); vertex++;
    int_set(dom,	(g_size +1), CFG_NIL); dom++;
    int_set(bucket,	(g_size +1), CFG_NIL); bucket++;
    int_set(ancestor,	(g_size +1), CFG_NIL); ancestor++;
    int_set(child,	(g_size +1), CFG_NIL); child++;
    *label  = CFG_NIL;	label++;
    *size   = 0;		size++;

    for (v = 0; v < g_size; v++) {
	if (!(CFG_node(cfg, v)->freed)) {
            label[v]	= v;
            size[v]	= 1;
        }
	else {
            label[v]	= CFG_NIL;
            size[v]	= 0;
	}
	/*
	 *  Using xput on an xarray guarantees that all the elements
	 *  we want will be there.
	 */
	dt[v] = d;
    }
}

/* 
 *  Do depth first search on control flow graph 
 *  to initialize vertex[], semi[], and parent[]
 */
static void DFS(CfgInstance cfg, CfgNodeId v)
{
    CfgEdgeId succ;
    CfgNodeId kid;

    /*
     *  skip over numbers corresponding to
     *  freed CFG nodes
     */
    while (CFG_node(cfg, n)->freed)
        n++;

    vertex[n] = v;
    semi[v]   = n++;

    /*
     *  succ is a cfg edge
     */
    succ = go_forward ? CFG_node(cfg, v)->outs : CFG_node(cfg, v)->ins;
    
    for (; succ != CFG_NIL;
	 succ = go_forward ? CFG_edge(cfg, succ)->outNext
	 : CFG_edge(cfg, succ)->inNext)
    {
	kid = go_forward ? CFG_edge(cfg, succ)->dest
	    : CFG_edge(cfg, succ)->src;

	if (semi[kid] == CFG_NIL) {
	    parent[kid] = v;
	    DFS(cfg, kid);
	}
    }
}


/*
 *  In bottom-up traversal of depth-first spanning tree,
 *  compute semi doms and implicitly define immediate doms
 */
static void get_sdoms(CfgInstance cfg)
{
    int i;		/* loop */
    int u, v, w;	/* vertex names */
    CfgEdgeId pred;	/* loop */

    /*
     *  Following loop should skip the root (prenumbered as 0)
     */
    for (i = n-1; i; i--)
        /* skip any nodes not in current CfgInstance cfg */
        if ((vertex[i] != CFG_NIL) && !(CFG_node(cfg, i)->freed)) {
            w = vertex[i];

            /* step 2 - compute semi dominators */
	    pred = go_forward ? CFG_node(cfg, w)->ins : CFG_node(cfg, w)->outs;
	    for (; pred != CFG_NIL;
		 pred = go_forward
		 ? CFG_edge(cfg, pred)->inNext
		 : CFG_edge(cfg, pred)->outNext)
	    {
		v = go_forward ? CFG_edge(cfg, pred)->src
		    : CFG_edge(cfg, pred)->dest;

		/*
		 *  Make sure we don't process an unreachable prececessor here
		 *  (original does this by making special preds list).
		 */
		if (semi[v] != CFG_NIL) 
		{
		    u = EVAL(v);

		    if (semi[u] < semi[w])
			semi[w] = semi[u];
		}
	    }

            add_to_bucket(w, vertex[semi[w]]);
            LINK(parent[w], w);

            /* step 3 - implicitly define immediate dominators */
	    if (parent[w] != CFG_NIL)
	    {
		while (bucket[parent[w]] != CFG_NIL) {
		    v = Delete(parent[w]);
		    u = EVAL(v);
		    dom[v] = (semi[u] < semi[v]) ? u : parent[w];
		}
	    }
        }
}


/* traverse df spanning tree, explicitly defining immediate doms and
 * building dominator tree */
static void build_dt(CfgNodeId root)
{
    int i;
    int w;

    /* step 4 - explicitly define immediate dominators */
    dom[root] = CFG_NIL;

    /*
     *  This loop should skip the root
     */
    for (i = 1; i < n; i++)
    {
	/*
	 *  Skip any unreachable node
	 */
	if (vertex[i] != CFG_NIL)
	{
	    w = vertex[i];

	    if (dom[w] != vertex[semi[w]])
		dom[w] = dom[dom[w]];

	    add_dt_elem(w);
	}
    }

    /*
     *  Prenumbering makes testing for non-immediate ancestry easier
     */
    n = 0;
    prenumber(root);
}


/*
 *  add edge (v, w) to the forest (contained in depth-first spanning tree)
 */
static void LINK(int v, int w)
{
    int s;
    int tmp;

    s = w;

    while (semi[label[w]] < semi[label[child[s]]]) {
        if (size[s] + size[child[child[s]]] >= 2 * size[child[s]]) {
            ancestor[child[s]] = s;
            child[s] = child[child[s]];
        }
        else {
            size[child[s]] = size[s];
            ancestor[s] = child[s];
            s = ancestor[s];
        }
    }

    label[s] = label[w];
    size[v] += size[w];

    if (size[v] < 2 * size[w]) {
        tmp = s;
        s = child[v];
        child[v] = tmp;
    }

    while (s != CFG_NIL) {
        ancestor[s] = v;
        s = child[s];
    }
}

static int EVAL(int v)
{
    if (ancestor[v] == CFG_NIL)
        return label[v];

    COMPRESS(v);

    if (semi[label[ancestor[v]]] >= semi[label[v]])
        return label[v];

    return label[ancestor[v]];
}

static void COMPRESS(int v)
{
    /* assumes ancestor[v] != CFG_NIL */
    if (ancestor[ancestor[v]] == CFG_NIL)
        return;

    COMPRESS(ancestor[v]);

    if (semi[label[ancestor[v]]] < semi[label[v]])
        label[v] = label[ancestor[v]];

    ancestor[v] = ancestor[ancestor[v]];
}

static void add_to_bucket(int elem, int bckt)
{
    int New;

    New = f_new((Generic *) &buckets);
    buckets[New].node = elem;
    buckets[New].next = bucket[bckt];
    bucket[bckt] = New;
}

static int Delete(int bckt)
{
    int tmp;
    int top;

    tmp = bucket[bckt];
    bucket[bckt] = buckets[tmp].next;
    top = buckets[tmp].node;
    f_dispose((Generic) buckets, tmp);
    return top;
}

/*
 *  insert node for elem to dominator tree
 */
static void add_dt_elem(int elem)
{
    int idom;           /* imm. dominator of elem */

    dt[elem].kids = CFG_NIL;
    
    if (dom[elem] == CFG_NIL) {
        dt[elem].idom = CFG_NIL;
	dt[elem].next = CFG_NIL;
    }
    else {
	idom = dom[elem];
	dt[elem].idom = idom;

	dt[elem].next = dt[idom].kids;
	dt[idom].kids = elem;
    }
}

/* 
 *  free all space used in computation of dominator tree
 *  (but not tree itself)
 */
static void free_all()
{
    f_free((Generic)buckets);
    buckets = (BucketType *) 0;

    free_mem((void*)(parent	-1));	parent		= (int *) 0;
    free_mem((void*)(semi	-1));	semi		= (int *) 0;
    free_mem((void*)(vertex	-1));	vertex		= (int *) 0;
    free_mem((void*)(dom	-1));	dom		= (int *) 0;
    free_mem((void*)(bucket	-1));	bucket		= (int *) 0;
    free_mem((void*)(ancestor	-1));	ancestor	= (int *) 0;
    free_mem((void*)(label	-1));	label		= (int *) 0;
    free_mem((void*)(size	-1));	size		= (int *) 0;
    free_mem((void*)(child	-1));	child		= (int *) 0;
}

void dom_free(CfgInstance cfg, Boolean forward_direction)
{
    DomTree *dt_addr;

    dt_addr = forward_direction? (&(cfg->predom)):(&(cfg->postdom));

    free_mem((void*)*dt_addr);
    *dt_addr = (DomTree) 0;
}

static  char pt_str[75];

static void print_dom_tree(int node)
{
    int     kid;

    if (strlen(pt_str) < (size_t)72) strcat(pt_str, "   ");

    printf("%s%d\n", pt_str, node);

    for (kid = dt[node].kids; (kid != CFG_NIL); kid = dt[kid].next)
	print_dom_tree(kid);

    /*
     *  Unindent
     */
    if (strlen(pt_str) >= (size_t)3) pt_str[strlen(pt_str) -3] = '\0';
}

void dom_print(DomTree dtree, CfgNodeId root)
{
    pt_str[0] = '\0';

    dt = dtree;
    printf("Dominator tree rooted at %d:\n", root);
    print_dom_tree(root);
}

static void prenumber(int v)
{
    int kid;

    dt[v].prenum = ++n;

    for (kid = dt[v].kids; kid != CFG_NIL; kid = dt[kid].next)
	prenumber(kid);

    dt[v].last = n;
}

CfgNodeId dom_lca(DomTree dtree, CfgNodeId a, CfgNodeId b)
{
    if ((a == CFG_NIL) || (b == CFG_NIL)) return CFG_NIL;

    if (DOM_is_dom(dtree,a,b)) return a;

    while ((b != CFG_NIL) && !DOM_is_dom(dtree,b,a))
	b = dtree[b].idom;

    return b;
}

DomTree
cfg_get_predom(CfgInstance cfg)
{
    if (!(cfg->predom)) dom_build(cfg, true);
    
    return cfg->predom;
}

DomTree
cfg_get_postdom(CfgInstance cfg)
{
    if (!(cfg->postdom))  dom_build(cfg, false);

    return cfg->postdom;
}

Boolean cfg_is_reachable(CfgInstance cfg, CfgNodeId cn)
{
    if (!(cfg->predom)) dom_build(cfg, true);

    if ((cn < 0) ||
	(cn >= f_curr_size((Generic) cfg->cfgNodes)) ||
	CFG_node(cfg, cn)->freed)
    {
	fprintf(stderr, "cfg_is_reachable: invalid node!\n");
	return false;
    }
    return BOOL((DOM_idom(cfg->predom, cn) != CFG_NIL) ||
		(cn == cfg->start)
		);
}

Boolean cfg_reaches_end(CfgInstance cfg, CfgNodeId cn)
{
    if (!(cfg->postdom)) dom_build(cfg, false);

    if ((cn < 0) ||
	(cn >= f_curr_size((Generic) cfg->cfgNodes)) ||
	CFG_node(cfg, cn)->freed)
    {
	fprintf(stderr, "cfg_reaches_end: invalid node!\n");
	return false;
    }
    return BOOL((DOM_idom(cfg->postdom, cn) != CFG_NIL) ||
		(cn == cfg->end)
		);
}

CfgNodeId dom_idom(DomTree dtree, CfgNodeId id)
{
    return DOM_idom(dtree, id);
}

CfgNodeId dom_kids(DomTree dtree, CfgNodeId id)
{
    return DOM_kids(dtree, id);
}

CfgNodeId dom_next(DomTree dtree, CfgNodeId id)
{
    return DOM_next(dtree, id);
}

Boolean dom_is_dom(DomTree dtree, CfgNodeId a, CfgNodeId b)
{
    return BOOL( DOM_is_dom(dtree, a, b)
		);
}
