/* $Id: normal.C,v 1.1 1997/06/25 15:10:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************/
/*                     NORMALIZER                                           */
/* This program gets as an input an expression and transforms it into a sum */
/* of products.                                                             */
/* The procedures pre_distribute and post_distribute can be passed as       */
/* arguments to walk_expression.                                            */
/****************************************************************************/

#include <stdio.h>
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <include/frontEnd/astsel.h>
#include <include/frontEnd/astrec.h>
#include <include/frontEnd/astcons.h>
#include <libs/moduleAnalysis/expnSimplifier/normal.h>



/*	local definitions to control direction of distribution	*/

#define FROM_LEFT 1
#define FROM_RIGHT 0

#define IS_DISTRIBUTIVE 1
#define IS_NOT_DISTRIBUTIVE 0

#define PLUS 1
#define MINUS 0





/*****************************************************************************/
/* normalize_expr( )	-- reorganizes the expr into a sum_of_products form  */
/*									     */
/*	INPUT:	AST_NODE which is the root of expression		     */
/*	OUTPUT:	none							     */
/*	SIDE EFFECTS:	updates the AST in place.			     */
/******************************************************************************/

void  normalize_expr(AST_INDEX passed_node)

  // AST_INDEX passed_node;      /* passed_node is always an expression */

{

  AST_INDEX returned_node;


   walk_expression(passed_node,pre_distribute,post_distribute,NULL);
   tree_replace_free(passed_node,normalize(passed_node));

}


/******************************************************************************/
/* Help function used to apply pre_distribute and post_distribute to the      */
/* right hand side of an assignment                                           */
/******************************************************************************/

int walk_assign(AST_INDEX stmt)

  // AST_INDEX stmt;

{



  if (is_assignment(stmt))
     {walk_expression(gen_ASSIGNMENT_get_rvalue(stmt),pre_distribute,post_distribute,NULL);

      tree_replace_free(gen_ASSIGNMENT_get_rvalue(stmt),normalize(gen_ASSIGNMENT_get_rvalue(stmt)));

     }

  return(WALK_CONTINUE);
}
 


/*****************************************************************************/
/* This procedure determines to which nodes we  are going to perform a       */
/* distribute.                                                               */
/*****************************************************************************/


int pre_distribute (AST_INDEX expr, Generic parm)

  // AST_INDEX expr;
  // Generic parm;

{


  if (is_binary_times(expr) ||
      is_binary_divide(expr)||
      is_binary_minus(expr) ||
      is_binary_plus(expr)  ||
      is_unary_minus(expr)  ||
      is_binary_exponent(expr)||
      is_identifier(expr)||
      is_constant(expr))

    return(WALK_CONTINUE);

  else return(WALK_ABORT);

}




/*****************************************************************************/
/* Post_distribute calls procrdure push_star that actually		     */
/* performs the distribution.                                                */
/*****************************************************************************/

int post_distribute(AST_INDEX passed_node,Generic parm)

  // AST_INDEX passed_node;
  // Generic parm;

{
 
 AST_INDEX returned_node;
 



  if (is_binary_times(passed_node))
     {
       returned_node = push_star(passed_node);
       tree_replace_free(passed_node,returned_node);
     }

  return(WALK_CONTINUE);

}







/****************************************************************************/
/* This procedure takes as an input an expression that has been transformed */
/* into a sum of prosucts and tranforms it into an equivalent expression    */
/* that corresponds to a left leaning tree.                                 */
/*                                                                          */
/* This is done in three steps :                                            */
/*                                                                          */
/* First, the input tree is transformed into an equivalent tree that has    */
/* only PLUS nodes. All the MINUS nodes are pushed as far as possible into  */
/* the tree, until they reach the leaves. This means that the created tree  */
/* will have only UNARY_MINUS nodes ( not any BINARY_MINUS nodes). This is  */
/* done by procedure push_minus.                                            */
/*                                                                          */
/* Second, the tree is transformed to a left leaning tree by procedure      */
/* left_lean.                                                               */
/*                                                                          */
/* Third, we push any UNARY_MINUS node of the left leaning tree up so that  */
/* the PLUS node  above it is transformed to a MINUS node.                  */
/****************************************************************************/

AST_INDEX normalize(AST_INDEX passed_node)

  // AST_INDEX passed_node;

{
 AST_INDEX returned_node;


  returned_node = push_minus(passed_node,PLUS);
  passed_node = left_lean(returned_node,AST_NIL);
  passed_node = juggle_minus(passed_node);
  replace_left_most_minus(passed_node);

  return(passed_node);

}




/****************************************************************************/
/* This procedure is only called on TIMES or DIVIDE nodes. If the left son  */
/* is distributive, we distribute the right son over the left. Else if      */
/* the right son is distributive, we distribute the left son over the right.*/
/****************************************************************************/

AST_INDEX push_star (AST_INDEX passed_node)

  // AST_INDEX passed_node;

