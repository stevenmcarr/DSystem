/* $Id: change.C,v 1.1 1997/06/25 14:42:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/change.c					*/
/*									*/
/*	change -- notify and update changes				*/
/*           pedcp_NoteChanges                                          */ 
/*           pedcp_TreeWillChange                                       */
/*	     pedcp_TreeChanged                                          */
/*	     pedcp_SelectNode                                           */
/*	     pedcp_GetLine                                              */
/*	     pedcp_UpdateNodeInfo                                       */
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ned_notify.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/frontEnd/ast/groups.h>

/************************/
/*  Change notification	*/
/************************/


/*ARGSUSED*/
void pedcp_NoteChanges(PEditorCP pedcp, int kind, Boolean autoScroll, FortTreeNode xnode,
		       int first, int last, int delta)
//     PEditorCP    pedcp;
//     int          kind;
//     Boolean      autoScroll;
//     FortTreeNode xnode;
//     int 	 first, last, delta;
{
    int line, sel1, sel2;
    			/* a new selection */
    FortTreeNode node;

    if (kind == NOTIFY_SEL_CHANGED)  /* only selection changed */
    {
	if( !autoScroll )
	{ /* show it */
	    ed_GetSelection(R(pedcp)->editor, &line, &sel1, &sel2);
	    if (line == UNUSED)
		ed_ViewEnsureVisible(R(pedcp)->editor, R(pedcp)->srcPane, 
				     makePoint(0, (sel1 + sel2)/2));
	    else
		ed_ViewEnsureVisible(R(pedcp)->editor, R(pedcp)->srcPane, 
				     makePoint(0, line));
	}
	
	ed_GetSelectedNode(R(pedcp)->editor, &node);
	
	/* if this is a list node */
	if (is_list(node))
	    node = list_first(node);
	
	if (R(pedcp)->ped)
	    pedUpdate((PedInfo)R(pedcp)->ped, node);
	
    }
    else if ( kind == NOTIFY_DOC_CHANGED || kind == NOTIFY_SEL_CHANGED )
    { 
      if ( autoScroll )
      {
	ed_GetSelection(R(pedcp)->editor, &line, &sel1, &sel2);
	if( line != UNUSED  &&  sel1 > sel2 )
	{/* why scroll just for insertion points? */
	    ed_ViewEnsureVisible(R(pedcp)->editor, R(pedcp)->srcPane,
				 makePoint(sel1-1, line));
	}
      }

      if( kind == NOTIFY_DOC_CHANGED )  
	pedcp_markChanged(pedcp);

      sdlg_NoteChange(R(pedcp)->searcher,
		      kind, autoScroll, node, first, last, delta);

    }
    else
    {
      fprintf(stderr, "Unexpected notification kind in pedcp_NoteChanges\n");
    }
}

/*ARGSUSED*/
void pedcp_TreeWillChange(PEditorCP pedcp, FortTreeNode node)
//     PEditorCP    pedcp;
//     FortTreeNode node;
{
    ed_TreeWillChange(R(pedcp)->editor, node);
}

/*ARGSUSED*/
void pedcp_TreeChanged(PEditorCP pedcp, FortTreeNode node)
//     PEditorCP pedcp;
//     FortTreeNode node;
{
    ed_TreeChanged(R(pedcp)->editor, node);
}

void pedcp_SelectNode(PEditorCP pedcp, FortTreeNode node)
//     PEditorCP pedcp;
//     FortTreeNode node;
{
    int line1, line2, char1, char2;
    int oldline, oldsel1, oldsel2;
    
    /* where is this node? */
    ed_NodeToText(R(pedcp)->editor, node, &line1, &char1, &line2, &char2);
    
    /* what is selected?   */
    ed_GetSelection(R(pedcp)->editor, &oldline, &oldsel1, &oldsel2);
    
    if (oldline == UNUSED)
    {/* shift cursor - 6 approximates the do indentation - this needs
	to be made more exact somehow */
	ed_SetSelection(R(pedcp)->editor, line1, 7, 6);
    }
    else
    {/* a single line selected */
	ed_SetSelection(R(pedcp)->editor, line1, 7, 6);
    }
}

char  *
pedcp_GetLine(PEditorCP pedcp, FortTreeNode node)
//     PEditorCP    pedcp;
//     FortTreeNode node;
{
    int sLine, sChar, eLine, eChar;
    
    ed_NodeToText(R(pedcp)->editor, node, &sLine, &sChar, &eLine, &eChar);
    return (char *) ed_GetTextLine(R(pedcp)->editor, sLine);
}



/*------------------------------------------------------------------------

   UpdateNodeInfo takes a node of the ast which has changed and updates
   all the information needed. Currently it updates the subscript side
   array.

   KEF - Oct. 1989 
   cwt Mar 90 - add updates dep vector info 

*/

void pedcp_UpdateNodeInfo(PEditorCP pedcp, AST_INDEX node, int opts)
//     PEditorCP  pedcp;
//     AST_INDEX  node;		/* current loop	*/
//     int        opts;		/* options	*/
{ 
    PedInfo    ped;
    AST_INDEX  outer_loop;	/* outermost loop	*/
    
    ped = (PedInfo) R(pedcp)->ped;
    
    /*-----------------------*/
    /* update subscript info */
    
    if (opts & UPDATE_SUBS)
    {
      subs_attach( PED_ROOT(ped), PED_FTT(ped), PED_INFO(ped));
    }
    
    /*--------------------------------------*/
    /* update DG / dep vector info			*/
    
    if ((opts & UPDATE_DTINFO) || (opts & UPDATE_DG))
    {
	/* find outermost loop containing region, 			*/
	/* then update everything within				*/
	
	for (outer_loop = node; node != AST_NIL; node = out(node))
	{
	    if (is_loop(node))
		outer_loop = node;	    
	}
	
	/*----------------------------------*/
	/* updates loops/deps within region	*/
	
	if (opts & UPDATE_DG)
	{
	    ped_dg_update(ped, outer_loop);		/* update DG	*/
	}
	else
	{
	    ped_dt_update(ped, outer_loop);		/* update DT	*/
	}
    }
    
}
