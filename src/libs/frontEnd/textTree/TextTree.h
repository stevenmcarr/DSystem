/* $Id: TextTree.h,v 1.13 1997/03/11 14:30:05 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	TextTree.h						*/
/*									*/
/*	TextTree -- text-and-structure view of an abstract syntax tree	*/
/*									*/
/************************************************************************/




#ifndef TextTree_h
#define TextTree_h

	/* Definitions Needed For This .h File	*/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#ifndef font_h
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>
#endif

#ifndef point_h
#include <libs/graphicInterface/support/graphics/point.h>
#endif

#ifndef rect_h
#include <libs/graphicInterface/support/graphics/rect.h>
#endif

typedef Generic TextTree;
typedef Generic TextTreeView;

typedef Generic TT_Tree;
typedef Generic TT_TreeNode;
typedef Generic TT_LineTag;


#include <libs/support/arrays/FlexibleArray.h>

#ifndef nil
#define nil             0    /* null pointer */
#endif

typedef struct{int x[20];} TT_MaxToken;

typedef struct
  {
    TT_TreeNode lineNode;
    int         bracket;
    int		indent;
    int         conceal;
    TT_LineTag	tt_tag;

    TextString	text;
    Boolean	textValid;

    TT_MaxToken	token;		/* size is really instance's 'tokenSize' */

  } TT_Line;


#define tt_EXTRASIZE	5    /* in 'Generic' units */

typedef FUNCTION_POINTER(Boolean, tt_Parse1Func,
 (TT_Tree ftt, TextString text, TT_MaxToken *token));
typedef FUNCTION_POINTER(Boolean, tt_Parse2Func,
 (TT_Tree ftt, int goal, Flex *lines, int first, int count,
  TT_TreeNode *node));
typedef FUNCTION_POINTER(void, tt_Unparse1Func,
 (TT_Tree ftt, TT_Line line, TextString *text));
typedef FUNCTION_POINTER(void, tt_Unparse2Func,
 (TT_Tree ftt, TT_TreeNode node, int indent, Flex **lines));
typedef FUNCTION_POINTER(void, tt_Maptext1Func,
 (TT_Tree ftt, TT_Line line, int firstChar, TT_TreeNode *node));
typedef FUNCTION_POINTER(void, tt_Maptext2Func,
 (TT_Tree ftt, int firstLine, TT_TreeNode *node));
typedef FUNCTION_POINTER(void, tt_Mapnode1Func,
 (TT_Tree ftt, TT_Line line, TT_TreeNode node,
  int *firstChar, int *lastChar));
typedef FUNCTION_POINTER(void, tt_CacheCallback, 
 (TextTree tt, TT_TreeNode node, int firstLine, int lastLine, int indent));
typedef FUNCTION_POINTER(void, tt_Mapnode2Func,
 (TT_Tree ftt, TT_TreeNode node, int *firstLine, int *lastLine,
  int *indent, TextTree tt, tt_CacheCallback cacheProc));
typedef FUNCTION_POINTER(Boolean, tt_Synch2Func,
 (TT_Tree ftt, TT_TreeNode node, int *goal));
typedef FUNCTION_POINTER(void, tt_Copy1Func,
 (TT_Tree ftt, TT_MaxToken oldToken, TT_MaxToken newToken));
typedef FUNCTION_POINTER(void, tt_Copy2Func,
 (TT_Tree ftt, TT_TreeNode oldNode, TT_TreeNode *newNode));
typedef FUNCTION_POINTER(void, tt_Destroy1Func,
 (TT_Tree ftt, TT_MaxToken *token));
typedef FUNCTION_POINTER(void, tt_Destroy2Func,
 (TT_Tree ftt, TT_TreeNode node));
typedef FUNCTION_POINTER(void, tt_SetRootFunc,
 (TT_Tree ftt, TT_TreeNode node));
typedef FUNCTION_POINTER(TT_TreeNode, tt_GetFatherFunc,
 (TT_Tree ftt, TT_TreeNode node));
typedef FUNCTION_POINTER(void, tt_GetExtraFunc,
 (TT_Tree ftt, TT_TreeNode node, int k, int *value));
typedef FUNCTION_POINTER(void, tt_SetExtraFunc,
 (TT_Tree ftt, TT_TreeNode node, int k, int value));

typedef struct
  {
    tt_Parse1Func	parse1;
    tt_Parse2Func	parse2;
    tt_Unparse1Func	unparse1;
    tt_Unparse2Func	unparse2;
    tt_Maptext1Func	maptext1;
    tt_Maptext2Func	maptext2;
    tt_Mapnode1Func	mapnode1;
    tt_Mapnode2Func	mapnode2;
    tt_Synch2Func	synch2;
    tt_Copy1Func	copy1;
    tt_Copy2Func	copy2;
    tt_Destroy1Func	destroy1;
    tt_Destroy2Func	destroy2;
    tt_SetRootFunc	setRoot;
    tt_GetFatherFunc	getFather;
    tt_GetExtraFunc	getExtra;
    tt_SetExtraFunc	setExtra;
  } TT_Methods;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void, tt_Init, (void));
