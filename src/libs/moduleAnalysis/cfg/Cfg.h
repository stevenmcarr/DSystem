/* $Id: Cfg.h,v 1.5 1997/03/11 14:35:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 *
 *
 *  -- Cfg.h
 *
 *  Public C++ header file for AST-based Control Flow Graph package.
 *  If you're using C, include cfg.h instead.
 *
 *********************************************************************/

#ifndef Cfg_h
#define Cfg_h

#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif

/*********************************************************************
 *
 * Interval Tree Iterators
 *
 * Given the following Tarjan interval tree:
 * <node id>(level,type)       //  Original program
 *  0(0,Acyclic)
 *     1(0,Acyclic)            //  program example
 *     2(1,Interval)           //  do i = 1, n
 *        3(1,Acyclic)         //     dummy = 0
 *        4(1,Acyclic)         //     dummy = 0
 *     10(0,Acyclic)           //  enddo
 *     5(1,Interval)           //  do i = 1, n
 *        6(1,Acyclic)         //     dummy = 0
 *        7(2,Interval)        //     do j = 1, n
 *           8(2,Acyclic)      //        dummy = 0
 *        11(1,Acyclic)        //     enddo
 *     9(0,Acyclic)            //  enddo
 *
 * The order in which the iterators will return <node id>'s is for
 * PreOrder:         0 1 2 3 4 10 5 6 7 8 11 9  [top-down, forward]
 * PostOrder:        1 3 4 2 10 6 8 7 11 5 9 0  [bottom-up, forward]
 * ReversePreOrder:  9 11 8 7 6 5 10 4 3 2 1 0  [bottom-up, backward]
 * ReversePostOrder: 0 9 5 11 7 8 6 10 2 4 3 1  [top-down, backward]
 *
 * Usage:
 * CfgNodeId cn;
 * for (IntervalGraphIter iter(cfg, order);
 *      (cn = iter()) != CFG_NIL;) { ... }
 *
 * If <only_level> >= 0, then only the nodes at the given nesting
 * level will be visited.
 */
class IntervalGraphIter {
public:
  IntervalGraphIter(CfgInstance    my_cfg,          // Constructor
		    TraversalOrder order         = PreOrder,
		    int            my_only_level = -1);
  ~IntervalGraphIter();                             // Destructor
  CfgNodeId operator() ();                          // Advance operator
  void      reset(TraversalOrder order = PreOrder); // Reset to start

private:
  CfgInstance cfg;
  int         only_level;
  int         node_cnt;
  CfgNodeId   *nodes;
  int         pos;
  int         inc;
};

    
/*********************************************************************
 *
 * Control Flow Graph Iterators
 *
 * Here, unlike for IntervalGraphIter iterators, we assume that
 * order does not matter (except for ChildrenForward,
 * ChildrenBackward).
 *
 * Usages:
 *
 * CfgNodeId pred, succ;
 *
 * // Loop over predecessors of succ:
 * for (CfgPredsIter iter(cfg, succ); (pred = iter()) != CFG_NIL;)
 * { ... }
 *
 * // Loop over successors of pred:
 * for (CfgSuccsIter iter(cfg, pred); (succ = iter()) != CFG_NIL;)
 * { ... }
 *
 * // Loop over successors of pred, 2nd version:
 * for (CfgMeetIter iter(cfg, pred, Succs);
 *      (succ = iter()) != CFG_NIL;)
 * { ... }
 *
 * NOTE: This iterator resets itself to the beginning after cycling
 * through all its members.  I.e., the same iterator can be reused to
 * cycle through the same elements multiple times.
 *
 * If <do_tarjans> is set to true, then
 *
 *  - back edges are ignored,
 *
 *  - edges leaving the interval are treated as if coming from the
 *    interval header.
 *
 * "Nested iterators" allow to iterate over preds of succs, etc.
 *
 * <SuccsI> is <Succs> + any target t of any node in an interval headed
 * by <node> if t is not that interval; i.e., <SuccsI> treats
 * jumps out of the interval as if they would originate from <node>.
 */
class CfgMeetIter;               // Forward declaration
typedef enum { Preds,            // Predecessors
	       PredsI,           // Preds + exit headers
	       PredsT,           // Preds within same interval
	       Succs,            // Successors
	       SuccsI,           // Succs + exit jumps
	       SuccsT,           // Succs within same interval
	       ChildrenForward,  // Children of this header node
	       ChildrenBackward, // Children of this header node
	       PredsOfSuccs,     // Nodes w/ same predecessor
	       SuccsOfPreds      // Nodes w/ same successor
	       } Meetees;

class CfgMeetIter {
public: 
  CfgMeetIter(CfgInstance my_cfg,           // Constructor
	      CfgNodeId   my_node,
	      Meetees     my_meetees,
	      Boolean     my_do_tarjans = true);
  ~CfgMeetIter();                               // Destructor
  CfgNodeId operator() ();                      // Advance operator
  void      reset(CfgNodeId my_node = CFG_NIL); // Reset to (new) start

private:
  CfgInstance cfg;            // Control flow graph
  TarjTree    tarjans;        // Interval tree
  CfgNodeId   node;           // Starting node
  CfgNodeId   cur_node;       // Current node
  CfgEdgeId   cur_edge;       // Edge to current node
  Meetees     meetees;        // Type of neighbors to iterate through
  Boolean     do_tarjans;     // Include back edges ?
  Boolean     is_nested;      // Is this a "nested iterator" ?
  CfgMeetIter *nested_iter;   // Nested iter (for SamePred, SameSucc)
  Meetees     nested_meetees; // Meetees for nested iter
  Boolean     do_exit_jumps;  // Let exit jumps originate from header ?
  CfgNodeId   cur_exit_node;  // Current exit jump;
  int         node_cnt;
  CfgNodeId   *nodes;
  int         pos;

  // Private methods
  void    stepCur_node();
  Boolean tarj_exclude_edge(CfgEdgeId edge);
};


class CfgPredsIter : public CfgMeetIter {
public:
  CfgPredsIter(const CfgInstance &my_cfg, CfgNodeId my_node)
    : CfgMeetIter(my_cfg, my_node, Preds, true) {}
};


class CfgSuccsIter : public CfgMeetIter {
public:
  CfgSuccsIter(const CfgInstance &my_cfg, CfgNodeId my_node)
    : CfgMeetIter(my_cfg, my_node, Succs, true) {}
};

#endif /* ifndef Cfg_h */
