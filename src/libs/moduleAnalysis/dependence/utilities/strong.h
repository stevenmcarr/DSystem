/* $Id: strong.h,v 1.9 1997/03/11 14:36:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/pt/strong.h					*/
/*	Last edited: June 16, 1988 at 3:15 pm by bbc			*/
/*									*/
/*	strong.h -- strongly connected regions				*/
/*									*/
/*	Included if explicitly managing strongly connected regions	*/
/*									*/
/************************************************************************/
/* Included by anyone who needs to explicitly manage strongly
   connected regions */

#ifndef strong_h
#define strong_h

#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif					
#ifndef	general_h
#include <libs/support/misc/general.h>
#endif				

#ifndef	list_h
#include <libs/support/lists/list.h>
#endif


#define		NIL	-1		/* end of list value		*/

typedef struct adj_list_node {
     AST_INDEX	statement;
     AST_INDEX  do_node; 	/* Highest nested Do enclosing this. */
     int        carried_dep;
     int	region;
     int        loop;
     int        rlink;
     int	first_in_list;
     Boolean	marked;
     int	lowlink;
     int	dfnumber;
     int	stacklink;
     Boolean	instack;
} Adj_Node ;
 
typedef struct adj_list_link {
     int	next_in_list;
     int	node_index;
     Boolean    carried;
} Adj_Link;
 
typedef struct region_type {
     Boolean    parallel;
     int        first_stmt;
     Boolean    visited;     /* Used in distributing of statements */
} Region_Type;

typedef struct Loop_Type_struct { 
     UtilList	*stmts;
     Boolean    parallel;
} Loop_Type;

typedef struct adj_list {
     Adj_Node	*node_array;
     Adj_Link 	*link_array;
     Region_Type *region_array;
     Loop_Type   *loop_array;
     int	last_used_node;
     int	nodes_allocated;     
     int	last_used_link;
     int	links_allocated;
     int        max_region;
     int        max_loop;
} Adj_List;




EXTERN( int, pt_allocnode,
		(Adj_List *adj_list, AST_INDEX statement, AST_INDEX do_node));


EXTERN( void, pt_strong_regions, ( Adj_List * adj_list ));


EXTERN( int, pt_getindex, ( Adj_List * adj_list, AST_INDEX stmt ));


EXTERN( Adj_List *,  pt_create_adj_list, (void));


EXTERN( void, pt_destroy_adj_list, ( Adj_List * adj_list ));


EXTERN( void, pt_add_edge,
		(Adj_List * adj_list, AST_INDEX from_stmt,
		 AST_INDEX to_stmt, Boolean carried ) );


EXTERN( char *,  pt_print_regions, ( Generic AL, Generic handle ));


/*	The following are matched to the #defines in ped_include/pt.h	*/
/*	and are used as the return values of pt_split_loop()		*/
#define		SPLIT_BOTH		3
#define		SPLIT_UPPER_ONLY	1
#define		SPLIT_LOWER_ONLY	2
#define		SPLIT_NONE		4
EXTERN( int, pt_split_loop, (Adj_List *adj_list, AST_INDEX node) );


#endif
