/* $Id: simple.C,v 1.1 1997/06/25 15:10:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************/
/*                    EXPRESSION SIMPLIFIER                    */
/*                                                             */
/* A routine to simplify subscript expressions.                */
/* First it calls normalize_expr to normalize the expression   */
/* turning it into a sum of products. Division and             */
/* exponentiation are treated as special cases. An extra pass  */
/* is made over the expression before the normalization, in    */
/* which the left and right children of division and           */
/* exponentiation nodes are simplified. After normalization    */
/* the expression is broken down to an array of terms with     */
/* integer coefficients. The array is then sorted to bring     */
/* terms tith the same AST together. After that, the terms     */
/* with the same AST are combined. Finally the routine builds  */
/* the new simplified tree from the term array and it          */
/* substitutes it for the old one on the AST.                  */
/* The simplifier expects as a parameter an array that holds   */
/* the names of the induction variables of each one of the 7   */
/* loop levels.							*/
/***************************************************************/


/******************************************************************/
/* to put the constants on the right we need to change            */
/* recreate_tree. Then we have to change find_negative_constants: */
/* if (invert_constant(left)                                      */
/* and                                                            */
/* gen_binary_times(tree_copy(left),find_negative_constants(right)*/
/*                                                                */
/* Right now the term i* -3 (the constant -3) will be transformed */
/* to i* (-3) (unary minus 3).                                    */
/*  $$$$$$$  changed it to construct (-i)*3  $$$$$$$$$$$$$$$      */
/******************************************************************/
/* BUT, insert_unary_minus is going to push a minus to the left   */
/* most leaf. -a -4*a becomes (-a)*5. Inconsistent.               */
/******************************************************************/

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <libs/support/misc/general.h>
#include <include/bstring.h>

#include <libs/moduleAnalysis/expnSimplifier/normal.h>
#include <libs/moduleAnalysis/expnSimplifier/simple.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <include/frontEnd/astsel.h>
#include <include/frontEnd/astrec.h>
#include <include/frontEnd/astcons.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/support/arrays/ExtensibleArray.h>

#include <libs/frontEnd/fortTree/FortTree.h>



/****************************************************************/
/* Locally defined structures, for local use.			*/
/****************************************************************/

struct tableau_entry_type
       {
         AST_INDEX astptr;
         int       score;
         int       coeff;
         int       op;
       };

struct SymTable_NestLev_Type
   {
    nest_level_type	nesting_level_array;
    SymDescriptor D;
   };


/****************************************************************/
/* local definitions for identification of arithmetic operations*/
/****************************************************************/

# define PLUS 1
# define MINUS 0
# define TIMES 3
# define DIVIDE 4
# define EXPONENT 5

# define GREATER_THAN 1
# define LESS_THAN 2
# define EQUAL 3

# define MAX_OP_SCORE 200             /* the maximum score that any operation
                                         can get */

# define MIN_OP_SCORE 10              /* the minimum score that any operation
                                         can get. */


/***********************************************************************/
/* before calling simplify we need to create the nesting level array   */
/* because the simplifier needs to know the nesting level of the       */
/* variables in order to compute the score of the induction variables. */
/* The nesting level array is an array of pointers to characters. Each */
/* element of the array is a pointer to a character (the name of an    */
/* induction variable). The level of the induction variable is the     */
/* index of the array.                                                 */
/***********************************************************************/ 

int walk_simplify(AST_INDEX stmt, int dummy, SymDescriptor sym_t)
{
  nest_level_type	nesting_level_array;
  int i;


  nesting_level_array = (nest_level_type) get_mem (sizeof(nest_level_type),"nesting_level_array");


  // initialize_nesting_level_array(nesting_level_array);  

  if (is_assignment(stmt))
    simplify(gen_ASSIGNMENT_get_rvalue(stmt), nesting_level_array, sym_t);
   
  free_mem (nesting_level_array);

  return(WALK_CONTINUE);
}





/******************************************************************************/
/* The basic routine that performs the simplification.                        */
/* First it builds a tableau that is an array with one entry for each term    */
/* of the expression.                                                         */
/*                                                                            */
/* The fields of the tableau entry are :                                      */
/*                                                                            */
/*       astptr   : a pointer to the tree of the term.                        */
/*       coeff    : the constant coefficient of the term.                     */
/*       score    : a score used to sort the terms of the array into          */
/*                  some order.                                               */
/*       op       : the controlling operator of the element.                  */
/*                                                                            */
/* Example :                                                                  */
/* The expression                                                             */
/*                                                                            */
/*          A - 2*B + 5                                                       */
/*                                                                            */
/* will be represented as :                                                   */
/*                                                                            */
/*                                                                            */
/*                +-----------------+                                         */
/*         score  |  ?  |  ?  |  ?  |                                         */
/*         op     |  +  |  -  |  +  |                                         */
/*         coeff  |  1  |  2  |  5  |                                         */
/*         astptr |  |  |  |  |  |  |                                         */
/*                +--|--|--|--|--|--+                                         */
/*                   A     B    NIL                                           */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* After building the tableau, the routine sorts the tableau according to     */
/* the score of each node. The result of this is that the terms that have     */
/* the same score will end up in adjacent locations in the tableau, and so    */
/* we will be able to combine them. After combining the terms that have the   */
/* the same AST, the routine rebuilds the expression tree and it puts it      */
/* back on the AST ogf the original program.                                  */
/******************************************************************************/
      

int simplify(AST_INDEX passed_node, nest_level_type nesting_level_array, 
             SymDescriptor D)
{
  int		 term_count,tableau_size;
  tableau_type	 tableau;
  AST_INDEX	 returned_tree;
  AST_INDEX	 temp_node;
  int		 i;


  walk_simplify_divide(passed_node, nesting_level_array, D); 
  normalize_expr(passed_node);  


  term_count = get_term_count(passed_node);
  tableau_size = term_count + 1;

  tableau = allocate_tableau(tableau_size); 
  
  make_tableau (passed_node,tableau, tableau_size, process_term, PLUS, 
                nesting_level_array, D);
  
  sort(tableau, tableau_size, nesting_level_array, D);

  combine_terms(tableau, tableau_size);

  returned_tree = recreate_tree(tableau, tableau_size);

  returned_tree = find_negative_constants(returned_tree); 

  tree_replace_free(passed_node, returned_tree);

  xfree ((int*)tableau);

  return(WALK_CONTINUE);
}




