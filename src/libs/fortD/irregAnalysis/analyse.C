/* $Id: analyse.C,v 1.20 1997/06/24 17:38:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * The functions in this file perform analysis (mainly on the AST)
 * for handling irregular problems (inspectors, executors etc.)
 */

/**********************************************************************
 * Revision History:
 * $Log: analyse.C,v $
 * Revision 1.20  1997/06/24 17:38:57  carr
 * Support 64-bit Pointers
 *
 * Revision 1.19  1997/03/11  14:28:37  carr
 * newly checked in as revision 1.19
 *
Revision 1.19  94/03/21  14:28:59  patton
fixed comment problem`

Revision 1.18  94/02/27  20:15:08  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

 * Revision 1.26  1994/02/27  19:46:02  reinhard
 * Added sn2arr_node(), root2name_node(), str_tolower().
 *
 * Revision 1.25  1994/01/18  19:52:11  reinhard
 * Updated include paths according to Makefile change.
 * Handle value-based distributions.
 *
 * Revision 1.24  1993/10/04  17:28:15  reinhard
 * Change in val_is_irreg5():  call val_dep_parse() only if !ivar_flag.
 *
 * Revision 1.23  1993/10/04  15:41:00  reinhard
 * Added va_is_irreg*()'s + other functions.
 *
 * Revision 1.22  1993/09/25  23:03:30  reinhard
 * Robustified findlast_stmt()
 *
 * Revision 1.21  1993/09/25  15:38:08  reinhard
 * Added save_program().
 *
 */

#undef is_open
#include <string.h>
#include <strstream.h>
#include <assert.h>
#include <iostream.h>
#include <ctype.h>
#include <stdlib.h>
#include <libs/support/strings/rn_string.h>
#include <libs/fortD/irregAnalysis/analyse.h>
#include <libs/frontEnd/ast/AstIter.h>
#include <libs/support/database/context.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/ftExportSimple.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/moduleAnalysis/cfg/Cfg.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/moduleAnalysis/valNum/val.h>
#include <libs/fortD/irregAnalysis/Mall.h>
#include <libs/fortD/localInfo/fd_symtable.h>

/*------------------- GLOBAL DECLARATIONS -------------------*/

extern const char *valType[];          // values.C

EXTERN(AST_INDEX, add_else,        (AST_INDEX node));
EXTERN(int,       tarj_level2,     (TarjTree tarjans, CfgNodeId cn));
EXTERN(AST_INDEX, if2if_endif,     (AST_INDEX node));
EXTERN(Boolean,   is_loop,         (AST_INDEX stmt));
EXTERN(Boolean,   pt_is_loop_with_inductive, (AST_INDEX loop_node));


/*------------------ LOCAL DECLARATIONS ---------------------*/

EXTERN(int *,     qcount,          (void* data, int nel, int size,
				    Compare_type comp, int &count));
EXTERN(void *,    qcompress,       (void* data, int nel, int size,
				    Compare_type comp, int &count));
EXTERN(char *,    ftt_NodeToStr,     (FortTextTree ftt,
				      FortTreeNode node));
EXTERN(int,       int_comp,        (int *first, int *second));
EXTERN(const char *, find_pattern,        (const char *str,
					   const char *pattern));
EXTERN(const char *, find_pattern_insens, (const char *str,
					   const char *pattern));
EXTERN(char *,    concatStrs,      (const char *str1, const char *str2));
EXTERN(char *,    prefix_str,      (char prefix, const char *str));
EXTERN(char *,    prefix_type,     (int type, const char *str));
EXTERN(Boolean,   match_pattern,   (const char *&str, const char *pattern));
EXTERN(Boolean,   match_pattern_insens,   (const char *&str, const char *pattern));
EXTERN(Boolean,   match_boolean,   (const char *str));
EXTERN(Boolean,   str_prefix,      (const char *str, const char *prefix));
EXTERN(AST_INDEX, pt_loop_gen_cnt_node, (AST_INDEX loop_node));
EXTERN(int,       pt_loop_cnt,          (AST_INDEX loop_node)); 
EXTERN(Boolean,   pt_loop_is_const,     (AST_INDEX loop_node)); 
EXTERN(const char *, loop_gen_ivar,        (AST_INDEX node));
EXTERN(AST_INDEX, loop_gen_body,        (AST_INDEX loop_node));
EXTERN(AST_INDEX, loop_gen_body_last,   (CfgInstance cfg,
                                         AST_INDEX loop_node));
EXTERN(void,      cat_loop_triplet,     (AST_INDEX loop_node,
					 ostrstream &buf));
EXTERN(AST_INDEX, level2node,           (Values      val,
					 CfgInstance cfg,
					 AST_INDEX   start_node,
					 int         level));
EXTERN(void,      cat_CoVarPairs,       (CfgInstance  cfg,
					 FortTextTree ftt,
					 CoVarPair    *rv,
					 AST_INDEX    node,
					 int          level,
					 ostrstream   &buf));
EXTERN(void,      cat_CoVarPairs_field, (CfgInstance  cfg,
					 Values       val,
					 FortTextTree ftt,
					 CoVarPair    *rv,
					 AST_INDEX    node,
					 ValNumber    vn,
					 int          level,
					 int          max_level,
					 ostrstream   &buf,
					 AST_INDEX ind_node = AST_NIL));
EXTERN(Boolean,   is_list_or_NIL,       (AST_INDEX node));
EXTERN(Boolean,   is_reduction_type,    (NODE_TYPE node));
EXTERN(AST_INDEX, sn2arr_node,          (CfgInstance   cfg,
					 SymDescriptor symtab,
					 SsaNodeId     sn));
EXTERN(Boolean,   is_irreg_ref,  (AST_INDEX ref_node, CfgInstance cfg));
EXTERN(Boolean,   is_irreg_subsc, (Values val, ValNumber vn));
EXTERN(Boolean,   val_is_irreg2,   (Values    val,
                                   ValNumber vn));
EXTERN(Boolean,   val_is_irreg4,   (Values    val,
                                   ValNumber vn,
                                   Boolean   &ivar_flag,
                                   int       &limit_level));
EXTERN(Boolean,   val_is_irreg5,   (Values    val,
                                   ValNumber vn,
                                   Boolean   &ivar_flag,
                                   int       &limit_level,
                                   CoVarPair *&rv));
EXTERN(AST_INDEX, cfg_node_to_nearest_ast, (CfgInstance cfg,
					    CfgNodeId   cn,
					    Boolean     postdom));
EXTERN(AST_INDEX, cfg_node_to_near_ast, (CfgInstance cfg,
					 CfgNodeId   cn,
					 Boolean     change_source = false));
EXTERN(Boolean,   cfg_is_branch,        (CfgInstance cfg, CfgNodeId cn,
					 AST_INDEX   &node,
					 Boolean     change_source));
EXTERN(Boolean,   cfg_is_if,            (CfgInstance cfg, CfgNodeId cn,
					 AST_INDEX &node));
EXTERN(Boolean,   pt_is_cond_jump,     (AST_INDEX &node,
					 Boolean change_source = false));
EXTERN(Boolean,   pt_is_label,          (AST_INDEX &node,
					 Boolean change_source = false));
EXTERN(void,      pt_extract_label,     (AST_INDEX node));
EXTERN(AST_INDEX, cfg_node_to_ast_func, (CfgInstance    cfg,
					 CfgNodeId      cn,
					 Boolean        prepend,
					 ListInsertFunc &insert_func,
					 Boolean skip_comments = true));
EXTERN(AST_INDEX, cfg_node_to_predom_ast, (CfgInstance cfg,
					   CfgNodeId   cn));
EXTERN(AST_INDEX, cfg_node_to_postdom_ast, (CfgInstance cfg,
					   CfgNodeId   cn));
EXTERN(const char *,    ref2varName,     (AST_INDEX ref_node));
EXTERN(void,      catRefStr,       (AST_INDEX ref_node, const char *res_str));
EXTERN(void,      catRefBuf,       (AST_INDEX  ref_node,
				    ostrstream &res_buf));
