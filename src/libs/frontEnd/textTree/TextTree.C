/* $Id: TextTree.C,v 1.1 1997/06/24 17:54:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/TextTree/TextTree.c					*/
/*									*/
/*	TextTree -- text-and-structure view of an abstract syntax tree	*/
/*	Last edited: October 16, 1992 at 11:43 am			*/
/*									*/
/************************************************************************/




/* #include <ned.h> */
#include <libs/frontEnd/textTree/TextTree.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/FlexibleArray.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>

#include <libs/support/database/context.h>




/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* temporary */

typedef struct
  {
    int			id;
    TT_TreeNode		node;

  } TableElement;




typedef struct
  {
    int			first;
    int			last;
    int			delta;
    TT_TreeNode		node;

    TT_TreeNode		firstNode;
    TT_TreeNode		lastNode;
    int			indent;		/* only for 'cachedMapnode2' */

  } Changes;




typedef struct
  {
    /* creation parameters */
      TT_Methods	methods;
      int		tokenSize;
      TT_Tree		tree;
      TT_TreeNode	root;

    /* hi-lo parser interface */
      Flex *		lines;
      int		maxWidth;

    /* changes */
      int		freeze;
      Boolean		updateDeferred;
      Boolean		updateFromLines;

      Changes		pending;
      Changes		done;

      Flex *		linesToDestroy;

    /* mapping etc */
      int		cacheID;	/* for 'mapnode2' */

      int		nextNodeID;
      Flex *		hashtable;

  } tt_Repr;


#define	R(ob)		((tt_Repr *) ob)







/************************/
/* Miscellaneous	*/
/************************/




#define INFINITY	9999		/* needs to fit in a 'short' */




static Changes tt_noChanges =
  {
    INFINITY,	/* first     */
    -1,		/* last      */
    0,		/* delta     */
    nil,	/* node      */
    nil,	/* firstNode */
    nil,	/* lastNode  */
    UNUSED	/* indent    */
  };






/************************/
/* Forward declarations	*/
/************************/




STATIC(void,            initLine,(TextTree tt, int lineNum, TextString text));
STATIC(void,            finiLine,(TextTree tt, int lineNum, Boolean destroyToken,
                                  Boolean defer));
STATIC(void,            updateFromLines,(TextTree tt));
STATIC(void,            updateFromTree,(TextTree tt));
STATIC(void,            updateLines,(TextTree tt, TT_TreeNode stmtNode, int firstLine,
                                     int oldLastLine, int *newLastLine, int indent));
STATIC(void,            noChanges,(void));
STATIC(void,            noteChangedLines,(TextTree tt, int first, int last, int delta));
STATIC(void,            includePendingNode,(TextTree tt, int lineNum,
                                            TT_TreeNode *lineNode));
STATIC(void,            noteChangedTree,(TextTree tt, TT_TreeNode node));
STATIC(void,            doneChanging,(TextTree tt, int first, int last, int delta));
STATIC(void,            initHashtable,(TextTree tt));
STATIC(void,            finiHashtable,(TextTree tt));
STATIC(void,            readHashtable,(TextTree tt, DB_FP *fp));
STATIC(void,            writeHashtable,(TextTree tt, DB_FP *fp));
STATIC(void,            enterInHashtable,(TextTree tt, int id, TT_TreeNode node));
STATIC(int,             lookInHashtable,(TextTree tt, int id, TT_TreeNode *node));

STATIC(Boolean,         parse1,(TextTree tt, TextString text, TT_MaxToken *token));
STATIC(Boolean,         parse2,(TextTree tt, int goal, Flex *lines, int start,
                                int count, TT_TreeNode *node));
STATIC(void,            unparse1,(TextTree tt, TT_Line line, TextString *text));
STATIC(void,            unparse2,(TextTree tt, TT_TreeNode node, int indent, Flex **lines));
STATIC(void,            maptext1,(TextTree tt, TT_Line line, int firstChar,
                                  TT_TreeNode *node));
STATIC(void,            maptext2,(TextTree tt, int firstLine, TT_TreeNode *node));
STATIC(void,            mapnode1,(TextTree tt, TT_Line line, TT_TreeNode node,
                                  int *firstChar, int *lastChar));
STATIC(void,            mapnode2,(TextTree tt, TT_TreeNode node, int *firstLine,
                                  int *lastLine, int *indent));
STATIC(Boolean,         synch2,(TextTree tt, TT_TreeNode node, int *goal));
STATIC(void,            copy1,(TextTree tt, TT_Line *oldLine, TT_Line *newLine));
STATIC(void,            copy2,(TextTree tt, TT_TreeNode oldNode, TT_TreeNode *newNode));
STATIC(void,            destroy1,(TextTree tt, TT_Line *line));
STATIC(void,            destroy2,(TextTree tt, TT_TreeNode node));
STATIC(void,            setRoot,(TextTree tt, TT_TreeNode node));
STATIC(TT_TreeNode,     getFather,(TextTree tt, TT_TreeNode node));
STATIC(void,            getExtra,(TextTree tt, TT_TreeNode node, int k, int *value));
STATIC(void,            setExtra,(TextTree tt, TT_TreeNode node, int k, int value));

STATIC(TT_TreeNode,     lowestCommonAncestor,(TextTree tt, TT_TreeNode node1,
                                              TT_TreeNode node2));
STATIC(int,             distanceToRoot,(TextTree tt, TT_TreeNode node));
STATIC(Boolean,         getLineToken,(TextTree tt, int lineNum, TT_Line *line,
                                      Boolean needText));
