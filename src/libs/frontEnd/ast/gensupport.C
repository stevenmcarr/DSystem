/* $Id: gensupport.C,v 1.1 1997/06/24 17:41:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdarg.h>

#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <include/frontEnd/metamap.def>
#include <include/frontEnd/nodedef.def>
#include <libs/frontEnd/ast/strutil.h>


#include <libs/frontEnd/ast/aphelper.h>

/*
 * This is a new version (7-31-87) of gen_node. This version does not require
 * that the subtrees being glued into the tree are of the expected form. For
 * example, sons may be left empty with no placeholders being forced into 
 * them.
 */

AST_INDEX gen_node (NODE_TYPE type, ...)
{
  va_list         arg_list;
  AST_INDEX       newnode;
  META_TYPE       sontype;
  AST_INDEX       newsons[AST_MAX_SON];
  Generic         i, n;

     /* get these sons into variables that I can actually access...*/
     /* I know there must be a better way to do all of this....    */

  va_start (arg_list, type);
    {
    n = nodedef[type].number_of_sons;
    for (i = 0; i < n; i++) newsons[i] = (AST_INDEX)va_arg(arg_list, Generic);
    }
  va_end (ap);

  newnode = ast_alloc (type);
  for (i = 0; i < n; i++)
    {     /* install the ith son */
       sontype = nodedef[type].sons[i];
       if (NOT (is_OPTIONAL (sontype)))
         {     /* required son */
            if (is_LIST (sontype))
              newsons[i] = gen_no_meta_list (newsons[i], PLACE_REQUIRED, THE_TYPE(sontype));
	    else
              newsons[i] = gen_no_meta_node (newsons[i], PLACE_REQUIRED, THE_TYPE(sontype));
         }
       else if (is_RECOMMENDED (sontype))
         {     /* recommended son */
	    if (is_LIST (sontype))
              newsons[i] = gen_no_meta_list (newsons[i], PLACE_OPTIONAL, THE_TYPE(sontype));
	    else
              newsons[i] = gen_no_meta_node (newsons[i], PLACE_OPTIONAL, THE_TYPE(sontype));
         }
       ast_put_son_n (newnode, i + 1, newsons[i]);
    }

  return newnode;
}

AST_INDEX 
gen_new_node (NODE_TYPE type)
{
    AST_INDEX       newnode = ast_alloc (type);
    int             i;
    META_TYPE       sontype;

    for (i = 0; i < (int)nodedef[type].number_of_sons; i++)
    {

	sontype = nodedef[type].sons[i];

	if (is_LIST (sontype))
	{
	    if (!is_ANY_META (sontype))
	    {
		if (is_OPTIONAL (sontype))
                {
                    if (is_RECOMMENDED (sontype))
		        ast_put_son_n (newnode, i + 1,
				   gen_meta_list (AST_NIL, PLACE_OPTIONAL, THE_TYPE(sontype)));
                }
		else
		    ast_put_son_n (newnode, i + 1,
				   gen_meta_list (AST_NIL, PLACE_REQUIRED, THE_TYPE(sontype)));
	    }
	    else
		ast_put_son_n (newnode, i + 1, list_create (AST_NIL));
	}
	else
	{
	    if (is_OPTIONAL (sontype))
            {
                if (is_RECOMMENDED (sontype))
		    ast_put_son_n (newnode, i + 1,
			           gen_meta_node (AST_NIL, PLACE_OPTIONAL, THE_TYPE(sontype)));
            }
	    else
		ast_put_son_n (newnode, i + 1,
			       gen_meta_node (AST_NIL, PLACE_REQUIRED, THE_TYPE(sontype)));
	}
    }
    return newnode;
}

void
gen_coerce_node (AST_INDEX node, NODE_TYPE type)
{
    ast_put_node_type(node, type);
}

