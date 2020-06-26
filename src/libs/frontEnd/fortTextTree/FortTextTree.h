/* $Id: FortTextTree.h,v 1.16 2000/01/12 23:13:42 mjbedy Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTextTree.h						*/
/*									*/
/*	FortTextTree -- text-and-structure view of a FortTree		*/
/*									*/
/************************************************************************/




#ifndef FortTextTree_h
#define FortTextTree_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTree/FortTree.h>

#include <libs/frontEnd/prettyPrinter/sigcomments.h>

typedef Generic FortTextTree;
typedef Generic FortTextTreeView;

#include <libs/frontEnd/fortTextTree/MapInfo_c.h>


/* line bracketing kinds  -- must be the same as in TextTree.h! */

#define ftt_SIMPLE	0
#define ftt_OPEN	1
#define ftt_CLOSE	2





/************************/
/*  Initialization	*/
/************************/

EXTERN(void, ftt_Init, (void));
EXTERN(void, ftt_Fini, (void));

EXTERN(FortTextTree, ftt_Open,
       (Context context, DB_FP *fp, FortTree ft));
EXTERN(void, ftt_Close, (FortTextTree ftt));
EXTERN(void, ftt_Save, (FortTextTree ftt, Context context, DB_FP *fp));
EXTERN(void, ftt_SaveAnnots, (FortTextTree ftt, Context context, DB_FP *fp));



/************************/
/*  Contents as tree	*/
/************************/

EXTERN(FortTree, ftt_Tree, (FortTextTree ftt));
EXTERN(FortTreeNode, ftt_GetFather, (FortTextTree ftt, FortTreeNode node));
EXTERN(FortTreeNode, ftt_Root, (FortTextTree ftt));
EXTERN(void, ftt_TreeWillChange, (FortTextTree ftt, FortTreeNode node));
EXTERN(void, ftt_TreeChanged, (FortTextTree ftt, FortTreeNode node));

EXTERN(Boolean, ftt_NodeToID, (FortTextTree ftt, FortTreeNode node, int *id));
EXTERN(Boolean, ftt_IDToNode, (FortTextTree ftt, int id, FortTreeNode *node));
EXTERN(char *, ftt_NodeToStr, (FortTextTree ftt, FortTreeNode node));



/************************/
/*  Contents as text	*/
/************************/

#define ftt_PLACEHOLDER_STYLE	    STYLE_ITALIC
#define	ftt_BEGIN_PLACEHOLDER_STYLE 128

EXTERN(Point, ftt_GetDocSize, (FortTextTree ftt));
EXTERN(int, ftt_NumLines, (FortTextTree ftt));
EXTERN(void, ftt_GetLine, (FortTextTree ftt, int lineNum,
 TextString *text));
EXTERN(char *, ftt_GetTextLine, (FortTextTree ftt, int lineNum));
EXTERN(void, ftt_SetLine, (FortTextTree ftt, int lineNum,
 TextString text));
EXTERN(void, ftt_SetTextLine, (FortTextTree ftt, int lineNum,
 char *text));
EXTERN(void, ftt_InsertLine, (FortTextTree ftt, int lineNum,
 TextString text));
EXTERN(void, ftt_InsertTextLine, (FortTextTree ftt, int lineNum,
 char *text));
EXTERN(void, ftt_DeleteLine, (FortTextTree ftt, int lineNum));

EXTERN(void, ftt_GetLineInfo, (FortTextTree ftt, int lineNum,
 FortTreeNode *node, int *bracket));
EXTERN(int, ftt_GetLineIndent, (FortTextTree ftt, int lineNum));
EXTERN(int, ftt_GetLineLength, (FortTextTree ftt, int lineNum));

EXTERN(void, ftt_GetConceal, (FortTextTree ftt, int lineNum,
 Boolean *conceal));
EXTERN(void, ftt_SetConceal, (FortTextTree ftt, int lineNum1,
 int lineNum2, Boolean conceal));
EXTERN(void, ftt_SetConcealNone, (FortTextTree ftt, int lineNum1,
 int lineNum2));
EXTERN(void, ftt_SetConcealCount, (FortTextTree ftt, int lineNum,
 int iconceal));
EXTERN(void, ftt_GetConcealCount, (FortTextTree ftt, int lineNum,
 int *iconceal));

EXTERN(Boolean, ftt_IsErroneous, (FortTextTree ftt, int lineNum));
EXTERN(void, ftt_GetErrorMessage, (FortTextTree ftt, int lineNum,
 char *Message));



/************************/
/*  Mapping		*/
/************************/

EXTERN(Boolean, ftt_TextToNode, (FortTextTree ftt, int line1,
 int char1, int line2, int char2, FortTreeNode *node));
EXTERN(Boolean, ftt_NodeToText, (FortTextTree ftt, FortTreeNode node,
 int *line1, int *char1, int *line2, int *char2));
EXTERN(void, ftt_GetChanges, (FortTextTree ftt, int *first, int *last,
 int *delta, FortTreeNode *node));