STATIC(Boolean,         cachedMapnode2,(TextTree tt, TT_TreeNode node,
                                        TT_TreeNode *stmtNode, int *firstLine,
                                        int *lastLine, int *indent));
STATIC(Boolean,         cachedMaptext2,(TextTree tt, int firstLine, int lastLine,
                                        TT_TreeNode *node));
STATIC(void,            deferLineDestruction,(TextTree tt, TT_Line *line));
STATIC(void,            performDeferredDestruction,(TextTree tt));
STATIC(void,            enterInCache,(TextTree tt, TT_TreeNode node, int first,
                                      int last, int delta));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void tt_Init()
{
  /* nothing */
}




void tt_Fini()
{
  /* nothing */
}




/*ARGSUSED*/

TextTree tt_Open(Context context, DB_FP *fp, TT_Tree tree, TT_TreeNode root, 
                 TT_Methods *methods, int tokenSize)
{
  TextTree tt;
  Flex * table;

  /* allocate a new instance */
    tt = (TextTree) get_mem(sizeof(tt_Repr),"TextTree");
    if( (Generic) tt == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(tt)->tree = tree;
      R(tt)->root = root;
      R(tt)->methods = *methods;
      R(tt)->tokenSize = tokenSize;

    /* line array */
      R(tt)->maxWidth = UNUSED;
      unparse2(tt, root, 0, &R(tt)->lines);

  /* initialize the changes info */
    R(tt)->freeze = 0;
    R(tt)->updateDeferred = false;
    R(tt)->pending = tt_noChanges;
    R(tt)->done    = tt_noChanges;
    R(tt)->linesToDestroy = flex_create(sizeof(TT_Line));

  /* initialize the mapping info */
    /* mapping cache */
      if( fp == DB_NULLFP )
        R(tt)->cacheID = 1;
      else
        (void) db_buffered_read(fp, (char *) &R(tt)->cacheID, sizeof(int));

    /* statement id stuff */
      initHashtable(tt);

      if( fp == DB_NULLFP )
        R(tt)->nextNodeID = 1;
      else
        { (void) db_buffered_read(fp, (char *) &R(tt)->nextNodeID, sizeof(int));
          readHashtable(tt, fp);
        }

  return tt;
}




TextTree tt_Create(TT_Tree tree, TT_TreeNode root, 
                 TT_Methods *methods, int tokenSize)
{
  TextTree tt;
  Flex * table;
  
  /* allocate a new instance */
  tt = (TextTree) get_mem(sizeof(tt_Repr),"TextTree");
  if( (Generic) tt == 0 ) return UNUSED;
  
  /* initialize the parts */
  /* set creation parameters */
  R(tt)->tree = tree;
  R(tt)->root = root;
  R(tt)->methods = *methods;
  R(tt)->tokenSize = tokenSize;
  
  /* line array */
  R(tt)->maxWidth = UNUSED;
  unparse2(tt, root, 0, &R(tt)->lines);
  
  /* initialize the changes info */
  R(tt)->freeze = 0;
  R(tt)->updateDeferred = false;
  R(tt)->pending = tt_noChanges;
  R(tt)->done    = tt_noChanges;
  R(tt)->linesToDestroy = flex_create(sizeof(TT_Line));
  
  /* initialize the mapping info */
  /* mapping cache */
  R(tt)->cacheID = 1;
  
  /* statement id stuff */
  initHashtable(tt);
  
  R(tt)->nextNodeID = 1;
  
  return tt;
}




int tt_Read(TextTree tt, DB_FP *fp)
{
  (void) db_buffered_read(fp, (char *) &R(tt)->cacheID, sizeof(int));
  
  (void) db_buffered_read(fp, (char *) &R(tt)->nextNodeID, sizeof(int));
  readHashtable(tt, fp);
  
  return 0;
}




int tt_Write(TextTree tt, DB_FP *fp)
{
  /* save persistent information */
  (void) db_buffered_write(fp, (char *) &R(tt)->cacheID, sizeof(int));
  (void) db_buffered_write(fp, (char *) &R(tt)->nextNodeID, sizeof(int));
  writeHashtable(tt, fp);
  return 0 ; /* success ?! */
}




void tt_Close(TextTree tt)
{
  int k;
  int numLines = flex_length(R(tt)->lines);
  
  /* destroy statement id stuff */
  finiHashtable(tt);
  
  /* destroy line descriptions */
  for( k = 0;  k < numLines;  k++ )
    /*
      -- JMC 10/93 changed to destroy only the text descriptions, not 
      the underlying token representation 
      finiLine(tt,k,true,false);
    */
    finiLine(tt,k,false,false);
  
  /* 
    destroy2(tt,R(tt)->root); 
    -- JMC 8/93 commented out since this destroys an ft associated with
    an ftt (and thus this tt); tt should only destroy itself, not things
    to which it has been attached.
    */
  
  flex_destroy(R(tt)->lines);
  flex_destroy(R(tt)->linesToDestroy);
  free_mem((void*) tt);
}




/*ARGSUSED*/

void tt_Save(TextTree tt, Context context, DB_FP *fp)
{
  /* save persistent information if appropriate */
    if( context != CONTEXT_NULL )
      { (void) db_buffered_write(fp, (char *) &R(tt)->cacheID, sizeof(int));
        (void) db_buffered_write(fp, (char *) &R(tt)->nextNodeID, sizeof(int));
        writeHashtable(tt, fp);
      }
}









/************************/
/*  Contents as tree	*/
/************************/




TT_TreeNode tt_Root(TextTree tt)
{
  return R(tt)->root;
}




TT_TreeNode tt_GetFather(TextTree tt, TT_TreeNode node)
{
  return getFather(tt, node);
}




void tt_TreeWillChange(TextTree tt, TT_TreeNode node)
{
  int first,last,indent,dummy;

  /* ASSERT: no pending changes now */

  while( ! synch2(tt, node, &dummy) )  node = getFather(tt, node);
  mapnode2(tt, node, &first, &last, &indent);
  R(tt)->pending.node   = node;
  R(tt)->pending.first  = first;
  R(tt)->pending.last   = last;
  R(tt)->pending.indent = indent;
}




void tt_TreeChanged(TextTree tt, TT_TreeNode node)
{
  noteChangedTree(tt,node);
  updateFromTree(tt);
}




Boolean tt_NodeToID(TextTree tt, TT_TreeNode node, int *id)
{
  getExtra(tt, node, 4, id);

  if( *id == 0 )
    { *id = R(tt)->nextNodeID ++;
      tt_setTagNode(tt, node, *id);
    }

  return true;		/* arf, should be false if not a stmt */
}




Boolean tt_IDToNode(TextTree tt, int id, TT_TreeNode *node)
{
  int real_id;

  (void) lookInHashtable(tt, id, node);

  /* validate the table entry */
    if( *node != UNUSED )
      { tt_getTagNode(tt, *node, &real_id);
        if( real_id != id )  *node = UNUSED;
      }

  return BOOL( *node != UNUSED );
}










/************************/
/*  Contents as text	*/
/************************/




Point tt_GetDocSize(TextTree tt)
{
  Flex * lines = R(tt)->lines;
  int len = flex_length(lines);
  int k, width;

/* TEMPORARY */
R(tt)->maxWidth = 999;
/********************************
  if( R(tt)->maxWidth == UNUSED )
    { R(tt)->maxWidth = 0;
      for( k = 0;  k < len;  k++ )
        { width = tt_GetLineLength(tt, k);
          if( R(tt)->maxWidth < width )
            R(tt)->maxWidth = width;
        }
    }
*********************************/

  return makePoint(R(tt)->maxWidth,len);
}




int tt_NumLines(TextTree tt)
{
  return flex_length(R(tt)->lines);
}




void tt_GetLine(TextTree tt, int lineNum, TextString *text)
{
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,true) )
    *text = copyTextString(line.text);
  else
    *text = emptyTextString;
}




