/* $Id: AST_SSA_Graph.h,v 1.9 1997/03/11 14:28:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AST_SSA_Graph_h
#define AST_SSA_Graph_h

/**********************************************************************
 * Class for AST_INDEX based accesses to the CFG/SSA graph,
 * derived from class AST_Graph.
 */
/**********************************************************************
 * Revision history:
 * $Log: AST_SSA_Graph.h,v $
 * Revision 1.9  1997/03/11 14:28:24  carr
 * newly checked in as revision 1.9
 *
 * Revision 1.9  94/03/21  13:45:08  patton
 * fixed comment problem
 * 
 * Revision 1.8  94/02/27  20:14:12  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.6  1994/02/27  19:38:14  reinhard
 * Added Boolean with_values parameter.
 *
 * Revision 1.5  1994/01/18  19:43:44  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.4  1993/06/09  23:42:11  reinhard
 * Cleaned up include hierarchy.
 *
 */

#ifndef AST_Graph_h
#include <libs/fortD/irregAnalysis/AST_Graph.h>
#endif

#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif

/*------------------- FORWARD DECLARATIONS ------------------*/

class AST_SSA_Graph;

/*********************************************************************/
/*** Declaration of class AST_SSA_graph ******************************/
/*********************************************************************/
class AST_SSA_Graph : public AST_Graph
{
  CfgInfo cfgGlobals;                   // cfg information

public:
  // Constructors
  AST_SSA_Graph(CfgInstance my_cfg);            // cfg computed already
  AST_SSA_Graph(FortTree    ft,                 // cfg not computed yet
		Boolean     with_values = false);

  // Destructor
  ~AST_SSA_Graph();
};

#endif _AST_SSA_GRAPH
