/* $Id: MapInfo.h,v 1.3 1997/03/11 14:29:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************
	MapInfo.h

	MapInfo	-- provides a source line mapping between the internal 
		and external forms.

 ****************************************************************************/

#ifndef MapInfo_h
#define MapInfo_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/frontEnd/ast/treeutil.h>

class MapLineEntry;  /* dummy declaration for MapInfo */

/************************************************************************/
/*									*/
/*   Class:  MapInfo                                                    */
/*									*/
/*   Description:  Maps source lines and variables to AST indices.      */
/*                 This is used for a PFC-generated dependence graph.   */
/*									*/
/************************************************************************/


class MapInfo {

    MapLineEntry     *MapEntries;
    SinglyLinkedList *EntryList;
    int               Size;

    SinglyLinkedList *MapLineToVarList(int LineNum);

    public:
      
        MapInfo(FortTextTree ftt);
        ~MapInfo();

        AST_INDEX MapLineToIndex(int LineNum);
        AST_INDEX MapVarToIndex(int Line,
				char *Var,
				int n);
        void ProcessMapLine(FortTextTree ftt,
			    int          ftt_line,
			    int          output_line);
        void ConstructInfo(AST_INDEX           node,
			   ReferenceAccessType atype);



};

#endif	/* MapInfo_h */