void tt_SetLine(TextTree tt, int lineNum, TextString text)
{
  TT_Line line;
  int indent, conceal, tt_tag;

  noteChangedLines(tt,lineNum,lineNum,0);

  /* save the old line details */
    (void) getLineToken(tt,lineNum,&line,false);
    indent  = line.indent;
    conceal = line.conceal;
    tt_tag  = line.tt_tag;

  finiLine(tt,lineNum,true,true);
  initLine(tt,lineNum,text);

  /* restore the old line details */
    (void) getLineToken(tt,lineNum,&line,false);
    line.indent  = indent;
    line.conceal = conceal;
    line.tt_tag  = tt_tag;
    flex_set_one(R(tt)->lines, lineNum, (char *) &line);

    /* also copy tt_tag into the new line's node */
      tt_setTagNode(tt, line.lineNode, tt_tag);

  updateFromLines(tt);
}




void tt_InsertLine(TextTree tt, int lineNum, TextString text)
{
  TT_Line line;

  noteChangedLines(tt, lineNum, lineNum-1, 1);

  /* insert the new line */
    (void) flex_insert(R(tt)->lines,lineNum,1);
    initLine(tt,lineNum,text);

  /* default indentation is same as previous line, cf. 'updateLines' */
    (void) getLineToken(tt,lineNum,&line,false);
    line.indent = tt_GetLineIndent(tt, lineNum-1);    /* works even if 'lineNum == 0' */
    flex_set_one(R(tt)->lines, lineNum, (char *) &line);

  updateFromLines(tt);
}




void tt_DeleteLine(TextTree tt, int lineNum)
{
  noteChangedLines(tt,lineNum,lineNum,-1);

  finiLine(tt,lineNum,true,true);
  (void) flex_delete(R(tt)->lines,lineNum,1);

  updateFromLines(tt);
}




void tt_GetLineInfo(TextTree tt, int lineNum, Boolean needText, TT_Line *info)
{
  (void) getLineToken(tt,lineNum,info,needText);
}




int tt_GetLineIndent(TextTree tt, int lineNum)
{
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,true) )
    return line.indent;
  else
    return 0;
}




int tt_GetLineLength(TextTree tt, int lineNum)
{
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,true) )
    return line.text.num_tc;
  else
    return 0;
}




void tt_SetConceal(TextTree tt, int lineNum1, int lineNum2, Boolean conceal)
{
  Flex * lines = R(tt)->lines;
  int k;
  TT_Line line;

  for( k = lineNum1;  k <= lineNum2;  k++ )
    if( getLineToken(tt,k,&line,false) )
      { if( conceal )
          line.conceal = min(255,line.conceal+1);
        else
          line.conceal = max(0,line.conceal-1);
        flex_set_one(lines, k, (char *) &line);
      }

  noteChangedLines(tt,lineNum1,lineNum2,0);
}




void tt_SetConcealNone(TextTree tt, int lineNum1, int lineNum2)
{
  Flex * lines = R(tt)->lines;
  int k;
  TT_Line line;

  for( k = lineNum1;  k <= lineNum2;  k++ )
    if( getLineToken(tt,k,&line,false) )
      { line.conceal = 0;
        flex_set_one(lines, k, (char *) &line);
      }

  noteChangedLines(tt,lineNum1,lineNum2,0);
}