{AST_INDEX right_son, left_son;
 


  right_son = gen_BINARY_TIMES_get_rvalue2(passed_node);
  left_son = gen_BINARY_TIMES_get_rvalue1(passed_node);

  if (distributive(left_son) == IS_DISTRIBUTIVE)

     passed_node = distribute(passed_node,right_son,left_son,FROM_RIGHT);

  else if (distributive(right_son) == IS_DISTRIBUTIVE)
      
     passed_node = distribute(passed_node,left_son,right_son,FROM_LEFT);


  return(passed_node);
}



/******************************************************************************/
/* This routine performs the distribute. It is called with the distribute     */ 
/* and the over expression. The first argument, passed_node, is used only to  */
/* determine the kind of operation we have (TIMES or DIVIDE). The last        */
/* argument, from_left, is used to determine how the new nodes will be built. */
/* If from_left is FROM_LEFT then the nodes built from the distribute will    */
/* have the distribute expression as the left son and the sons of the         */
/* over  expression as the right son. If from_left is FROM_RIGHT then the     */
/* opposite happens.                                                          */
/******************************************************************************/


AST_INDEX distribute(AST_INDEX passed_node,AST_INDEX distribute_expr,
		     AST_INDEX over_expr,int from_left)

  // AST_INDEX passed_node,distribute_expr,over_expr;
  // int from_left;

{
  AST_INDEX left_term,right_term,un_signed;


  if (is_binary_plus(over_expr))
     {  

       if (from_left == FROM_LEFT)
         {
          left_term = gen_BINARY_TIMES(tree_copy(distribute_expr),tree_copy(gen_BINARY_PLUS_get_rvalue1(over_expr)));
          right_term = gen_BINARY_TIMES(tree_copy(distribute_expr),tree_copy(gen_BINARY_PLUS_get_rvalue2(over_expr)));
          
         }



       else if (from_left == FROM_RIGHT)
          {
            left_term = gen_BINARY_TIMES(tree_copy(gen_BINARY_PLUS_get_rvalue1(over_expr)),tree_copy(distribute_expr));
            right_term = gen_BINARY_TIMES(tree_copy(gen_BINARY_PLUS_get_rvalue2(over_expr)),tree_copy(distribute_expr));
             }


       left_term = push_star(left_term);
       right_term = push_star(right_term);



       passed_node = gen_BINARY_PLUS(left_term,right_term);


     }


  else if (is_binary_minus(over_expr))
     {

       if (from_left == FROM_LEFT)
         {
          left_term = gen_BINARY_TIMES(tree_copy(distribute_expr),tree_copy(gen_BINARY_MINUS_get_rvalue1(over_expr)));
          right_term = gen_BINARY_TIMES(tree_copy(distribute_expr),tree_copy(gen_BINARY_MINUS_get_rvalue2(over_expr)));
          }

       else if (from_left == FROM_RIGHT)
          {
           left_term = gen_BINARY_TIMES(tree_copy(gen_BINARY_MINUS_get_rvalue1(over_expr)),tree_copy(distribute_expr));
           right_term = gen_BINARY_TIMES(tree_copy(gen_BINARY_MINUS_get_rvalue2(over_expr)),tree_copy(distribute_expr));
          }



       left_term = push_star(left_term);
       right_term = push_star(right_term);



       passed_node = gen_BINARY_MINUS(left_term,right_term);


     }


  else if (is_unary_minus(over_expr))
    {
      if (from_left == FROM_LEFT)
         un_signed = gen_BINARY_TIMES(tree_copy(distribute_expr),tree_copy(gen_UNARY_MINUS_get_rvalue(over_expr)));
      
       else if (from_left == FROM_RIGHT)
          un_signed = gen_BINARY_TIMES(tree_copy(gen_UNARY_MINUS_get_rvalue(over_expr)),tree_copy(distribute_expr));



      un_signed = push_star (un_signed);
      passed_node = gen_UNARY_MINUS(un_signed);
      }

      return ( passed_node);
    
}






/****************************************************************************/
/* This procedure takes as an input an expression that has been tranformed  */
/* into a sum of products and tranforms it to an equivalent left leaning    */
/* expression                                                               */
/****************************************************************************/


AST_INDEX left_lean (AST_INDEX passed_node,AST_INDEX new_root)

  // AST_INDEX passed_node,new_root;

{
  if (is_binary_plus(passed_node))
     {
       new_root = left_lean(gen_BINARY_PLUS_get_rvalue1(passed_node),new_root);
       new_root = left_lean( gen_BINARY_PLUS_get_rvalue2(passed_node),new_root);
     }

  else
    {
      if  ( new_root == AST_NIL)
         new_root = tree_copy(passed_node);
      else new_root = gen_BINARY_PLUS(new_root,tree_copy(passed_node));
    }


return(new_root);
}






/******************************************************************************/
/* This routine pushes the minus as far as possible into a PLUS node. This    */
/* will transform the expression into an equivalent one that has only PLUS    */
/* nodes. All the minus's will be pushed to the leaves (identifiers or TIMES  */
/* nodes), and so we will have an expression with only unary_minus's (not     */
/* binary_minus's).                                                           */      
/******************************************************************************/



