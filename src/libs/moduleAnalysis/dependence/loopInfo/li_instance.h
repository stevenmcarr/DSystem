/* $Id: li_instance.h,v 1.10 1997/03/11 14:36:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************

   Changed SH abstraction to the more useful LI (Loop Info) abstraction.
   Added several new routines to support the manipulation of shared and
   private variables and statement insertion/deletion.
   - Vas, May 1988.

   NOTE: As long as the dependence info used by ParaScope is that generated 
   by PSERVE, remember to convert variable names into upper case before
   making any comparisons using strcmp(). This will be unnecessary when
   ParaScope becomes completely independent of PSERVE. 
   -Vas, May 1988.
 ****************************************************************************/

#ifndef li_instance_h
#define li_instance_h

#include <stdio.h>

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef	FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif
#ifndef	MapInfo_c_h
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>
#endif

#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	depType_h
#include <libs/moduleAnalysis/dependence/interface/depType.h>
#endif


/* VarType(s) allowed by PFC and local file storage interface
	within the Slist structure */
typedef enum { var_shared = 0, var_common = 1, var_equiv_common = 2, 
		 var_reserved = 3, var_private = 4 }	VarType;

/*-----------------------------------------------------------------------*/
/* Slist is a list of SHARED variables or PRIVATE variables in the loop. */
/*-----------------------------------------------------------------------*/


struct slist {
    char 	*name;
    AST_INDEX	def_before;
    AST_INDEX	use_after;
    VarType	why;
    Boolean	user;
    char	*cblock;
    int		dim;
    struct slist *next;
};

typedef struct slist Slist;


/* Loop_info is a linked list, with one link for each loop in the program.
   Each link has info about the loop, and a pointer to an shvar_list */

typedef struct loop_info Loop_info;


/* LI_Instance is the abstraction that maintains a handle on all info 
   associated with each loop as a whole. */

typedef struct Loops	LI_Instance;

/* Control flow types - stored in loop_info->cflow */

#define NOFLOW          1
#define STRUCT          2
#define UNSTRUCT        3
#define BACK_BRANCH     4

typedef	enum	
{li_noflow, li_struct, li_unstruct, li_back_branch } li_flow_type;

/* Constants for selective viewing of the shared variable list */
#define	ALL_SHARED	1
#define	NO_MOD_DEF	2
#define	MAY_MOD		3
#define MAY_USE		4
#define	COMMON		5
#define USER_SPEC_SHAR	6
#define USER_SPEC_PRIV  7




/***********************************************************************
	Routines to create and destroy the LI_Instance
 ***********************************************************************/

/* allocate the memory used by the LI_Instance
 */
EXTERN( LI_Instance *,  li_create_instance, (void) );

/* free the memory used by the LI_Instance
 */
EXTERN( void,	li_free,	( LI_Instance * LI ) );


/***********************************************************************
	Routines to access  Loop_info
 ***********************************************************************/

/*	field access function for Loop_info
*/
EXTERN( int,	loop_info_loopLevel,	( Loop_info	*lptr ) );


/***********************************************************************
	Routines to create and destroy an Slist
 ***********************************************************************/

EXTERN( Slist *,	 li_create_Slist, (void) );

EXTERN( void,	  li_init_Slist, 
		(Slist * sptr, char * name, char * block, int dim) );


/***********************************************************************
	Routines to save and read the LI_Instance  to a file
 ***********************************************************************/

/*
 * li_save_index() returns true if the save was successfull.
 */
EXTERN( Boolean, li_save_index, (FortTextTree ftt, LI_Instance * LI,
		 FILE * indexFP));

/*
 * readindex() returns the LI_Instance read from the ifile_ptr.
 * -- generally used when also restoring a DG_Instance and EL_Instance
 */
EXTERN( LI_Instance *,  readindex, (FILE	* ifile_ptr, MapInfoOpaque * map) );


/**********************************************
  Routines to support shared and private variables.	from /el/el.c NEW TO el.h
 **********************************************/

/*---------------------------------------------------------------------------
  check_if_shared() - check if the var at "astindex" must be in shared
  storage. If yes, return a ptr to the link in the Slist where info about 
  this var can be found.
 -------------------------------------------------------------------------- */
EXTERN( Boolean,	check_if_shared,
		(LI_Instance * LI, DepType dtype, char *name, Slist	** Q) );



/******************************************************************
  routines to support addition and deletion of entire loops.	from /el/li.c
 ******************************************************************/