/***************************************************************************/
/* This is the first pass that is made on the expression AST.              */
/* This is done in order to deal with special cases of the BINARY_DIVIDE   */
/* and BINARY_EXPONENT nodes.                                              */
/***************************************************************************/




void walk_simplify_divide(AST_INDEX passed_node, nest_level_type nesting_level_array,
                          SymDescriptor D)
{

  SymTab_NestLev_Type SymTab_NestLev;


   SymTab_NestLev = (SymTab_NestLev_Type) get_mem (2*sizeof(D),"SymTab_NestLev");


   SymTab_NestLev->D = D;
   SymTab_NestLev->nesting_level_array = nesting_level_array;

   walk_expression(passed_node, (WK_EXPR_CLBACK)pre_distribute, 
                   (WK_EXPR_CLBACK)simp_divide, (Generic)SymTab_NestLev);
   tree_replace_free(passed_node, normalize(passed_node));

   free_mem (SymTab_NestLev);

}




/**************************************************************************/
/* This is the last pass over the expression tree. The purpose of this    */
/* routine is to find all the constant nodes and determine if the value   */
/* of the constant is negative. If so it replaces the constant with it's  */
/* complement and then transforms the corresponding plus (or minus) node  */
/* to a minus (or plus).                                                  */
/* Also, this routine turns the (-1)A terms that are produced by the      */
/* normalizer to -A terms.                                                */
/**************************************************************************/

AST_INDEX find_negative_constants(AST_INDEX root)
{

 AST_INDEX right,binary_left, child, binary_right;
 AST_INDEX returned_node;

 if (is_binary_plus(root)) 
   {
    right = gen_BINARY_PLUS_get_rvalue2(root);
    if (is_constant(right))
      {
        if (invert_constant(right) == true)  /* we found a negative constant
                                                and we replaced it with it's
                                                complement */
          returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));
        else
          returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));

      }

     else if (is_binary_times(right))
       {
        binary_left = gen_BINARY_TIMES_get_rvalue1(right);
        binary_right = gen_BINARY_TIMES_get_rvalue2(right);

        if (is_constant(binary_right))
          {
          if (invert_constant(binary_right) == true)   /* we found a negative constant
                                                         and we replaced it with it's
                                                         complement */
	    {
             if (atoi(gen_get_text(binary_right)) == 1)
                right = binary_left;
             returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));
	   }
          else
             returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));
        }
       else returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));

      }
     else returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_PLUS_get_rvalue1(root)),tree_copy(right));


  }


 else if (is_binary_minus(root))
   {
    right = gen_BINARY_MINUS_get_rvalue2(root);
    if (is_constant(right))
      {
        if (invert_constant(right) == true)  /* we found a negative constant
                                                and we replaced it with it's
                                                complement */
          returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));
        else
          returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));
      }

     else if (is_binary_times(right))
       {
        binary_left = gen_BINARY_TIMES_get_rvalue1(right);
        binary_right = gen_BINARY_TIMES_get_rvalue2(right);

        if (is_constant(binary_right))

          {
          if (invert_constant(binary_right) == true)   /* we found a negative constant
                                                         and we replaced it with it's
                                                         complement */
	    {
             if (atoi(gen_get_text(binary_right)) == 1)
                 right = binary_left;

             returned_node = gen_BINARY_PLUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));
	   }
          else
             returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));
        }
       else returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));

      }
    else returned_node = gen_BINARY_MINUS(find_negative_constants(gen_BINARY_MINUS_get_rvalue1(root)),tree_copy(right));

  }


/* this should never occur */

  else if (is_unary_minus(root))
    {
     child = gen_UNARY_MINUS_get_rvalue(root);
     if (is_constant(child))
       {
        if (invert_constant(child) == true)  /* we found a negative constant
                                                and we replaced it with it's
                                                complement */
           returned_node = tree_copy(child);
        else
           returned_node = tree_copy(root);
      }

     else if (is_binary_times(child))
       {
        binary_left = gen_BINARY_TIMES_get_rvalue1(child);
        binary_right = gen_BINARY_TIMES_get_rvalue2(child);

        if (is_constant(binary_right))

          {
          if (invert_constant(binary_right) == true)   /* we found a negative constant
                                                         and we replaced it with it's
                                                         complement */
            {
             if (atoi(gen_get_text(binary_right)) == 1)
                child = binary_left;
             returned_node = tree_copy(child);
	   }

          else
             returned_node = tree_copy(root);
        }
       else
             returned_node = tree_copy(root);
       }
    else returned_node = tree_copy(root);
   }


  else    /* this is the left_most leaf */
   {

      if (is_constant(root))
        {
         if (invert_constant(root) == true)
            returned_node = gen_UNARY_MINUS(tree_copy(root));
         else
            returned_node = tree_copy(root);
       }

      else if (is_binary_times(root))
        {
         binary_left = gen_BINARY_TIMES_get_rvalue1(root);
         binary_right = gen_BINARY_TIMES_get_rvalue2(root);

         if (is_constant(binary_right))
	   {
           if (invert_constant(binary_right) == true)   /* we found a negative constant
                                                         and we replaced it with it's
                                                         complement */
             {
              if (atoi(gen_get_text(binary_right)) == 1)
                 root = binary_left;

             returned_node = gen_BINARY_TIMES(gen_UNARY_MINUS(tree_copy(binary_left)),tree_copy(binary_right));
	    }
           else
             returned_node = tree_copy(root);
	 }
      else
           returned_node = tree_copy(root);
       }
      else returned_node = tree_copy(root);
    }


return(returned_node);

}


Boolean	invert_constant(AST_INDEX root)
{
  char	*name;
  int	 value;
  char	 name_size[3];


  name = (char *) get_mem (sizeof(name_size), "name");

  name = gen_get_text(root);
  value = atoi(name);
  if (value < 0)
    {
     value = -value;
     sprintf(name,"%d",value);
     gen_put_text(root,name,STR_CONSTANT_INTEGER);
     return(true);
   }
  else
     return(false);
}


