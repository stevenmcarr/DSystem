/* $Id: cfg_edges.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_edges.c
 *
 */



#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_edges.h>
#include <libs/moduleAnalysis/cfg/cfg_labels.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>

#define IN 0
#define NEXT 1

STATIC(void, do_alt_edges, (CfgInstance cfg, AST_INDEX node));
STATIC(void, do_control_flow, (CfgInstance cfg, AST_INDEX node));
STATIC(void, add_open_goto, (CfgInstance cfg, AST_INDEX node));
STATIC(CfgNodeId, get_next_node, (CfgInstance cfg, AST_INDEX node));
STATIC(void, add_lbl_ref_edge, (CfgInstance cfg, AST_INDEX node, AST_INDEX lbl, 
                                int cdLabel));
STATIC(void, add_edge, (CfgInstance cfg, CfgNodeId from, CfgNodeId to, int cdLabel));
STATIC(void, add_assign_edge, (LabelMap map, int lblId, Generic nodePassed));
STATIC(Boolean, invalid_goto, (AST_INDEX src, AST_INDEX dest));

static Boolean missing_label = false;

/*
 *  Insert edges to CFG for ast rooted at root.  CFG nodes already built.
 *  Should be called from each subprogram CfgInstance for a module,
 *  or on a fragment CfgInstance for construction of partial cfg.
 */
Boolean cfg_build_edges(CfgInstance cfg, AST_INDEX root)
{
    int cdLabel;
    Boolean error_missing_label = missing_label;

    /*
     *  add edges for all nodes in current statement nesting level
     */
    while (!is_null_node(root)) {
	if (gen_get_node_type(root) == GEN_COMMENT) {
	    root = list_next(root);
	    continue;
	}

        /*
	 *  add edges to nodes other than the next node,
	 *  next will be caught later
	 */
        if (poss_alternate_edges(root))
            do_alt_edges(cfg, root);

	/*
	 *  Add edge to DO from PREHEADER, if PREHEADER exists --- phh 7 Mar 91
	 */
	if (is_loop(root)) {
	    if (cfg_preheader_map(cfg, root) != CFG_NIL)
		add_edge(cfg, cfg_preheader_map(cfg, root),
			 cfg_header_map(cfg, root), CD_FALLTHROUGH);
	}

        /*
	 *  control flow includes any node
	 *  that should not get an edge to the next node
	 */
        if (control_flow(root)) {
            do_control_flow(cfg, root);
	    /*
	     *  Need to insert handling for computed goto fall-through
	     *  -- done inside do_control_flow
	     */
	}
        /* add edge to next node */
        else if ((cfg_node_exists(cfg, root)) &&
		 (!is_else(root) || is_null_node(in(root))) &&
		 (!is_null_node(cfg->endAst) ||
		  (root != cfg->astnode) ||
		  (is_null_node(in(cfg->astnode)))
		  )
		 )
	{
	    add_edge(cfg, cfg_node_map(cfg, root),
		     get_next_node(cfg, root), CD_FALLTHROUGH);
	}

        /* 
	 *  Stmts between PARALLEL (parbegin) and PARALLEL_CASE (parallel:)
	 *  are all just declarations.
	 *					Paul Havlak, 27 August 91
	 *
	 *  if (is_parallel(root)) {
	 *	tmp = list_first(gen_PARALLEL_get_stmt_LIST(root));
	 *
	 *	if (!is_null_node(tmp)) {
	 *		if (cfg_node_exists(cfg, tmp))
	 *			add_edge(cfg, cfg_node_map(cfg, root),
	 *				cfg_node_map(cfg, tmp), 
	 *				CD_FALLTHROUGH);
	 *		else
	 *			add_edge(cfg, cfg_node_map(cfg, root),
	 *				get_next_node(cfg, tmp),
	 *				CD_FALLTHROUGH);
	 *	}
	 *	cfg_build_edges(cfg, tmp);
	 *  }
	 */

        /* add edge to root's statement list */
        if (has_stmt_list(root)) {
            if ((!is_global(root)) &&
		(!is_if(root)) && 
		(!is_parallel(root)))
	    {
		CfgNodeId rootId;

		if (is_loop(root))
		    rootId = cfg_header_map(cfg, root);
		else
		    rootId = cfg_node_map(cfg, root);

		/*
		 *  Edge from PARALLEL CASE (parallel:) to nested statment
		 *  is unconditional.
		 *				Paul Havlak, 27 August 91
		 */
		if (is_parallel_case(root))
		    cdLabel = CD_FALLTHROUGH;
		else
		    cdLabel = CD_ENTER;

                if (cfg_node_exists(cfg, in(root)))
		{
		    if (is_entry(in(root))) /* only possible at top level */
			add_edge(cfg, rootId,
				 get_next_node(cfg, in(root)), cdLabel);
		    else
			add_edge(cfg, rootId,
				 cfg_node_map(cfg, in(root)), cdLabel);
		}
		else if (is_if(in(root)))
		    add_edge(cfg, rootId,
			     cfg_node_map(cfg, in(in(root))), cdLabel);
		
		else if (!(is_null_node(in(root))))
                    add_edge(cfg, rootId,
			     get_next_node(cfg, in(root)), cdLabel);

		/*
		 *  If statement list is empty, only time to bother
		 *  adding an edge is from one parallel case to next
		 *  (sequential semantics).
		 */
		else if (is_parallel_case(root))
		    add_edge(cfg, rootId, get_next_node(cfg, root), cdLabel);
            }
            error_missing_label = BOOL(error_missing_label || 
				       cfg_build_edges(cfg, in(root)));
        }

        if (!is_null_node(cfg->endAst))
        {
            if (root == cfg->endAst)
            {
		/*
		 *  end of non-structured construct -- exit while loop
		 */
		break;
            }
        }
        else if (cfg->astnode == root)
        {
	    /*
	     *  back to root of structured construct -- exit while loop
	     */
	    break;
        }

	root = list_next(root);
    }

    /*
     *  fix error return
     */
    error_missing_label = BOOL(error_missing_label || missing_label);
    missing_label = false;
    return error_missing_label;
}