/*
 * el_add_loop(): add a new Loop info node. The node is added immediately
 * following the Loop info node corresponding to the "prev_loop" loop. If
 * "prev_loop" is AST_NIL, the new loop is added at the head of the Linfo
 * list.  Return a pointer to this node, cast as a Loop_info *.
 *  	LI_Instance	*LI;
 *	AST_INDEX	prev_loop;
 *  	AST_INDEX	lindex;	 ast index of new loop. 
 *  	int		level;   loop level of new loop. 
 */
EXTERN( Loop_info *,  el_add_loop, ( LI_Instance * LI, 
		 AST_INDEX prev_loop, AST_INDEX lindex, int level) );

/*
 * el_remove_loop(): remove this loop from the Linfo list, and free its 
 * storage.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	loop;	ast index of loop to be removed.
 */
EXTERN( void, el_remove_loop, ( LI_Instance * LI, AST_INDEX loop ) );


/******************************************************************
  routines to manipulate the shared and private vars for each loop. from /dt/li.c
 ******************************************************************/

/* 
 * el_remove_shared_var(): remove var from shared var list of loop, and return 
 * a ptr to its Slist node cast as a Generic. If var is not found, return
 * (Generic) NULL.
 *  	LI_Instance	*LI;	handle to LI abstraction.
 *	AST_INDEX	loop;   astindex of loop hdr.
 *  	char		*var;	var name to be removed
 */
EXTERN( Generic,  el_remove_shared_var, 
		( LI_Instance * LI, AST_INDEX loop, char * var ) );

/* 
 * el_add_shared_var(): add Slist node (gptr) to shared list of loop, 
 * if gptr->name does not already exist in the list.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	 loop;
 *	  Slist		*s;
 */
EXTERN( void, el_add_shared_var, 
		( LI_Instance * LI, AST_INDEX loop, Slist * s ) );

/* 
 * el_force_add_shared_var(): add Slist node (gptr) to shared list of loop.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	 loop;
 *	  Slist		*s;
 */
EXTERN( void, el_force_add_shared_var, 
		( LI_Instance * LI, AST_INDEX loop, Slist * s ) );

/* 
 * el_remove_private_var(): remove var from private var list of loop, and 
 * return a ptr to its Slist node cast as a Generic. 
 *  	LI_Instance	*LI;	handle to LI abstraction.
 *	AST_INDEX	loop;   astindex of loop hdr.
 *  	char		*var;	var name to be removed
 */
EXTERN( Generic, el_remove_private_var, 
		( LI_Instance * LI, AST_INDEX loop, char * var ) );

/* 
 * el_add_private_var(): add Slist node (gptr) to private list of loop unless 
 * gptr->name  already exists in the list.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	 loop;
 *	  Slist		*p;
 */
EXTERN( void, el_add_private_var, 
		( LI_Instance * LI, AST_INDEX loop, Slist * p ) );

/* 
 * el_add_private_up(): add var to private list of this loop and all outer loops
 * unless var  already exists in the list.
 *  	LI_Instance	*LI;	handle to LI abstraction.
 *	AST_INDEX	loop;   astindex of loop hdr.
 *  	char		*var;	var name to be removed
 */
EXTERN( void, el_add_private_up, 
		( LI_Instance * LI, AST_INDEX loop, char * var ) );

/*
 * el_create_new_node(): create a new Slist node, and put the specified
 * info in it. Return a ptr to it. This routine should
 * be called if a new variable (i.e. one that is NOT already in the shared
 * or private lists of the loop) is to be added to one of these lists.
 * The ptr returned by this routine can then be passed to el_add_shared_var
 * or el_add_private_var to insert the var into one of these lists.
 *     Slist	*s;	Return value
 *     char		*name;		( default: "" )
 *     AST_INDEX	def_before;	( default: AST_NIL )
 *     AST_INDEX	use_after;	( default: AST_NIL )
 *     VarType		why;		( default: var_shared )
 *     char		*cblock;	( default: "" )
 *     int		dim;		( default: 0 )
 */
EXTERN( Slist *,  el_create_new_node, 
		( char * name, AST_INDEX def_before, AST_INDEX use_after,
		 VarType why, char * cblock, int dim ) );

/*
 * el_change_shared_var_name(): change the name of this shared variable.
 * This is called after a scalar variable has been expanded into an array.
 *	  LI_Instance	*LI;
 *	AST_INDEX	loop;
 *	char		*oldname;
 *	char		*newname;
 *	AST_INDEX	def_before;
 *	AST_INDEX	use_after;
 *	int		dims;
 */