EXTERN(void, tt_Fini, (void));

EXTERN(TextTree, tt_Open, (Context context, DB_FP *fp,
 TT_Tree tree, TT_TreeNode root, TT_Methods *methods, int tokenSize));
EXTERN(void, tt_Close, (TextTree tt));
EXTERN(void, tt_Save, (TextTree tt, Context context, DB_FP *fp));
EXTERN(int, tt_Duplicate, (Context oldContext,
			   Context newContext));

EXTERN(TextTree, tt_Create, 
       (TT_Tree tree, TT_TreeNode root, TT_Methods *methods, int tokenSize));
EXTERN(int, tt_Read, (TextTree tt, DB_FP *fp));
EXTERN(int, tt_Write, (TextTree tt, DB_FP *fp));



/************************/
/*  Contents as tree	*/
/************************/

EXTERN(TT_TreeNode, tt_Root, (TextTree tt));
EXTERN(TT_TreeNode, tt_GetFather, (TextTree tt, TT_TreeNode node));
EXTERN(void, tt_TreeWillChange, (TextTree tt, TT_TreeNode node));
EXTERN(void, tt_TreeChanged, (TextTree tt, TT_TreeNode node));

EXTERN(Boolean, tt_NodeToID, (TextTree tt, TT_TreeNode node,
 int *id));
EXTERN(Boolean, tt_IDToNode, (TextTree tt, int id,
 TT_TreeNode *node));




/************************/
/*  Contents as text	*/
/************************/

EXTERN(Point, tt_GetDocSize, (TextTree tt));
EXTERN(int, tt_NumLines, (TextTree tt));
EXTERN(void, tt_GetLine, (TextTree tt, int lineNum,
 TextString *text));
EXTERN(void, tt_SetLine, (TextTree tt, int lineNum, TextString text));
EXTERN(void, tt_InsertLine, (TextTree tt, int lineNum,
 TextString text));
EXTERN(void, tt_DeleteLine, (TextTree tt, int lineNum));

EXTERN(void, tt_GetLineInfo, (TextTree tt, int lineNum,
 Boolean needText, TT_Line *info));
EXTERN(int, tt_GetLineIndent, (TextTree tt, int lineNum));
EXTERN(int, tt_GetLineLength, (TextTree tt, int lineNum));

EXTERN(void, tt_SetConceal, (TextTree tt, int lineNum1, int lineNum2,
 Boolean conceal));
EXTERN(void, tt_SetConcealNone, (TextTree tt, int lineNum1,
 int lineNum2));
EXTERN(void, tt_GetConceal, (TextTree tt, int lineNum,
 Boolean *conceal));
EXTERN(void, tt_SetConcealCount, (TextTree tt, int lineNum,
 int iconceal));
EXTERN(void, tt_GetConcealCount, (TextTree tt, int lineNum,
 int *iconceal));




/************************/
/*  Mapping		*/
/************************/

EXTERN(Boolean, tt_TextToNode, (TextTree tt, int firstLine,
 int firstChar, int lastLine, int lastChar, TT_TreeNode *node));
EXTERN(Boolean, tt_NodeToText, (TextTree tt, TT_TreeNode node,
 int *firstLine, int *firstChar, int *lastLine, int *lastChar));
EXTERN(void, tt_GetChanges, (TextTree tt, int *first, int *last,
 int *delta, TT_TreeNode *node));




/************************/
/*  Viewing		*/
/************************/

EXTERN(short, tt_ViewScreenModuleIndex, (void));
EXTERN(Point, tt_ViewSize, (Point charSize, short font));
EXTERN(void, tt_ViewInit, (TextTree tt, TextTreeView pane,
 Rectangle viewRect));
EXTERN(void, tt_ViewGet, (TextTree tt, TextTreeView pane,
 Rectangle *viewRect));
EXTERN(void, tt_ViewSet, (TextTree tt, TextTreeView pane,
 Rectangle viewRect));
EXTERN(void, tt_ViewScroll, (TextTree tt, TextTreeView pane,
 int dx, int dy));

EXTERN(void, tt_BeginEdit, (TextTree tt));
EXTERN(void, tt_EndEdit, (TextTree tt));




/************************/
/*  Private		*/
/************************/

EXTERN(void, tt_setTagNode, (TextTree tt, TT_TreeNode node,
 TT_LineTag tag));
EXTERN(void, tt_getTagNode, (TextTree tt, TT_TreeNode node,
 TT_LineTag *tag));
EXTERN(void, tt_setTagLine, (TextTree tt, int lineNum,
 TT_LineTag tag));
EXTERN(void, tt_getTagLine, (TextTree tt, int lineNum, 
TT_LineTag *tag));




#endif
