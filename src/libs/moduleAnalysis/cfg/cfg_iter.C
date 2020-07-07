/* $Id: cfg_iter.C,v 1.6 2001/10/12 19:28:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************
 *
 * Iterators for the Control Flow Graph.
 *
 ***********************************************************************/

#include <assert.h>
#include <iostream>
using namespace std;
#ifndef Cfg_h
#include <libs/moduleAnalysis/cfg/Cfg.h>
#endif


/*------------------ LOCAL DECLARATIONS ---------------------*/

EXTERN(int,       tarj_level2,        (TarjTree tarjans,
				       CfgNodeId cn));
EXTERN(Boolean,   cfg_has_exit_jumps, (CfgInstance cfg,
				       TarjTree tarjans,
				       CfgNodeId cn));
EXTERN(CfgNodeId, cfg_predom_pred,    (CfgInstance cfg, CfgNodeId cn));
EXTERN(CfgNodeId, cfg_postdom_succ,   (CfgInstance cfg, CfgNodeId cn));
EXTERN(Boolean,   cfg_connected,      (CfgInstance cfg, CfgNodeId src,
				       CfgNodeId dest));
EXTERN(Boolean,   cfg_is_exit_edge, (CfgInstance cfg, TarjTree tarjans,
				     CfgEdgeId edge));
EXTERN(CfgNodeId, cfg_get_exit_header, (CfgInstance cfg,
					TarjTree tarjans, CfgNodeId cn));


/**********************************************************************
 * tarj_level2()
 *
 * Note that tarj_level() assigns to the headers of intervals the
 * level of the interval we are heading.
 * This is not what we want here; instead, we want loop headers to be
 * at a shallower level than the body of the loop.
 */
int
tarj_level2(TarjTree tarjans, CfgNodeId cn)
{
  int level = tarj_is_header(tarjans, cn)
    ? tarj_level(tarjans, cn) - 1
      : tarj_level(tarjans, cn);

  return level;
}


/**********************************************************************
 * cfg_connected()  True iff there is an edge from <src> to <dest>.
 */
Boolean
cfg_connected(CfgInstance cfg, CfgNodeId src, CfgNodeId dest)
{
  Boolean   connected = false;
  CfgEdgeId edge = cfg_first_from_cfg(cfg, src);

  while ((edge != CFG_NIL) && !connected)
  {
    connected = (Boolean)(cfg_edge_dest(cfg, edge) == dest);
    edge      = cfg_next_from(cfg, edge);
  }

  return connected;
}


/**********************************************************************
 * cfg_predom_pred()  Return predominating predecessor, if any.
 *                    This includes immediately preceeding loops, even
 *                    if there are jumps out of it.
 */
CfgNodeId
cfg_predom_pred(CfgInstance cfg, CfgNodeId cn)
{
  CfgNodeId only_pred_cn;
  DomTree   predom   = cfg_get_predom(cfg);
  DomTree   postdom  = cfg_get_postdom(cfg);
  CfgNodeId other_cn = dom_idom(predom, cn);
  TarjTree  tarjans  = cfg_get_intervals(cfg);

  if (((dom_idom(postdom, other_cn) == cn)
       || (tarj_is_header(tarjans, other_cn)
	   && (cfg_node_fanout(cfg, other_cn) == 2)))
      && cfg_connected(cfg, other_cn, cn))
  {
    only_pred_cn = other_cn;
  }
  else
  {
    only_pred_cn = CFG_NIL;
  }
  return only_pred_cn;
}


/**********************************************************************
 * cfg_postdom_succ()  Return postdominating successor, if any.
 */
CfgNodeId
cfg_postdom_succ(CfgInstance cfg, CfgNodeId cn)
{
  CfgNodeId only_succ_cn;
  DomTree   predom   = cfg_get_predom(cfg);
  DomTree   postdom  = cfg_get_postdom(cfg);
  CfgNodeId other_cn = dom_idom(postdom, cn);

  only_succ_cn = ((dom_idom(predom, other_cn) == cn)
		  && cfg_connected(cfg, cn, other_cn))
    ? other_cn : CFG_NIL;

  return only_succ_cn;
}


/*********************************************************************
 * cfg_has_exit_jumps()
 */
Boolean
cfg_has_exit_jumps(CfgInstance cfg, TarjTree tarjans, CfgNodeId cn)
{
  Boolean   has_jumps;
  CfgNodeId postdom;
  Boolean   is_header = (Boolean)((cn != CFG_NIL) && tarj_is_header(tarjans, cn));

  if (is_header)
  {
    postdom   = cfg_postdom_succ(cfg, cn);
    has_jumps = (Boolean)(postdom == CFG_NIL);
  }
  else
  {
    has_jumps = false;
  }

  return has_jumps;
}