/*
 *  unguarded spread nodes (ex. CALL, I/O stmts w/ alt returns, etc.),
 *	end-iteration stmt of DO, and ENTRY stmt
 */
static void do_alt_edges(CfgInstance cfg, AST_INDEX node)
{
    AST_INDEX tmp = AST_NIL;

    /*
     *  Add edge from last statement of iteration to header of DO
     */
    if (is_loop(node)) {
	if (is_label_def(tmp = get_close_label(node))) 
	{
	    add_edge(cfg, cfg_node_map(cfg, tmp),
		     cfg_header_map(cfg, node), CD_FALLTHROUGH);
	}

	tmp = AST_NIL;
    }


    else if (is_call(node))
        tmp = list_last(gen_INVOCATION_get_actual_arg_LIST
			(gen_CALL_get_invocation(node)));

    else if (is_read_long(node))
        tmp = list_last(gen_READ_LONG_get_kwd_LIST(node));

    else if (is_write(node))
        tmp = list_last(gen_READ_LONG_get_kwd_LIST(node));

    else if (is_subroutine(node) ||
	     is_function(node) ||
	     is_program(node) ||
	     is_entry(node))

        add_edge(cfg, cfg->start, cfg_node_map(cfg, node), CD_INVALID);

    else if (is_inquire(node))
        tmp = list_last(gen_INQUIRE_get_kwd_LIST(node));

    else if (is_open(node))
        tmp = list_last(gen_OPEN_get_kwd_LIST(node));

    else if (is_close(node))
        tmp = list_last(gen_CLOSE_get_kwd_LIST(node));

    else if (is_backspace_long(node))
        tmp = list_last(gen_BACKSPACE_LONG_get_kwd_LIST(node));

    else if (is_endfile_long(node))
        tmp = list_last(gen_ENDFILE_LONG_get_kwd_LIST(node));

    else if (is_rewind_long(node))
        tmp = list_last(gen_REWIND_LONG_get_kwd_LIST(node));

    for ( ; !is_null_node(tmp); tmp = list_prev(tmp)) {
        if (is_return_label(tmp))
            add_lbl_ref_edge(cfg, node,
			     gen_RETURN_LABEL_get_lbl_ref(tmp),
			     CD_INVALID);
        else if (is_end_specify(tmp))
            add_lbl_ref_edge(cfg, node,
			     gen_END_SPECIFY_get_lbl_ref(tmp),
			     CD_IO_END);
        else if (is_err_specify(tmp))
            add_lbl_ref_edge(cfg, node,
			     gen_ERR_SPECIFY_get_lbl_ref(tmp),
			     CD_IO_ERR);
    }
}