EXTERN( void, el_change_shared_var_name, 
		( LI_Instance * LI, AST_INDEX loop, 
		 char * oldname, char * newname,
		 AST_INDEX def_before, AST_INDEX use_after, int dims ) );

/*
 * el_change_private_var_name(): change the name of this private variable.
 * Used by strip mine.
 *	  LI_Instance	*LI;
 *	AST_INDEX	loop;
 *	char		*oldname;
 *	char		*newname;
 */
EXTERN( void, el_change_private_var_name, 
		( LI_Instance * LI, AST_INDEX loop, char * oldname, char * newname ) );

/*
 * el_flip_private (): when outer i loop is interchanged with inner j loop
 * add i to the private list of the i loop, and remove j from the private list 
 * of the i loop.
 * Used in loop interchange.
 */

EXTERN( void, el_flip_private , 
		( LI_Instance * LI, AST_INDEX new_outer, AST_INDEX new_inner ) );

/*
 * el_copy_shared_list(): copy the shvar_list from loop n1 to loop n2.
 * If loop n2 already has a shvar_list, the shared vars of n1 will
 * be UNIONed with the shared vars of n2.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	n1;
 *	  AST_INDEX	n2;
 */
EXTERN( void, el_copy_shared_list, 
		( LI_Instance * LI, AST_INDEX n1, AST_INDEX n2 ) );

/*
 * el_copy_private_list(): copy the pvar_list from loop n1 to loop n2.
 * If loop n2 already has a pvar_list, the private vars of n1 will
 * be UNIONed with the private vars of n2.
 *	  LI_Instance	*LI;
 *	  AST_INDEX	n1;
 *	  AST_INDEX	n2;
 */
EXTERN( void, el_copy_private_list, 
		( LI_Instance * LI, AST_INDEX n1, AST_INDEX n2 ) );


/* 
 * el_get_shared_list():  return list of shared vars in a string 
 *	  LI_Instance	*LI;
 */
EXTERN( char *,  el_get_shared_list, ( LI_Instance * LI ) );

/*  
 * el_get_shared_info(): returns a string that contains information
 *   	about the shared variable var.
 *      LI_Instance    *LI;
 *   	char	       *var;
 *   	Generic         handle;
 *      GetTextCallback get_text;
 */
typedef FUNCTION_POINTER(char *, GetTextCallback, (Generic ped, AST_INDEX num));

EXTERN( char *,  el_get_shared_info, 
		( LI_Instance * LI, char * var,
		 Generic handle, GetTextCallback get_text) );

/*   
 *  el_get_loop_info(): set LI->cur_loop to the node in the Loop_info
 *  list where info for this loop is kept. Return false if such a node cannot
 *  be found. Else return true.
 *	  LI_Instance	*LI;
 *        AST_INDEX     node;
 */
EXTERN( Boolean,  el_get_loop_info, 
		( LI_Instance * LI, AST_INDEX node));

/*  
 * el_get_private_info(): returns a string that contains information
 *   	about the private variable var.
 *	  LI_Instance	*LI;
 *   	char	    *var;
 *   	Generic      handle;
 *      GetTextCallback get_text;
 */
EXTERN( char *,  el_get_private_info, 
		( LI_Instance * LI, char * var,
		  Generic handle, GetTextCallback get_text ) );

/* 
 * el_get_private_list():  return list of private vars in a string 
 *	  LI_Instance	*LI;
 */
EXTERN( char *,  el_get_private_list, ( LI_Instance * LI ) );

/* 
 * el_get_first_shared_node(): return a ptr to the Slist node that contains
 *  info about the first shared var for the current loop. "length" is set
 *  to the length of the shared var name. 
 *	  LI_Instance	*LI;
 *  	int	*length;
 */
EXTERN( Slist *,  el_get_first_shared_node, 
		( LI_Instance * LI, int * length, int type ) );

/* 
 * el_get_num_shared_vars(): return the number of shared vars, and set "longest" 
 *  to be the length of the longest shared var name 
 *	  LI_Instance	*LI;
 *   	int	*longest;
 */
EXTERN( int, el_get_num_shared_vars, ( LI_Instance * LI ) );

/* 
 * el_get_num_private_vars(): return the number of shared vars, and set "longest" 
 *  to be the length of the longest private var name 
 *	  LI_Instance	*LI;
 *   	int	*longest;
 */