/***************************************************************************/
/* Routine that allocates memory for a new tableau. We allocate a number   */
/* of tableau entries equal to tableau_size.                               */
/***************************************************************************/



tableau_type allocate_tableau(int tableau_size)
{
 int i;
 tableau_type tableau;

 tableau = (tableau_entry **) xalloc ( tableau_size, sizeof(tableau_entry *));

 return(tableau);

 }




/*****************************************************************************/
/* Routine that counts the number of BINARY_PLUS or BINARY_MINUS nodes of    */
/* a left leaning tree.                                                      */
/*****************************************************************************/


int get_term_count(AST_INDEX passed_node)
{
  int count = 0;

  while (is_binary_plus(passed_node)||
         is_binary_minus(passed_node))

    {
      count ++;
      if (is_binary_plus(passed_node))
         passed_node = gen_BINARY_PLUS_get_rvalue1(passed_node);
      else 
         passed_node = gen_BINARY_MINUS_get_rvalue1(passed_node);
    }

  return (count);

}


/*****************************************************************************/
/* Routine that counts the number of BINARY_TIMES  nodes of a left leaning   */
/* tree.                                                                     */
/*****************************************************************************/


int get_factor_count(AST_INDEX passed_node)
{
  int count = 0;

  while (is_binary_times(passed_node))

    {
      count ++;
      passed_node = gen_BINARY_TIMES_get_rvalue1(passed_node);
    }

  return (count);

}





/****************************************************************************/
/* Make_tableau takes as an input a left_leaning tree and returns the       */
/* tableau that corresponds to that tree.                                   */
/* It is passed as an argument a function process.                          */
/* This function can either be  process_term or process_factor.             */
/* Process_factor gives scores to each factor of a complex term.            */
/* Process_term builds a normalizes a term so that it is left leaning, then */
/* sorts it in canonical order and finally it recreates the term from the   */
/* tableau and assignes it as a score the score of the induction variable   */
/* of the term.                                                             */
/****************************************************************************/


void make_tableau(AST_INDEX root, tableau_type tableau, int tableau_size,
                  ProcessTermFunc process, int left_op, 
                  nest_level_type nesting_level_array, SymDescriptor D)
{
 int i;
 tableau_entry *j;


 for (i = tableau_size - 1; i>=1; i-- )

   { 
    
     if (is_binary_plus(root))
        {
         tableau[i] = (*process)(gen_BINARY_PLUS_get_rvalue2(root),
                                 PLUS, nesting_level_array, D);

         root = gen_BINARY_PLUS_get_rvalue1(root);
        }
     else if (is_binary_minus(root))
        {
         tableau[i] = (*process)(gen_BINARY_MINUS_get_rvalue2(root),
                                 MINUS, nesting_level_array, D);

         root = gen_BINARY_MINUS_get_rvalue1(root);
        }
     else if (is_binary_times(root))
       {
         tableau[i] = (*process)(gen_BINARY_TIMES_get_rvalue2(root), 
                                 TIMES, nesting_level_array, D);

         root = gen_BINARY_TIMES_get_rvalue1(root);
       }
   }

 tableau[0] = (*process)(root, left_op, nesting_level_array, D);

}






/**********************************************************************************/
/* Routine that puts individual terms into canonical order and computes  the term */
/* score for use by the sum simplifier. It also computes all the fields of the    */
/* tableau entry that corresponds to this term. It returns the tableau entry.     */
/* Process_term buidls a tableau for the term. The entries of the tableau are the */
/* individual factors of the term. The tableau is built so that we can sort the   */
/* factors or the term in canonical order. Also, the induction variable is pushed */
/* to the left. Then, the term is recreated from the tableau, and is returned in  */
/* the form of an entry for the expression tableau.                               */
/**********************************************************************************/


tableau_entry *process_term (AST_INDEX  root, int op, nest_level_type nesting_level_array,
                             SymDescriptor D)
{

 AST_INDEX node;
 tableau_entry *returned_term;
 int	count, term_tableau_size;
 tableau_type tableau;
 int	i;


  node = normalize_term (root,AST_NIL);
  count = get_factor_count (node);
  term_tableau_size = count + 1;

  tableau = allocate_tableau(term_tableau_size);

  make_tableau(node,tableau,term_tableau_size,process_factor,TIMES,nesting_level_array,D);
  sort(tableau,term_tableau_size,nesting_level_array,D);  
  
  returned_term = recreate_term(tableau,term_tableau_size,op);

  xfree ((int*)tableau);

  return (returned_term);

}



/**********************************************************************************/
/* routine that computes the score of individual factors in a complex term. The   */
/* scores are used to sort the term into canonical order.                         */
/**********************************************************************************/