/* 
 *  guarded SPREAD, IF w/ labelled end stmt, return, goto, parallel stmts
 */
static void do_control_flow(CfgInstance cfg, AST_INDEX node)
{
    AST_INDEX tmp;
    AST_INDEX cls_lbl;
    int count;

    if (is_if(node)) {
        cls_lbl = get_close_label(node);
        if (!is_null_node(cls_lbl))
            add_edge(cfg, cfg_node_map(cfg, cls_lbl),
		     get_next_node(cfg, node), CD_FALLTHROUGH);
    }

    /* ast return node only has scc node if it has an expression */
    else if ((is_return(node)) && (cfg_node_exists(cfg, node))) {
        add_edge(cfg, cfg_node_map(cfg, node), cfg->end, CD_FALLTHROUGH);
    }

    else if (is_goto(node))
        add_lbl_ref_edge(cfg, node,
			 gen_GOTO_get_lbl_ref(node), CD_FALLTHROUGH);

    /*
     *  Do nothing for parallel node -- hook up parallel cases according
     *  to straightforward sequential schedule.
     *						Paul Havlak, 27 August 91
     *
     *  else if (is_parallel(node)) {
     *	    for (tmp = list_first(gen_PARALLEL_get_parallel_case_LIST(node));
     *		!is_null_node(tmp);
     *		tmp = list_next(tmp))
     *
     *		add_edge(cfg, cfg_node_map(cfg, node),
     *				cfg_node_map(cfg, tmp), CD_INVALID);
     *  }
     */

    /*
     *  do nothing for parallel_case node, just prevent edge from it to next 
     *  parallel_case in list
     *
     *  no, go ahead and put those edges in (sequential semantics) phh
     */

    else if (is_assigned_goto(node))
    {
	if (!is_null_node(gen_ASSIGNED_GOTO_get_lbl_ref_LIST(node))) {
	    count = list_length(gen_ASSIGNED_GOTO_get_lbl_ref_LIST(node));

	    for (tmp = list_last(gen_ASSIGNED_GOTO_get_lbl_ref_LIST(node));
		 !is_null_node(tmp); tmp = list_prev(tmp))

		add_lbl_ref_edge(cfg, node, tmp, count--);
	}
	else
	    add_open_goto(cfg, node);
    }

    else if (is_computed_goto(node))
    {
	count = list_length(gen_COMPUTED_GOTO_get_lbl_ref_LIST(node));

	for (tmp = list_last(gen_COMPUTED_GOTO_get_lbl_ref_LIST(node));
	     !is_null_node(tmp); tmp = list_prev(tmp))

	    add_lbl_ref_edge(cfg, node, tmp, count--);

	/*
	 *  Bug fix -- possible to fall through on computed goto
	 *                                          -- phh 28 Mar 91
	 */
	add_edge(cfg, cfg_node_map(cfg, node),
		 get_next_node(cfg, node), CD_FALLTHROUGH);
    }

    else if (is_arithmetic_if(node)) {
	add_lbl_ref_edge(cfg, node,
			 gen_ARITHMETIC_IF_get_lbl_ref3(node),
			 CD_POSITIVE);
	add_lbl_ref_edge(cfg, node,
			 gen_ARITHMETIC_IF_get_lbl_ref2(node),
			 CD_ZERO);
	add_lbl_ref_edge(cfg, node,
			 gen_ARITHMETIC_IF_get_lbl_ref1(node),
			 CD_NEGATIVE);
    }
    /*
     * end else }
     */
}