EXTERN(AST_INDEX, begin_ast_stmt,  (AST_INDEX node));
EXTERN(AST_INDEX, next_executable_stmt,  (AST_INDEX node));
EXTERN(CfgNodeId, cfg_node_from_near_ast, (CfgInstance cfg,
					   AST_INDEX node));
EXTERN(AST_INDEX, find_outmost_loop, (AST_INDEX node));
EXTERN(AST_INDEX, find_enclosing_DO, (AST_INDEX node));
EXTERN(AST_INDEX, root2name_node,    (AST_INDEX node));
EXTERN(Boolean,   node_is_root,      (AST_INDEX node));
EXTERN(Boolean,   is_in_main,        (AST_INDEX node));
EXTERN(AST_INDEX, node2root,         (AST_INDEX node));
EXTERN(STR_INDEX, root2name,         (AST_INDEX node));
EXTERN(const char *,    root2name_str,     (AST_INDEX node));
EXTERN(AST_INDEX, find_stmt_list,  (AST_INDEX node));
EXTERN(AST_INDEX, find_last_stmt,  (AST_INDEX node));
EXTERN(AST_INDEX, get_stmt_list,              (AST_INDEX node));
EXTERN(AST_INDEX, assert_is_subscript,        (AST_INDEX node));
EXTERN(AST_INDEX, assert_father_is_subscript, (AST_INDEX node));
EXTERN(char *,    str_tolower,                (const char *str));
EXTERN(const char *,    ftt_ast2str,     (FortTextTree ftt, AST_INDEX node));
EXTERN(AST_INDEX, find_next_loop,  (AST_INDEX start_node));
EXTERN(AST_INDEX, move_before_comments, (AST_INDEX node));
EXTERN(Boolean,   vn_no_duplicates, (CfgInstance cfg, ValNumber vn));
EXTERN(const char *,    node2name,     (AST_INDEX expr_node));
EXTERN(int,       count_nest_iters, (CfgInstance cfg,
				     AST_INDEX   subsc_node,
				     AST_INDEX   lim_node));
EXTERN(int,       tarj_level_max,  (CfgInstance cfg));
EXTERN(const char *,    save_program,    (FortTree ft, FortTextTree ftt,
				    const char *extension));


/*********************************************************************
 * qcount()  Counts # of different entries in list.
 *           Same format as qsort() from stdlib.h.
 * NOTE: this returns a newly allocated array of pointers, which must
 *       then be deallocated by the caller.
 */
int*                         // # of occurences of datum[1:result]
qcount(void*        data,    // Input data
       int          nel,     // # of input data
       int          size,    // Size of each input datum
       Compare_type comp,    // Comparison function
       int          &count)  // # of different input data
{
  int  *raw_counts = new int[nel];
  int  *counts;
  char *prev   = (char *) data;
  //char *finish = (char *) (data + nel*size);
  char *finish = ((char*)data + nel*size);
  
  if (nel > 0) {                   // Are there any input data ?
    qsort(data, nel, size, comp);  // Sort the given list
    
    count = 0;
    raw_counts[0] = 1;
    for (char* current = prev + size; current < finish; current += size)
    {
      if (comp(prev, current))     // prev, current are different ?
      {
	count++;
	raw_counts[count] = 0;
	prev = current;
      }
      raw_counts[count]++;
    }
    count++;
    
    // Copy <raw_counts> into (potentially) smaller array <counts>
    counts = new int[count];
    for (int i = 0;  i < count; i++) {
      counts[i] = raw_counts[i];
    }
    delete raw_counts;
  }
  else                               // No input data ?
  {
    count = 0;
    counts = NULL;
  }
  
  return counts;
}


/*********************************************************************
 * qcompress()  Eliminates duplicates in input list.
 *              Same format as qsort() from stdlib.h.
 * NOTE: this returns a newly allocated array of pointers, which must
 *       then be deallocated by the caller.
 */
void *
qcompress(void*        data,    // Input data
	  int          nel,     // # of input data
	  int          size,    // Size of each input datum
	  Compare_type comp,    // Comparison function
	  int          &count)  // # of different input data
{
  int  *counts;
  //void *compressed_data;
  char *compressed_data;
  int  compressed_size;
  int  i, indx;

  counts = qcount(data, nel, size, comp, count);  // Sort the given list
  compressed_size = count * size;
  compressed_data = new char[compressed_size];

  for (indx = 0, i = 0;  i < count; indx += counts[i], i++) {
    (void) memcpy(&compressed_data[i*size],
		  &(((char*)data)[indx*size]), size);
  }
  delete counts;

  return compressed_data;
}


/*********************************************************************
 * int_comp()  int comparison function, as used by qsort().
 */
int
int_comp(int *first, int *second)
{
  return (*first - *second);
}


/**********************************************************************
 * match_pattern()  IF <pattern> matches beginning of <str>,
 *                  THEN advance <str> after <pattern>, return TRUE,
 *                  else return FALSE.
 */
Boolean 
match_pattern(const char *&str, const char *pattern)
{
  Boolean match = (Boolean) !strncmp(str, pattern, strlen(pattern));

  if (match) {
    str = &str[strlen(pattern)];
  }

  return match;
}


/**********************************************************************
 * match_pattern_insens()  Like match_pattern(), but converts str to
 *                         lower case.
 */
Boolean 
match_pattern_insens(const char *&str, const char *pattern)
{
  char    *str_lower = str_tolower(str);
  Boolean match      = (Boolean) !strncmp(str_lower, pattern, strlen(pattern));
  delete str_lower;

  if (match) {
    str = &str[strlen(pattern)];

    while (*str == ' ') str++; // Eat spaces    
  }

  return match;
}


/**********************************************************************
 * find_pattern()   IF <pattern> occurs in <str>,
 *                  THEN return rest after <pattern>,
 *                  else return 0.
 */
const char *
find_pattern(const char *str, const char *pattern)
{
  const char *res = strstr(str, pattern);

  if (res)
  {
    res = &res[strlen(pattern)];
  }

  return res;
}


/**********************************************************************
 * find_pattern_insens()   IF <pattern> occurs in <str>,
 *                         THEN return rest after <pattern>,
 *                         else return 0.
 */
const char *
find_pattern_insens(const char *str, const char *pattern)
{
  int        offset;
  char       *str_lower = str_tolower(str);
  const char *res       = strstr(str_lower, pattern);

  if (res)
  {
    // Want ptr to original string, not to lower case version
    offset = res - str_lower + strlen(pattern);
    res = &str[offset];
  }
  delete str_lower;

  return res;
}


/**********************************************************************
 * str_prefix()  True iff  <prefix> is prefix of <str>
 */
Boolean
str_prefix(const char *str, const char *prefix)
{
  return (Boolean) !strncmp(str, prefix, strlen(prefix));
}


/**********************************************************************
 * concatStrs()  Concatenate two strings
 */
char * 
concatStrs(const char *str1, const char *str2)
{
  int   len  = strlen(str1) + strlen(str2) + 1;
  char  *str = new char[len];

  (void) strcpy(str, str1);
  (void) strcat(str, str2);

  return str;
}


/**********************************************************************
 * prefix_str()  Prefix <str> with <prefix>
 */
char * 
prefix_str(char prefix, const char *str)
{
  int   len  = strlen(str) + 2;
  char  *new_str = new char[len];

  new_str[0] = prefix;
  (void) strcpy(&new_str[1], str);

  return new_str;
}


/**********************************************************************
 * prefix_with_type()  Prefix <str> according to <type>
 */
char * 
prefix_type(int type, const char *str)
{
  int  len      = strlen(str) + 2;
  char *new_str = new char[len];
  char prefix   = MallType_prefixes[type];

  new_str[0] = prefix;
  (void) strcpy(&new_str[1], str);

  return new_str;
}


/**********************************************************************
 * match_boolean()  Try to match boolean input value.
 */
Boolean
match_boolean(const char *str)
{
  Boolean bool = (Boolean) (str_prefix(str, "true")
			    || str_prefix(str, "1"));

  if (!(bool
	|| str_prefix(str, "false")
	|| str_prefix(str, "0"))) {
    cerr << "WARNING match_boolean(): Could not match boolean in \"" <<
      str << "\"\n";
  }

  return bool;
}