/*********************************************************************
 * cfg_is_exit_edge()  True iff <edge> is exit edge (jump out of loop,
 *                     not back to header).
 */
Boolean
cfg_is_exit_edge(CfgInstance cfg, TarjTree tarjans, CfgEdgeId edge)
{
  CfgNodeId src, dest, header;
  Boolean   is_exit = false;

  dest = cfg_edge_dest(cfg, edge);

  if (cfg_node_fanin(cfg, dest) == 1)    // Fanin(Header) = 2
  {
    // Get predecessor of <cn>  (no critical edges !)
    src     = cfg_edge_src(cfg, edge);
    header  = tarj_loop_exited(tarjans, src, dest);
    is_exit = (Boolean)((header != src) && (header != CFG_NIL));
  }

  return is_exit;
}


/*********************************************************************
 * cfg_get_exit_header()  If this node is target of an exit edge (ie,
 *                        the sink of a forward edge jumping out of a
 *                        loop), then return the header of the outermost
 *                        loop exited by that edge.
 */
CfgNodeId
cfg_get_exit_header(CfgInstance cfg, TarjTree tarjans, CfgNodeId cn)
{
  CfgNodeId pred, header;
  CfgEdgeId edge;

  // <cn> has only one predecessor ?
  if (cfg_node_fanin(cfg, cn) == 1)
  {
    // Get predecessor of <cn>  (no critical edges !)
    edge   = cfg_first_to_cfg(cfg, cn);
    pred   = cfg_edge_src(cfg, edge);
    header = tarj_loop_exited(tarjans, pred, cn);
  }
  else
  {
    header = CFG_NIL;
  }

  return header;
}


/*********************************************************************
 *                                                                   *
 *            Methods of class IntervalGraphIter                     *
 *                                                                   *
 *********************************************************************/

/*********************************************************************
 * Constructor
 */
IntervalGraphIter::IntervalGraphIter(CfgInstance    my_cfg,
				     TraversalOrder order,
				     int            my_only_level)
: cfg        (my_cfg),
  only_level (my_only_level),
  node_cnt   (cfg_node_max(cfg)),
  nodes      (new CfgNodeId[node_cnt])
{
  reset(order);
}


/*********************************************************************
 * Destructor
 */
IntervalGraphIter::~IntervalGraphIter()
{
  delete nodes;
}


/*********************************************************************
 * Advance operator
 */
CfgNodeId
IntervalGraphIter::operator() ()
{
  CfgNodeId current = ((pos < node_cnt) && (pos >= 0))
    ? nodes[pos] : CFG_NIL;

  pos += inc;

  return current;
}

    
/*********************************************************************
 * reset()   Reset iterator to start.
 */
void
IntervalGraphIter::reset(TraversalOrder order)
{
  TraversalOrder ord;
  Boolean        reverse;
  TarjTree       tarjans = cfg_get_intervals(cfg);

  // Since cfg_tarj_next() does not support reverse orders, we first
  // iterate through all nodes in forward order and store the order
  // in <nodes>.
  ord = ((order == PreOrder) || (order == ReversePreOrder)) ?
    PreOrder : PostOrder;
  node_cnt = 0;
  for (CfgNodeId current = CFG_NIL;
       (current = cfg_tarj_next(cfg, current, ord)) != CFG_NIL;)
  {
    if ((only_level < 0)
	|| (tarj_level2(tarjans, current) == only_level))
    {
      nodes[node_cnt++] = current;
    }
  }
  reverse = (Boolean)((order == ReversePreOrder) || (order == ReversePostOrder));
  inc     = reverse ? -1 : 1;
  pos     = reverse ? node_cnt - 1 : 0;
}


/*********************************************************************
 *                                                                   *
 *            Methods of class CfgMeetIter                           *
 *                                                                   *
 *********************************************************************/

/*********************************************************************
 * Constructor
 */
CfgMeetIter::CfgMeetIter(CfgInstance my_cfg,
			 CfgNodeId   my_node,
			 Meetees     my_meetees,
			 Boolean     my_do_tarjans)