tableau_entry *process_factor (AST_INDEX root, int op, nest_level_type nesting_level_array,
                               SymDescriptor D)
{
 tableau_entry *new_entry;
 AST_INDEX sub_list,name,func_name,param_list,parameter,simplified_param;
 AST_INDEX subscript1,subscript2, simplified_subscript;
 AST_INDEX node,entry;
 AST_INDEX subscript1_pointer;


  node = root;
  new_entry = allocate_tableau_entry();

  if (is_unary_minus(root))   /* should never get a unary minus here */
    {
     new_entry->coeff = 1;
     new_entry->score = score_node(root,nesting_level_array,D);
     new_entry->astptr = tree_copy(root);
     new_entry->op = op;


    }

  else if (is_constant(root))
    {
     new_entry->coeff = atoi (gen_get_text(root));
     new_entry->score = score_node(root,nesting_level_array,D);
     new_entry->astptr = AST_NIL;
     new_entry->op = op;
   }

  else if (is_identifier(root))
    {
     new_entry->coeff = 1;
     new_entry->astptr = tree_copy(root);
     new_entry->score = score_ref(root,nesting_level_array,D);
     new_entry->op = op;
   }


  else if (is_subscript(root))
    {
     sub_list = gen_SUBSCRIPT_get_rvalue_LIST(root);
     name = gen_SUBSCRIPT_get_name(root);


     if (!(list_empty(sub_list)))
       {
         subscript1 = list_first(sub_list);


       /* The while loop simplifies the subscript expressions
          of the array refference. */


         while (subscript1 != AST_NIL)
             {
              if (list_next(subscript1) != AST_NIL)
                 subscript2 = list_next(subscript1);
              else 
		{
                 subscript2 = list_next(subscript1);
                 subscript1_pointer = subscript1;
                 subscript1 = tree_copy(subscript1);
	       }
      
              simplify(subscript1,nesting_level_array,D);

              if (subscript2 != AST_NIL)
                 {
                  tree_replace_free(list_prev(subscript2),subscript1);
                  subscript1 = subscript2;
		}
              else 
                 {
		  tree_replace_free(subscript1_pointer, subscript1);
                  subscript1 = AST_NIL;
                 }                

	    }
      }



     new_entry->score = score_ref(root,nesting_level_array,D);
     new_entry->coeff = 1;
     new_entry->astptr = tree_copy(root);
     new_entry->op = op;

   }


  else if (is_invocation(root))
    {
     func_name = gen_INVOCATION_get_name(root);
     param_list = gen_INVOCATION_get_actual_arg_LIST(root);

     if (!(list_empty(param_list)))
       {
         subscript1 = list_first(param_list);

     while (subscript1 != AST_NIL)
         {
              if (list_next(subscript1) != AST_NIL)
                 subscript2 = list_next(subscript1);
              else
                {
                 subscript2 = list_next(subscript1);
                 subscript1_pointer = subscript1;
                 subscript1 = tree_copy(subscript1);
               }

              simplify(subscript1,nesting_level_array,D);

              if (subscript2 != AST_NIL)
		{
                  tree_replace_free(list_prev(subscript2),subscript1);
                  subscript1 = subscript2;
                }
              else
		{
                  tree_replace_free(subscript1_pointer, subscript1);
                  subscript1 = AST_NIL;
		}

            }
       }


     new_entry->score = score_ref(func_name,nesting_level_array,D);
     new_entry->coeff = 1;
     new_entry->astptr = tree_copy(root);
     new_entry->op = op;

   }


  else if (is_binary_divide(root)||
           is_binary_exponent(root))
    {
     new_entry->coeff = 1;
     new_entry->score = score_node(root,nesting_level_array,D);
     new_entry->astptr = tree_copy(root);
     new_entry->op = op;
   }
  
return(new_entry);

}







/*******************************************************************************/
/* routine that takes as an input a term tableau and constructs a single entry */
/* for the expression tableau for the particular term.                         */
/* The constants of all the entries in the term tableau are combined to give   */
/* the constant coefficient of the term.                                       */
/* The tree bodies of the  entries are combined to create the bode of the term.*/
/*******************************************************************************/


tableau_entry *recreate_term(tableau_type tableau, int tableau_size, int op)
{
 int coeff,score;
 AST_INDEX root;
 tableau_entry *new_entry;
 int i;

 coeff = 1;
 root = AST_NIL;

 for (i=0; i<=tableau_size - 1 ; i++)
   {
     coeff = coeff * tableau[i]->coeff;
     if (tableau[i]->astptr != AST_NIL)
        if (root == AST_NIL)
           root = tableau[i]->astptr;
        else 
           root = gen_BINARY_TIMES(root,tableau[i]->astptr);

   }

  score = tableau[0]->score;

  new_entry = allocate_tableau_entry();
  new_entry->coeff = coeff;
  new_entry->score = score;
  new_entry->astptr = root;
  new_entry->op = op;


  return(new_entry);

}


/*****************************************************************************/
/* Combines the constant coefficients of two adjacent terms with identical   */
/* trees. After combination the second tree is nulled out by setting         */
/* it's coefficient to 0.                                                    */
/*****************************************************************************/



void combine_terms (tableau_type tableau, int tableau_size)
{
 int i;
 int combining_op;


  for ( i=tableau_size - 2; i>=0; i--)
    {
     if (tableau[i]->op == tableau[i+1]->op)
        combining_op = PLUS;
     else combining_op = MINUS;

     if (tableau[i]->score == tableau[i+1]->score)
       {
         if (ast_equal ( tableau[i]->astptr,tableau[i+1]->astptr) == true)
	     {
               tableau[i]->coeff = compute_val (combining_op,tableau[i]->coeff,tableau[i+1]->coeff);
               tableau[i+1]->coeff = 0;
               tableau[i+1]->astptr = AST_NIL;
     
               if (tableau[i]->coeff < 0)
		 {
                  if (tableau[i]->op == MINUS)
                     tableau[i]->op = PLUS;
                  else tableau[i]->op = MINUS;
                  
                  tableau[i]->coeff = - tableau[i]->coeff;
		}
	     }
       }
   }



}







/*****************************************************************************/
/* Computes the result of applying the operation OP to VAL1 and VAL2.        */
/* This routine is used when recreating a term from the term tableau in      */
/* to compute the constant coefficient of the new term.                      */
/*****************************************************************************/


int compute_val (int op, int val1, int val2)
{
 int result;


 if (op == PLUS)
    result = val1 + val2;
 else if (op == MINUS)
    result = val1 - val2;
 else if (op == TIMES)
     result = val1 * val2;
 else if (op == DIVIDE)
     result = val1 / val2;
 else if (op == EXPONENT)
     result =  pow( (double)val1, (double)val2 );

 return(result);

}




/******************************************************************************/
/* This routine inserts a unary minus to the far left end of a left  leaning  */
/* tree. It iterates to the left until it finds a node that corresponds to    */
/* an identifier or a constant. Then it replaces this node with a unary_minus */
/* node.                                                                      */
/* This  routine is used in order to avoid having a unary minus at the left   */
/* end of an expression.                                                      */
/******************************************************************************/


