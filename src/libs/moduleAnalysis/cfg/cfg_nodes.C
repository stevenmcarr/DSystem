/* $Id: cfg_nodes.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_nodes.c
 *
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/cfg/cfg_nodes.h>
#include <libs/moduleAnalysis/cfg/cfg_labels.h>
#include <libs/moduleAnalysis/cfg/cfg_ids.h>
/* #include <fort/ftx_ast.h> */

#define has_label(n)  ((labelled_stmt(n)) && (is_label_def(gen_get_label(n))))
#define INIT_LIST_SIZE 16
STATIC(Boolean, gets_node,(AST_INDEX node));
STATIC(CfgNodeId, make_node,(CfgInstance cfg, AST_INDEX node, 
                             int lbl));
STATIC(void, add_to_common_list,(CfgInstance cfg, AST_INDEX node));
STATIC(int, get_label,(CfgInstance cfg, AST_INDEX node));
STATIC(void, mark_assigned_label,(CfgInstance cfg, AST_INDEX node));

/* 
 *  Recursively traverse AST root, building and initializing CFG nodes where
 *  needed.  realRoot is flag for top node of ast received.
 */
Boolean
cfg_build_nodes(CfgInstance cfg, AST_INDEX root)
{
    AST_INDEX cls_lbl;	    /* close label assoc'd with block if, etc. */
    int lbl;		    /* label assoc'd with current stmt */
    Boolean err = false;
    CfgNodeId tmp;

    while (!is_null_node(root)) {
	if (is_error(root))
	    return true;

	lbl = CFG_NO_LABEL;

	/*
	 *  Just keep COMMON statements in a list for later.
	 */
	if (common_stmt(root))
	    add_to_common_list(cfg, root);

	if (/* ftx_ */ is_executable(root) && !is_format(root))
	    lbl = get_label(cfg, root);

	if (is_assign(root))
	    mark_assigned_label(cfg, root);

	if (gets_node(root)) { 
	    if (lbl && is_loop(root)) {
		/*
		 *  Insert two nodes for each loop:
		 *  pre-header will have lbl and type PREHEADER
		 *  actual header will have no lbl and one of DO types.
		 *
		 *  These changes entail make_node checking for a lbl
		 *  on a loop node passed.  It's ok to have no PREHEADER
		 *  for an unlabeled loop because there can only be one
		 *  control flow edge into the loop (from outside the loop --
		 *  there is exactly one other, from the end of the loop).
		 *
		 *  By definition and construction, node number are such that
		 *	(if there is a preheader):
		 *
		 *	preheader == actual_header + 1
		 *					    phh 7 Mar 91
		 *
		 *  The above ordering of the values seems strange but the
		 *  CFG nodes must be added in this order, to make building
		 *  the pcf graph convenient.  Adding the preheader second
		 *  makes the ast-to-CFG map point to the preheader -- which
		 *  should become the destination of *most* pcf edges.	The
		 *  only exception is the one edge from the normal termination
		 *  of a loop iteration.
		 */

		/* header */
		tmp = make_node(cfg, root, CFG_NO_LABEL);

		/* pre-header */
		tmp = make_node(cfg, root, lbl);
	    }
	    else
		tmp = make_node(cfg, root, lbl);
	}

	if (is_parallel(root))
	    err = BOOL( (((int) err) |
			 ((int)
			  cfg_build_nodes(cfg,
					  gen_PARALLEL_get_stmt_LIST(root))
			  ))
		       );

	if (has_stmt_list(root))
	    err = BOOL( (((int) err) |
			 ((int) cfg_build_nodes(cfg, in(root))))
		       );

	/*
	 *  add enough nodes for each loop head
	 *  if nested loops share last stmt
	 */
	/*  if (is_loop(root)) {
	 *	lbl = test_share(root);
	 *
	 *	if (lbl)
	 *	tmp = make_node(cfg_lookup_label(CFG_Inst->lblMap, lbl),
	 *		lbl);
	 *  }
	 */

	/* make CONTINUE for close label node if exists */
	if ((has_close_lbl_def(root)) && (!new_instance(root))) {
	    cls_lbl = get_close_label(root);
	    if (is_label_def(cls_lbl)) {
		lbl = get_label(cfg, cls_lbl);
		tmp = make_node(cfg, cls_lbl, lbl);
	    }
	}

	if (!is_null_node(cfg->endAst))
	{
	    if (root == cfg->endAst)
	    {
		/*
		 *  end of unstructured construct
		 *  (cfg->astnode is entry, cfg->endAst is exit)
                 */
		return err;
	    }
	}
	else if ((is_null_node(list_next(root))) &&
		 (cfg->astnode == out(root)))
	{
	    /*
	     *  end of structured construct (end is nil)
	     */
	    lbl = get_label(cfg, get_close_label(out(root)));
	    /*
	     *  Forced to add end here, instead of later,
	     *  so as to hook up its stmt label.
	     */
	    cfg->end = cfg_node_new_id(cfg);
	    CFG_node(cfg,cfg->end)->lbldef  = lbl;

	    return err;
	}
	else if (cfg->astnode == root)
	{
	    /*
	     *  back to root of structured construct (end is nil)
	     */
	    return err;
	}

	root = list_next(root);
    }

    return err;
}