/*
 *  Add cfg edge from node to lblDef
 *  -- order seems backwards because this is passed to run_through_labels
 *	called by add_open_goto below.
 *
 *  This is only done for open assigned gotos (no label list),
 *  so the edge labeling is arbitrary (indicated by passing CD_INVALID)
 *
 *  Need to squirrel away this CfgInstance pointer first.
 */
static CfgInstance myCfg;

static void add_assign_edge(LabelMap map, int lblId, Generic nodePassed)
{
    AST_INDEX lblDef, nodeAst;
    CfgNodeId nodeCfg;

    if (!cfg_label_assigned(map, lblId)) return;

    /*
     *  Node passed must be an ASSIGNED_GOTO...
     *  Large risk here of accidentally inserting edge into
     *  structured region
     */
    nodeAst = (AST_INDEX) nodePassed;		/* the source */
    lblDef = label_node(map, lblId);		/* the destination */
    if (invalid_goto(nodeAst, lblDef)) return;

    nodeCfg = cfg_node_map(myCfg, nodeAst);

    if (cfg_node_exists(myCfg, lblDef))
	add_edge(myCfg, nodeCfg, cfg_node_map(myCfg, lblDef), CD_INVALID);
    else
	add_edge(myCfg, nodeCfg, get_next_node(myCfg, lblDef), CD_INVALID);
}

/*
 *  add all labelled stmts whose labels have bein in ASSIGN 
 *  as possible out edges
 */
static void add_open_goto(CfgInstance cfg, AST_INDEX node)
{
    myCfg = cfg;
    cfg_run_through_labels(cfg->lblMap, add_assign_edge, (Generic) node);
}

/*
 *  return CFG node for stmt following ast stmt "node" in control flow
 *  -- typically used when normal sink of edge is not executable for some
 *  reason (e.g., is a Format stmt).
 */