void insert_unary_minus(AST_INDEX root)
{
AST_INDEX new_root;
 
 while ( !(is_identifier(root)) &&
         !(is_constant(root))   &&
         !(is_subscript(root))  &&
         !(is_invocation(root)) &&
         !(is_binary_divide(root)) &&
         !(is_binary_exponent(root)))

   {
     if (is_unary_minus(root))
        root = gen_UNARY_MINUS_get_rvalue(root);


    /* a binary plus or binary minus should never occur here */

     else if (is_binary_plus(root))
        root = gen_BINARY_PLUS_get_rvalue1(root);

     else if (is_binary_minus(root))
        root = gen_BINARY_MINUS_get_rvalue1(root);

     else if (is_binary_times(root))
        root = gen_BINARY_TIMES_get_rvalue1(root);

   }

new_root = gen_UNARY_MINUS(tree_copy(root));
tree_replace_free(root,new_root);

}



/************************************************************************/
/* Routine that converts the expression tableau to an AST. It builds    */
/* new CONSTANT nodes for the coefficients and new TIMES nodes to       */
/* multiply them into their terms. Finally it builds a new left leaning */
/* tree.                                                                */
/************************************************************************/


AST_INDEX recreate_tree (tableau_type tableau, int tableau_size)
{
  int term_count,i;
  AST_INDEX root;
  Boolean	found;
  AST_INDEX temp;
  tableau_entry *t;
  char *string_val;
  char name[4];



  string_val = (char *) get_mem (sizeof(name),"string_val");


  term_count = 0;
  for (i=0; i<=tableau_size - 1 ; i++)
    {
      if (tableau[i]->coeff != 0)
	{
          term_count ++;
          if (i > term_count - 1)
	    tableau[term_count - 1] = tableau[i];
	}
    }


  tableau_size = term_count;


   

  for (i= 0 ; i<=tableau_size - 1 ; i++)
    {
     if (tableau[i]->astptr == AST_NIL)
       {
	 temp = gen_CONSTANT();
        sprintf(string_val,"%d",tableau[i]->coeff);
        gen_put_text(temp,string_val,STR_CONSTANT_INTEGER);
        tableau[i]->astptr = temp;
      }


     else if (tableau[i]->coeff != 1)
             {
              temp = gen_CONSTANT();
              sprintf(string_val,"%d",tableau[i]->coeff);
              gen_put_text(temp,string_val,STR_CONSTANT_INTEGER);

              tableau[i]->astptr = gen_BINARY_TIMES(tableau[i]->astptr,temp);
	    }
   }




  if (tableau_size ==0)
    {
     temp = gen_CONSTANT();
     gen_put_text(temp,"0",STR_CONSTANT_INTEGER);
     return(temp);
   }



/* Try to avoid a minus at the beginning of the expression */

  else if (tableau[0]->op == MINUS)
       {
        found = false;
        i = 1;

        while ((tableau[i]->score == tableau[0]->score)&&
               (found == false) &&

               (i <= tableau_size - 1))
           
  	  {
           if (tableau[i]->op == PLUS)
              found = true;
           if (found == false)
              i++ ;
          }

        if (found == true)
	   {
            t = tableau[0];
            tableau[0] = tableau[i];
            tableau[i] = t;
           }
        else insert_unary_minus(tableau[0]->astptr);                                             
 
      }


        root = tableau[0]->astptr;
        for (i=1 ; i<=tableau_size - 1 ; i++)
            {
             if (tableau[i]->op == PLUS)
                {
                 root = gen_BINARY_PLUS(root,tableau[i]->astptr);
                 free_mem(tableau[i]);
	       }
             else if (tableau[i]->op == MINUS)
	       {
                root = gen_BINARY_MINUS(root,tableau[i]->astptr);
                free_mem(tableau[i]);
	      }
	   }
      
     
    return(root);

}


  


     
/**********************************************************************/
/* Routine that gives a numeric value to a node type. The values are  */
/* chosen to make the smaller expressions come out with higher scores */
/*                                                                    */
/* All constants get a score of 0.                                    */
/*                                                                    */
/* The array identifiers get a score in the range :                   */
/* 1 : SYMMAX.   ( Where SYMMAX is the maximum index in the symbol    */
/*                 table.)                                            */
/*                                                                    */
/* All other identifiers (except or the induction variables) get a    */
/* score in the range :                                               */
/* ( SYMMAX + 2 ) : ( 2*SYMMAX + 1 )                                  */
/*                                                                    */
/* The induction variables get a score in the range :                 */
/* 2*SYMMAX + NESTING_LEVEL + 2  (where NESTING_LEVEL gets values     */
/*                                from 1 to 7.)                       */
/*                                                                    */
/* All operators get a score in the range :                           */
/* ( 3*SYMMAX + 2 + MIN_OP_SCORE ) : ( 4*SYMMAX + 2 + MAX_OP_SCORE )  */
/* The operators that are actually given a score are the DIVIDE and   */
/* EXPONENT. CALL and SUBSCRIPT nodes are scored through score_ref.   */
/* They receive a score according to the name of the array or the     */
/* procedure name.                                                    */
/**********************************************************************/



int score_node(AST_INDEX node, nest_level_type nesting_level_array, SymDescriptor D)
{
 int score;
 int sym_max;

 
 sym_max = fst_MaxIndex(D);

  if (is_identifier(node))
    score = score_ref(node,nesting_level_array,D);
   
  else if (is_unary_minus(node))
     {score = 10;
      score = score + 3*sym_max + 2;}

  else if (is_binary_plus(node))         /* actually never used */
     {score = 101;
      score = score + 3*sym_max + 2;}

  else if (is_binary_minus(node))        /* actually never used */
     {score = 102;
      score = score + 3*sym_max + 2;}

  else if (is_binary_times(node))        /* actually never used */
     {score = 103;
      score = score + 3*sym_max + 2;}

  else if (is_binary_divide(node))
     {score = 104;
      score = score + 3*sym_max + 2;}

  else  if (is_binary_exponent(node))
     {score = 105;
      score = score + 3*sym_max + 2;}

  else if (is_subscript(node))          /* actually never used */
     {score = 106;
      score = score + 3*sym_max + 2;}

  else if (is_invocation(node))         /* actually never used */
     {score = 107;
      score = score + 3*sym_max + 2;}

  else if (is_constant(node))
    score = 0;
     
  else 
     {score = 150;
      score = score + 3*sym_max + 2;}

return(score);

}
  