void tt_GetConceal(TextTree tt, int lineNum, Boolean *conceal)
{
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,false) )
    *conceal = BOOL(line.conceal > 0);
  else
    *conceal = false;
}




void tt_SetConcealCount(TextTree tt, int lineNum, int iconceal)
{
  Flex * lines = R(tt)->lines;
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,false) )
      { line.conceal = iconceal;
        flex_set_one(lines, lineNum, (char *) &line);
      }

  noteChangedLines(tt,lineNum,lineNum,0);
}




void tt_GetConcealCount(TextTree tt, int lineNum, int *iconceal)
{
  TT_Line line;

  if( getLineToken(tt,lineNum,&line,false) )
    *iconceal = line.conceal;
}






/************************/
/*  Mapping		*/
/************************/




Boolean tt_TextToNode(TextTree tt, int firstLine, int firstChar, int lastLine, 
                      int lastChar, TT_TreeNode *node)
{
  int maxLine = flex_length(R(tt)->lines) - 1;
  TT_Line line;
  TT_TreeNode node1,node2,trialNode;
  int firstResult,lastResult,dummy;

  firstLine = MAX(0,MIN(maxLine,firstLine));
  lastLine  = MAX(0,MIN(maxLine,lastLine ));

  if( firstChar == UNUSED  &&  lastChar == UNUSED )
    { maptext2(tt, firstLine, &node1);
      maptext2(tt, lastLine,  &node2);
      *node = lowestCommonAncestor(tt,node1,node2);
      mapnode2(tt, *node, &firstResult, &lastResult, &dummy);
      if( firstResult == firstLine && lastResult == lastLine )
        { /* the result is the range -- maximize */
            trialNode = *node;
            while( firstResult == firstLine  &&  lastResult == lastLine )
              { *node = trialNode;
                trialNode = getFather(tt, trialNode);
                mapnode2(tt, trialNode, &firstResult, &lastResult, &dummy);
              }
           return true;
        }
      else
        { /* the result is not a selection */
           return false;
        }
    }
  else
    { (void) flex_get_buffer(R(tt)->lines,firstLine,1,(char *) &line);
      maptext1(tt,line,firstChar,&node1);
      maptext1(tt,line,lastChar, &node2);
      *node = lowestCommonAncestor(tt,node1,node2);
      mapnode1(tt,line,*node,&firstResult,&lastResult);  /* same line */
      if( firstResult == firstChar && lastResult == lastChar )
        { /* the result is a selection -- maximize */
            trialNode = *node;
            while( firstResult == firstChar  &&  lastResult == lastChar )
              { *node = trialNode;
                trialNode = getFather(tt, trialNode);
                mapnode1(tt,line,trialNode,&firstResult,&lastResult);
              }
           return true;
        }
      else
        { /* the result is not a selection */
           return false;
        }
    }
}




Boolean tt_NodeToText(TextTree tt, TT_TreeNode node, int *firstLine, int *firstChar, 
                      int *lastLine, int *lastChar)
{
  TT_Line line;
  TT_TreeNode trialNode;
  int trialLine,dummy;

  mapnode2(tt, node, firstLine, lastLine, &dummy);
  if( *firstLine != *lastLine  ||  *firstLine != UNUSED )
    { *firstChar = UNUSED;
      *lastChar  = UNUSED;
      if( *firstLine == *lastLine )
        { maptext2(tt, *firstLine, &trialNode);
          if( trialNode == node )
            { (void) flex_get_buffer(R(tt)->lines,*firstLine,1,(char *)&line);
              mapnode1(tt,line,node,firstChar,lastChar);
            }
        }
      return true;
    }
  else
    { /* walk up the tree until a range of lines for the node can be found */
        trialNode = getFather(tt, node);
        while( *firstLine == UNUSED  &&  *lastLine == UNUSED )
          { mapnode2(tt, trialNode, firstLine, lastLine, &dummy);
            trialNode = getFather(tt, trialNode);
          }

      /* search the line range for the node */
        for( trialLine = *firstLine; trialLine <= *lastLine; trialLine++ )
          { (void) flex_get_buffer(R(tt)->lines,trialLine,1,(char *) &line);
            mapnode1(tt,line,node,firstChar,lastChar);
            if( *firstChar != UNUSED  || *lastChar != UNUSED )
              { /* return a character range within a line */
                  *firstLine = trialLine;
                  *lastLine  = trialLine;
		 break;
              }
          }

      return false;
    }
}




void tt_GetChanges(TextTree tt, int *first, int *last, int *delta, TT_TreeNode *node)
{
  /* return the answers */
    *first = min(R(tt)->done.first, R(tt)->pending.first);
    *last  = max(R(tt)->done.last, R(tt)->pending.last);
    *delta = R(tt)->done.delta + R(tt)->pending.delta;
    *node = nil;

  /* clear the change variables */
    R(tt)->done = tt_noChanges;
    R(tt)->pending = tt_noChanges;
}






/************************/
/*  Viewing		*/
/************************/




short tt_ViewScreenModuleIndex()
{
  return 0;
}




/*ARGSUSED*/

Point tt_ViewSize(Point charSize, short font)
{
  return Origin;
}




/*ARGSUSED*/

void tt_ViewInit(TextTree tt, TextTreeView pane, Rectangle viewRect)
{
  /* ... */
}




