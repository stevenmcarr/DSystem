/* $Id: FortUnparse1.C,v 1.2 1997/10/30 15:27:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      FortTextTree/FortUnparse1.c					*/
/*                                                                      */
/*      FortUnparse1 -- low-level unparser for FortTrees                */
/*                                                                      */
/************************************************************************/

#include <ctype.h>
#include <string.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/FortUnparse1.h>

#include <libs/support/arrays/FlexibleArray.h>

#include <libs/support/memMgmt/mem.h>

/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/************************/
/*  Static data         */
/************************/


/* what kind of result is wanted */

typedef enum
  {
    UNPARSE_TEXT,
    UNPARSE_MAP_NODE,
    UNPARSE_MAP_CHAR,
    UNPARSE_EXPAND
  } UnparseMode;

static UnparseMode unp1_mode;




/* when text is wanted */

static TextString unp1_text;

static int unp1_textIndentPos;
static int unp1_textSize;




/* when node/char mapping is wanted */

static FortTreeNode unp1_node;
static int unp1_firstChar;
static int unp1_lastChar;




/* when expansion is wanted */

static Flex * unp1_expansions;
static int unp1_expandChar;




/* unparsing predicate table */

STATIC(Boolean, pred_show, (FortTreeNode node));
STATIC(Boolean, pred_comma, (FortTreeNode node));
STATIC(Boolean, pred_emphasize, (FortTreeNode node));
STATIC(Boolean, pred_exists, (FortTreeNode node));
STATIC(Boolean, pred_empty_dim, (FortTreeNode node));
STATIC(Boolean, pred_last_ep, (void));
STATIC(Boolean, pred_first, (FortTreeNode node));
STATIC(Boolean, pred_placeholder, (FortTreeNode node));

typedef FUNCTION_POINTER (Boolean,PredFunc,(FortTreeNode));

static struct pred_map
  {
    char *key;
    PredFunc  routine;
  } unp1_pred_map[] =
  {
    "show",             pred_show,
    "comma",		pred_comma,
    "emphasize",        pred_emphasize,
    "exists",           pred_exists,
    "emptydim",         pred_empty_dim,
    "parens",           unp1_pred_parens,
    "first",            pred_first,
    "placeholder",      pred_placeholder,
    "",                 0
  };







/************************/
/* Forward declarations */
/************************/


STATIC(void,            outputLine, (FortTextTree ftt, TT_Line line));
STATIC(void,		line_output, (TT_Line *line, char ch));
STATIC(void,		line_outputText, (TT_Line *line));
STATIC(void,		line_outputSon, (TT_Line *line, int k));
STATIC(void,		line_tab, (TT_Line *line));
STATIC(Boolean,		line_eval, (TT_Line *line, char *funcName, int k));
STATIC(void,		line_missing, (TT_Line *line, char *funcName, int selector, 
                                       int phase));

STATIC(void,            outputNode, (FortTreeNode node));
STATIC(void,		node_outputText, (FortTreeNode node));
STATIC(void,		node_outputSon, (FortTreeNode node, int k));
STATIC(void,		node_tab, (FortTreeNode node));
STATIC(Boolean,		node_eval, (FortTreeNode node, char *funcName, int selector));
STATIC(void,		node_missing, (FortTreeNode node, char *funcName, int selector,
                                       int phase));

STATIC(void,            output, (FortTreeNode node, char ch));
STATIC(void,		outputString, (FortTreeNode node, char *s));

STATIC(void,            addNodeExpansion, (FortTreeNode node, int entry));
STATIC(void,            addLineExpansion, (FortTreeNode node, int value, META_TYPE type));

STATIC(PredFunc,        lookupPredicate, (char *name));
STATIC(Boolean,         evalNodePredicate, (char *funcName, int selector, 
                                            FortTreeNode node));
STATIC(Boolean,         evalTokenPredicate, (char *funcName, int selector, 
                                             fx_StatToken st, FortTreeNode parent));