/**********************************************************************
 * pt_loop_gen_cnt_node()  Generate an expression for the # of
 *                         iterations of the given loop.
 */
AST_INDEX
pt_loop_gen_cnt_node(AST_INDEX loop_node)
{
  int       iter_cnt;
  AST_INDEX iter_cnt_node;
  Boolean   loop_normal_flag;

  assert (is_do(loop_node));           // Make sure it's a loop node

  (void) pt_loop_get_count(loop_node, &iter_cnt, &iter_cnt_node,
			   &loop_normal_flag);

  return iter_cnt_node;
}


/**********************************************************************
 * pt_loop_cnt()  Count the iterations of the given loop.
 */
int
pt_loop_cnt(AST_INDEX loop_node)
{
  int       iter_cnt;
  AST_INDEX iter_cnt_node;
  Boolean   iter_cnt_flag;
  Boolean   loop_normal_flag;

  assert (is_do(loop_node));   // Make sure it's a loop node

  if (pt_is_loop_with_inductive(loop_node))
  {
    iter_cnt_flag = pt_loop_get_count(loop_node, &iter_cnt,
				      &iter_cnt_node, &loop_normal_flag);

    tree_free(iter_cnt_node);    // We don't want the expression tree
  }
  else
  {
    iter_cnt_flag = false;
  }

  if (!iter_cnt_flag) {
    iter_cnt = UNKNOWN_SIZE;
  }

  return iter_cnt;
}


/**********************************************************************
 * pt_loop_is_const()  Determine whether <loop_node> has a const # of
 *                     iterations.
 */
Boolean
pt_loop_is_const(AST_INDEX loop_node)    
{    
  int       iter_cnt;
  AST_INDEX iter_cnt_node;
  Boolean   iter_cnt_flag;
  Boolean   loop_normal_flag;

  assert (is_do(loop_node));  // Make sure it's a loop node

  iter_cnt_flag = pt_loop_get_count(loop_node, &iter_cnt,
				    &iter_cnt_node, &loop_normal_flag);

  tree_free(iter_cnt_node);   // We don't want the expression tree

  return iter_cnt_flag;
}


/**********************************************************************
 * loop_gen_ivar()  Generate induction variable of given loop.
 */
const char *
loop_gen_ivar(AST_INDEX loop_node)
{
  assert (is_do(loop_node));           // Make sure it's a loop node

  return gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(loop_node)));
}


/**********************************************************************
 * loop_gen_body()  Select body of given loop.
 */
AST_INDEX
loop_gen_body(AST_INDEX loop_node)
{
  AST_INDEX body_node;

  if (is_do(loop_node))
  {
    body_node = gen_DO_get_stmt_LIST(loop_node);
  }
  else
  {
    body_node = list_next(loop_node);
  }

  return body_node;
}


/**********************************************************************
 * loop_gen_body_last()  If <loop_node> is a DO, return body stmt list;
 *                       otherwise, return  last stmt in body.
 */
AST_INDEX
loop_gen_body_last(CfgInstance cfg, AST_INDEX loop_node)
{
  AST_INDEX  body_node;
  CfgNodeId  loop_id, body_id;
  TarjTree   tarjans;

  if (is_do(loop_node))
  {
    body_node = gen_DO_get_stmt_LIST(loop_node);
    //body_node = list_last(body_node);
  }
  else
  {
    tarjans   = cfg_get_intervals(cfg);
    loop_id   = cfg_node_from_ast(cfg, loop_node);   // Find cfg node
    body_id   = tarj_inners_last(tarjans, loop_id);
    body_node = cfg_node_to_predom_ast(cfg, body_id);
    //body_node = list_next(loop_node);
  }

  return body_node;
}


/**********************************************************************
 * cat_loop_triplet  Generate string of the form "1:100" or "1:100:2"
 */
void
cat_loop_triplet(AST_INDEX loop_node, ostrstream &buf)
{
  AST_INDEX var_node;
  AST_INDEX lo_node, hi_node, step_node; // Bounds & step 
  int       lo, hi, step;               
  Boolean   lo_flag, hi_flag, step_flag;

  assert (is_do(loop_node));           // Make sure it's a loop node

  pt_get_loop_bounds(loop_node, &var_node, &lo_node, &hi_node,
		     &step_node);
  lo_flag   = pt_eval(lo_node, &lo);
  hi_flag   = pt_eval(hi_node, &hi);
  
  if (step_node == AST_NIL) {
    step      = 1;
    step_flag = false;
  } else {
    step_flag = pt_eval(step_node, &step);
  }

  if (lo_flag) {
    cerr << "WARNING cat_loop_triplet(): " <<
      "non-constant lower bound, loop_node = " << loop_node << ".\n";
    buf << "???:";
  } else {
    buf << lo << ":";
  }

  if (hi_flag) {
    cerr << "WARNING cat_loop_triplet(): " <<
      "non-constant higher bound, loop_node = " << loop_node << ".\n";
    buf << "???";
  } else {
    buf << hi;
  }

  if (step_flag) {
    cerr << "WARNING cat_loop_triplet(): " <<
      "non-constant stride, loop_node = " << loop_node << ".\n";
    buf << ":???";
  } else {
    if (step != 1) {
      buf << ":" << step;
    }
  }
}


/**********************************************************************
 * level2node()  Return level <level> loop enclosing <start_node>
 */
AST_INDEX
level2node(Values      val,
	   CfgInstance cfg,
	   AST_INDEX   start_node,
	   int         level)
{
  AST_INDEX node;
  CfgNodeId id;
  TarjTree  tarjans = cfg_get_intervals(cfg);  // Interval tree
  
  // Walk outward on the interval tree until we reach the loop at
  // nesting level <level>
  for (id = cfg_containing_loop(cfg, start_node);
       tarj_level2(tarjans, id) >= level;
       id = tarj_outer(tarjans, id));
  
  // Retrieve the AST_INDEX corresponding to cfg node <id>
  node = cfg_node_to_ast(cfg, id);
  
  if (node == AST_NIL)
  {
    cerr << "WARNING level2node(): node == AST_NIL for start_node = "
      << start_node << ", level = " << level << ", id = " << id <<
	".\n";
  }
  
  return node;
}


/**********************************************************************
 * cat_CoVarPairs()
 *
 * Example:
 * --------
 *   do i = 1, 100                                     // Level 1
 * C   gather (x(i + 2*10 - 3 : i + 2*50 - 3 : 2*5))   // Level 1
 *     do j = 10, 50, 5                                // Level 2
 *       ... = x(i + 2*j - 3)                          // Level 2
 *     enddo
 *   enddo
 *
 * Given the <level> of the "gather" and the <node> and <rv> for the
 * "x(i + 2*j - 3)" reference, generate the
 * "x(i + 2*2 - 3 : i + 2*50 - 3)."
 *
 * Example:
 * --------
 *   do i = 1, 100
 *     do j = 10, 50, 3
 * C     gather (x(i + 2*j - 3))
 *       ... = x(i + 2*j - 3)
 *     enddo
 *   enddo
 *
 * Given the <level> of the "gather" and the <node> and <rv> for the
 * "x(i + 2*j - 3)" reference, generate the "x(i + 2*j - 3)."
 *
 * Current constraints:
 * --------------------
 * - <level> encloses at most one loop in which <node> is dependent;
 *   i.e., in the above examples, the gather cannot be hoisted out of
 *   the i loop.
 */