/*ARGSUSED*/

void tt_ViewGet(TextTree tt, TextTreeView pane, Rectangle *viewRect)
{
  /* ... */
}




/*ARGSUSED*/

void tt_ViewSet(TextTree tt, TextTreeView pane, Rectangle viewRect)
{
  /* ... */
}




/*ARGSUSED*/

void tt_ViewScroll(TextTree tt, TextTreeView pane, int dx, int dy)
{
  /* ... */
}




void tt_BeginEdit(TextTree tt)
{
  R(tt)->freeze += 1;
}




void tt_EndEdit(TextTree tt)
{
  R(tt)->freeze -= 1;
  if( R(tt)->freeze == 0  &&  R(tt)->updateDeferred )
    if( R(tt)->updateFromLines )
      updateFromLines(tt);
    else
      updateFromTree(tt);
}






/********************************/
/*  Private Methods		*/
/********************************/




/*************/
/* "ID" Tags */
/*************/


/* Sad story: the 'tt_tag' fields of tokens within the line array */
/*    are not always up to date.  If a node is assigned an id,    */
/*    the corresponding token is not adjusted except later by     */
/*    accident.  The 'tt_tag' field of a token returned by a      */
/*    call to 'getLineToken' IS always correct, as are the tags   */
/*    in the nodes themselves and the contents of the hashtable.  */




void tt_setTagNode(TextTree tt, TT_TreeNode node, TT_LineTag tag)
{
  setExtra(tt, node, 4, (int)tag);
  if( tag != 0 )
    enterInHashtable(tt, tag, node);
}




void tt_getTagNode(TextTree tt, TT_TreeNode node, TT_LineTag *tag)
{
  getExtra(tt, node, 4, tag);
}




void tt_setTagLine(TextTree tt, int lineNum, TT_LineTag tag)
{
  TT_Line line;

  getLineToken(tt, lineNum, &line, false);
  line.tt_tag = tag;
  flex_set_one(R(tt)->lines, lineNum, (char *) &line);

  tt_setTagNode(tt, line.lineNode, tag);
}