STATIC(char*,           GET_TEXT, (FortTreeNode node));
STATIC(Boolean,		is_function_name, (FortTreeNode node));
STATIC(Boolean,		is_subroutine_name, (FortTreeNode node));






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/************************/
/*  Initialization      */
/************************/




void unp1_Init()
{
  unp1_textSize = 100;
  unp1_text = createTextString(unp1_textSize, "unp1_Init");
}




void unp1_Fini()
{
  destroyTextString(unp1_text);
}






/************************/
/*  Unparsing           */
/************************/




void unp1_Unparse(FortTextTree ftt, TT_Line line, TextString *text)
{
  /* initialize result variables */
    unp1_mode = UNPARSE_TEXT;

  /* unparse the given line */
    outputLine(ftt,line);

  /* return result variables */
    *text = copyTextString(unp1_text);
}


void unp1_TextToNode(FortTextTree ftt, TT_Line line, int firstChar, FortTreeNode *node)
{
  /* initialize result variables */
    unp1_mode = UNPARSE_MAP_CHAR;
    unp1_firstChar = firstChar;
    unp1_node = line.lineNode;      /* default reply */

  /* unparse the given line */
    outputLine(ftt,line);

  /* return result variables */
    *node = unp1_node;
}




void unp1_NodeToText(FortTextTree ftt, TT_Line line, FortTreeNode node, 
                     int *firstChar, int *lastChar)
{
  /* initialize result variables */
    unp1_mode = UNPARSE_MAP_NODE;
    unp1_node = node;
    unp1_firstChar = UNUSED;	/* default */
    unp1_lastChar = UNUSED;	/* default */

  /* unparse the given line */
    outputLine(ftt,line);

  /* return result variables */
    *firstChar = unp1_firstChar;
    *lastChar  = unp1_lastChar;
}






/************************/
/*  Expanding		*/
/************************/




void unp1_Expandee(FortTextTree ftt, TT_Line line, int expandChar, Flex *expansions)
{
  /* initialize result variables */
    unp1_mode = UNPARSE_EXPAND;
    unp1_expandChar = expandChar;
    unp1_expansions = expansions;

  /* unparse the given line */
    outputLine(ftt,line);
}