: cfg          (my_cfg),
  tarjans      (cfg_get_intervals(cfg)),
  meetees      ((Meetees)(((my_meetees == SuccsOfPreds) ? Preds
		: ((my_meetees == PredsOfSuccs) ? Succs : my_meetees)))),
  do_tarjans   (my_do_tarjans),
  nested_iter  (NULL),
  is_nested    ((Boolean)((my_meetees == PredsOfSuccs)
		|| (my_meetees == SuccsOfPreds))),
  nested_meetees ((Meetees)((my_meetees == SuccsOfPreds) ? Succs
		  : ((my_meetees == PredsOfSuccs) ? Preds : -1))),
  node_cnt     (cfg_node_max(cfg)),
  nodes        (new CfgNodeId[node_cnt]),
  node         (CFG_NIL)
{
  reset(my_node);
}


/*********************************************************************
 * Destructor
 */
CfgMeetIter::~CfgMeetIter()
{
  delete nested_iter;
  delete nodes;
}


/*********************************************************************
 * Advance operator
 */
CfgNodeId
CfgMeetIter::operator() ()
{
  CfgNodeId result_node = CFG_NIL;

  if (is_nested)   // Do we have a child iterator ?
  {
    do             // Find next <cur_node>, if needed
    {
      if ((cur_node == CFG_NIL) ||           // At the beginning ?
	  ((result_node = (*nested_iter)())  // Iterate child
	   == CFG_NIL))                      // Child finished ?
      {
	stepCur_node();                      // Iterate self
	if (cur_node != CFG_NIL)             // Self not finished ?
	{
	  delete nested_iter;                // Get new child
	  nested_iter = new CfgMeetIter(cfg, cur_node, nested_meetees,
					do_tarjans);
	}
      }
    } while ((cur_node != CFG_NIL)           // Self not finished ?
	     && ((result_node == CFG_NIL)    // Child finished ?
		 || (result_node == node))); // Found me, myself, & I ?
  }
  else
  {
    stepCur_node();
    result_node = cur_node;
  }

  return result_node;
}


/*********************************************************************
 * stepCur_node()  Advance <cur_node>.
 */