void
cat_CoVarPairs(CfgInstance  cfg,
	       FortTextTree ftt,
	       CoVarPair    *rv,    // Loop info for <node>
	       AST_INDEX    node,   // Subscript node
	       int          level,  // Print relative to <level>
	       ostrstream   &buf)   // Print into <buf>
{
  int         cur_level;
  int         coeff;
  ValNumber   sym;
  AST_INDEX   loop_node, control_node, ind_node;
  const char  *ind_str;
  Boolean     empty_buf = true;
  int         vec_level = -1;
  Values      val       = cfgval_get_values(cfg);
  ValNumber   vn        = cfgval_get_val(cfg, node);
  int         max_level = val_get_level(val, vn);

  // Make sure we can handle this <rv>
  for (cur_level = 0; cur_level <= max_level; cur_level++)
  {
    sym   = rv[cur_level].sym;
    if (!val_is_const(val, sym))
    {
      cerr << "WARNING cat_CoVarPairs(): Ignoring symbolic value " <<
	sym << " at level " << cur_level << ".\n";
    }
  }

  // Determine whether we have to print a triplet;
  // ie, whether there is an enclosed loop we depend on
  for (cur_level = level+1; cur_level <= max_level; cur_level++)
  {
    coeff = rv[cur_level].coeff;
    if (coeff != 0)                // Dependent on this ivar ?
    {
      if (vec_level >= 0)          // Already dependent on other ivar ?
      {
	cerr << "WARNING cat_CoVarPairs(): " <<
	  "Can't handle subscript \"" << ftt_ast2str(ftt, node) <<
	    "\", node = " << node << ".\n";
      }
      else
      {
	vec_level = cur_level;
      }
    }
  }

  if (vec_level < 0)  // We don't have a vector of values ?
  {
    // Just print the subscript
    cat_CoVarPairs_field(cfg, val, ftt, rv, node, vn, level, max_level,
			 buf);
  }
  else
  {
    // Print a triplet
    loop_node    = level2node(val, cfg, node, vec_level);
    if (is_loop(loop_node))
    {
      control_node = gen_get_loop_control(loop_node);

      // Print lower bound
      ind_node = gen_INDUCTIVE_get_rvalue1(control_node);
      cat_CoVarPairs_field(cfg, val, ftt, rv, node, vn, level,
			   max_level, buf, ind_node);
      
      // Print upper bound
      buf << ":";
      ind_node = gen_INDUCTIVE_get_rvalue2(control_node);
      cat_CoVarPairs_field(cfg, val, ftt, rv, node, vn, level,
			   max_level, buf, ind_node);

      // Print step
      ind_node = gen_INDUCTIVE_get_rvalue3(control_node);
      if (ind_node != AST_NIL)
      {
	buf << ":";
	coeff = rv[vec_level].coeff;
	if (coeff != 1)             // Non-unit coefficient ?
	{
	  buf << coeff << "*";      // Print "4*"
	}
	ind_str = ftt_ast2str(ftt, ind_node);
	buf << ind_str;
      }
    }
    else
    {
      cerr << "WARNING cat_CoVarPairs(): loop_node = " << loop_node <<
	" is not a loop; node = " << node << ".\n";
    }
  }
}


/**********************************************************************
 * cat_CoVarPairs_field()  Helper for cat_CoVarPairs()
 */
void
cat_CoVarPairs_field(CfgInstance  cfg,
		     Values       val,
		     FortTextTree ftt,
		     CoVarPair    *rv,   // Loop info for <node>
		     AST_INDEX    node,  // Subscript node
		     ValNumber    vn,    // Value of <node>
		     int          level, // Print relative to <level>
		     int          max_level, // level of <node>
		     ostrstream   &buf,      // Print into <buf>
		     AST_INDEX    ind_node)  // Loop bound node
{
  int        cur_level;
  int        coeff;
  Boolean    empty_buf = true;
  AST_INDEX  loop_node;
  const char *ind_str;

  // Print loop variant part
  for (cur_level = 1; cur_level <= max_level; cur_level++)
  {
    coeff = rv[cur_level].coeff;
    if (coeff != 0)        // Dependent on this ivar ?
    {
      if (!empty_buf)
      {
	if (coeff > 0) {
	  buf << "+";
	} else {
	  buf << "-";
	  coeff = -coeff;
	}
      }
      empty_buf = false;
      if (coeff != 1)             // Non-unit coefficient ?
      {
	buf << coeff << "*";      // Print "4*"
      }

      // Examples for <level> of reference, <cur_level> of loop:
      // gather x(1:100, 10:20)  # level     0 (from cfg: cur_level 1)
      // do i = 1, 100           # cur_level 1
      //   gather x(i, 10:20)    # level     1 (from cfg: cur_level 2)
      //   do j = 10, 20         # cur_level 2
      //     gather x(i, j)      # level     2
      //     ... x(i, j) ...
      if (level < cur_level)      // Outside of loop => print bound
      {
	if (ind_node == AST_NIL)
	{
	  cerr << "WARNING cat_CoVarPairs_field(): " <<
	    "ind_node == AST_NIL.\n";
	}
	else
	{
	  ind_str = ftt_ast2str(ftt, ind_node);
	  buf << ind_str;
	}
      }
      else                        // Inside of loop => print var
      {
	loop_node = level2node(val, cfg, node, cur_level);
	buf << loop_gen_ivar(loop_node); // Print "i"	
      }
    }
  }
	  
  // Print loop invariant part
  coeff = rv[0].coeff;
  if (coeff != 0)        // Dependent on this ivar ?
  {
    if (!empty_buf)
    {
      if (coeff > 0) {
	buf << "+";
      } else {
	buf << "-";
	coeff = -coeff;
      }
    }
    buf << coeff;
  }
}


/**********************************************************************
 * is_list_or_NIL()  Guard for parameters passed to list processing
 *                   routines
 */
Boolean
is_list_or_NIL(AST_INDEX node)
{
  return BOOL(node == AST_NIL || is_list(node));
}


/**********************************************************************
 * is_reduction_type()  Determine whether this is a binary reduction op
 */
Boolean
is_reduction_type(NODE_TYPE node)
{
  return (Boolean) ((node == GEN_BINARY_PLUS)
		    || (node == GEN_BINARY_MINUS)
		    || (node == GEN_BINARY_TIMES)
		    || (node == GEN_BINARY_DIVIDE));
}


/*********************************************************************
 * sn2arr_node()  Check whether <sn> references an array.
 */
AST_INDEX
sn2arr_node(CfgInstance cfg, SymDescriptor symtab, SsaNodeId sn)
{
  fst_index_t   node_name;
  AST_INDEX     ref_node;
  AST_INDEX     arr_node = AST_NIL;

  // Does <sn> use/define an array variable ?
  ref_node = ssa_node_to_ast(cfg, sn);
  if (ref_node != AST_NIL)
  {
    node_name = ssa_node_name(cfg, sn);
    if ((node_name != NIL)
	&& FS_IS_ARRAY(symtab, node_name))
    {
      arr_node = ref_node;
    }
  }

  return arr_node;
}


/*********************************************************************
 * is_irreg_ref()  Check whether <sn> irregularly references an array.
 */
Boolean
is_irreg_ref(AST_INDEX ref_node, CfgInstance cfg)
{
  AST_INDEX     subsc_node;
  AST_INDEX     subsc_list;
  ValNumber     vn;
  Values        val      = cfgval_get_values(cfg);
  Boolean       is_irreg = false;

  // 5/25/93 RvH: The handling of unsubscripted array references
  //              probably needs some more work
  if (is_subscript(ref_node))  // Does it have a subscript ?
  {
    // Loop over all subscripts
    subsc_list = gen_SUBSCRIPT_get_rvalue_LIST(ref_node);
    for (subsc_node = list_first(subsc_list);
	 !is_irreg && (subsc_node != AST_NIL);
	 subsc_node = list_next(subsc_node))
    {
      // Check whether subscript is irregular
      vn       = cfgval_get_val(cfg, subsc_node);
      //is_irreg = is_irreg_subsc(val, vn);
      is_irreg = val_is_irreg2(val, vn);
    }
  }

  return is_irreg;
}


/*********************************************************************
 * is_irreg_subsc()  Check whether <vn> is an irregular subscript.
 *
 * 5/27/93 RvH: for now, the following subscript types are classified as
 * regular:
 *
 * - constants ("100"),
 * - induction variables ("i"),
 * - linear combinations of regular subscript types ("i+1", "4*i-3-k").
 *
 * Eventually this has to be closer coordinated with the "regular"
 * compiler.
 */