static CfgNodeId get_next_node(CfgInstance cfg, AST_INDEX node)
{
    AST_INDEX nxt;
    AST_INDEX fthr;
    AST_INDEX cls_lbl;

    /*
     *  no CFG node, but specifies next node as END -
     *  check before trying next stmt
     */
    if ((is_stop(node)) ||
	(is_return(node)) ||
	(node == cfg->endAst) ||
	(is_null_node(cfg->endAst) && (node == cfg->astnode))
	)
        return (cfg->end);

    /*
     *  no pcf edges from one parallel_case to next parallel_case.
     *  must go out level
     *  
     *  Change:  last statement inside parallel case should get
     *  edge to next parallel case (sequential semantics).
     *						Paul Havlak, 27 August 91
     */

    /*
     *  try normal condition of next statement in program being next node
     */
    nxt = list_next(node);

    if (is_entry(nxt))
	return (get_next_node(cfg, nxt));

    else if (cfg_node_exists(cfg, nxt))
	return (cfg_node_map(cfg, nxt));

    /*
     *  next statement doesn't work, try to find one in stmt
     *  list that will
     */
    while (((!cfg_node_exists(cfg, nxt)) && (!is_null_node(nxt))) ||
	   (is_entry(nxt)))
    {
	/*
	 *  if hit a stop or return in list, specifys next node as END
	 */
	if ((is_stop(nxt)) || (is_return(nxt)))
	    return (cfg->end);

	/*
	 *  if node is an if, actual node_stmt is
	 *  first guard in if-guard list
	 */
	if (is_if(nxt))
	    return (cfg_node_map(cfg, in(nxt)));

	nxt = list_next(nxt);
    }
    if (cfg_node_exists(cfg, nxt))
	return (cfg_node_map(cfg, nxt));

    /*
     *  else, no usable stmts in current level block, check father node
     */
    fthr = out(node);

    if (is_global(fthr))
        return CFG_NIL;

    if ((fthr == cfg->astnode) && is_null_node(cfg->endAst))
        return (cfg->end);

    /* for do loop, set up loop header */
    if (is_loop(fthr)) {

	if (is_label_def(cls_lbl = get_close_label(fthr))) 
	    return cfg_node_map(cfg, cls_lbl);

	/*
	 *  Now building preheader nodes -- this is the one jump to the
	 *  header that shouldn't be moved to the preheader...
	 *  The second node in the ASTMAP list (the one after the 
	 *  preheader, returned by cfg_node_map) should be the regular
	 *  loop node.
	 *
	 *  This should be equivalent to return(cfg_node_map(cfg, fthr) -1)
	 *
	 *  However, if there is *no* preheader, then cfg_node_map should
	 *  return the right thing.
	 *
	 *                                            phh 8 Mar 91
	 *
	 *  cfg_header_map now always does the right thing. phh 15 Aug 91
	 */
	return (cfg_header_map(cfg, fthr));
    }

    if (is_guard(fthr)) {
        nxt = out(fthr);
        cls_lbl = get_close_label(nxt);

        if (is_label_def(cls_lbl))
            return (cfg_node_map(cfg, cls_lbl));

        return (get_next_node(cfg, nxt));
    }

    if ((is_logical_if(fthr)) || (is_block_data(fthr)))
        return (get_next_node(cfg, fthr));

    /*
     *  Changed handling of parallel case so that last statement inside of 
     *  parallel case gets edge to next parallel case.
     *						Paul Havlak 27 August 91
     */
    if ((is_if(fthr)) || (is_debug(fthr)) ||
	(is_parallel(fthr)) || (is_parallel_case(fthr))) {
        cls_lbl = get_close_label(fthr);

        if (is_label_def(cls_lbl))
            return (cfg_node_map(cfg, cls_lbl));

        return (get_next_node(cfg, fthr));
    }
    fprintf(stderr, "get_next_node: failure\n");
    return cfg->end;
}


/*
 *  add edge from CFG node for ast node "node" to CFG node for ast node "lbl"
 */
static void add_lbl_ref_edge(CfgInstance cfg, AST_INDEX node, 
                             AST_INDEX lbl, int cdLabel)
{
    AST_INDEX nxt;
    int lbl_val;

    get_label_int(lbl, lbl_val);

    nxt = cfg_lookup_label(cfg->lblMap, lbl_val);

    if (invalid_goto(node, nxt))
    {
	fprintf(stderr, "cfg_edges:  jump into structure!\n");
	return;
    }

    if (is_null_node(nxt))
    {
	/*
	 *  Make jump to missing label go to end node
	 */
	missing_label = true;
        add_edge(cfg, cfg_node_map(cfg, node),
		 cfg->end, cdLabel);
    }
    else if (cfg_node_exists(cfg, nxt))
    {
        add_edge(cfg, cfg_node_map(cfg, node),
		 cfg_node_map(cfg, nxt), cdLabel);
    }
    else
    {
        add_edge(cfg, cfg_node_map(cfg, node),
		 get_next_node(cfg, nxt), cdLabel);
    }
}

/*
 *  add edge between nodes from, to
 */