/*
 *  add COMMON statement to list
 */
static void add_to_common_list(CfgInstance cfg, AST_INDEX node)
{
    int index;

    /*
     * Put AST node index for both COMMON and TASK_COMMON in the same  array
     */
    if ( cfg->comnAstList == (AST_INDEX *) 0 )
	cfg->comnAstList = 
	    (AST_INDEX *)f_alloc(INIT_LIST_SIZE, sizeof(AST_INDEX),
				 "COMMON ast nodes list",
				 (f_init_callback) 0);
    index = f_new((Generic *) &cfg->comnAstList);
    cfg->comnAstList[index] = node;
}


/*
 *  return label assoc'd with stmt node, or 0
 */
static int get_label(CfgInstance cfg, AST_INDEX node)
{
    AST_INDEX fthr = out(node);
    int lbl = CFG_NO_LABEL;

    if (is_if(node))
	return CFG_NO_LABEL;

    if (has_label(node))
	get_label_int(gen_get_label(node), lbl);

    else if (is_label_def(node))
	get_label_int(node, lbl);

    else if ((is_guard(node)) && ((is_first_in_list(node)) == true) &&
	     (has_label(fthr)))
	get_label_int(gen_get_label(fthr), lbl);	

    if (lbl != CFG_NO_LABEL)
	cfg->lblMap = cfg_insert_label((LabelMap) cfg->lblMap, lbl, node);

    return lbl;
}

/*
 *  Most stmts get cfg nodes --
 *  cprop version of this was much pickier
 */
static Boolean gets_node(AST_INDEX node)
{
    if (is_if(node) || is_format(node))
	return false;

    if (/* ftx_ */ is_executable(node)) 
	return true;

    if (is_f77_subprogram_stmt(node))
	return true;

    if (is_entry(node))
	return true;

    return false;
}

/*
 *  Build cfg node for ast node 'node' and label 'lbl'.
 */
static CfgNodeId make_node(CfgInstance cfg, AST_INDEX node, int lbl)
{
    int nodeId;

    /*
     *  Now calling this routine twice for loops with labels;
     *  when called with the lbl, build a PREHEADER.  phh 7 Mar 91
     * 
     *  Changes in handling of PREHEADERs.  So that there will be
     *  at most one CFG node per AST_INDEX, we move the PREHEADER to
     *  hang off the label_def.                       phh Oct 91
     */
    nodeId = cfg_node_new_id(cfg);

    if ((lbl != CFG_NO_LABEL) && (is_loop(node)))
    {
	node = gen_get_label(node);
    }

    CFG_node(cfg,nodeId)->astnode = node;
    CFG_node(cfg,nodeId)->lbldef  = lbl;

    if (!is_null_node(node)) {
	cfg_node_put_map(cfg, node, nodeId);
    }

    return nodeId;
}

void mark_assigned_label(CfgInstance cfg, AST_INDEX node)
{
    int lbl;

    get_label_int(gen_ASSIGN_get_lbl_ref(node), lbl);

    cfg->lblMap = cfg_insert_label((LabelMap) cfg->lblMap, lbl, AST_NIL);
}