/*************************************************************************/
/* Routine that gives a score to an identifier.                          */
/* The scores are used in order to sort the factors of each term, and to */
/* sort the terms of the initial expression.                             */
/* The sorting of the factors within a particular term is done in order  */
/* to push the induction variable of the term to the left.               */
/* After that we assign the term the score of the induction variable.    */
/* After processing each of the terms as described above we sort the     */
/* terms of the original expression. Since the terms have been assigned  */
/* the scores that correspond to the induction variables, after this     */
/* sorting the nodes that contain the same induction variable will end   */
/* up in adjacent locations in the expression tableau. This will enable  */
/* to combine the terms that contain the same induction variable.        */
/* will enable the combining of terms with the same induction variable.  */
/*************************************************************************/



int score_ref(AST_INDEX root, nest_level_type nesting_level_array, SymDescriptor D)
{int score;
 int sloc;
 int sym_max;
 AST_INDEX root1;



 sym_max = fst_MaxIndex(D);

 if (is_subscript(root))
    root1 = gen_SUBSCRIPT_get_name(root);
 else root1 = root;

 sloc = fst_QueryIndex(D,gen_get_text(root1));

 score = sym_max - sloc + 1;
 
 if (!(is_subscript(root)))
     score = score + sym_max + 1;

 if (nesting_level(root1,nesting_level_array) != 0)     /* this is an induction variable */
    score = score + sloc + nesting_level(root1,nesting_level_array);

 return(score);

}
  

  



/************************************************************************/
/* routine  that calls the ped_simplifier on both children of a DIVIDE  */
/* or EXPONENT node and then constructs a new node with the simplified  */
/* children.                                                            */
/* This routine is called on the original expression before it is       */
/* normalized.                                                          */
/* It also handles special cases of the result of the simplification.   */
/* If both of the results of the simplification are constants, the      */
/* routine calculates the result of applying division or exponentiation.*/
/* If only the result of the right child is a constant, then the        */
/* checks if this constant is 1, and if so, determines that the result  */
/* of the simplificatin is the (simplified) left child.                 */
/* This is done because we want to eliminate special cases, such  as    */
/* division by one, that would inhibit further simplification.          */
/* Since this is done before normalizing and simlifying the original    */
/* expression, in the case of a division by one, we can further         */
/* simplify the result of the first pass over the expression.           */
/************************************************************************/


int simp_divide(AST_INDEX passed_node, SymTab_NestLev_Type SymTab_NestLev)
{
 AST_INDEX left,right,returned_node;
 int left_val, right_val,returned_val;
 AST_INDEX new_left,new_right;
 SymDescriptor D;
 nest_level_type	nesting_level_array;


  D = SymTab_NestLev->D;
  nesting_level_array = SymTab_NestLev->nesting_level_array;

  if (is_binary_divide(passed_node)||
      is_binary_exponent(passed_node))

    {
     if (is_binary_divide(passed_node))
        {
         simplify(gen_BINARY_DIVIDE_get_rvalue1(passed_node),nesting_level_array,D);
         simplify(gen_BINARY_DIVIDE_get_rvalue2(passed_node),nesting_level_array,D);
        }
     else if (is_binary_exponent(passed_node))
       {
        simplify(gen_BINARY_EXPONENT_get_rvalue1(passed_node),nesting_level_array,D);
        simplify(gen_BINARY_EXPONENT_get_rvalue2(passed_node),nesting_level_array,D);
      }


     if (is_binary_divide(passed_node))
        left = gen_BINARY_DIVIDE_get_rvalue1(passed_node);
     else if (is_binary_exponent(passed_node))
        left = gen_BINARY_EXPONENT_get_rvalue1(passed_node);

     if (is_binary_divide(passed_node))
        right = gen_BINARY_DIVIDE_get_rvalue2(passed_node);
     else if (is_binary_exponent(passed_node))
        right = gen_BINARY_EXPONENT_get_rvalue2(passed_node);



/* if the dimplification produces numbers on both sides, 
   calculate the result of the division over the two numbers. */

     if (is_constant(left)&&
         is_constant(right))
         
       {
        left_val = atoi(gen_get_text(left));
        right_val = atoi(gen_get_text(right));

        generate_new_constant(left_val,right_val,passed_node);

      }

    else if (is_constant(left)&&
             is_unary_minus(right))
      {
        left_val = atoi(gen_get_text(left));

        new_right = gen_UNARY_MINUS_get_rvalue(right);
        right_val = -atoi(gen_get_text(new_right));

        generate_new_constant(left_val,right_val,passed_node);

      }

     else if (is_constant(right)&&
             is_unary_minus(left))
       {
        new_left = gen_UNARY_MINUS_get_rvalue(left);
        left_val = -atoi(gen_get_text(new_left));

        right_val = atoi(gen_get_text(right));

        generate_new_constant(left_val,right_val,passed_node);

      }

     else if (is_unary_minus(right)&&
             is_unary_minus(left))
       {

        new_left = gen_UNARY_MINUS_get_rvalue(left);
        left_val = -atoi(gen_get_text(new_left));

        new_right = gen_UNARY_MINUS_get_rvalue(right);
        right_val = -atoi(gen_get_text(new_right));

        generate_new_constant(left_val,right_val,passed_node);

      }



             

    else if (is_constant(right))
      {
       right_val = atoi(gen_get_text(right));
       if (right_val == 1)
          tree_replace_free(passed_node,tree_copy(left));

     }




    else if (is_constant(left))
      {
       left_val = atoi(gen_get_text(left));
       if ((left_val == 1)&&
           is_binary_exponent(passed_node))
          tree_replace_free(passed_node, tree_copy(left));
     }

  
   }

return(WALK_CONTINUE);

}






/************************************************************************/
/* Routine that takes the results of applying simplify on both sides    */
/* of a division node, if they are constants, and returns the constant  */
/* result.                                                              */
/************************************************************************/