void tt_getTagLine(TextTree tt, int lineNum, TT_LineTag *tag)
{
  TT_Line line;

  getLineToken(tt, lineNum, &line, false);
  *tag = line.tt_tag;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void initLine(TextTree tt, int lineNum, TextString text)
{
  TT_Line line;

  line.lineNode = nil;
  line.conceal = 0;
  line.tt_tag  = 0;

  (void) parse1(tt,text,&line.token);
  line.text = copyTextString(text);		/* in case parser2 needs it */
  line.textValid = false;			/* since it needs unparsing */
  flex_set_one(R(tt)->lines,lineNum,(char *) &line);
}




static
void finiLine(TextTree tt, int lineNum, Boolean destroyToken, Boolean defer)
{
  TT_Line line;

  (void) flex_get_buffer(R(tt)->lines,lineNum,1,(char *) &line);

  /* free the text */
    if( line.text.tc_ptr != nil )
      { free_mem((void*) line.text.tc_ptr);
        line.text.tc_ptr = nil;
      }
    line.text.num_tc    = 0;
    line.text.ephemeral = false;
    line.textValid      = true;

  /* free the token */
    if( destroyToken )
      if ( defer )
        deferLineDestruction(tt, &line);
      else
        destroy1(tt, &line);
}




static
void updateFromLines(TextTree tt)
{
  int first,last,delta,dummy;
  Flex * lines = R(tt)->lines;
  int firstLine,oldLastLine,newLastLine,count,indent;
  Boolean haveOld,done,ok;
  TT_TreeNode oldNode,newNode;
  int goal,k;

  typedef struct
    { TT_TreeNode node;
      int     goal;
      int     first,last,indent;
    } SynchCandidate;

# define MAXSYNCH  10    /* must be > 1 */
  SynchCandidate synch[1 + MAXSYNCH];

  if( R(tt)->freeze == 0 )
    { first = R(tt)->pending.first;
      last  = R(tt)->pending.last;
      delta = R(tt)->pending.delta;

      /* determine the minimum subtree affected by the change */
        haveOld = cachedMaptext2(tt, first, last, &oldNode);
        if( ! haveOld )  oldNode = R(tt)->root;

      /* precompute candidate synch nodes -- cf. 'clipToken' */
        k = 1;  done = false;
        while( k <= MAXSYNCH  &&  ! done )
          { while( ! synch2(tt, oldNode, &synch[k].goal) )
              oldNode = getFather(tt,oldNode);
            synch[k].node = oldNode;
            mapnode2(tt, oldNode, &synch[k].first, &synch[k].last, &synch[k].indent);
            k += 1;
            if( oldNode == R(tt)->root )
              done = true;
            else if( k < MAXSYNCH )
              oldNode = getFather(tt, oldNode);
            else  /* k == MAXSYNCH */
              oldNode = R(tt)->root;
          }

      /* reparse the affected subtree */
        k = 1;  done = false;
        while( ! done )
          { oldNode     = synch[k].node;
            goal        = synch[k].goal;
            firstLine   = synch[k].first;
            oldLastLine = synch[k].last;
            newLastLine = oldLastLine + delta;
            indent      = synch[k].indent;
            count       = 1 + (newLastLine - firstLine);
            ok = parse2(tt, goal, lines, firstLine, count, &newNode);
            if( ok || oldNode == R(tt)->root )
              done = true;
            else
              { if( count > 1 )  destroy2(tt, newNode);
                k += 1;
              }
          }

      /* adjust the tree accordingly */
        R(tt)->cacheID += 1;
        if( oldNode == R(tt)->root )
          { R(tt)->root = newNode;
            setRoot(tt,newNode);
          }
        else
          tree_replace(oldNode,newNode);    /* >>> BOGUS <<< */

      /* calculate correct line tokens for affected line range */
        updateLines(tt, newNode, firstLine, newLastLine, &dummy, indent);

      /* destroy left over tree parts */
        performDeferredDestruction(tt);
        if( firstLine != oldLastLine )  destroy2(tt,oldNode);

      R(tt)->updateDeferred = false;
      doneChanging(tt, firstLine, oldLastLine, delta);
    }
  else
    { R(tt)->updateDeferred  = true;
      R(tt)->updateFromLines = true;
    }
}




static
void updateFromTree(TextTree tt)
{
  TT_TreeNode node;
  Flex * lines = R(tt)->lines;
  Boolean haveOld;
  TT_Tree stmtNode;
  int firstLine, oldLastLine, newLastLine, indent;

  if( R(tt)->freeze == 0 )
    { node = R(tt)->pending.node;

      /* determine the range of lines affected by the change */
        haveOld = cachedMapnode2(tt, node, &stmtNode, &firstLine, &oldLastLine, &indent);
        if( ! haveOld )
          { stmtNode     = R(tt)->root;
            firstLine    = 0;
            oldLastLine  = flex_length(lines) - 1;
            indent       = 0;
          }

      R(tt)->cacheID += 1;
      updateLines(tt, stmtNode, firstLine, oldLastLine, &newLastLine, indent);

      R(tt)->updateDeferred = false;
      doneChanging(tt, firstLine, oldLastLine, newLastLine-oldLastLine);
    }
  else
    { /* ASSERT: at most one 'tt_TreeChanged' is deferred */

      R(tt)->updateDeferred  = true;
      R(tt)->updateFromLines = false;
    }
}




static
void updateLines(TextTree tt, TT_TreeNode stmtNode, int firstLine, 
                 int oldLastLine, int *newLastLine, int indent)
{
  Flex * lines = R(tt)->lines;
  TT_Line line;
  Flex * newLines;
  int k;

  /* create a new range of lines */
    unparse2(tt, stmtNode, indent, &newLines);
    *newLastLine = firstLine + flex_length(newLines) - 1;

  /* copy 'newLines' into 'lines[firstLine..oldLastLine]' */
    for( k = firstLine;  k <= oldLastLine;  k++ )
      finiLine(tt,k,false,false);
    for( k = firstLine;  k <= max(oldLastLine,*newLastLine);  k++ )
      { if( k <= *newLastLine )
          { (void) flex_get_buffer(newLines, k-firstLine, 1, (char *) &line);
            if( k <= oldLastLine )
              flex_set_one(lines, k, (char *) &line);
            else
              flex_insert_one(lines, k, (char *) &line);
          }
        else
          flex_delete(lines, *newLastLine + 1 /*sic!*/, 1);
      }
    flex_destroy(newLines);
}




static
void noteChangedLines(TextTree tt, int first, int last, int delta)
{
  int firstToAdjust,oldFirst,oldLast;

  if( R(tt)->pending.delta == INFINITY )
    { /* can't include new change because don't know delta */
        R(tt)->pending.first     = 0;
        R(tt)->pending.firstNode = nil;
        R(tt)->pending.last      = INFINITY;
        R(tt)->pending.lastNode  = nil;
    }
  else
    { /* adjust this change to be in old coords */
        firstToAdjust = (R(tt)->pending.last + 1) + R(tt)->pending.delta;
        if( first >= firstToAdjust )
          oldFirst = first - R(tt)->pending.delta;
        else
          oldFirst = first;
        if( last  >= firstToAdjust )
          oldLast = last - R(tt)->pending.delta;
        else
          oldLast = last;

      /* include this change in remembered change limits */
        if( oldFirst < R(tt)->pending.first )
          { R(tt)->pending.first = oldFirst;
            includePendingNode(tt, first, &R(tt)->pending.firstNode);
          }

        if( oldLast > R(tt)->pending.last )
          { R(tt)->pending.last = oldLast;
            includePendingNode(tt, last, &R(tt)->pending.lastNode);
          }

        if( delta == INFINITY )
          R(tt)->pending.delta = INFINITY;
        else
          R(tt)->pending.delta = delta + R(tt)->pending.delta;
    }
}




static
void includePendingNode(TextTree tt, int lineNum, TT_TreeNode *lineNode)
{
  int len = flex_length(R(tt)->lines);
  TT_Line line;

  if( 0 <= lineNum  &&  lineNum < len )
    { (void) getLineToken(tt, lineNum, &line, false);
      if( line.lineNode != nil )
        *lineNode = line.lineNode;
    }
}




static
void noteChangedTree(TextTree tt, TT_TreeNode node)
{
  /* ASSERT: at most one tree change is deferred */

  R(tt)->pending.node = node;
}




static
void doneChanging(TextTree tt, int first, int last, int delta)
{
  R(tt)->pending = tt_noChanges;

  R(tt)->done.first = first;
  R(tt)->done.last  = last;
  R(tt)->done.delta = delta;
  R(tt)->done.node  = nil;
}




/**************************/
/* Statement-id hashtable */
/**************************/


/* sad joke: not a hashtable at all */




static
void initHashtable(TextTree tt)
{
  Flex * table;

  table = flex_create(sizeof(TableElement));
  R(tt)->hashtable = table;
}




static
void finiHashtable(TextTree tt)
{
  flex_destroy(R(tt)->hashtable);
}




static
void readHashtable(TextTree tt, DB_FP *fp)
{
  Flex * table = R(tt)->hashtable;
  int len;
  int k;
  TableElement elem;

  (void) db_buffered_read(fp, (char*)&len, sizeof(len));

  for( k = 0;  k < len;  k++ )
    { (void) db_buffered_read(fp, (char*)&elem, sizeof(elem));
      (void) flex_set_one(table, k, (char *) &elem);
    }
}

static
void writeHashtable(TextTree tt, DB_FP *fp)
{
  Flex * table = R(tt)->hashtable;
  int len = flex_length(table);
  int k;
  TableElement elem;

  (void) db_buffered_write(fp, (char*)&len, sizeof(len));

  for( k = 0;  k < len;  k++ )
    { (void) flex_get_buffer(table, k, 1, (char *) &elem);
      (void) db_buffered_write(fp, (char*)&elem, sizeof(elem));
    }
}




static
void enterInHashtable(TextTree tt, int id, TT_TreeNode node)
{
  TT_TreeNode dummy;
  int k;
  TableElement e;

  k = lookInHashtable(tt, id, &dummy);

  /* change the table entry to hold new value */
    e.id   = id;
    e.node = node;
    (void) flex_set_one(R(tt)->hashtable, k, (char *) &e);
}




static
int lookInHashtable(TextTree tt, int id, TT_TreeNode *node)
{
  Flex * table = R(tt)->hashtable;
  int len = flex_length(table);
  int k;
  TableElement e;
  Boolean found;

  /* look for the desired table entry */
    k = 0;  found = false;
    while( k < len && ! found )
      { (void) flex_get_buffer(table, k, 1, (char *) &e);
        if( e.id == id )
          found = true;
        else
          k += 1;
      }

  /* returned values depend on whether found or not */
    if( found )
      { *node = e.node;
        return k;
      }
    else
      { *node = UNUSED;
        return len;
      }
}




static void
removeFromHashtable(TextTree tt, int id, TT_TreeNode node)
{
  int k;
  TT_TreeNode foundNode;

  k = lookInHashtable(tt, id, &foundNode);

  /* ASSERT: 'node != UNUSED' so a separate test for UNUSED is unnecessary */

  if( foundNode == node )
    flex_delete(R(tt)->hashtable, k, 1);
}




static
void removeAllFromHashtable(TextTree tt, TT_TreeNode node)
{
  int id;
  TT_TreeNode elem;
  int numSons, k;

  if( node == AST_NIL )  return;

  getExtra(tt, node, 4, &id);
  if( id != 0 )
    removeFromHashtable(tt, id, node);

  switch( NT(node) )
    {
      case GEN_LIST_OF_NODES:
        elem = list_first(node);
        while( elem != AST_NIL )
          { removeAllFromHashtable(tt, elem);
            elem = list_next(elem);
          }
        break;

      default:
	numSons = gen_how_many_sons(gen_get_node_type(node));
	for( k = 1; k <= numSons; k++ )
	  removeAllFromHashtable(tt, gen_get_son_n(node,k));
        break;
    }
}



/*************************/
/* Customization methods */
/*************************/


static
Boolean parse1(TextTree tt, TextString text, TT_MaxToken *token)
{
  return (R(tt)->methods.parse1) (R(tt)->tree, text, token);
}




static
Boolean parse2(TextTree tt, int goal, Flex *lines, int start, 
               int count, TT_TreeNode *node)
{
  return (R(tt)->methods.parse2) (R(tt)->tree, goal, lines, start, count, node);
}




static
void unparse1(TextTree tt, TT_Line line, TextString *text)
{
  (R(tt)->methods.unparse1) (R(tt)->tree, line, text);
}




static
void unparse2(TextTree tt, TT_TreeNode node, int indent, Flex **lines)
{
  (R(tt)->methods.unparse2) (R(tt)->tree, node, indent, lines);
}




static
void maptext1(TextTree tt, TT_Line line, int firstChar, TT_TreeNode *node)
{
  (R(tt)->methods.maptext1) (R(tt)->tree, line, firstChar, node);
}




static
void maptext2(TextTree tt, int firstLine, TT_TreeNode *node)
{
  (R(tt)->methods.maptext2) (R(tt)->tree, firstLine, node);
}




static
void mapnode1(TextTree tt, TT_Line line, TT_TreeNode node, 
              int *firstChar, int *lastChar)
{
  (R(tt)->methods.mapnode1) (R(tt)->tree, line, node, firstChar, lastChar);
}




static
void mapnode2(TextTree tt, TT_TreeNode node, int *firstLine, 
              int *lastLine, int *indent)
{
  int id;

  /* TRICK: new FortTree has root == nil, cf. 'updateFromLines' */
  if( node == nil )
    { *firstLine = 0;
      *lastLine  = -1;
      *indent    = 0;
    }
  else
    { /* see if answer is cached in the node */
        getExtra(tt, node, 0, &id);

      if( id == R(tt)->cacheID )
        { getExtra(tt, node, 1, firstLine);
          getExtra(tt, node, 2, lastLine);
          getExtra(tt, node, 3, indent);
        }
      else
        (R(tt)->methods.mapnode2) (R(tt)->tree, node, firstLine, lastLine, indent,
                                   tt, enterInCache);
    }
}




static
Boolean synch2(TextTree tt, TT_TreeNode node, int *goal)
{
  return (R(tt)->methods.synch2) (R(tt)->tree, node, goal);
}




static
void copy1(TextTree tt, TT_Line *oldLine, TT_Line *newLine)
{
  (R(tt)->methods.copy1) (R(tt)->tree, oldLine->token, newLine->token);
}




static
void copy2(TextTree tt, TT_TreeNode oldNode, TT_TreeNode *newNode)
{
  (R(tt)->methods.copy2) (R(tt)->tree, oldNode, newNode);
}




static
void destroy1(TextTree tt, TT_Line *line)
{
  removeAllFromHashtable(tt, line->lineNode);
  (R(tt)->methods.destroy1) (R(tt)->tree, &line->token);
}




static
void destroy2(TextTree tt, TT_TreeNode node)
{
  removeAllFromHashtable(tt, node);
  (R(tt)->methods.destroy2) (R(tt)->tree, node);
}




static
void setRoot(TextTree tt, TT_TreeNode node)
{
  (R(tt)->methods.setRoot) (R(tt)->tree, node);
}




static
TT_TreeNode getFather(TextTree tt, TT_TreeNode node)
{
  return (R(tt)->methods.getFather) (R(tt)->tree, node);
}




static
void getExtra(TextTree tt, TT_TreeNode node, int k, int *value)
{
  (R(tt)->methods.getExtra) (R(tt)->tree, node, k, value);
}




static
void setExtra(TextTree tt, TT_TreeNode node, int k, int value)
{
  (R(tt)->methods.setExtra) (R(tt)->tree, node, k, value);
}




static
TT_TreeNode lowestCommonAncestor(TextTree tt, TT_TreeNode node1, TT_TreeNode node2)
{
  int d1, d2;

  if( node1 == nil )
    return node2;
  else if( node2 == nil )
    return node1;
  else
    { d1 = distanceToRoot(tt, node1);
      d2 = distanceToRoot(tt, node2);

      /* walk node1 up the tree until it reaches node2's height */
        while( d1 > d2 )
          { node1 = tt_GetFather(tt, node1);
            d1--;
          }

      /* walk node2 up the tree until it reaches node1's height */
        while( d2 > d1 )
          { node2 = tt_GetFather(tt, node2);
            d2--;
          }

      /* walk both up the tree until they are equal */
        while( node1 != node2 )
         { node1 = tt_GetFather(tt, node1);
           d1--;
           node2 = tt_GetFather(tt, node2);
           d2--;
         }

      return node1;
    }
}




static
int distanceToRoot(TextTree tt, TT_TreeNode node)
{
  int height = 0;

  while( node != R(tt)->root )
   { height++;
     node = tt_GetFather(tt, node);
   }
  return height;
}




static
Boolean getLineToken(TextTree tt, int lineNum, TT_Line *line, Boolean needText)
{
  Flex * lines = R(tt)->lines;

  if( 0 <= lineNum  &&  lineNum < flex_length(lines) )
    { (void) flex_get_buffer(lines,lineNum,1,(char *) line);
      if( line->lineNode != nil )
        tt_getTagNode(tt, line->lineNode, &line->tt_tag);
      else
        line->tt_tag = 0;
      if( needText && ! line->textValid )
        { destroyTextString(line->text);
          unparse1(tt,*line,&line->text);
          line->textValid = true;
          flex_set_one(lines,lineNum,(char *) line);
        }
      return true;
    }
  else
    return false;
}




static
Boolean cachedMapnode2(TextTree tt, TT_TreeNode node, TT_TreeNode *stmtNode, 
                       int *firstLine, int *lastLine, int *indent)
{
  int first,last,ind,dummy;

  while( ! synch2(tt, node, &dummy) )  node = getFather(tt, node);

  if( node == R(tt)->pending.node  &&  R(tt)->pending.first != INFINITY )
    { *stmtNode  = R(tt)->pending.node;
      *firstLine = R(tt)->pending.first;
      *lastLine  = R(tt)->pending.last;
      *indent    = R(tt)->pending.indent;
      return true;
    }
  else
    { /* THIS IS WRONG IN GENERAL BUT 'PED' RELIES ON IT AND LUCKS OUT */
      mapnode2(tt, node, &first, &last, &ind);
      if( first == last )
        { *stmtNode  = node;
          *firstLine = first;
          *lastLine  = last;
           *indent   = ind;
          return true;
        }
      else
        return false;
    }
}




/*ARGSUSED*/

static
Boolean cachedMaptext2(TextTree tt, int firstLine, int lastLine, TT_TreeNode *node)
{
  *node = lowestCommonAncestor(tt, R(tt)->pending.firstNode, R(tt)->pending.lastNode);
  return BOOL( *node != nil );
}




static
void deferLineDestruction(TextTree tt, TT_Line *line)
{
  Flex * ltd = R(tt)->linesToDestroy;
  int next = flex_length(ltd);

  flex_insert_one(ltd, next, (char *) line);
}




static
void performDeferredDestruction(TextTree tt)
{
  Flex * ltd = R(tt)->linesToDestroy;
  int len = flex_length(ltd);
  int k;
  TT_Line line;

  for( k = 0;  k < len;  k++ )
    { (void) flex_get_buffer(ltd, k, 1, (char *) &line);
      destroy1(tt, &line);
    }

  flex_delete(ltd, 0, len);
}




static
void enterInCache(TextTree tt, TT_TreeNode node, int first, int last, int indent)
{
  setExtra(tt, node, 0, R(tt)->cacheID);
  setExtra(tt, node, 1, first);
  setExtra(tt, node, 2, last);
  setExtra(tt, node, 3, indent);
}