AST_INDEX push_minus(AST_INDEX passed_node,int sign)

  // AST_INDEX passed_node;
  // int sign;

{
   AST_INDEX left,right,created_node;

   if (is_binary_plus(passed_node))

      {
        left = push_minus(gen_BINARY_PLUS_get_rvalue1(passed_node),sign);
        right = push_minus(gen_BINARY_PLUS_get_rvalue2(passed_node),sign);

        created_node = gen_BINARY_PLUS(left,right);

      }

    else if (is_binary_minus(passed_node))
      {

         if (sign == MINUS)
            {    
              left = push_minus(gen_BINARY_PLUS_get_rvalue1(passed_node),MINUS);
              right = push_minus(gen_BINARY_PLUS_get_rvalue2(passed_node),PLUS);
       
              created_node = gen_BINARY_PLUS(left,right);

            }


          else if (sign == PLUS)
           {
	     right = push_minus(gen_BINARY_PLUS_get_rvalue2(passed_node),MINUS);
             left = push_minus(gen_BINARY_PLUS_get_rvalue1(passed_node),PLUS);

             created_node = gen_BINARY_PLUS(left,right);
           }

       }


    else if (is_unary_minus(passed_node))
        {
          passed_node = gen_UNARY_MINUS_get_rvalue(passed_node);

          if (sign == MINUS)
             created_node = push_minus(passed_node,PLUS);
      
         else 
             created_node = push_minus(passed_node,MINUS);

        }


     else   /* this is a TIMES node or a  leaf */

       {   
          if (sign == MINUS)
            created_node = gen_UNARY_MINUS (tree_copy(passed_node));
          else 
            created_node = tree_copy(passed_node);


       }



 return(created_node);

}







/******************************************************************************/
/* This procedure is only called on left leaning trees  that have only PLUS   */
/* nodes (not any MINUS nodes. It walks the tree and if it finds a PLUS node  */
/* with a right son that is a unary_minus, it transforms it into a MINUS      */
/* node.                                                                      */
/******************************************************************************/

AST_INDEX juggle_minus(AST_INDEX passed_node)

  // AST_INDEX passed_node;

{AST_INDEX right,returned_node;

  if (is_binary_plus(passed_node))
     {
      right = gen_BINARY_PLUS_get_rvalue2(passed_node);

      if (is_unary_minus(right))
          
       returned_node = gen_BINARY_MINUS(juggle_minus(tree_copy(gen_BINARY_PLUS_get_rvalue1(passed_node))),tree_copy(gen_UNARY_MINUS_get_rvalue(right)));


      else 
       returned_node = gen_BINARY_PLUS(juggle_minus(tree_copy(gen_BINARY_PLUS_get_rvalue1(passed_node))),tree_copy(right));


      }

  else returned_node = tree_copy (passed_node);

  return(returned_node);
      

}




/*************************************************************************/
/* routine that wealks a left leaning tree until it reaches the          */
/* right_most leaf, keeping track of the last PLUS node it encounters.   */
/* If the rightmost leaf is a UNARY_MINUS, it exchanges it with the last */
/* PLUS node it encountered.                                             */
/*************************************************************************/

void replace_left_most_minus(AST_INDEX node)

  // AST_INDEX node;

{
 AST_INDEX leftmost_leaf;
 AST_INDEX last_plus;
 AST_INDEX new_const;



 last_plus = 0;
 


/* we walk the tree until we find the left_most leaf. */
/* we also keep track of the last plus node we encounter */

 
 while (is_binary_plus(node)||
        is_binary_minus(node))
  
   {
    if (is_binary_plus(node))
       last_plus = node;

    if (is_binary_plus(node))
       node = gen_BINARY_PLUS_get_rvalue1(node);
    else 
       node = gen_BINARY_MINUS_get_rvalue1(node);    
  }


  leftmost_leaf = tree_copy(node);     /* this is the leftmost leaf */

  if (is_unary_minus(leftmost_leaf))
    {
     if (last_plus !=0)
       {

        tree_replace_free(node,tree_copy(gen_BINARY_PLUS_get_rvalue2(tree_copy(last_plus))));
        tree_replace_free(last_plus, gen_BINARY_MINUS(tree_copy(gen_BINARY_PLUS_get_rvalue1(tree_copy(last_plus))),
                                                    tree_copy(gen_UNARY_MINUS_get_rvalue(leftmost_leaf))));
       }
  
     else 
       {
        new_const = gen_CONSTANT();
        gen_put_text(new_const,"-1",STR_CONSTANT_INTEGER);
        
        tree_replace_free(node,gen_BINARY_TIMES(new_const,tree_copy(gen_UNARY_MINUS_get_rvalue(leftmost_leaf))));
      }
   }


}

 


/**************************************************************************/
/* This procedure determines if we can perform a distribute on a node.    */
/**************************************************************************/
int distributive (AST_INDEX expr)

  // AST_INDEX expr;

{
  if (is_binary_plus(expr)||
      is_binary_minus(expr)||
      is_unary_minus(expr))

    return(IS_DISTRIBUTIVE);
   else return(IS_NOT_DISTRIBUTIVE);
}