void generate_new_constant(int left_val, int right_val, AST_INDEX passed_node)
{ AST_INDEX new_const;
  char *string_val;
  char name[4];
  int returned_val;


 string_val = (char *) get_mem (sizeof(name),"string_val");

 if (is_binary_divide(passed_node))
     returned_val = compute_val(DIVIDE,left_val,right_val);
 else if (is_binary_exponent(passed_node))
     returned_val = compute_val(EXPONENT,left_val,right_val);


  new_const = gen_CONSTANT();
  sprintf(string_val,"%d",returned_val);
  gen_put_text(new_const,string_val,STR_CONSTANT_INTEGER);
  tree_replace_free(passed_node, new_const);

  free_mem (string_val);

}



/*************************************************************************/
/* routine that allocates and returns a new tableau entry.               */
/*************************************************************************/

tableau_entry *allocate_tableau_entry()

{
 tableau_entry *new_entry;

 new_entry = (tableau_entry *) get_mem (sizeof(tableau_entry),"tableau_entry");

 new_entry->astptr = AST_NIL;
 new_entry->op = 0;
 new_entry->score = 0;
 new_entry->coeff = 0;


 return(new_entry);

}





/************************************************************************/
/* Insertion sort to put a term or expression teableau into canonical   */
/* order. Insertion sort< is chosen for it's stability, since we want to */
/* leave otherwise equal subtrees in their original order               */
/************************************************************************/

void sort(tableau_type tableau, int tableau_size, nest_level_type nesting_level_array,
          SymDescriptor D)
{
 int probe,i;
 Boolean	found,shift;
 tableau_entry *temp;


 for (i=1;i<=tableau_size - 1 ;i++)
   {
    probe = i;
    temp = tableau[i];
    found = false;

    while ((found == false)&&
           (probe > 0))

      {
       if (temp->score > tableau[probe - 1]->score)
          shift = true;
       else if (temp->score < tableau[probe - 1]->score)
          shift = false;
       else if (compare_trees (temp->astptr,tableau[probe - 1]->astptr,nesting_level_array,D) == GREATER_THAN)
          shift = true;
       else shift = false;



       if (shift == true)
          {
           tableau[probe] = tableau[probe - 1];
           probe--;
           }
       else found = true;
	   
     }

   tableau[probe] = temp;
	 
  }



}
   
  

/**************************************************************************/
/* routine that compares two trees with the same score to decide which    */
/* should come first in order. The routine compares the nodes of the      */
/* trees favoring the smaller trees.                                      */
/**************************************************************************/
        
int compare_trees (AST_INDEX r1, AST_INDEX r2, nest_level_type nesting_level_array,
                   SymDescriptor D)
{int score1, score2;
 AST_INDEX sub_list1,sub_list2;


 score1 = score_node(r1,nesting_level_array,D);
 score2 = score_node(r2,nesting_level_array,D);


 if (score1 < score2) 
    return(LESS_THAN);
 else if (score1 > score2)
    return(GREATER_THAN);
 else if ((score1 == 0) &&
          (score2 == 0))
     return(EQUAL);
 else 
   {
    if (is_identifier(r1)||
        is_constant(r1))
       return(EQUAL);
    else if (is_unary_minus(r1))
       return(compare_trees(gen_UNARY_MINUS_get_rvalue(r1),gen_UNARY_MINUS_get_rvalue(r2),nesting_level_array,D));
    else if (is_binary_plus(r1)||
             is_binary_minus(r1)||
             is_binary_times(r1)||
             is_binary_divide(r1)||
             is_binary_exponent(r1))
      {
       score1 = compare_trees(get_left_child(r1),get_left_child(r2),nesting_level_array,D);
       if (score1 != EQUAL)
          return(score1);
       else 
          return(compare_trees(get_right_child(r1),get_right_child(r2),nesting_level_array,D));
      }

    else if (is_subscript(r1))
      {
       score1 = compare_trees(gen_SUBSCRIPT_get_name(r1),gen_SUBSCRIPT_get_name(r2),nesting_level_array,D);
       if (score1 != EQUAL)
          return(score1);
       else
	 {
          sub_list1 = gen_SUBSCRIPT_get_rvalue_LIST(r1);
          sub_list2 = gen_SUBSCRIPT_get_rvalue_LIST(r2);
          while (!(list_empty(sub_list1)))
             {
              if (list_empty(sub_list2))
                 return(GREATER_THAN);
              else
		{
                 score1 = compare_trees(list_first(sub_list1),list_first(sub_list2),nesting_level_array,D);
                 if (score1 != EQUAL)
                    return(score1);
                 else 
		   {
                    sub_list1 = list_next(sub_list1);
                    sub_list2 = list_next(sub_list2);
	           }
               }
            }
     
          if (list_empty(sub_list2))
             return(EQUAL);
          else 
             return(LESS_THAN);
	}
     }
                


    else if (is_invocation(r1))
      {
       score1 = compare_trees(gen_INVOCATION_get_name(r1),gen_INVOCATION_get_name(r2),nesting_level_array,D);
       if (score1 != EQUAL)
          return(score1);
       else
         {
          sub_list1 = gen_INVOCATION_get_actual_arg_LIST(r1);
          sub_list2 = gen_INVOCATION_get_actual_arg_LIST(r2);
          while (!(list_empty(sub_list1)))
	    {
              if (list_empty(sub_list2))
                 return(GREATER_THAN);
              else
                {
                 score1 = compare_trees(list_first(sub_list1),list_first(sub_list2),nesting_level_array,D);
                 if (score1 != EQUAL)
                    return(score1);
                 else
                   {
                    sub_list1 = list_next(sub_list1);
                    sub_list2 = list_next(sub_list2);
		  }
               }
            }

          if (list_empty(sub_list2))
             return(EQUAL);
          else
             return(LESS_THAN);
        }
     }
  
  }

}



/*************************************************************************/
/* Routine that compares twqo AST's to determine if they are equal.      */
/* It walks through both trees, determining if the roots are equal.      */
/* If so  , it call itself on the children of the roots.                 */
/*************************************************************************/