static void add_edge(CfgInstance cfg, CfgNodeId from, CfgNodeId to, int cdLabel)
{
    CfgEdgeId edg;

    if ((from < 0) || (from >= f_curr_size((Generic) cfg->cfgNodes)) ||
	(to < 0) || (to >= f_curr_size((Generic) cfg->cfgNodes)) ||
	(CFG_node(cfg,from)->freed) || (CFG_node(cfg,to)->freed)) {
        fprintf(stderr, "attempt to add illegal edge from %d to %d\n",
		from, to);
        return;
    }

    /*
     *  Not the best place to do this, but edges from the PREHEADER node
     *  for a loop should really be from the corresponding loop node
     *  (unless the edge is from the PREHEADER node *to* the loop node
     *  -- that's a special edge that we don't want to turn into a cycle).
     *                                                 phh 8 Mar 91
     */
    if (is_label_def(CFG_node(cfg,from)->astnode))
    {
	CfgNodeId header = cfg_header_map(cfg, CFG_node(cfg,from)->astnode);

	if (to != header)
	    from = header;
    }

    ++(CFG_node(cfg,to)->fanIn);
    ++(CFG_node(cfg,from)->fanOut);

    edg = cfg_edge_new_id(cfg);
    CFG_edge(cfg,edg)->src = from;
    CFG_edge(cfg,edg)->dest = to;
    /*
     *  CFG_edge(cfg,edg)->join = CFG_node(cfg,to)->fanIn;
     */
    if (cdLabel == CD_INVALID)
	CFG_edge(cfg,edg)->label = CFG_node(cfg,from)->fanOut;
    else
	CFG_edge(cfg,edg)->label = cdLabel;

    /*
     *  each edge is actually in 2 lists, the ins list of the
     *  destination node and the outs list of the source node
     */
    CFG_edge(cfg,edg)->outNext = CFG_node(cfg,from)->outs;
    CFG_node(cfg,from)->outs = edg;

    CFG_edge(cfg,edg)->inNext = CFG_node(cfg,to)->ins;
    CFG_node(cfg,to)->ins = edg;
}

/*
 *  the following procedures are provided for other modules
 */

/*
 *  return true if any cfg edge into node is executable
 */
Boolean cfg_is_exec(CfgInstance cfg, CfgNodeId node)
{
    CfgEdgeId edge;
    AST_INDEX astnode;

    /*
     *  only way into a START node is to start there (no ins),
     *  so always executable because if it gets asked about,
     *  you started there
     */
    astnode = CFG_node(cfg,node)->astnode;
    if ((new_instance(astnode) || is_entry(astnode)) &&
	(CFG_node(cfg,node)->ins == CFG_NIL))

	return true;

    for (edge = CFG_node(cfg,node)->ins;
	 (edge != CFG_NIL) && (!(CFG_edge(cfg,edge)->exec));
	 edge = CFG_edge(cfg,edge)->inNext)
        ;

    return ((edge != CFG_NIL) ? true : false);
}

/*
 *  return true if exactly 1 cfg edge into node is executable
 */
Boolean cfg_first_time(CfgInstance cfg, CfgNodeId node)
{
    int cnt = 0;
    CfgEdgeId edge;

    for (edge = CFG_node(cfg,node)->ins; edge != CFG_NIL;
	 edge = CFG_edge(cfg,edge)->inNext)
        if ((CFG_edge(cfg,edge)->exec) && (++cnt > 1))
	    return false;

    return true;
}

/*
 *  Illegal to jump into structure
 */
Boolean invalid_goto(AST_INDEX src, AST_INDEX dest)
{
    if (is_guard(dest) && (is_first_in_list(dest) == true))
    {
	/*
	 *  This takes care of GOTO an IF, which is in the tree as first
	 *  guard of an IF node.
	 */
	dest = out(dest);
    }

    while (!is_null_node(src) && (src != out(dest)) && (src != dest))
    {
	src = out(src);
    }
    if ((src == out(dest)) || (src == dest)) 
	return false;
    else
	return true;
}
