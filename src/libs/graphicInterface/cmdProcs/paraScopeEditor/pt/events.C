/* $Id: events.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*    ped_cp/pt/events.c						*/ 
/*    Last edited : May 9, 89 by jss                                    */
/*    Routines connected with event variable synchronization            */
/************************************************************************/  

#include <stdlib.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pt/private_pt.h>

#define  NOT_GOOD  6643411  /* Arbitrary large number  */
#define FORWARD    0
#define BACKWARD   1

/*
 *  Routine to compute dependence distance in simple cases  
 */
 
int  find_distance_vector (AST_INDEX src, AST_INDEX sink, STR_INDEX induction_var)
{
    AST_INDEX   src_subscript;
    AST_INDEX   sink_subscript;
    AST_INDEX   src_var_node;
    AST_INDEX   sink_var_node; 
    AST_INDEX   src_const_node;
    AST_INDEX   sink_const_node;
    int         src_const;
    int		sink_const;
    STR_INDEX   src_var;
    STR_INDEX   sink_var;
    


      if (!((is_subscript(src)) && (is_subscript(sink)))) 
           return(1);                         /* scalar dependences */
   	
   
      src_subscript = list_first(gen_SUBSCRIPT_get_rvalue_LIST(src));
      
      sink_subscript = list_first(gen_SUBSCRIPT_get_rvalue_LIST(sink));
  

    
      /*
       *  These are lists of nodes
       *  Construct distance term  
       */

       while ((src_subscript != NIL) && (sink_subscript != NIL))
       {
       	  /* Source */
	  switch (ast_get_node_type(src_subscript)) {
	  case GEN_BINARY_PLUS :
     	       	src_var_node   = gen_BINARY_PLUS_get_rvalue1(src_subscript);
		src_const_node = gen_BINARY_PLUS_get_rvalue2(src_subscript);
		if (is_identifier(src_var_node))
		   src_var = gen_get_symbol(src_var_node);
		else return(NOT_GOOD);
		if (is_constant(src_const_node))
		   src_const = atoi(gen_get_text(src_const_node));	
		else return(NOT_GOOD);
	        break;

	    case GEN_BINARY_MINUS :
     	     	src_var_node   = gen_BINARY_MINUS_get_rvalue1(src_subscript);
		src_const_node = gen_BINARY_PLUS_get_rvalue2(src_subscript);
		if (is_identifier(src_var_node))
		   src_var = gen_get_symbol(src_var_node);
           	else return(NOT_GOOD);
		if (is_constant(src_const_node))
		   src_const = atoi((gen_get_text(src_const_node)));	
		else return(NOT_GOOD);
		break;
		   	     
             case GEN_IDENTIFIER :
		src_var = gen_get_symbol(src_subscript);
		src_const = 0;
		break;
	     default:
	        return(NOT_GOOD);
          
             }
             switch (ast_get_node_type(sink_subscript)) {
	     
  	     case GEN_BINARY_PLUS :
	        sink_var_node   = gen_BINARY_PLUS_get_rvalue1(sink_subscript);
		sink_const_node = gen_BINARY_PLUS_get_rvalue2(sink_subscript);
		if (is_identifier(sink_var_node))
		   sink_var = gen_get_symbol(sink_var_node);
		else return(NOT_GOOD);
		if (is_constant(sink_const_node))
		   sink_const = atoi(gen_get_text(sink_const_node));
		else return(NOT_GOOD);
		break;
	     
	     case GEN_BINARY_MINUS :
		sink_var_node   = gen_BINARY_MINUS_get_rvalue1(sink_subscript);
		sink_const_node = gen_BINARY_MINUS_get_rvalue2(sink_subscript);
		if (is_identifier(sink_var_node))
		   sink_var = gen_get_symbol(sink_var_node);
                else return(NOT_GOOD);		
		if (is_constant(sink_const_node))
		   sink_const = -(atoi(gen_get_text(sink_const_node)));
		else return(NOT_GOOD);   
		break;

             case GEN_IDENTIFIER :
		sink_var = gen_get_symbol(sink_subscript);
		sink_const = 0;	     
		break;
	     default :
	         return(NOT_GOOD);
	     } 

             if (strcmp(string_table_get_text(src_var),string_table_get_text(induction_var))
                  == 0)
	     
	   /*  if (src_var == induction_var)      */
	         return (src_const - sink_const);
	     src_subscript = list_next(src_subscript);	 
	     sink_subscript = list_next(sink_subscript);
	     
	  }
	  return(NOT_GOOD);   
       }
       