Boolean
is_irreg_subsc(Values val, ValNumber vn)
{
  ValType   vn_type;
  Boolean   is_irreg;
  CoVarPair *rv;
  int       level, cur_level;

  vn_type  = val_get_val_type(val, vn);
  is_irreg = (Boolean) (!(vn_type == VAL_CONST)
			&& !(vn_type == VAL_IVAR)
			&& !(vn_type == VAL_OK_MU));

  if (is_irreg)      // Subscript not proven regular yet ?
  {
    //is_irreg = (vn_type == VAL_ARRAY);
    level = val_get_level(val, vn);
    rv = val_dep_parse(val, vn, level);
    for (cur_level = 0;
         (cur_level <= level)
         && (val_is_const(val, rv[cur_level].sym));
         cur_level++);
    val_dep_free(rv);
    is_irreg = (Boolean) (cur_level <= level);  // Any symbolic parts ?
  }

  return is_irreg;
}


/*********************************************************************
 * val_is_irreg2()  Check whether <vn> is an irregular subscript.
 */
Boolean
val_is_irreg2(Values    val,
             ValNumber vn)
{
  Boolean   ivar_flag;
  int       limit_level;
  CoVarPair *rv;

  return val_is_irreg5(val, vn, ivar_flag, limit_level, rv);
}


/*********************************************************************
 * val_is_irreg4()  Check whether <vn> is an irregular subscript.
 */
Boolean
val_is_irreg4(Values    val,
             ValNumber vn,
             Boolean   &ivar_flag,
             int       &limit_level)
{
  CoVarPair *rv;

  return val_is_irreg5(val, vn, ivar_flag, limit_level, rv);
}


/*********************************************************************
 * val_is_irreg5()  Check whether <vn> is an irregular subscript.
 */
Boolean
val_is_irreg5(Values    val,
             ValNumber vn,
             Boolean   &ivar_flag,
             int       &limit_level,
             CoVarPair *&rv)
{
  Boolean   irreg_flag;
  ValNumber sym;
  int       level, cur_level;
  int       coeff;
  ValType   vn_type = val_get_val_type(val, vn);

  level      = val_get_level(val, vn);
  irreg_flag = false;
  ivar_flag  = (Boolean) (vn_type == VAL_OK_MU);  // Is an aux ind var ?

  if (!ivar_flag)
  {
    rv = val_dep_parse(val, vn, level);
    for (cur_level = 1;
         (cur_level <= level) && !irreg_flag;
         cur_level++)
    {
      coeff     = rv[cur_level].coeff;
      ivar_flag = (Boolean) ((cur_level > 0) && (coeff != 0));
      if (ivar_flag && (limit_level < 0))
      {
        limit_level = cur_level;
      }
      sym        = rv[cur_level].sym;
      irreg_flag = (Boolean) !val_is_const(val, sym);  // Any symbolic part ?
    }
  }

  return irreg_flag;
}


/**********************************************************************
 * cfg_node_to_near_ast()  Given <cn>, try to find DIRECTLY
 *                         corresponding <node> such that prepending /
 *                         appending to <node> works correctly.
 */
AST_INDEX
cfg_node_to_near_ast(CfgInstance cfg,
		     CfgNodeId   cn,
		     Boolean     change_source)
{
  AST_INDEX node;

  node = cfg_node_to_ast(cfg, cn);

  switch (ast_get_node_type(node))
  {
  case AST_NULL_NODE:
    (void) cfg_is_branch(cfg, cn, node, change_source);
    break;

  case GEN_GOTO:
  case GEN_RETURN:
    (void) pt_is_cond_jump(node, change_source);
    break;

  case GEN_GUARD:
    (void) cfg_is_if(cfg, cn, node);
    break;

  case GEN_LABEL_DEF:
    (void) pt_is_label(node, change_source);
    break;

  case GEN_PROGRAM:
  case GEN_SUBROUTINE:
  case GEN_FUNCTION:
    node = AST_NIL;    // Don't want to hoist things out of the program
    break;
  }

  return node;
}


/**********************************************************************
 * cfg_is_branch()  Check whether <cn> is at the beginning of a branch.
 *                  Add empty else if necessary.
 */ 
Boolean
cfg_is_branch(CfgInstance cfg,
	      CfgNodeId   cn,
	      AST_INDEX   &node,
	      Boolean     change_source)
{
  AST_INDEX prev_node, next_node, guard_list;
  CfgNodeId prev_cn, next_cn;
  CfgEdgeId in_edge;
  Boolean   is_branch;

  is_branch = (Boolean) 
    (// No direct mapping found ?
     ((node = cfg_node_to_ast(cfg, cn)) == AST_NIL)

     // Only one outgoing edge ?
     && (cfg_node_fanout(cfg, cn)
	 == 1)

     // Only one incoming edge ?
     && (cfg_node_fanin(cfg, cn)
	 == 1)

     // Valid preceeding node ?
     && ((prev_cn = cfg_edge_src(cfg, in_edge =
				 cfg_first_to_cfg(cfg, cn)))
	 != CFG_NIL)

     // Directly corresponding AST node for preceeding node ?
     && ((prev_node = cfg_node_to_ast(cfg, prev_cn))
	 != AST_NIL)

     // This AST node is a guard ?  Or a logical if ?
     && ((is_guard(prev_node)) || (is_logical_if(prev_node))));

  if (is_branch
      
      // Valid successor node ?
      && ((next_cn = cfg_edge_dest(cfg, cfg_first_from_cfg(cfg, cn)))
	  != CFG_NIL))
  {
    if (is_guard(prev_node))
    {
      // Check whether we are at an already existing branch
      if (// Directly corresponding AST node for successor node ?
	  ((next_node = cfg_node_to_ast(cfg, next_cn)) != AST_NIL)

	  // <cn> dominates successor node ?
	  && (dom_idom(cfg_get_predom(cfg), next_cn) == cn))
      {
	// Grab guard, to make sure things are inserted BEFORE next_node
	node = prev_node;
      }
      else
      {
	// Incoming edge has label 0 ?
	if (cfg_edge_label(cfg, in_edge) == 0)
	{
	  // Is there already an added else branch ?
	  node = list_next(prev_node);
	  if (node == AST_NIL)
	  {
	    // Are we supposed to add an empty else branch ?
	    if (change_source)
	    {
	      node = add_else(prev_node);
	    }
	  }
	  else
	  {
	    if (is_guard(node))
	    {
	      node = gen_GUARD_get_stmt_LIST(node);
	    }
	    else
	    {
	      cerr << "WARNING cfg_is_branch(): " <<
		"node = " << node << 
		  " is not a guard.\n";
	    }
	  }
	}
      }
    }
    else if (is_logical_if(prev_node) && change_source)
    {
      node       = if2if_endif(prev_node);
      guard_list = gen_IF_get_guard_LIST(node);
      node       = list_first(guard_list);

      // Incoming edge has false label ?
      if (cfg_edge_label(cfg, in_edge) == 0)
      {
	node = add_else(node);
      }
    }
  }

  return is_branch;
}


/**********************************************************************
 * cfg_is_if()  Check whether <cn> is a guard.
 *              If so, then make sure stmts get mapped apropriately.
 */
Boolean
cfg_is_if(CfgInstance cfg,
	  CfgNodeId   cn,
	  AST_INDEX   &node)
{
  int     fanout;
  Boolean is_if;

  // Check whether <node> corrsponds to an IF
  is_if = (Boolean) is_guard(node);

  if (is_if)
  {
    fanout = cfg_node_fanout(cfg, cn);
    switch (fanout) {
    case 1:
      // Map <cn> to first stmt in guard
      assert(is_list(node = gen_GUARD_get_stmt_LIST(node)));
      node = list_first(node);
      break;
      
    case 2:
      // Map <cn> to the IF (instead of the guard of the true branch)
      assert(is_list(node = ast_get_father(node)));
      assert(is_if(node = ast_get_father(node)));
      break;
    }
  }

  return is_if;
}


/**********************************************************************
 * pt_is_cond_jump()  Check whether <node> is a conditional jump.
 *                    If so, then convert it into an IF-ENDIF with
 *                    a GOTO in it.
 */
