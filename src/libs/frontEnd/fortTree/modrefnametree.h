/* $Id: modrefnametree.h,v 1.7 1997/03/11 14:29:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef modrefnametree_h
#define modrefnametree_h

#include <libs/support/trees/NonUniformDegreeTree.h>

/* local includes */
#include <libs/frontEnd/fortTree/modrefname.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/frontEnd/fortTree/ilist.h>

class ModRefNameTreeNode: public NonUniformDegreeTreeNode {
public:
	ModRefNameInfo *refs;
	CodeBlockType type;
	int block_id;
	InvocationList *ilist;

	// ------------ construct the tree of information ------------
	ModRefNameTreeNode(CodeBlockType t, int code_block_id, ModRefNameTreeNode *parent) 
           : NonUniformDegreeTreeNode (parent){
		refs = new ModRefNameInfo;
		type = t;
		block_id = code_block_id;
		ilist = new InvocationList;
	};
	~ModRefNameTreeNode() { delete refs; delete ilist; };
};

#endif modrefnametree_h