void unp1_Expand(FortTextTree ftt, fx_Expansion ex, FortTreeNode *New, 
                 FortTreeNode *focus)
{
  FortTreeNode old, previous;

  /* warn of proposed change */
    ftt_TreeWillChange(ftt, ex.node);

  /* create the new piece */
    if( ex.type & LIST )
      { *New = list_create(ph_from_mtype(ex.type & ~LIST));
        *focus = list_first(*New);
      }
    else
      { *New = ph_from_mtype(ex.type);
        *focus = *New;
      }

  if( is_error(ex.node) )
    { /* install the error expansion */
        /* ASSERT: all parse 2 errors will expand in the 0th part */

        old = gen_ERROR_get_part0(ex.node);
        tree_replace(old, AST_NIL);
        gen_ERROR_put_part0(ex.node, *New);
        tree_free(old);
    }
  else if( NOT(is_list(ex.node)) )
    { /* install an ordinary son */
        old = gen_get_son_n(ex.node, ex.value);
        tree_replace(old, AST_NIL);
        gen_put_son_n(ex.node, ex.value, *New);
        tree_free(old);
    }
  else if( ex.value == 0 )
    { /* install the first list element */
       (void) list_insert_first(ex.node, *New);
    }
  else
    { /* install an arbitrary list element */
        previous = list_retrieve(ex.node, ex.value);
        (void) list_insert_after(previous, *New);
    }

  /* notify of change */
    ftt_TreeChanged(ftt, ex.node);
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/


static
ftt_FormatCustomProcs unp1_lineCustomProcs =
{
  (OutputFunc)line_output,
  (OutputTextFunc)line_outputText,
  (OutputSonFunc)line_outputSon,
  (TabFunc)line_tab,
  (EvalFunc)line_eval,
  (MissingFunc)line_missing
};


static
void outputLine(FortTextTree ftt, TT_Line line)
{
  char * fmt;
  fx_StatToken st;

  /* note the requestor */
    ft_AstSelect(ftt_Tree(ftt));

  /* initialize result variables */
    unp1_textIndentPos = FIXED_INDENT + line.indent;
    unp1_text.num_tc   = 0;
    ftt_style = STYLE_NORMAL;

  /* unparse the line */
    st = *(fx_StatToken *) &line.token;
    fmt = ftt_getTokenFormat(st.token);
    if ( fmt )
      { /* tag the beginning of the node as being part of the parent node */
          if( unp1_mode == UNPARSE_MAP_NODE  &&  line.lineNode == unp1_node )
            unp1_firstChar = unp1_text.num_tc;

        ftt_format(fmt, (Generic) &line, &unp1_lineCustomProcs);

        /* tag the end of the node as being part of the node */
          if( unp1_mode == UNPARSE_MAP_NODE  &&  line.lineNode == unp1_node )
            unp1_lastChar = unp1_text.num_tc - 1;
      }
    else
      outputNode(st.tree);
}




static
void line_output(TT_Line *line, char ch)
{
  output(line->lineNode, ch);
}




static
void line_outputText(TT_Line *line)
{
  node_outputText(line->lineNode);
}




static
void line_outputSon(TT_Line *line, int k)
{
  fx_StatToken st;

  st = *(fx_StatToken *) &line->token;
  outputNode(st.part[k-1]);
}




static
void line_tab(TT_Line *line)
{
  node_tab(line->lineNode);
}




static
Boolean line_eval(TT_Line *line, char *funcName, int k)
{
  fx_StatToken st;

  st = *(fx_StatToken *) &line->token;
  return evalTokenPredicate(funcName, k, st, line->lineNode);
}




static
void line_missing(TT_Line *line, char *funcName, int selector, int phase)
{
  fx_StatToken st;
  FortTreeNode node = line->lineNode;

  if( phase != 1 )  return;

  st = *(fx_StatToken *) &line->token;

  /* check for expansions of missing parts */
    if( unp1_mode == UNPARSE_EXPAND  &&
        unp1_text.num_tc == unp1_expandChar )
      { if( strcmp(funcName, "exists") == 0 )
          { /* check for a missing part */
            if( st.token == SFUNCTION_STAT && selector == 2 )
              { /* type length */
                addLineExpansion(node, 3, GEN_type);
              }
            if( st.token == SSUBROUTINE_STAT && selector == 3 )
              { /* formal arg list */
                addLineExpansion(node, 4, GEN_formal);
              }

            switch( st.token )
              { case SDEBUG_STAT:
                case SFUNCTION_STAT:
                case SSUBROUTINE_STAT:
                case SBLOCK_DATA_STAT:
                case SDO_STAT:
                case SDO_LABEL_STAT:
                case SDOALL_STAT:
                case SDOALL_LABEL_STAT:
                case SELSE_IF_STAT:
                case SELSE_STAT:
                case SIF_STAT:
                case SOTHER_PROCESSES_STAT:
                case SPARALLEL_STAT:
                case SPARALLEL_PID_STAT:
                case SPARALLELLOOP_STAT:
                case SPARALLELLOOP_LABEL_STAT:
                case SPARBEGIN_STAT:
                case SPARBEGIN_PID_STAT:
                case SPROGRAM_STAT:
                case SELSE_WHERE_STAT:
                case SWHERE_BLOCK_STAT:
                  if( selector == 1 )
                    { /* label on the bracket */
                      addLineExpansion(node, 1, GEN_lbl_def);
                    }
                  break;

                case SEND_ALL_STAT:
                case SEND_DO_STAT:
                case SEND_IF_STAT:
                case SEND_STAT:
                case SPAREND_STAT:
                case SEND_WHERE_STAT:
                  if( selector == 1 )
                    { /* label on the bracket */
                      addLineExpansion(node, 2, GEN_lbl_def);
                    }
                  break;
          }
        }
      else if( strcmp(funcName, "show") == 0 )
        { /* check for a hidden part */
          switch( st.token )
            { case SPROGRAM_STAT:
                if( selector == 2 )
                  { /* program name */
                    addLineExpansion(node, 3, GEN_name);
                  }
                break;

              case SBLOCK_DATA_STAT:
                if( selector == 2 )
                  { /* block data name */
                    addLineExpansion(node, 3, GEN_name);
                  }
                break;
            }
        }
      }
}


static
ftt_FormatCustomProcs unp1_nodeCustomProcs =
{
  (OutputFunc)output,
  (OutputTextFunc)node_outputText,
  (OutputSonFunc)node_outputSon,
  (TabFunc)node_tab,
  (EvalFunc)node_eval,
  (MissingFunc)node_missing
};


static
void outputNode(FortTreeNode node)
{
  FortTreeNode son;
  int sonCount;
  char * fmt;
  char ch;

  if( node == AST_NIL )
    return;

  /* tag the beginning of the node as being part of the node */
    if( unp1_mode == UNPARSE_MAP_NODE  &&  node == unp1_node )
      unp1_firstChar = unp1_text.num_tc;
      
  if( is_list(node) )
    { sonCount = 0;
      for( son = list_first(node); son != AST_NIL; son = list_next(son) )
        { if( unp1_mode == UNPARSE_EXPAND  &&
              unp1_text.num_tc == unp1_expandChar  &&
              !is_logical_if(tree_out(node)) &&
              !is_where(tree_out(node)) )
            addNodeExpansion(node, sonCount);
          outputNode(son);
          sonCount++;

          if( !is_last_in_list(son) )
            { if( unp1_mode == UNPARSE_EXPAND  &&
                  unp1_text.num_tc == unp1_expandChar  &&
                  !is_logical_if(tree_out(node)) &&
                  !is_where(tree_out(node)) )
                addNodeExpansion(node, sonCount);
              output( node, ( ft_GetComma(son) ? ',' : ' ' ) );

              if( unp1_mode == UNPARSE_EXPAND  &&
                  unp1_text.num_tc == unp1_expandChar  &&
                  !is_logical_if(tree_out(node)) &&
                  !is_where(tree_out(node)) )
                addNodeExpansion(node, sonCount);
              output(node, ' ');
            }
        }
      if( unp1_mode == UNPARSE_EXPAND  &&
          unp1_text.num_tc == unp1_expandChar  &&
          !is_logical_if(tree_out(node)) &&
          !is_where(tree_out(node)) )
        addNodeExpansion(node, sonCount);
    }
  else if( is_error(node) )
    { ftt_toggleBold();
      fmt = GET_TEXT(gen_ERROR_get_tree(node));
      while( ch = *fmt++ )
       { if( isascii(ch) )
           output(node, ch);
         else if( toascii(ch) == 0 )
           ftt_style |= ftt_BEGIN_PLACEHOLDER_STYLE;
         else
           { ftt_style |= ftt_PLACEHOLDER_STYLE;
             output(node, toascii(ch));
             ftt_style &= ~ftt_PLACEHOLDER_STYLE;
             ftt_style &= ~ftt_BEGIN_PLACEHOLDER_STYLE;
           }
       }
      ftt_toggleBold();
    }
  else if( is_place_holder(node) )
    { fmt = GET_TEXT(node);
      ftt_style |= ftt_PLACEHOLDER_STYLE;
      ftt_style |= ftt_BEGIN_PLACEHOLDER_STYLE;
      output(node, fmt[0]);
      ftt_style &= ~ftt_BEGIN_PLACEHOLDER_STYLE;
      outputString(node, fmt+1);
      ftt_style &= ~ftt_PLACEHOLDER_STYLE;
    }
  else
    { fmt = ftt_getNodeFormat(NT(node));
      ftt_format(fmt, (Generic) node, &unp1_nodeCustomProcs);
    }

  /* tag the end of the node as being part of the node */
    if( unp1_mode == UNPARSE_MAP_NODE  &&  node == unp1_node )
      unp1_lastChar = unp1_text.num_tc - 1;
}




static
void node_outputSon(FortTreeNode node, int k)
{
  outputNode(gen_get_son_n(node, k));
}




static
void node_tab(FortTreeNode node)
{
  while( unp1_text.num_tc < unp1_textIndentPos )
    output(node, ' ');
}




static
Boolean node_eval(FortTreeNode node, char *funcName, int selector)
{
  return evalNodePredicate(funcName, selector, node);
}




static
void node_missing(FortTreeNode node, char *funcName, int selector, int phase)
{
  int son_index;

  if( phase != 1 )  return;

  /* check for expansions of missing sons */
    if( unp1_mode == UNPARSE_EXPAND  &&
        unp1_text.num_tc == unp1_expandChar )
      { son_index = selector;
        if( strcmp(funcName, "exists") == 0 )
          { /* add a missing son */
            if( is_array_decl_len(node)  &&
                ( is_dimension(tree_out(node)) ||
                  is_common_elt(tree_out(node)) ) )
              { /* allow only dimensions */
                if( son_index == 3 )
                  addNodeExpansion(node, son_index);
              }
            else
              addNodeExpansion(node, son_index);
          }
        else if( strcmp(funcName, "show") == 0 )
          { /* check for a hidden son */
            if( is_common_elt(node) )
              { /* common block name */
                addNodeExpansion(node, son_index);
              }
          }
      }
}




static
void output(FortTreeNode node, char ch)
{
  switch( unp1_mode )
    {
      case UNPARSE_TEXT:
	if( unp1_text.num_tc >= unp1_textSize )
	  { unp1_textSize += 100;
            unp1_text.tc_ptr = (TextChar *) reget_mem(
		(void *) unp1_text.tc_ptr,
		unp1_textSize * sizeof(TextChar), "unparse1");
          }
        unp1_text.tc_ptr[unp1_text.num_tc] = makeTextChar(ch,ftt_style);
        break;

      case UNPARSE_MAP_CHAR:
        if( unp1_text.num_tc == unp1_firstChar )
          unp1_node = node;
        break;
    }

  unp1_text.num_tc++;
}




static
void outputString(FortTreeNode node, char *s)
{
  while (*s)
    output(node, *s++);
}




static
void node_outputText(FortTreeNode node)
{
  outputString(node, GET_TEXT(node));
}




static
void addNodeExpansion(FortTreeNode node, int entry)
{
  fx_Expansion ex;
  FortTreeNode parent;
  NODE_TYPE ptype;
  META_TYPE type;
  int son;

  /* figure the expansion pieces */
    if( is_list(node) )
      { /* figure a list element expansion */
          parent = tree_out(node);
          son = gen_which_son(parent, node);
          ptype = gen_get_node_type(parent);
          type = THE_TYPE(get_son(son, ptype));
      }
    else
      { /* figure a missing son expansion */
          ptype = gen_get_node_type(node);
          type = THE_TYPE(get_son(entry, ptype));
      }

  /* add the expansion */
    ex.node   = node;
    ex.value  = entry;
    ex.type   = type;
    ex.title  = ftt_expansionName(type | META);
    ex.who    = UNPARSE1_EXPANDER;
    ex.first  = true;
    flex_insert_one(unp1_expansions, flex_length(unp1_expansions),
							(char *) &ex);
}




static
void addLineExpansion(FortTreeNode node, int value, META_TYPE type)
{
  fx_Expansion ex;

  /* add the expansion */
    ex.node   = node;
    ex.value  = value;
    ex.type   = type;
    ex.title  = ftt_expansionName(type | META);
    ex.who    = UNPARSE1_EXPANDER;
    ex.first  = true;
    flex_insert_one(unp1_expansions, flex_length(unp1_expansions),
							(char *) &ex);
}




static
PredFunc lookupPredicate(char *name)
{
  int k;

  for( k = 0;  unp1_pred_map[k].key;  k++ )
    if( strcmp(unp1_pred_map[k].key,name) == 0 )
      return unp1_pred_map[k].routine;

  return (PredFunc) UNUSED;
}




static
Boolean evalNodePredicate(char *funcName, int selector, FortTreeNode node)
{
  PredFunc func;
  FortTreeNode arg;
  Boolean negate,result;

  negate = BOOL( funcName[0] == '!' );
  if( negate )  funcName++;

  func = lookupPredicate(funcName);
  arg  = ( selector == 0  ?  node  :  gen_get_son_n(node,selector) );
  result = func(arg);

  if( negate )
    return NOT(result);
  else
    return result;
}




static
Boolean evalTokenPredicate(char *funcName, int selector, fx_StatToken st, 
                           FortTreeNode parent)
{
  PredFunc func;
  FortTreeNode arg;
  Boolean negate,result;

  negate = BOOL( funcName[0] == '!' );
  if( negate )  funcName++;

  func = lookupPredicate(funcName);
  arg  = ( selector == 0  ?  parent  :  st.part[selector - 1] );
  result = func(arg);

  if( negate )
    return NOT(result);
  else
    return result;
}




/************************/
/*  Unparse predicates  */
/************************/




static
Boolean pred_show(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return ft_GetShow(node);
}




static
Boolean pred_comma(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return ft_GetComma(node);
}




static
Boolean pred_emphasize(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return ft_GetEmphasis(node);
}




static
Boolean pred_exists(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return BOOL(node != AST_NIL);
}

static
Boolean pred_empty_dim(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return BOOL (gen_get_node_type(node) == GEN_DIM &&
	       gen_get_son_n(node, 1) == AST_NIL &&
	       gen_get_son_n(node, 2) == AST_NIL);
}


/*************************************************
 *
 *    IsBinaryOperator(node)                            
 *
 *     check if the type of node is a binary 
 *     operator.
 *
 *   Input: node to check type
 *
 *   Output: true if node is an operator, 
 *           else false
 *
 *************************************************/


static Boolean IsBinaryOperator(FortTreeNode node)
  {
   switch(gen_get_node_type(node))
     {
      case GEN_BINARY_EXPONENT:
      case GEN_BINARY_TIMES:
      case GEN_BINARY_DIVIDE:
      case GEN_BINARY_PLUS:
      case GEN_BINARY_MINUS: 
      case GEN_BINARY_CONCAT:
      case GEN_BINARY_AND:
      case GEN_BINARY_OR:
      case GEN_BINARY_EQ:
      case GEN_BINARY_NE:
      case GEN_BINARY_GE:
      case GEN_BINARY_GT:
      case GEN_BINARY_LE:
      case GEN_BINARY_LT:
      case GEN_BINARY_EQV:
      case GEN_BINARY_NEQV:
        return true;
         
      default: return false;
     }
  }


/*************************************************
 *
 *    IsUnaryOperator(node)                            
 *
 *     check if the type of node is a unary 
 *     operator.
 *
 *   Input: node to check type
 *
 *   Output: true if node is an operator, 
 *           else false
 *
 *************************************************/


static Boolean IsUnaryOperator(FortTreeNode node)
  {
   switch(gen_get_node_type(node))
     {
      case GEN_UNARY_MINUS:
      case GEN_UNARY_NOT:
       return true;
      
       default:
         return false;
      }
  }


/*************************************************
 *
 *    IsOperator(node)                            
 *
 *     check if the type of node is a binary 
 *     or unary operator.
 *
 *   Input: node to check type
 *
 *   Output: true if node is an operator, 
 *           else false
 *
 *************************************************/


static Boolean IsOperator(FortTreeNode node)
  {
    return BOOL(IsBinaryOperator(node) || IsUnaryOperator(node));
  }


/*************************************************
 *
 *    OperatorPrecedence(node)
 *
 *    determine the precedence value of an 
 *    operator
 *
 *   Input: operator node to check precedence
 *
 *   Output: precendence of operator, 
 *
 *************************************************/


static int OperatorPrecedence(FortTreeNode node)
  {
   switch(gen_get_node_type(node))
     {
      case GEN_UNARY_MINUS:      return UNARY_MINUS_PRECEDENCE;
      case GEN_UNARY_NOT:        return UNARY_NOT_PRECEDENCE;
      case GEN_BINARY_EXPONENT:  return BINARY_EXPONENT_PRECEDENCE;
      case GEN_BINARY_TIMES:     return BINARY_TIMES_PRECEDENCE;
      case GEN_BINARY_DIVIDE:    return BINARY_DIVIDE_PRECEDENCE;
      case GEN_BINARY_PLUS:      return BINARY_PLUS_PRECEDENCE;
      case GEN_BINARY_MINUS:     return BINARY_MINUS_PRECEDENCE;
      case GEN_BINARY_CONCAT:    return BINARY_CONCAT_PRECEDENCE;
      case GEN_BINARY_EQ:        return BINARY_EQ_PRECEDENCE;
      case GEN_BINARY_NE:        return BINARY_NE_PRECEDENCE;
      case GEN_BINARY_GE:        return BINARY_GE_PRECEDENCE;
      case GEN_BINARY_GT:        return BINARY_GT_PRECEDENCE;
      case GEN_BINARY_LE:        return BINARY_LE_PRECEDENCE;
      case GEN_BINARY_LT:        return BINARY_LT_PRECEDENCE;
      case GEN_BINARY_AND:       return BINARY_AND_PRECEDENCE;
      case GEN_BINARY_OR:        return BINARY_OR_PRECEDENCE;
      case GEN_BINARY_EQV:       return BINARY_EQV_PRECEDENCE;
      case GEN_BINARY_NEQV:      return BINARY_NEQV_PRECEDENCE;
     }
  }


/*************************************************
 *
 *    OperatorInterpretation(node)
 *
 *     check the interpretation of an operator
 *     either left-to-right or right-to-left
 *
 *   Input: operator to check interpretation
 *
 *   Output: LEFT if left-to-right
 *           RIGHT if right-to-left
 *
 *************************************************/


static DirectionValue OperatorInterpretation(FortTreeNode node)
  {
   switch(gen_get_node_type(node))
     {
      case GEN_BINARY_EXPONENT:  return RIGHT;

      default:                   return LEFT;
     }
  }


/*************************************************
 *
 *    WhichChild(parent,node)
 *
 *     check which child node is of parent
 *     either left-to-right or right-to-left
 *
 *   Input: parent
 *          child
 *
 *   Output: LEFT or RIGHT child
 *
 *************************************************/


static DirectionValue WhichChild(FortTreeNode parent, FortTreeNode node)
  {
    if (ast_which_son(parent,node) == 2)
      return(RIGHT);
    else
      return(LEFT);
  }


/*************************************************
 *
 *   AssociativeOperator(node)
 *
 *   determine if operator is associative
 *
 *   Input: operator node
 *
 *   Output: true if operator is associative,
 *           else false
 *
 *************************************************/


static Boolean AssociativeOperator(FortTreeNode node)
  {
   switch(gen_get_node_type(node))
     {
      case GEN_BINARY_MINUS:
      case GEN_BINARY_DIVIDE:
        return false;

      default:
	return true;
     }
  }


/*************************************************
 *
 *    LeftMostChild(node)
 *
 *    determine if node is a left-most child of a
 *    expression subtree
 *
 *   Input: node to be checked
 *
 *   Output: true if no node to the right in 
 *           expression, else false
 *
 *************************************************/


static Boolean LeftMostChild(FortTreeNode node)
  {
   FortTreeNode parent;

     parent = ast_get_father(node);
     if (NOT(IsOperator(parent)))
       return true;
     else
       if (IsUnaryOperator(parent) || WhichChild(parent,node) == LEFT)
         return LeftMostChild(parent);
       else
         return(false);
  }


/*************************************************
 *
 *    unp1_pred_parens(node)
 *
 *    determine if semantics of ast requires 
 *    parentheses around node 
 *
 *   Input: node to be checked
 *
 *   Output: true if parens are necessary,
 *           else false
 *
 *************************************************/

       
Boolean unp1_pred_parens(FortTreeNode node)
{
 FortTreeNode parent;

  /* ASSERT: relevant 'ft_AstSelect' has been done */

  if (gen_get_parens(node))    

     /* user specified parens, or determined
	during parsing */

    return true;
  else
    {
     parent = tree_out(node);
     if (IsOperator(parent) && IsOperator(node))
       if (IsBinaryOperator(parent) && IsBinaryOperator(node))
         if (OperatorInterpretation(parent) == WhichChild(parent,node))

	    /* if interpretation and child are the same then parens 
	       are only needed if the precedence of the child is 
	       less than the precedence of the parent */

           if (OperatorPrecedence(node) < OperatorPrecedence(parent))
	     return true;
           else
	     return false;
         else

	    /* if interpretation and child are the opposite then parens 
	       are only needed if the precedence of the child is 
	       less than the precedence of the parent or the precedences
	       are equivalent and the parent operator is not
	       associative. */

	   if ((OperatorPrecedence(node) < OperatorPrecedence(parent)) ||
	       ((OperatorPrecedence(node) == OperatorPrecedence(parent)) &&
		NOT(AssociativeOperator(parent))))
	     return true;
           else
	     return false;
       else
         if (IsBinaryOperator(parent) && IsUnaryOperator(node))

	   /* if a unary operator is a child of a binary operator
	      then parens are needed if the unary operator is 
	      a right child or if its parent does not require parentheses
	      and it is not the left-most child of the expression */

	   if (WhichChild(parent,node) == RIGHT)
	     return true;
           else
	     if (NOT(unp1_pred_parens(parent)))
	       if (NOT(LeftMostChild(parent)))
	         return true;
	       else
	         return false;
	     else
	       return false;
	 else


	    /* two successive unary operators means second requires
	       parentheses and a unary parent with a binary child
	       requires parentheses around child because of lower
	       precedence. */

	     return true;
     else
       return false;
    }
}




static
Boolean pred_first(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return is_first_in_list(node);
}




static
Boolean pred_placeholder(FortTreeNode node)
{
  /* ASSERT: relevant 'ft_AstSelect' has been done */
  return BOOL (is_place_holder(node));
}




/************************/
/*  Unparse accessors   */
/************************/




static
char * GET_TEXT(FortTreeNode node)
{
  META_TYPE type;
  char * s;
  int len,k;
  static char text[MAXNAMESIZE];

  /* ASSERT: relevant 'ft_AstSelect' has been done */
  if( is_place_holder(node) )
    { type = gen_get_meta_type(node);
      if( type == GEN_lbl_def  || type == GEN_lbl_ref )
        s = "lbl";
      else if( is_function_name(node) )
        s = "func-name";
      else if( is_subroutine_name(node) )
        s = "subr-name";
      else
        { (void) strcpy(text, gen_meta_type_get_text(type));
          s = text;
          len = strlen(s);
          for( k = 0;  k < len;  k++ )
            if( text[k] == '_' )  text[k] = '-';
        }
    }
  else
    s = gen_get_text(node);

  return s;
}




static
Boolean is_function_name(FortTreeNode node)
{
  FortTreeNode father = ast_get_father(node);
  META_TYPE type   = gen_get_meta_type(node);
  NODE_TYPE ftype  = NT(father);
  NODE_TYPE fftype = NT(ast_get_father(father));

  if( type == GEN_name )
    { if( ftype == GEN_FUNCTION || ftype == GEN_STMT_FUNCTION )
        return true;
      else if( ftype == GEN_INVOCATION )
        return NOT( fftype == GEN_CALL || fftype == GEN_TASK );
      else
        return false;
    }
  else
    return false;
}




static
Boolean is_subroutine_name(FortTreeNode node)
{
  FortTreeNode father = ast_get_father(node);
  META_TYPE type   = gen_get_meta_type(node);
  NODE_TYPE ftype  = NT(father);
  NODE_TYPE fftype = NT(ast_get_father(father));

  if( type == GEN_name )
    { if( ftype == GEN_SUBROUTINE || ftype == GEN_ENTRY )
        return true;
      else if( ftype == GEN_INVOCATION )
        return BOOL( fftype == GEN_CALL || fftype == GEN_TASK );
      else
        return false;
    }
  else
    return false;
}