Boolean
pt_is_cond_jump(AST_INDEX &node,
		Boolean   change_source)
{
  AST_INDEX stmt_list, log_if_node;
  Boolean   is_cond_jump;

  // Check whether <node> is conditional jump
  is_cond_jump = (Boolean) ((is_goto(node) || is_return(node))
    && is_list(stmt_list = ast_get_father(node))
      && is_logical_if(log_if_node = ast_get_father(stmt_list)));

  if (is_cond_jump)
  {
    if (change_source)
    {
      node = if2if_endif(log_if_node);
    }
    else
    {
      node = AST_NIL;
    }
  }

  return is_cond_jump;
}


/**********************************************************************
 * pt_is_label()  Check whether <node> is a label.
 *                If so, then generate a continue stmt and move
 *                the label to it.
 */
Boolean
pt_is_label(AST_INDEX &node,
	    Boolean   change_source)
{
  Boolean   is_label;

  // Check whether <node> is a label
  is_label = (Boolean) is_label_def(node);

  if (is_label)
  { 
    if (change_source)
    {
      // Lookup statement where label belongs
      node = ast_get_father(node);

      // Move label up into a new CONTINUE
      pt_extract_label(node);
    }
    else
    {
      node = AST_NIL;
    }
  }

  return is_label;
}


/**********************************************************************
 * pt_extract_label()  Check whether <node> has a label.
 *                     If so, then generate a continue stmt before
 *                     <node>and move the label to it.
 */
void
pt_extract_label(AST_INDEX node)    
{
  AST_INDEX cont_node, label_node;

  if ((labelled_stmt(node))
      && (is_label_def(gen_get_label(node))))
  { 
    // Grab label
    label_node = gen_get_label(node);
    
    // Generate CONTINUE w/ same label
    cont_node = gen_CONTINUE(tree_copy(label_node));
    
    // Delete old label
    pt_tree_replace(label_node, AST_NIL);
    
    // Insert CONTINUE before statement
    (void) list_insert_before(node, cont_node);
  }
}


/**********************************************************************
 * cfg_node_to_nearest_ast()  Given <cn>, try to find the nearest
 *                            <node> such that prepending / appending
 *                            to <node> works correctly.
 *
 *  For prepending, use cfg_get_mydom = cfg_get_postdom;
 *  for appending,  use cfg_get_mydom = cfg_get_predom.
 */
AST_INDEX
cfg_node_to_nearest_ast(CfgInstance cfg,
			CfgNodeId   cn,
			Boolean     postdom)
{
  AST_INDEX node;
  DomTree   dom;                      // Predominator/Postdominator tree
  CfgNodeId next_cn = cn;
  
  node = cfg_node_to_ast(cfg, cn);    // Try to map <cn> to <node>
  
  if ((node == AST_NIL)               // No direct mapping found ?
      && (cn != cfg_start_node(cfg))) // Not the CFG root ?
  {
    // Generate pre/postdominator tree
    dom = postdom ? cfg_get_postdom(cfg) :  cfg_get_predom(cfg);
    next_cn = cn;
    do {
      next_cn = dom_idom(dom, next_cn);   // Find pre/postdominator
      if (next_cn == CFG_NIL) {
	cerr << "WARNING cfg_node_to_nearest_ast(): " <<
	  "cannot find <node> for <cn> = " << cn << ".\n";
      } else {
	node = cfg_node_to_ast(cfg, next_cn);
      }
    } while ((next_cn != CFG_NIL) && (node == AST_NIL));
  }
  //else
  (void) cfg_is_if(cfg, cn, node);
  
  if (node == AST_NIL) {
    cerr << "WARNING cfg_node_to_nearest_ast(): cn = " << cn <<
      ", node == AST_NIL.\n";
  }
  
  return node;
}


/**********************************************************************
 * cfg_node_to_ast_func()  Assuming an ast node should be attached to
 *                         <cn>, find the corresponding ast <node> and
 *                         with which list operation the node should be
 *                         attached to <node>.
 */
AST_INDEX
cfg_node_to_ast_func(CfgInstance    cfg,
		     CfgNodeId      cn,
		     Boolean        prepend,
		     ListInsertFunc &insert_func,
		     Boolean        skip_comments)
{
  AST_INDEX node;
  Boolean   change_source = true;

  // Try to map <cn> to <node>
  node = cfg_node_to_near_ast(cfg, cn, change_source);

  if (node == AST_NIL)
  {
    cerr << "WARNING cfg_node_to_ast_func(): found no AST node," <<
      " cn = " << cn << ".\n";
  }
  else
  {
    // Handle if-branch-guards correctly
    if (is_list(node) && is_guard(ast_get_father(node)))
    {
      // Make it first stmt in branch
      insert_func = prepend ? &list_insert_last : &list_insert_first;
    }
    else
    {
      if (is_guard(node))
      {
	// Make it first stmt in branch
	node        = gen_GUARD_get_stmt_LIST(node);
	insert_func = &list_insert_first;
      }
      else
      {
	if (is_goto(node) || is_return(node))
	{
	  // Insert before GOTO or RETURN
	  insert_func = &list_insert_before;
	}
	else
	{
	  // Default: insert before/after this node
	  insert_func = prepend ? &list_insert_before
	    : &list_insert_after;
	}
      }

      // If we are about to prepend to a node w/ a label,
      // then move the label ahead
      pt_extract_label(node);
     
      // If desired, then move before any non-empty comments
      if (prepend && skip_comments)
      {
	node = move_before_comments(node);
      }
    }
  }

  return node;
}


/**********************************************************************
 * cfg_node_to_predom_ast()  Given <cn>, find the nearest predominating
 *                           <node>
 */
AST_INDEX
cfg_node_to_predom_ast(CfgInstance cfg,
		       CfgNodeId   cn)
{
  return cfg_node_to_nearest_ast(cfg, cn, false);
}


/**********************************************************************
 * cfg_node_to_postdom_ast() Given <cn>, find the nearest postdominating
 *                           <node>
 */
AST_INDEX
cfg_node_to_postdom_ast(CfgInstance cfg,
			CfgNodeId   cn)
{
  return cfg_node_to_nearest_ast(cfg, cn, true);
}


/**********************************************************************
 * ref2varName()  Get name of scalar or array reference
 */
const char *
ref2varName(AST_INDEX ref_node)
{
  const char *name = NULL;

  if (is_subscript(ref_node))
    ref_node = gen_SUBSCRIPT_get_name(ref_node);

  if (is_identifier(ref_node))
  {
    name = gen_get_text(ref_node);
  }
  else
  {
    cerr << "WARNING ref2varName():  No name for ref_node = " <<
      ref_node << ".\n";
  }

  return name;
} 


/**********************************************************************
 * catRefStr()  Get name of scalar or array reference
 */
void
catRefStr(AST_INDEX ref_node,     // node of reference
	  char      *res_str)     // Append to this string
{
  AST_INDEX name_node, subsc_node, subsc_list;
  char      *ref_str;
  int       i, rank;

  if (is_subscript(ref_node)) {   // Is it a subscripted expression ?
    name_node = gen_SUBSCRIPT_get_name(ref_node);  // Print identifier
    catRefStr(name_node, res_str);

    (void) strcat(res_str, "(");
    subsc_list = gen_SUBSCRIPT_get_rvalue_LIST(ref_node);
    rank = list_length(subsc_list);
    for (i=0, subsc_node = list_first(subsc_list); // Loop over subscripts
	 i < rank;
	 i++, subsc_node = list_next(subsc_node))
    {
      catRefStr(subsc_node, res_str);
      if (i < rank-1)
	(void) strcat(res_str, ", ");
    }
    (void) strcat(res_str, ")");
  } else
  {
    assert(is_identifier(ref_node) || is_constant(ref_node));
    ref_str = gen_get_text(ref_node);
    (void) strcat(res_str, ref_str);
  }
  assert(strlen(res_str) < MAX_REF_LENGTH);
} 


/**********************************************************************
 * catRefBuf()  Get name of scalar or array reference
 */