EXTERN( int, el_get_num_private_vars, ( LI_Instance * LI ) );


/* 
 * el_get_next_shared_node(): return a ptr to the next Slist node in the 
 *  shvar_list. The routine "el_get_first_node" must be called before this 
 *  one. "s" is a ptr to the current Slist node. "length" is set to the length 
 *  of the shared var name. 
 *	  LI_Instance	*LI;
 *  	Slist	*s;
 *  	int	*length;
 *	int	 type;
 */
EXTERN( Slist *,  el_get_next_shared_node, 
		( LI_Instance * LI, Slist * s, int * length, int type ) );

/* 
 *  is_private_var():  Whether variable is private with respect to loop
 */
EXTERN( Boolean, is_private_var, 
		( LI_Instance	*LI, AST_INDEX	 var, AST_INDEX	 loop) );

/* 
 * el_get_first_private_node(): return a ptr to the Slist node that contains
 *  info about the first private var for the current loop. "length" is set
 *  to the length of the private var name. 
 *	  LI_Instance	*LI;
 *  	int	*length;
 *	int	 type;
 */
EXTERN( Slist *, el_get_first_private_node, 
		( LI_Instance * LI, int * length, int type ) );

/* 
 * el_get_next_private_node(): return a ptr to the next Slist node in the
 *  pvar_list. The routine "el_get_first_private_node" must be called before 
 *  this one. "p" is a ptr to the current Slist node. "length" is set to the 
 *  length of the shared var name. 
 *      LI_Instance	*LI;
 *  	Slist	*p;
 *  	int	*length;
 */
EXTERN( Slist *, el_get_next_private_node, 
		( LI_Instance * LI, Slist * s, int * length, int type ) );

/**********************************************************************
 routines to support the code generator.		from /el/el.c
 **********************************************************************/

/*
 * el_parallelized(): return true if this loop has already been parallelized.
 *	  LI_Instance	*LI;
 */
EXTERN( Boolean, el_parallelized, ( LI_Instance	*LI ) );

/*
 * el_set parallelized_bit(): set parallelized bit to true.
 *	  LI_Instance	*LI;
 */
EXTERN( void, el_set_parallelized_bit, ( LI_Instance	*LI ) );

/*
 * find_loop(): find the Loop_info node that contains info for this loop,
 * and return a ptr to it. Return NULL if such a node is not found.
 */
EXTERN( Loop_info *, 	find_loop, 
		( LI_Instance * LI, AST_INDEX lindex ) );

/*
 * li_cur_loop()	returns the current loop selected in the LI_Instance
 */
EXTERN( Loop_info *,  li_cur_loop, ( LI_Instance	*LI ) );

/*
 * el_cflow():  returns true if the selected loop has control flow
 */
EXTERN( int, el_cflow, ( LI_Instance * LI ) ); 

/*
 * li_set_cflow():  set the selected loop to control flow
 */
EXTERN( void, li_set_cflow, ( LI_Instance * LI, int cflow ) ); 

/*
 * li_max_cflow():  
 *  sets the control flow equal to the new_cflow if the new_cflow
 *  is more restrictive than the old_cflow. The order of cflow restrictiveness
 *  is :  NOFLOW < STRUCT < UNSTRUCT < BACK_BRANCH
 */
EXTERN( void, li_max_cflow, ( LI_Instance * LI, int new_cflow ) ); 




/*	**********	The Following are Never Used or Defined	*********/
/* 
 * var_occurs_in_loop():
 *		Return true if specified var is found within the loop.
 */
/*	Never Used or Defined.						*/
EXTERN( Boolean, 		var_occurs_in_loop, 
		( LI_Instance	* LI, AST_INDEX	loop, char * var ) );

/* Returns the ast index of the variable name of a subscripted index	*/
/*	Never Used or Defined.						*/
EXTERN( AST_INDEX, el_gen_SUBSCRIPT_get_name, ( AST_INDEX index ) );

/*  el_is_identifier():  returns true if index is an identifier */
/*	Never Used or Defined.						*/
EXTERN( Boolean, el_is_identifier, ( AST_INDEX index ) );

/* 	el_is_subscript():  returns true if index is a subscript */
/*	Never Used or Defined.						*/
EXTERN( Boolean, el_is_subscript, ( AST_INDEX index ) );



/***********************************************************************/

EXTERN(Boolean, already_exists, (Slist *list, char *name));

#endif	/* li_instance_h */