void
CfgMeetIter::stepCur_node()
{
  CfgNodeId loop_node, src_node;
  CfgEdgeId edge;

  switch (meetees) {
  case PredsI:
    if (cur_exit_node != CFG_NIL)
    {
      cur_node      = cur_exit_node;
      cur_exit_node = CFG_NIL;
      break;
    }

    // No current exit node => fall through to Preds

  case Preds:
    // Find first candidate edge
    cur_edge = (cur_node == CFG_NIL)          // At start of iterator ?
      ? cfg_first_to_cfg(cfg, node)           // Get first edge
	: cfg_next_to(cfg, cur_edge);         // Get next edge

    // Find suitable edge
    while ((cur_edge != CFG_NIL)              // Not past last edge ?
	   && do_tarjans                      // Exlude nested nodes ?
	   && tarj_exclude_edge(cur_edge))
    {
      cur_edge = cfg_next_to(cfg, cur_edge);
    }

    // Map edge to node
    cur_node = (cur_edge == CFG_NIL)          // No edge found ?
      ? CFG_NIL                               // NULL node
	: cfg_edge_src(cfg, cur_edge);        // Source of edge

    // If we want to walk intervals, then we are interested in the
    // outermost loop exited when going from <cur_node> to <node>.
    if (do_tarjans && (cur_node != CFG_NIL))
    {
      loop_node = tarj_loop_exited(tarjans, cur_node, node);

      if (loop_node != CFG_NIL)
      {
	if (loop_node == node)
	{
	  // If we get here, then we have a predecessor within our loop
	  // that is not connected through a back edge (since back
	  // edges are already excluded) --- not good.
	  cout << "WARNING: CfgMeetIter(): loop_node = node = " <<
	    node << ".\n";
	}

	if (loop_node != cur_node)
	{
	  // If we get here, then we have a predecessor at a lower
	  // level then we are.
	  // This occurs when we have non-DO loops, like for example a
	  // DO-WHILE loop that exits from the end.

	  // 7/15/93 RvH: Disabled this, since we really want <cur_node>
	  cur_exit_node = loop_node;
	}
      }
    }
    break;

  case Succs:
  case SuccsI:
  case SuccsT:
    // Find first candidate edge
    cur_edge = (cur_node == CFG_NIL)        // At start of iterator ?
      ? cfg_first_from_cfg(cfg, node)       // Get first edge
	: ((cur_edge == CFG_NIL)            // No edges any more ?
	   ? CFG_NIL                        // Null edge
	   : cfg_next_from(cfg, cur_edge)); // Get next edge

    // Find suitable edge
    while ((cur_edge != CFG_NIL)            // Not past last edge ?
	   && do_tarjans                    // Exlude entry/back edges ?
	   && (tarj_exclude_edge(cur_edge)
	       || ((meetees == SuccsT)
		   && (cfg_is_exit_edge(cfg, tarjans, cur_edge)))))
    {
      cur_edge = cfg_next_from(cfg, cur_edge);
    }

    // Map edge to node
    if (cur_edge == CFG_NIL) // No edge found ?
    {
      if (do_exit_jumps)       // Include exit jumps ?
      {
	if (cur_exit_node == CFG_NIL)
	{
	  cur_exit_node = cfg_get_first_node(cfg);
	}

	// Brute force approach for now:
	// Loop through potential targets of exit nodes
	for (cur_node = CFG_NIL;
	     (cur_exit_node != CFG_NIL) && (cur_node == CFG_NIL);
	     cur_exit_node = cfg_get_next_node(cfg, cur_exit_node))
	{
	  if (// <cur_exit_node> has only one predecessor ?
	      (cfg_node_fanin(cfg, cur_exit_node) == 1)

	      // <cur_exit_node> not in interval headed by <node> ?
	      && !tarj_contains(tarjans, node, cur_exit_node))
	  {
	    // Get predecessor of <cur_exit_node> 
	    edge     = cfg_first_to_cfg(cfg, cur_exit_node);
	    src_node = cfg_edge_src(cfg, edge);

	    if (// <src_node> is not the loop itself ?
		(src_node != node)

		// <src_node> in interval headed by <node> ?
		&& tarj_contains(tarjans, node, src_node))
	    {
	      // <cur_node> is target of jump out of loop
	      cur_node = cur_exit_node;
	    }
	  }
	}

	// Exhausted all nodes ?
	if (cur_exit_node == CFG_NIL)
	{
	  do_exit_jumps = false;
	}
      }
      else
      {
	cur_node = CFG_NIL;
      }
    }
    else
    {
      cur_node = cfg_edge_dest(cfg, cur_edge);       // Source of edge
    }

    // If we want to walk intervals, then we are interested in the
    // outermost loop exited when going from <cur_node> to <node>.
    if (do_tarjans && (cur_node != CFG_NIL))
    {
      loop_node = tarj_loop_exited(tarjans, cur_node, node);

      if ((loop_node != CFG_NIL) && (loop_node != cur_node))
      {
	// If we get here, then we have a successor that is at a lower
	// level then we are and not in our interval - since we already
	// exclude successors within our interval.  Not good.
	cout << "WARNING: CfgMeetIter(): cur_node = " << cur_node <<
	    ", loop_node = " << loop_node << ", node = " << node <<
	      "; irreducible graph ?\n";
	cur_node = loop_node;
      }
    }
    break;

  case ChildrenForward:
    // At start of iterator ?
    if (cur_node == CFG_NIL)
    {
      cur_node = tarj_inners(tarjans, node);
    }
    else
    {
      cur_node = tarj_next(tarjans, cur_node);
    }
    break;

  case ChildrenBackward:
    cur_node = (pos > 0) ? nodes[--pos] : CFG_NIL;
    break;

  default:
    cout << "WARNING: CfgMeetIter(): meetees = " << meetees << ".\n";
  }
}


/*********************************************************************
 * reset()
 */
void
CfgMeetIter::reset(CfgNodeId my_node)
{
  if (my_node != CFG_NIL)      // Got a new starting node ?
  {
    node           = my_node;

    if (meetees == ChildrenBackward)
    {
      cur_node     = tarj_inners(tarjans, node);
      pos          = 0;
      nodes[pos++] = cur_node;
      while ((cur_node = tarj_next(tarjans, cur_node)) != CFG_NIL)
      {
	nodes[pos++] = cur_node;
      }
    }
  }

  do_exit_jumps  = (Boolean)(do_tarjans && (meetees == SuccsI) &&
                      cfg_has_exit_jumps(cfg, tarjans, node));
  cur_exit_node  = CFG_NIL;
  cur_node       = CFG_NIL;
  cur_edge       = CFG_NIL;
}


/**********************************************************************
 * tarj_exclude_edge()  Should we exclude this edge for interval walk ?
 */
Boolean
CfgMeetIter::tarj_exclude_edge(CfgEdgeId edge)
{
  CfgNodeId src_node  = cfg_edge_src(cfg, edge);
  CfgNodeId dest_node = cfg_edge_dest(cfg, edge);
  Boolean   result    = (Boolean)(tarj_contains(tarjans, src_node, dest_node)
                         || tarj_contains(tarjans, dest_node, src_node));

  return result;
}