void
catRefBuf(AST_INDEX  ref_node,      // Node of reference
	  ostrstream &res_buf)      // Append to this buffer
{
  AST_INDEX name_node, subsc_node, subsc_list;
  int       i, rank;

  if (is_subscript(ref_node)) {   // Is it a subscripted expression ?
    name_node = gen_SUBSCRIPT_get_name(ref_node);  // Print identifier
    catRefBuf(name_node, res_buf);

    res_buf << "(";
    subsc_list = gen_SUBSCRIPT_get_rvalue_LIST(ref_node);
    rank = list_length(subsc_list);
    for (i=0, subsc_node = list_first(subsc_list); // Loop over subscripts
	 i < rank;
	 i++, subsc_node = list_next(subsc_node))
    {
      catRefBuf(subsc_node, res_buf);
      if (i < rank-1)
	res_buf << ", ";
    }
    res_buf << ")";
  } else
  {
    if (is_identifier(ref_node) || is_constant(ref_node))
    {
      res_buf << gen_get_text(ref_node);
    }
    else
    {
      //cerr << "WARNING catRefBuf(): ref_node = " << ref_node <<
      //" is neither an identifier nor a constant.\n";
      res_buf << "???";
    }
  }
} 


/**********************************************************************
 * begin_ast_stmt()  Copied from dep/dg/save_dg.c, where it was
 *                   declared statically
 */
AST_INDEX
begin_ast_stmt(AST_INDEX node)
{
  while(node != AST_NIL && !is_statement(node))
    node = tree_out(node);

  return node;
} 


/**********************************************************************
 * next_executable_stmt()  Return next executable stmt after <node>
 */
AST_INDEX
next_executable_stmt(AST_INDEX node)
{
  AST_INDEX stmt_node = begin_ast_stmt(node);

  while(stmt_node != AST_NIL && !is_executable(stmt_node))
    stmt_node = list_next(stmt_node);

  return stmt_node;
} 


/**********************************************************************
 * cfg_node_from_near_ast()  Map <node> to nearby cfg node
 */
CfgNodeId
cfg_node_from_near_ast(CfgInstance cfg, AST_INDEX node)
{
  AST_INDEX stmt_node = next_executable_stmt(node);
  CfgNodeId cn        = CFG_NIL;

  while ((cn == CFG_NIL) && (stmt_node != AST_NIL))
  {
    cn        = cfg_node_from_ast(cfg, stmt_node);
    stmt_node = list_next(stmt_node);
  }

  return cn;
} 


/**********************************************************************
 * cfg_update_node_map()  
 */
//void
//cfg_update_node_map(CfgInstance cfg)
//{
//  AST_INDEX node;
//  CfgNodeId cn;
//
//  for (cn = cfg_get_first_node(cfg);
//       cn != CFG_NIL;
//       cn = cfg_get_next_node(cfg, cn))
//  {
//    node = cfg_node_to_ast(cfg, cn);
//    cfg_node_put_map(cfg, node, cn);
//  }
//} 


/**********************************************************************
 * find_enclosing_DO()  Given a node, find the next enclosing loop
 *                      Copied from dep/dg/save_dg.c, where it was
 *                      declared statically (sigh)
 */
AST_INDEX
find_enclosing_DO(AST_INDEX node)
{
  begin_ast_stmt(node);

  while(node != AST_NIL && !is_loop(node)) 
  {
    node = tree_out(node);
    begin_ast_stmt(node);
  }
  
  return( node );
} 


/**********************************************************************
 * find_outmost_loop()  Given a node, find the outermost enclosing loop
 */
AST_INDEX
find_outmost_loop(AST_INDEX node)
{
  AST_INDEX outer_node = node;
  AST_INDEX loop_node = AST_NIL;

  while (outer_node != AST_NIL)
  {
    if (is_loop(outer_node)) {
      loop_node = outer_node;
    }
    outer_node = tree_out(outer_node);
  }

  //if (loop_node == AST_NIL) {
  //  cerr << "WARNING find_outmost_loop(): " <<
  //    "did not find loop enclosing node = " << node << ".\n";
  //}

  return loop_node;
} 


/**********************************************************************
 * node_is_root()  Is this the root node of a program, function, etc. ?
 */
Boolean
node_is_root(AST_INDEX node)
{
  Boolean is_root = (Boolean) (is_subroutine(node)
			       || is_function(node)
			       || is_program(node)
			       || is_block_data(node));
  
  return is_root;
}


/**********************************************************************
 * node2root()  Find the root node of a program, function, etc.
 */
AST_INDEX
node2root(AST_INDEX start_node)
{
  AST_INDEX node;
  AstIter   iter(start_node);

  while (((node = iter()) != AST_NIL) && !node_is_root(node));
  
  if (node == AST_NIL) {
    cerr << "WARNING: node2root(): "
      << "did not find root for start_node = "
	<< start_node << ".\n";
  }

  return node;
}


/**********************************************************************
 * root2name_node()  Find the name node of a program, function, etc.
 */
AST_INDEX
root2name_node(AST_INDEX start_node)
{
  AST_INDEX root_node, name_node;

  root_node = node2root(start_node);

  if (root_node)
  {
    name_node =
      is_subroutine(root_node) ? gen_SUBROUTINE_get_name(root_node) :
    (is_function(root_node) ? gen_FUNCTION_get_name(root_node) :
     (is_program(root_node) ? gen_PROGRAM_get_name(root_node) :
      (is_block_data(root_node) ? gen_BLOCK_DATA_get_name(root_node) :
       AST_NIL)));
  }
  else
  {
    name_node = AST_NIL;
  }

  return name_node;
}

/**********************************************************************
 * root2name()  Find the name of a program, function, etc.
 */
STR_INDEX
root2name(AST_INDEX start_node)
{
  AST_INDEX name_node;
  STR_INDEX name;

  name_node = root2name_node(start_node);
  name = is_null_node(name_node) ? NIL_STR : gen_get_symbol(name_node);

  return name;
}


/**********************************************************************
 * root2name_str()  Find the name of a program, function, etc.
 */
const char *
root2name_str(AST_INDEX start_node)
{
  AST_INDEX name_node;
  char      *name_str;

  name_node = root2name_node(start_node);
  name_str = is_null_node(name_node) ? NULL : gen_get_text(name_node);

  return name_str;
}


/**********************************************************************
 * find_stmt_list()  Find the statement list of a program, function, etc.
 */
AST_INDEX
find_stmt_list(AST_INDEX start_node)
{
  AST_INDEX root_node;
  AST_INDEX stmt_list;

  root_node = node2root(start_node);

  if (root_node)
  {
    stmt_list = get_stmt_list(root_node);
  }
  else
  {
    cerr << "WARNING: find_stmt_list(): " <<
      "did not find stmt list for start_node = " << start_node <<
	".\n";
    stmt_list = AST_NIL;
  }

  return stmt_list;
}


/**********************************************************************
 * is_in_main()  Determine whether node is in main program
 */
Boolean
is_in_main(AST_INDEX start_node)
{
  AST_INDEX root_node = node2root(start_node);
  Boolean   in_main = root_node ? (Boolean) is_program(root_node) : false;

  return in_main;
}


/**********************************************************************
 * find_last_stmt()  Find the last statement in a program, function, etc.
 */
AST_INDEX
find_last_stmt(AST_INDEX start_node)
{
  AST_INDEX stmt_list = find_stmt_list(start_node);
  AST_INDEX last_node = list_last(stmt_list);

  if (is_return(last_node) || is_stop(last_node))
  {
    last_node = list_prev(last_node);
  }

  return last_node;
}


/**********************************************************************
 * get_stmt_list()  Find the beginning of the stmts of a program,
 *                  subroutine, etc. or return AST_NIL
 */
AST_INDEX
get_stmt_list(AST_INDEX node)
{
  AST_INDEX stmt_list = AST_NIL;

  switch (NT(node)) {
  case GEN_PROGRAM:
    stmt_list = gen_PROGRAM_get_stmt_LIST(node);
    break;
  case GEN_SUBROUTINE:
    stmt_list = gen_SUBROUTINE_get_stmt_LIST(node);
    break;
  case GEN_FUNCTION:
    stmt_list = gen_FUNCTION_get_stmt_LIST(node);
    break;
  case GEN_BLOCK_DATA:
    stmt_list = gen_BLOCK_DATA_get_stmt_LIST(node);
    break;
  }

  return stmt_list;
}