EXTERN(void, ftt_MapWalk, (FortTextTree ftt, MapInfoOpaque Map));




/************************/
/*  Expanding		*/
/************************/


EXTERN(int, ftt_IPExpandee, (FortTextTree ftt, int lineNum,
 int charNum));
EXTERN(int, ftt_NodeExpandee, (FortTextTree ftt, FortTreeNode node));
EXTERN(int, ftt_GetExpansionNames, (FortTextTree ftt,
 Boolean firstOnly, TextString nameList[], int numList[]));

EXTERN(void, ftt_Expand, (FortTextTree ftt, int choice,
 FortTreeNode *newNode, FortTreeNode *focus));




/************************/
/*  Viewing		*/
/************************/

EXTERN(short, ftt_ViewScreenModuleIndex, (void));
EXTERN(Point, ftt_ViewSize, (Point charSize, short font));
EXTERN(void, ftt_ViewInit, (FortTextTree ftt, FortTextTreeView pane,
 Rectangle viewRect));
EXTERN(void, ftt_ViewGet, (FortTextTree ftt, FortTextTreeView pane,
 Rectangle *viewRect));
EXTERN(void, ftt_ViewSet, (FortTextTree ftt, FortTextTreeView pane,
 Rectangle viewRect));
EXTERN(void, ftt_ViewScroll, (FortTextTree ftt, FortTextTreeView pane,
 int dx, int dy));

EXTERN(void, ftt_BeginEdit, (FortTextTree ftt));
EXTERN(void, ftt_EndEdit, (FortTextTree ftt));

/****************************/
/*  Importing and Exporting */
/****************************/

EXTERN(void, ftt_ImportFromFile,
		(Context context, FortTree ft, FortTextTree *ftt_p));

EXTERN(void, ftt_ImportFromTextFile, (char *filename, FortTextTree ftt));

EXTERN(FortTextTree, ftt_Create, (FortTree ft));
EXTERN(int, ftt_Read, (FortTextTree ftt, DB_FP *file));
EXTERN(int, ftt_Write, (FortTextTree ftt, DB_FP *file));

EXTERN(void, ftt_ExportToFile,
		(FortTextTree ftt,
		 FILE        *outf,
		 char         continuationCharacter,
		 SignificantCommentHandlers *schandlers));

typedef
FUNCTION_POINTER(void, PushFunc,   
		 (char *include_name, Generic other));
typedef
FUNCTION_POINTER(void, PopFunc,   
		 (char *include_name, Generic other));
typedef
FUNCTION_POINTER(void, EnterFunc,   
		 (char *line, Generic other));


EXTERN(void, ftt_TraverseText,
                (char        *loc,
		 PushFunc  Push,
		 PopFunc   Pop,
		 EnterFunc Enter,
		 Generic   other));

#if defined(OSF1) || defined(LINUX_ALPHA)

#include <include/rn_varargs.h>
#include <stdarg.h>

typedef
FUNCTION_POINTER(void, PushFuncV,   
		 (char *include_name, va_list args));
typedef
FUNCTION_POINTER(void, PopFuncV,   
		 (char *include_name, va_list args));
typedef
FUNCTION_POINTER(void, EnterFuncV,   
		 (char *line, va_list args));


EXTERN(void, ftt_TraverseTextV,
                (char        *loc,
		 PushFuncV  Push,
		 PopFuncV   Pop,
		 EnterFuncV Enter,
		 va_list    args));


#endif
/************************/
/*  Error Reporting	*/
/************************/


typedef FUNCTION_POINTER(int, ftt_ListingCallBack, 
			 (int lineCount, char *text, va_list args));    

EXTERN(void, ftt_Listing, (FortTextTree ftt, 
			   ftt_ListingCallBack reportGoodLine, 
			   ftt_ListingCallBack reportBadLine, 
			   ftt_ListingCallBack reportErrorLine, ...));

EXTERN(int, ftt_Export, (FortTextTree ftt,  int continuationCharacter, 
			  SignificantCommentHandlers *schandlers, 
			  ftt_ListingCallBack exportLine, ...));

#if 0
EXTERN(void, ftt_banner,
		(char *banner_msg));
EXTERN(void, ftt_badReporter,
		(int lineno, char *linetext, char *errortext));
EXTERN(void, ftt_goodReporter,
		(int lineno, char *linetext));
#endif

typedef
FUNCTION_POINTER(void, ftt_banner_callback,   
		 (char *banner_msg));
typedef
FUNCTION_POINTER(void, ftt_goodline_callback,
		 (int lineCount, char *lineText));
typedef
FUNCTION_POINTER(void, ftt_badline_callback,
		 (int lineCount, char *lineText, char *errorText));

EXTERN(void, ftt_ReportErrors, 
		(Context context,
		 FortTextTree ftt,
		 ftt_banner_callback banner,
		 ftt_goodline_callback good,
		 ftt_badline_callback   bad,
		 Boolean *has_errors_p));




/************************/
/*  Private		*/
/************************/

EXTERN(void, ftt_getTextTree, (FortTextTree ftt, TextTree *tt));


#endif