/*
 *  Routine to check if a given dependence is protected by  Event       
 *  Variable synchronization                                            
 */ 
int pt_is_protected(PedInfo ped, EDGE_INDEX dep_index, AST_INDEX loop)



{
        AST_INDEX	stmt;
	   
	AST_INDEX	post; 
	AST_INDEX	wait;
	
        AST_INDEX 	src;
        AST_INDEX	sink;
 	AST_INDEX       src_stmt;
	AST_INDEX	sink_stmt;
	
	AST_INDEX	call;
	AST_INDEX	call_name;
	AST_INDEX       call_args;
	STR_TEXT	call_string;
        STR_TEXT        post_string;	
	STR_TEXT	wait_string;
	
	Boolean         found_post, found_wait, found_src, found_sink;
	int             direction;
	int             synch_direction;
	
        int             dep_distance;
        int		synch_distance;
	
        AST_INDEX       induction_var_node;
	STR_INDEX       induction_var;

	DG_Edge         *edgeptr;



/* find the induction variable */
   if (!is_do(loop)) return(ERROR);
   induction_var_node = gen_INDUCTIVE_get_name(gen_DO_get_control(loop));   
   induction_var = ast_get_symbol(induction_var_node);

   edgeptr = dg_get_edge_structure( PED_DG(ped));
   src  = edgeptr[dep_index].src;
   sink = edgeptr[dep_index].sink;
   
   src_stmt = src;
   sink_stmt = sink;
   
   while (!(is_statement(src_stmt)))
      src_stmt = out(src_stmt);
   while (!(is_statement(sink_stmt)))
      sink_stmt = out(sink_stmt);
      

/* Find the distance if in the form  : Var + Constant  */  
   dep_distance = find_distance_vector(ast_get_father(src),
                      ast_get_father(sink),induction_var);

    found_src = false;
    found_sink = false;
    found_post = false;
    found_wait = false;
    
    post_string = ssave("post");
    wait_string = ssave("wait");

    /*
     *   go through the loop statements     
     */
   
     for(stmt = list_first(gen_DO_get_stmt_LIST(loop));
  	 stmt != AST_NIL;
	 stmt = list_next(stmt))
      {  
          if (stmt == src_stmt)
	     found_src = true;
	     
	  if (stmt == sink_stmt)
	  {
	     found_sink = true;
	     if (!(found_wait))  return(NOT_PROTECTED);
	     if ((found_src == true) && (stmt != src_stmt))
	         direction = FORWARD;  
	     else direction = BACKWARD;
	  }
	  
          if  (is_call(stmt))
	  {
	     call = gen_CALL_get_invocation(stmt);
	     call_name = gen_INVOCATION_get_name(call);
	     call_args = gen_INVOCATION_get_actual_arg_LIST(call);
	     call_string = gen_get_text(call_name); 
	     if (strcmp(call_string, post_string) == 0)
	     {
	        found_post = true;
	        post = call_args;
                if (!found_src) return(NOT_PROTECTED);
	     }
	     if (strcmp(call_string, wait_string) == 0)
	     {
	        if (found_post)
		   synch_direction = FORWARD;
		else synch_direction = BACKWARD;
	        found_wait = true;
	        wait = call_args;
	     }
	  }     
      }
      if ((found_post == true) && (found_wait == true))
      synch_distance = find_distance_vector(post, wait, induction_var); 
      else return(NOT_PROTECTED);

      if ((synch_distance == NOT_GOOD) ||(dep_distance == NOT_GOOD))
	     return(ERROR);

      if (synch_distance == dep_distance)   
	     return(PROTECTED);
	   
      if (synch_direction == BACKWARD) 
      {  	        
	 if (synch_distance == 0 ) return(NOT_PROTECTED);
	 if ((dep_distance % synch_distance) == 0) return (PROTECTED);
      }
      return(NOT_PROTECTED);
}