/**********************************************************************
 * assert_is_subscript()   Make sure a given AST is a subscript.
 */
AST_INDEX
assert_is_subscript(AST_INDEX node)
{
  assert(is_subscript(node));

  return node;
}


/**********************************************************************
 * assert_father_is_subscript()   Make sure the father of a given AST
 *                                is a subscript.
 */
AST_INDEX
assert_father_is_subscript(AST_INDEX node)
{
  assert(is_subscript(ast_get_father(list_head(node))));

  return node;
}


/**********************************************************************
 * str_tolower()  Return a copy of <str> in lower case.
 */
char *
str_tolower(const char *str)
{ 
  int  len = strlen(str);
  char *lower_str = new char[len+1];

  for (int i = 0; i < len + 1; i++) {
    lower_str[i] = tolower(str[i]);
  }

  return lower_str;
}


/**********************************************************************
 * ftt_ast2str()  Unparses <node> into a (newly generated) string.
 */
const char *
ftt_ast2str(FortTextTree ftt, AST_INDEX node)
{
  char *str;
  int  newlen;
  char *res;

  // Update ftt
  ftt_TreeChanged(ftt, node);

  // 5/26/93 RvH: ftt_NodeToStr() adds extra characters.
  //              It appears that ftt_NodeToText() does not compute
  //              line segments quite right.
  str = ftt_NodeToStr(ftt, node);
  newlen = strlen(str)-1;

  // Shave off leftmost char if it's a blank OR we have a subscript
  // that would not lose its terminating ')' this way
  if ((str[newlen] == ' ') || (str[newlen] == ',')
      || ((is_subscript(node))
	  && (str[newlen-1] == ')')))
  {
    newlen--;
  }

  // Shave of leftmost char
  res = new char[newlen+1];

  (void) strncpy(res, &str[1], newlen);
  delete str;
  res[newlen] = '\0';

  return res;
}


/**********************************************************************
 * find_next_loop()  Find the next loop, not including <start_node>.
 */
AST_INDEX
find_next_loop(AST_INDEX start_node)
{
  // Iterate through <only_stmts>, but not <only_inner>
  AstIter   iter(start_node, true, false);

  // Make one step away from current <node>
  AST_INDEX node = iter();

  while (((node = iter()) != AST_NIL) && (!is_loop(node)));

  return node;
}


/**********************************************************************
 * move_before_comments()  Move before any comments preceeding <node>.
 */
AST_INDEX
move_before_comments(AST_INDEX node)
{
  AST_INDEX new_node, text_node;

  do {
    new_node = node;
    node     = list_prev(new_node);
  } while ((node != AST_NIL)
	   && (is_comment(node))
	   && ((text_node = gen_COMMENT_get_text(node)) != AST_NIL)
	   && (strlen(gen_get_text(text_node)) > 0));

  return new_node;
}


/**********************************************************************
 * vn_no_duplicates()  Determine whether a value number might contain
 *                     duplicate values within itself.
 *
 * Examples for no duplicates: 5, 1:100
 * Examples for duplicates:    1:100 + 100:1,
 *                             n(1:100) if nothing known about n
 */
Boolean
vn_no_duplicates(CfgInstance cfg, ValNumber vn)
{
  Values  val     = cfgval_get_values(cfg);
  ValType vn_type = val_get_val_type(val, vn);
  //Boolean no_dupl = (vn_type != VAL_ARRAY);    // Be daring for now
  Boolean no_dupl = true;                        // Be *DARING* for now

  return no_dupl;
} 


/**********************************************************************
 * node2name()  Find first name in <expr_node>.
 */
const char *
node2name(AST_INDEX expr_node)
{
  AST_INDEX node;
  const char      *name = NULL;
  AstIter   iter(expr_node, false);  // only_stmts = false

  // First, try to find an identifier within expression
  while (((name == NULL) || (strlen(name) == 0))
	 && ((node = iter()) != AST_NIL))
  {
    if (is_identifier(node))
    {
      name = gen_get_text(node);
    }
  }

  // No identifier found ?  ==> take any node
  if ((name == NULL) || (strlen(name) == 0))
  {
    while (((name == NULL) || (strlen(name) == 0))
	   && ((node = iter()) != AST_NIL))
    {
      name = gen_get_text(node);
    }
  }

  if ((name == NULL) || (strlen(name) == 0))
  {
    cerr << "WARNING node2name():  " <<
      "Did not find name, node = " << node << ".\n";
  }

  return name;
}


/**********************************************************************
 * count_nest_iters()  Count the # of iters of the loop nest headers
 *                     surrounding <subsc_node>, up to <lim_node>.
 */
int                              // # of iterations (or UNKNOWN_SIZE)
count_nest_iters(CfgInstance cfg,         // Control flow graph
		 AST_INDEX   subsc_node,  // Subscript within loop nest
		 AST_INDEX   lim_node)    // Header of loop nest
{
  AST_INDEX loop_node;
  int       it_cnt;
  int       iter_cnt  = 1;
  TarjTree  tarjans   = cfg_get_intervals(cfg);
  AST_INDEX stmt_node = begin_ast_stmt(subsc_node);
  CfgNodeId cur_id    = cfg_node_from_ast(cfg, stmt_node);
  int       cur_level = tarj_level2(tarjans, cur_id);
  CfgNodeId lim_id    = cfg_node_from_ast(cfg, lim_node);
  int       lim_level = tarj_level2(tarjans, lim_id);


  if (!is_do(lim_node))   // Make sure we are handed a loop to copy
  {
    cerr << "WARNING count_nest_iters(): Got a non-DO loop; "
      << "lim_node = " << lim_node << ".\n";
    iter_cnt = UNKNOWN_SIZE;
  }

  while ((cur_level > lim_level) && (iter_cnt != UNKNOWN_SIZE))
  {
    // Walk outward on the interval tree
    cur_id = tarj_outer(tarjans, cur_id);
    cur_level--;
    
    // Retrieve the AST_INDEX corresponding to cfg node <cur_id>
    loop_node = cfg_node_to_ast(cfg, cur_id);

    // Make sure that our loop is a DO loop
    if (is_do(loop_node))
    {
      // Determine # of iterations of current loop
      it_cnt = pt_loop_cnt(loop_node);
      iter_cnt = (it_cnt == UNKNOWN_SIZE) ? UNKNOWN_SIZE
	: iter_cnt*it_cnt;
    }
    else
    {
      cerr << "WARNING count_nest_iters(): " <<
	"Attempt to count non-DO loop, AST_INDEX = " << loop_node <<
	  ".\n";
    }
  }

  // Make sure we ended up at the right loop level
  assert ((iter_cnt == UNKNOWN_SIZE) || (lim_node == loop_node));

  return iter_cnt;
}


/**********************************************************************
 * tarj_level_max()
 */
int
tarj_level_max(CfgInstance cfg)
{
  CfgNodeId         cn;
  TarjTree          tarjans   = cfg_get_intervals(cfg);
  int               level_max = 0;
  IntervalGraphIter iter(cfg);

  while ((cn = iter()) != CFG_NIL)
  {
    level_max = max(level_max, tarj_level(tarjans, cn));
  }

  return level_max;
}


/**********************************************************************
 * save_program()  Store the current program in file *.<extension>.
 *
 * 5/4/93 RvH: derived from dc_compile_save()
 */
const char *                                  // Name of generated file
save_program(FortTree ft, FortTextTree ftt, const char *extension)
{
  FILE      *outFP;
  const char      *name;
  char      buf[MAX_NAME] = "";
  AST_INDEX root_node = ft_Root(ft);

  name = root2name_str(root_node);
  if (name)
  {
    strcpy(buf, name);
  }
  else
  {
    strcpy(buf, "main");  // default name if unnamed program
  }
  strcat(buf, extension);

  if (!(outFP = fopen(buf, "w")))
  {
    printf("Unable to create %s!\n", buf);
  }
  else
  {
    ftt_TreeChanged(ftt, root_node);
    
    ftExportSimple(ft, ftt, outFP);
    
    fclose(outFP);
  }

  return (ssave(buf));
}