Boolean	ast_equal (AST_INDEX r1,AST_INDEX r2)
{Boolean	right_result;
 AST_INDEX	sub_list1,sub_list2;


 if (ast_get_node_type(r1) != ast_get_node_type(r2))
    return(false);
 else
   {
    if (is_unary_minus(r1))
       return(ast_equal(gen_UNARY_MINUS_get_rvalue(r1),gen_UNARY_MINUS_get_rvalue(r2)));
 
    else if (is_binary_plus(r1)||
             is_binary_minus(r1)||
             is_binary_times(r1)||
             is_binary_divide(r1)||
             is_binary_exponent(r1))
     {
      right_result = ast_equal(get_left_child(r1),get_left_child(r2));    
      if (right_result == true)
         return(ast_equal(get_right_child(r1),get_right_child(r2)));
      else return(false);
     }
 
   else if (is_subscript(r1))
      {
       right_result = ast_equal(gen_SUBSCRIPT_get_name(r1),gen_SUBSCRIPT_get_name(r2));
       if (right_result != true)
          return(right_result);
       else
         {

          sub_list1 = gen_SUBSCRIPT_get_rvalue_LIST(r1);
          sub_list2 = gen_SUBSCRIPT_get_rvalue_LIST(r2);
          while (!(list_empty(sub_list1)))
	    {
              if (list_empty(sub_list2))
                 return(false);
              else
                {
                 right_result = ast_equal(list_first(sub_list1),list_first(sub_list2));
                 if (right_result != true)
                    return(right_result);
                 else
                   {
                    sub_list1 = list_next(sub_list1);
                    sub_list2 = list_next(sub_list2);
		  }
               }
            }

          if (list_empty(sub_list2))
             return(true);
          else
             return(false);
        }
     }




   else if (is_invocation(r1))
     {
       right_result = ast_equal(gen_INVOCATION_get_name(r1),gen_INVOCATION_get_name(r2));
       if (right_result != true)
          return(right_result);
       else
         {

          sub_list1 = gen_INVOCATION_get_actual_arg_LIST(r1);
          sub_list2 = gen_INVOCATION_get_actual_arg_LIST(r2);
          while (!(list_empty(sub_list1)))
            {
              if (list_empty(sub_list2))
                 return(false);
              else
                {
                 right_result = ast_equal(list_first(sub_list1),list_first(sub_list2));
                 if (right_result != true)
                    return(right_result);
                 else
                   {
                    sub_list1 = list_next(sub_list1);
                    sub_list2 = list_next(sub_list2);
                  }
               }
            }

          if (list_empty(sub_list2))
             return(true);
          else
             return(false);
        }
     }

    else   /* this is an identifier or a constant   */
      {
       if (strcmp(gen_get_text(r1), gen_get_text(r2))==0)
           return(true);
       else return(false);
     }
  }
}


/****************************************************************************/
/* routine that takes as an input a term and returns an equivalent left     */
/* leaning term.                                                            */
/****************************************************************************/

AST_INDEX normalize_term(AST_INDEX passed_node,AST_INDEX new_root)
{

  if (is_binary_times(passed_node))
     {
       new_root = normalize_term( gen_BINARY_TIMES_get_rvalue1(tree_copy(passed_node)),tree_copy(new_root));
       new_root = normalize_term( gen_BINARY_TIMES_get_rvalue2(tree_copy(passed_node)),tree_copy(new_root));
     }

  else
    {
      if  ( new_root == AST_NIL)
         new_root = tree_copy(passed_node);
      else new_root = gen_BINARY_TIMES(tree_copy(new_root),tree_copy(passed_node));
    }


return(new_root);
}
       


/***************************************************************************/
/* Routine that takes as an input an identifier and returns it's nesting   */
/* level, if it is an induction variable, otherwise returns 0.             */
/***************************************************************************/

int nesting_level(AST_INDEX root, nest_level_type nesting_level_array)
{
 char *name;
 int i;

 name	= gen_get_text(root);

 for ( i=0; i<MAX_NEST_LEVEL_TYPE; i++)
   {
    if ((nesting_level_array)[i] == NULL)
         {
           return(0);    /* this is not an induction variable */
	 }

    if (strcmp(name,(nesting_level_array)[i])==0)
         {
          return (i);    /* this is an induction variable, 
                        return the nesting level */
	}
  }
 
  return(0);         /* this is not an induction variable */
} 



/************************************************************************/
/* routines that give the left and right children of binary nodes       */
/************************************************************************/

 
AST_INDEX get_left_child(AST_INDEX passed_node)
{ 
  if (is_binary_plus(passed_node))
     return(gen_BINARY_PLUS_get_rvalue1(passed_node));
  else if (is_binary_minus(passed_node))
     return(gen_BINARY_MINUS_get_rvalue1(passed_node));
  else if (is_binary_times(passed_node))
     return(gen_BINARY_TIMES_get_rvalue1(passed_node));
  else if (is_binary_divide(passed_node))
     return(gen_BINARY_DIVIDE_get_rvalue1(passed_node));
  else if (is_binary_exponent(passed_node))
     return(gen_BINARY_EXPONENT_get_rvalue1(passed_node));
}



AST_INDEX get_right_child(AST_INDEX passed_node)
{
  if (is_binary_plus(passed_node))
     return(gen_BINARY_PLUS_get_rvalue2(passed_node));
  else if (is_binary_minus(passed_node))
     return(gen_BINARY_MINUS_get_rvalue2(passed_node));
  else if (is_binary_times(passed_node))
     return(gen_BINARY_TIMES_get_rvalue2(passed_node));
  else if (is_binary_divide(passed_node))
     return(gen_BINARY_DIVIDE_get_rvalue2(passed_node));
  else if (is_binary_exponent(passed_node))
     return(gen_BINARY_EXPONENT_get_rvalue2(passed_node));
}



/************************************************************************/
/************************************************************************/
/*	LOCAL ROUTINES THAT MAY NOT NEED TO BE USED			*/  
/************************************************************************/
/************************************************************************/

#ifdef	LOCAL_POWER_FUNC

int power(int val1, int val2)
{
 int result,i;
 
/*
 result = 1;
 if (val2 >= 0)
   {
    for (i=1; i<= val2; i++)
      result = result * val1;
  }
 else 
   {
    for (i=1; i<= -val2; i++)
       result = result * val1;
  
    result = 1 / result;
  }
 */

result = pow((double) val1, (double) val2);
return(result);
}

#endif
