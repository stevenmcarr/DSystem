/* $Id: ClipboardCP.h,v 1.4 1997/03/11 14:30:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/ClipboardCP.h						*/
/*									*/
/*	ClipboardCP -- window showing the Rn global scrap		*/
/*	Last edited: July 17, 1990 at 11:27 am				*/
/*									*/
/************************************************************************/




#ifndef ClipboardCP_h
#define ClipboardCP_h


typedef Generic ClipboardCP;




/************************/
/*  Initialization	*/
/************************/

extern aProcessor	cb_Processor;
EXTERN(void, cbcp_NewCP, (Generic parent, Context     context,
 Generic avail));




/************************/
/*  Change notification	*/
/************************/

EXTERN(void, cbcp_NoteChanges, (ClipboardCP cbcp,
 Boolean autoscroll));




#endif
