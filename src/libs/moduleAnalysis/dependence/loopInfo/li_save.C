/* $Id: li_save.C,v 1.1 1997/06/25 15:09:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/********************************************************************	*/
/*									*/
/*	dep/li/li_save.c						*/
/*									*/
/*	li_save.c	-- Responsible for saving the Loop-Info		*/
/*			   Representation into a file.			*/
/*									*/
/*	Exports:							*/
/*	Boolean		li_save_index( )				*/
/*									*/
/*	Locals:								*/
/*	static void	li_save_Slist( )				*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/********************************************************************	*/


/*	*******************************************************************
 *	Include Files:
 */

#include <stdio.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>


/*	*******************************************************************
 *	Local Declarations:
 */

	STATIC(void,	li_save_Slist,( FortTextTree ftt, Slist *var, FILE *outFP ));
/*		Save contents of a node in the shared and private variable
 *	lists to file outFP, using the format conventions defined by PFC.
 */	



/*	**************************************************************** */
/*	================================================================ */
/*	**************************************************************** */

/*	--------------------------------------------------------------------	*/
/*
 * 			      li_save_index.c
 * 									
 *   INVOCATION:
 *	retcode = li_save_index ( ftt, LI, indexFP );
 *
 *   FUNCTION:
 *	Print information about the loop structure of the dependences
 *	into the file pointed to by indexFP.
 *	The representation used is compatible with that output
 *	from PFC.
 *      -- save contents of the shared and private variable lists
 *      to file indexFP, using the format conventions defined by PFC.
 *
 *   RETURNS:
 *	If everything is successful, a  0 is returned.
 *	If there is a problem,       a -1 is returned.
 *
 *   HISTORY:
 *	This code was modeled on the ft2text routine, pretty printer.
 *	Developed April, 1991 by Mike Paleczny.
 *	TimeKey=910425
 *   
 */

Boolean li_save_index(FortTextTree ftt, LI_Instance *LI, FILE *indexFP)
{
    Loop_info   *loopInfo;
    
    int          line;
    int          position;
    int          endingLine;
    int          endingPos;
    Slist       *var;
    
    /*	- - - - - - - START of CODE - - - - - - - - - - - - - - - - - 	*/
    
    
    /*  for each loop in the program, output its loop-location struct
     *	followed by the translation of an Slist structure for each
     *  variable occurring in the loop.
     */
    
    for(loopInfo=LI->Linfo; loopInfo!=NULL; loopInfo=loopInfo->next)
    {
	
	ftt_NodeToText( ftt, loopInfo->loop_hdr_index,
		       &line, &position,
		       &endingLine, &endingPos );
	++line;        /* increment so first line of code is one 	*/
	++endingLine;
	
	/*  output the loop_location information    */
	fprintf( indexFP,"%d %d %d\n", line, 0, loopInfo->loop_level );
	
	/* for each variable in loop, output Slist info  */
	for( var=loopInfo->shvar_list; var != NULL; var=var->next )
	{
	    li_save_Slist( ftt, var, indexFP );
	}
	
	/* for each variable in loop, output Slist info  */
	for( var=loopInfo->pvar_list; var != NULL; var=var->next )
	{
	    li_save_Slist( ftt, var, indexFP );
	}
	
    }
    
    return true;
} /* end_li_save_index */




/* --------------------------------------------------------------------
 *	  li_save_Slist:  -- save contents of a node in the shared and private
 *	  variable lists to file outFP, using the format conventions defined by PFC.
 *	  
 * -------------------------------------------------------------------- */
static void	li_save_Slist(FortTextTree ftt, Slist *var, FILE *outFP)
{
    int      line;
    int      position;
    int      endingLine;
    int      endingPos;
    
    /* field  1  */
    fprintf( outFP,"%.30s!", var->name );        /* identifier   */
    
    
    if( var->def_before != AST_NIL )            /* field  2  */
    {
	ftt_NodeToText( ftt, var->def_before,
		       &line, &position,
		       &endingLine, &endingPos );
	line++;
	fprintf( outFP,"%d!", line  );          /* line # of previous def */
    }
    else
    {
	fprintf( outFP,"!" );
    }
    
    
    if( var->use_after != AST_NIL )             /* field  3  */
    {
	ftt_NodeToText( ftt, var->use_after,
		       &line, &position,
		       &endingLine, &endingPos );
	line++;
	fprintf( outFP,"%d!", line  );          /* line # of next use  */
    }
    else
    {
	fprintf( outFP,"!" );
    }
    
    /* field  4  */
    fprintf( outFP,"%d!", var->why  );          /* variable type I   */
    
    /* field  5  */
    fprintf( outFP,"%.30s!", var->cblock  );    /* variable type II  */
    
    /* field  6  */
    fprintf( outFP,"%d\n", var->dim  );         /* subscript dimensions */
    
}  /* end_li_save_Slist */
