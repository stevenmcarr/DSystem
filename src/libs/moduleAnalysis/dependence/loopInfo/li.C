/* $Id: li.C,v 1.1 1997/06/25 15:09:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*                                                              */
/*	dep/li/li.c                               		*/
/*                                                              */
/*	li.c -- The loop-info abstraction.                      */
/*                                                              */
/*	The following functions should all be li_...		*/
/*	none of them use the EL_Instance			*/
/*                                                              */
/*	el_cflow()						*/
/*	li_set_cflow()						*/
/*	max_cflow()					   	*/

/*	find_loop()					        */
/*	already_exists()					*/
/*	check_if_shared()					*/
/*	el_get_loop_info()					*/
/*	dumpLinfo()						*/
/*	el_add_loop()						*/
/*	el_remove_loop()					*/
/*	el_parallelized()					*/
/*	el_set_parallelized_bit()				*/

/*	el_remove_shared_var()					*/
/*	el_add_shared_var()					*/
/*	el_force_add_shared_var()				*/
/*	el_remove_private_var()					*/
/*	el_add_private_var()					*/
/*	el_add_private_up()					*/
/*	el_create_new_node()				        */
/*	el_change_shared_var_name()				*/
/*	el_change_private_var_name()				*/
/*	el_copy_shared_list()					*/
/*	el_copy_private_list()					*/
/*	el_get_shared_list()					*/
/*	el_get_shared_info()					*/
/*	el_get_private_list()					*/
/*	el_get_num_shared_vars()				*/
/*	el_get_first_shared_node()				*/
/*	el_get_next_shared_node()				*/
/*	el_get_first_private_node()				*/
/*	el_get_next_private_node()				*/
/*      li_free()						*/
/****************************************************************/

/*	*********************************************************
 *	Include Files:
 */

#include <string.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/frontEnd/ast/groups.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>



/* -----------------------------------------------------------------------
   li_create_instance() - create a loop info structure.
 ----------------------------------------------------------------------- */
LI_Instance *
li_create_instance(void)
{
  LI_Instance *li;

  li = (LI_Instance *) get_mem(sizeof(LI_Instance), "dg_construct_LI");
  li -> Linfo = 0;
  li -> cur_loop = 0;
  li -> num_loops = 0;
  return li;
}


/*-----------------------------------------------------------------------

	li_create_Slist()	Allocate variable for shared/private lists

*/
Slist *
li_create_Slist(void)
{
	Slist	*sList;

	sList	= (Slist *) get_mem(sizeof(Slist), "li_create_Slist");
	sList->name	= NULL; /* (char *)ssave(""); */
	sList->why	= var_shared;
	sList->def_before = AST_NIL;      /* default for all refs */
	sList->use_after= AST_NIL;      /*    mpal, 910507      */
	sList->user	= false;
	sList->cblock	= NULL; /* (char *)ssave(""); */
	sList->dim	= 0;
	sList->next	= NULL;

	return (sList);
}


/*-----------------------------------------------------------------------

	li_init_Slist()	Initialize variable for shared/private lists

*/
void
li_init_Slist(Slist* sList, char* name, char* block, int dim)
{
	sList->name	= (char *)ssave(name);
	sList->why	= var_shared;
	sList->def_before = AST_NIL;      /* default for all refs */
	sList->use_after	= AST_NIL;      /*    mpal, 910507      */
	sList->user	= false;
	sList->cblock	= (char *)ssave(block);
	sList->dim	= dim;
	sList->next	= NULL;
}


/*-----------------------------------------------------------------------

	field access function for Loop_info
*/
int	
loop_info_loopLevel(Loop_info* lptr)
{
	return	(lptr->loop_level);
}


/********************************************
   Miscellaneous routines.  -Vas, May 1988.
 ********************************************/


/*****************
li_cur_loop()	returns the current loop selected in the LI_Instance
*****************/
Loop_info *
li_cur_loop(LI_Instance* LI)
{
    return (LI->cur_loop);
}

/*****************
el_cflow()	returns the control flow type of the 
	current loop.  This is one of the following.
	NOFLOW < STRUCT < UNSTRUCT < BACK_BRANCH
*****************/
int
el_cflow(LI_Instance* LI)
{
    return (LI->cur_loop->cflow);
}

/*****************
li_set_cflow()	sets the control flow type of the 
	current loop.  This is one of the following.
	NOFLOW < STRUCT < UNSTRUCT < BACK_BRANCH
*****************/
void
li_set_cflow(LI_Instance* LI, int cflow )
{
  LI->cur_loop->cflow	= cflow;
}

/*****************
li_max_cflow() 
  sets the control flow equal to the new_cflow if the new_cflow
  is more restrictive than the old_cflow. The order of cflow restrictiveness
  is :  NOFLOW < STRUCT < UNSTRUCT < BACK_BRANCH
*****************/
void
li_max_cflow(LI_Instance* li, int new_cflow)
{
    switch (li->cur_loop->cflow)
      { case NOFLOW : 
	  li->cur_loop->cflow = new_cflow;
	  break;
	case STRUCT :
	  if (!(new_cflow == NOFLOW))
	    li->cur_loop->cflow = new_cflow;	    
	  break;
	case UNSTRUCT :
	  if (new_cflow == BACK_BRANCH)
	    li->cur_loop->cflow = new_cflow;	    	    
	  break;
	case BACK_BRANCH:
	  break;
  }
}



/*
 * find_loop(): find the Loop_info node that contains info for this loop,
 * and return a ptr to it. Return NULL if such a node is not found.
 */

Loop_info *
find_loop(LI_Instance* LI, AST_INDEX lindex)
{
    Loop_info   *ptr;
    
    for(ptr=LI->Linfo; ptr!=NULL; ptr=ptr->next){
	if(ptr->loop_hdr_index == lindex)
	    return(ptr);
    }
    return(NULL);
}

/*
 * already_exists(): return true if name already exists in the specified
 * list. "list" must be one of the shared or private lists of a loop.
 */

Boolean
already_exists(Slist* list, char* name)
{
    Slist	*q;
    
    if(list == NULL)
	return(false);
    
    for(q = list; q!=NULL; q=q->next) {
	if(strcmp(q->name, name) == 0)
	    return(true);
    }
    
    return(false);
}

/*---------------------------------------------------------------------------
  check_if_shared() - check if the variable "s" must be in shared
  storage. If yes, return a ptr to the link in the Slist where info about 
  this var can be found.
 -------------------------------------------------------------------------- */
Boolean
check_if_shared(LI_Instance* LI, DepType dtype, char* s, Slist** Q)
{
    int		 rc = -1;
    Slist       *p;
    
    if (dtype==dg_call || dtype==dg_exit || 
		dtype==dg_io || dtype == dg_control) 
        return(true);

    if (dtype==dg_unknown) 
		return(false);   

    if (LI->cur_loop == NULL) 
		return(false);

    for (p = LI->cur_loop->shvar_list; p != NULL; p = p->next)
	{  
		if(!strcmp(p->name, s))	/* compare s with p->name */
		{
			*Q = p;
			return(true);
		}
	}
    return(false);
}

/*
 *  el_get_loop_info(): set LI->cur_loop to the node in the Loop_info
 *  list where info for this loop is kept. Return false if such a node cannot 
 *  be found. Else return true.
 */

Boolean
el_get_loop_info(LI_Instance* LI, AST_INDEX loop_hdr)
{
    Loop_info   *ptr;
    
    ptr = find_loop(LI, loop_hdr);
    if(ptr == NULL)
	return(false);
    else {
	LI->cur_loop = ptr;
	return(true);
    }
}

/*
 * dumpLinfo(): print out contents of all Loop_info nodes.
 * This routine is to be used for Debugging purposes only.
 */

static void
dumpLinfo(LI_Instance* LI)
{
    Slist *s;
    Slist *p;
    Loop_info   *lptr;
    int	i;
    
    if(LI->Linfo == NULL) 
	return;
    
    i = 0;
    
    for(lptr = LI->Linfo; lptr != NULL; lptr = lptr->next) {
	i++;
	printf("Loop %d (%d), level %d\n", i, lptr->loop_hdr_index, 
               lptr->loop_level);
	
	printf("  shared: ");
	for(s=lptr->shvar_list; s!=NULL; s=s->next)
	    printf("%s  ", s->name);
	
	printf("\n  private: ");
	for(p=lptr->pvar_list; p!=NULL; p=p->next)
	    printf("%s ", p->name);
	
	printf("\n");
    }
}


/********************************************************************
  Routines to add and delete entire loops.
     - Vas, May 1988.
 ********************************************************************/


/*
 * el_add_loop(): add a new Loop info node. The node is added immediately
 * following the Loop info node corresponding to the "prev_loop" loop. If
 * "prev_loop" is AST_NIL, the new loop is added at the head of the Linfo
 * list.  Return a pointer to this node, cast as a Generic.
 */

Loop_info *
el_add_loop(LI_Instance* LI, AST_INDEX prev_loop, AST_INDEX lindex, int level)
  /* lindex -  ast index of new loop */
  /* level -  loop level of new loop */
{
    Loop_info	*p, *ptr;
    
    p = find_loop(LI, prev_loop);
    
    /* insert new loop info node */
    ptr = (Loop_info *) get_mem(sizeof(Loop_info), "el_add_loop");
    ptr->loop_hdr_index 	= lindex;
    ptr->loop_level     	= level;
    ptr->parallelized    = false;
    ptr->shvar_list	= NULL;
    ptr->pvar_list	= NULL;
    ptr->prev	       	= p;
    
    if(p == NULL) {
	if(LI->Linfo == NULL) {
	    ptr->next = NULL;
	    LI->Linfo = ptr;
	}
	else {
	    ptr->next = LI->Linfo;
	    LI->Linfo->prev = ptr;
	    LI->Linfo = ptr;
	}
    }
    else {
	ptr->next	= p->next;
	if(p->next != NULL)
	    p->next->prev = ptr;
	p->next = ptr;
    }
    
    return((Loop_info *) ptr);

}

/*
 * el_remove_loop(): remove this loop from the Linfo list, and free its 
 * storage.
 */
void
el_remove_loop(LI_Instance* LI, AST_INDEX loop)
{
    Slist	*ptr, *saveptr;
    Loop_info	*q, *nextq, *prevq;
    
    q = find_loop(LI, loop);
    
    if(q == NULL) 
	return;
    prevq = q->prev;
    nextq = q->next;
    
    /* free this loop info node */
    ptr = q->shvar_list;
    while(ptr != NULL) {
	saveptr = ptr; 
	ptr = ptr->next;
	free_mem((void *)saveptr);
    }
    ptr = q->pvar_list;
    while(ptr != NULL) {
	saveptr = ptr; 
	ptr = ptr->next;
	free_mem((void *)saveptr);
    }
    free_mem((void *)q);
    
    if(prevq != NULL)
	prevq->next = nextq;
    else
	LI->Linfo = nextq;
    
    if(nextq != NULL)
	nextq->prev = prevq;
    
}

/*
 * el_parallelized(): return true if this loop has already been parallelized.
 */
Boolean
el_parallelized(LI_Instance* LI)
{
    if(LI->cur_loop == NULL) return(false);
    return(LI->cur_loop->parallelized);
}

/*
 * el_set parallelized_bit(): set parallelized bit to true.
 */
void
el_set_parallelized_bit(LI_Instance* LI)
{
    LI->cur_loop->parallelized = true;
}


/********************************************************************
  Routines to manipulate the shared and private variables in loops.
     - Vas, May 1988.
 ********************************************************************/


/* 
 * el_remove_shared_var(): remove var from shared var list of loop, and return 
 * a ptr to its Slist node cast as a Generic. 
 */
Generic  
el_remove_shared_var(LI_Instance* LI, AST_INDEX loop, char* var)
{
    Loop_info	*ptr;
    Slist	*s, *saves;
    
    ptr = find_loop(LI, loop);
    if(ptr == NULL || strlen(var) == 0) 
	return((Generic) NULL);
    
    /* find the Slist node that corresponds to this var */   
    for(s = ptr->shvar_list; s != NULL; s = s->next) {
	if(strcmp(var, s->name) == 0) break;
	saves = s;
    }
    
    if(s == NULL) return((Generic) NULL);
    
    /* else remove this node */
    if(ptr->shvar_list == s) {
	ptr->shvar_list = s->next;
	s->next = NULL;
	return((Generic) s);
    }
    else {
	saves->next = s->next;
	s->next = NULL;
	return((Generic) s);
    }
}


/* 
 * el_add_shared_var(): add Slist node to shared list of loop, if node->name
 * does not already exist in the list.
 */

void 
el_add_shared_var(LI_Instance* LI, AST_INDEX loop, Slist* s)
{
    Loop_info	*q;
    Slist	*ptr;
    
    q = find_loop(LI, loop);
    
    if(q == NULL || s == NULL || already_exists(q->shvar_list, s->name)) 
	return;
    
    /* get to end of shvar_list, then insert new node */
    ptr=q->shvar_list; 
    if(ptr == NULL) {
	q->shvar_list = s;
	s->next = NULL;
	return;
    }
    while(ptr->next != NULL) 
	ptr=ptr->next;
    
    ptr->next = s;
    s->next = NULL;
}


/* 
 * el_force_add_shared_var(): add Slist node to shared list of loop.
 */

void 
el_force_add_shared_var(LI_Instance*  LI, AST_INDEX loop, Slist* s)
{
    Loop_info	*q;
    Slist	*ptr;
    
    q = find_loop(LI, loop);
    
    if(q == NULL || s == NULL || already_exists(q->shvar_list, s->name)) 
	return;
    
    /* get to end of shvar_list, then insert new node */
    ptr=q->shvar_list; 
    if(ptr == NULL) {
	q->shvar_list = s;
	s->next = NULL;
	return;
    }
    while(ptr->next != NULL) 
	ptr=ptr->next;
    
    ptr->next = s;
    s->next = NULL;
}

/* 
 * el_remove_private_var(): remove var from private var list of loop, and 
 * return a ptr to its Slist node cast as a Generic. 
 * 
 * Modified Oct. 2, 1988 to remove var from PRIVATE stmt list if this
 * loop is a PARALLEL LOOP. -vas
 */

Generic  
el_remove_private_var(LI_Instance* LI, AST_INDEX loop, char* var)
{
    Loop_info	*ptr;
    Slist	*p, *savep;
    AST_INDEX	pvar, plist, stmt;
    int		rc;
    
    ptr = find_loop(LI, loop);
    if(ptr == NULL || strlen(var) == 0) 
	return((Generic) NULL);
    
    /* find the Slist node that corresponds to this var */   
    for(p = ptr->pvar_list; p != NULL; p = p->next) {
	if(strcmp(var, p->name) == 0) break;
	savep = p;
    }
    
    if(p == NULL) return((Generic) NULL);
    
    /* else remove this node */
    if(ptr->pvar_list == p) {
	ptr->pvar_list = p->next;
	p->next = NULL;
    }
    else {
	savep->next = p->next;
	p->next = NULL;
    }
    
    /* if this is a PARALLEL LOOP, update its PRIVATE stmt */
    
    if (is_parallelloop(loop)) {
	/* update its PRIVATE stmt */
	stmt = list_first(gen_PARALLELLOOP_get_stmt_LIST(loop));
	if(is_private(stmt)) {
	    plist = gen_PRIVATE_get_name_LIST(stmt);
	    for(pvar=list_first(plist); pvar!=AST_NIL; pvar=list_next(pvar)) {
		rc = strcmp(gen_get_text(pvar), var);
		if(rc == 0) {
		    list_remove_node(pvar);
		}
	    }
	    /* are there any private vars left ? */
	    if(ptr->pvar_list == NULL) {
		/* remove the PRIVATE stmt */
		list_remove_node(stmt);
	    }
	}
    }
    return((Generic)p);
}

/* 
 * el_add_private_var(): add Slist node to private list of loop unless 
 * node->name  already exists in the list.
 * 
 * Modified Oct. 2, 1988 to add var to PRIVATE stmt list if this
 * loop is a PARALLEL LOOP. -vas
 */

void 
el_add_private_var(LI_Instance* LI, AST_INDEX loop, Slist* p)
{
   Loop_info	*q;
   Slist	*ptr;
   AST_INDEX	pvar, plist, stmt;
   
   q = find_loop(LI, loop);
   if(q == NULL || p == NULL || already_exists(q->pvar_list, p->name)) 
      return;
   
   /* get to end of pvar_list, then insert new node */
   ptr = q->pvar_list; 
   if(ptr == NULL) {
       q->pvar_list = p;
       p->next = NULL;
   }
   else {
       while(ptr->next != NULL) 
	   ptr=ptr->next;
       ptr->next = p;
       p->next = NULL;
   }
   
   /* if this is a PARALLEL LOOP, update its PRIVATE stmt */
   
   if (is_parallelloop(loop)) {
       /* update its PRIVATE stmt */
       stmt = list_first(gen_PARALLELLOOP_get_stmt_LIST(loop));
       pvar = gen_IDENTIFIER();
       gen_put_text(pvar, p->name, STR_IDENTIFIER);
       
       if(is_private(stmt)) {
	   plist = gen_PRIVATE_get_name_LIST(stmt);
	   list_insert_last(plist, pvar);
       }
       else {
	   /* Need to generate a new PRIVATE stmt */
	   plist = list_create(AST_NIL);
	   list_insert_last(plist, pvar);
	   stmt = gen_PRIVATE(AST_NIL, plist);
	   list_insert_first(gen_PARALLELLOOP_get_stmt_LIST(loop), stmt);
       }
   }
}

/* 
 * el_add_private_up(): add var to private list of this loop and all outer loops
 * unless var  already exists in the list.
 * 
 */
void 
el_add_private_up(LI_Instance* LI, AST_INDEX loop, char* var)
{
    Slist	*varnode;
    
    while (loop != AST_NIL)
    {
	if (is_loop(loop))
	{
	    varnode = el_create_new_node(var,AST_NIL,AST_NIL,var_shared,"",0);
	    el_add_private_var(LI, loop, varnode);
	}
	loop = out(loop);
    }
}

/*
 * el_create_new_node(): create a new Slist node, and put the specified
 * info in it. Return a ptr to it, cast as a Generic. This routine should
 * be called if a new variable (i.e. one that is NOT already in the shared
 * or private lists of the loop) is to be added to one of these lists.
 * The ptr returned by this routine can then be passed to el_add_shared_var
 * or el_add_private_var to insert the var into one of these lists.
 */

Slist	*
el_create_new_node(char* name, AST_INDEX def_before, AST_INDEX use_after, 
                   VarType why, char* cblock, int dim)
{
    Slist	*s;
    
    s = (Slist *) get_mem(sizeof(Slist), "el_create_new_node");
    
    s->name	 = (char *)ssave (name);
    s->def_before = def_before;
    s->use_after  = use_after;
    s->why	 = why;
    s->cblock	 = (char *)ssave (cblock);
    s->dim	 = dim;
    s->next 	 = NULL;
    
    return	s;
}


/*
 * el_change_private_var_name(): change the name of this private variable.
 * 	Used by strip mine.
 */

void
el_change_private_var_name(LI_Instance* LI, AST_INDEX loop, char* oldname, char* newname)
{
    Slist *pnode;
    
    pnode = (Slist *)el_remove_private_var(LI,loop,oldname);
    if (pnode != NULL)
    {	sfree(pnode->name);
	pnode->name = (char *)ssave(newname);
	el_add_private_var( LI, loop, pnode);
    }
}

/*
 * el_change_shared_var_name(): change the name of this shared variable.
 * This is called after a scalar variable has been expanded into an array.
 */

void 
el_change_shared_var_name(LI_Instance* LI, AST_INDEX loop, 
                          char* oldname, char* newname, 
                          AST_INDEX def_before, AST_INDEX use_after, int dims)
{
    Loop_info	*lptr;
    Slist	*p;
    char		*ovar, *nvar;
    
    lptr = find_loop(LI, loop);
    if(lptr==NULL)
	return;
    
    ovar = (char *)ssave (oldname);
    nvar = (char *)ssave (newname);
    
    if(already_exists(lptr->shvar_list, ovar))
    {
   	if(!(nvar == NULL) && !(strlen(nvar) == 0)) {
	    for(p = lptr->shvar_list; p != NULL; p = p->next) {
      		if(strcmp(p->name, ovar) == 0) {
		    /*need to free p->name */
		    p->name       = (char *)ssave (nvar);
		    p->def_before = def_before;
		    p->use_after  = use_after;
		    p->dim        = dims;
		    break;
		}
      	    }
   	}
    }
    sfree (ovar);
    sfree (nvar);
}

/*
 * el_flip_private (): when outer i loop is interchanged with inner j loop
 * add i to the private list of the i loop, and remove j from the private list 
 * of the i loop.
 * Used in loop interchange.
 */

void
el_flip_private (LI_Instance* LI, AST_INDEX new_outer, AST_INDEX new_inner)
{
    Loop_info 	*inner, *outer;
    Slist	*pnode;
    AST_INDEX	 ivar;
    char		*outer_name;
    
    outer = find_loop (LI, new_outer);
    inner = find_loop (LI, new_inner);
    
    if ((outer == NULL) || (inner == NULL))
	return;
    
    /* remove the outer loop's index variable from the inner private list*/
    ivar = gen_DO_get_control (outer->loop_hdr_index);
    ivar = gen_INDUCTIVE_get_name(ivar);
    outer_name = (char *)ssave (gen_get_text(ivar));
    pnode = (Slist *)el_remove_private_var (LI, new_inner, outer_name);
    if (pnode == NULL)
	return;
    
    /* add the inner loop's index variable to its private list */
    ivar = gen_DO_get_control (inner->loop_hdr_index);
    ivar = gen_INDUCTIVE_get_name(ivar);
    sfree(pnode->name);
    pnode->name = (char *)ssave (gen_get_text(ivar));
    el_add_private_var (LI, new_inner, pnode);	
    sfree (outer_name);
}


/*
 * el_copy_shared_list(): copy the shvar_list from loop n1 to loop n2.
 * If loop n2 already has a shvar_list, the shared vars of n1 will
 * be UNIONed with the shared vars of n2.
 */

void
el_copy_shared_list(LI_Instance* LI, AST_INDEX n1, AST_INDEX n2)
{
    Loop_info	*l1, *l2;
    Slist	*p;
    Slist	*q;
    
    l1 = find_loop(LI, n1);
    l2 = find_loop(LI, n2);
    
    if(l1 == NULL || l2 == NULL) 
	return; 	/* could not find one or both loops */
    
    for(p=l1->shvar_list; p!=NULL; p=p->next) {
	if(!already_exists(l2->shvar_list, p->name)) {
	    q = el_create_new_node(p->name, p->def_before, p->use_after, 
				   p->why, p->cblock, p->dim);
	    el_add_shared_var(LI, n2, q);
	}
    }
}

/*
 * el_copy_private_list(): copy the pvar_list from loop n1 to loop n2.
 * If loop n2 already has a pvar_list, the private vars of n1 will
 * be UNIONed with the private vars of n2.
 */

void
el_copy_private_list(LI_Instance* LI, AST_INDEX n1, AST_INDEX n2)
{
    Loop_info	*l1, *l2;
    Slist	*p;
    Slist	*q;
    
    l1 = find_loop(LI, n1);
    l2 = find_loop(LI, n2);
    
    if(l1 == NULL || l2 == NULL) 
	return; 	/* could not find one or both loops */
    
    for(p=l1->pvar_list; p!=NULL; p=p->next) {
	if(!already_exists(l2->pvar_list, p->name)) {
	    q = el_create_new_node(p->name, p->def_before, p->use_after, 
				   p->why, p->cblock, p->dim);
	    el_add_private_var(LI, n2, q);
	}
    }
} /* end_el_copy_private_list */


/* 
 * el_get_shared_list():  return list of shared vars in a string.
 */

char *
el_get_shared_list(LI_Instance* LI)
{
    Slist       *curr;
    static char	str[2000];
    int		i = 0;
    
    str[0] = '\0';
    
    if (!LI->cur_loop)
	return str;
    
    curr = LI->cur_loop->shvar_list;
    
    while (curr != NULL)
    {
        if(strlen(str) > 0)
	    strcat(str, ", ");
	
	strcat(str, curr->name);
	
	curr = curr->next;
    }  
    
    /* kluge to remove mysterious trailing comma that appears only
       for nested loops */
    i = strlen(str) - 1;
    while(i >= 0 && str[i] == ' ')
	i--;
    if(str[i] == ',')
	str[i] = '\0';
    
    return   (str);
}

/* 
 * el_get_shared_info(): return a string that gives info about why the
 * specified variable must be shared.
 */

char *
el_get_shared_info(LI_Instance* LI, char* var, Generic handle, GetTextCallback get_text)
{
    Slist       *curr;
    static char	str[1000];
    char	cvar[30];
    int		rc;
    
    str[0] = '\0';
    
    if (!LI->cur_loop)
	return str;
    
    curr = LI->cur_loop->shvar_list;
    if(var == NULL) return(str);
    
    strcpy(cvar, var);
    
    while (curr != 0)
    {
	rc = strcmp(cvar, curr->name);
	if(rc == 0) {
	    strcat(str, curr->name);
	    if(curr->def_before != AST_NIL) {
		strcat(str, "  is defined before the loop at stmt:\n  ");
		strcat(str, get_text(handle, curr->def_before));
		strcat(str, "\n");
	    }    
	    if(curr->use_after != AST_NIL) {
		if(curr->def_before == AST_NIL) {
		    strcat(str, "  is used after the loop at stmt:\n  ");
		    strcat(str, get_text(handle, curr->use_after));
		    strcat(str, "\n");
		}
		else {
		    strcat(str, "and used after the loop at stmt:\n  ");
		    strcat(str, get_text(handle, curr->use_after));
		    strcat(str, "\n");
		}
	    }    
	    if(strcmp(curr->cblock, "_blank") != 0
	       && strcmp(curr->cblock, "_local_") != 0) {
		strcat(str, "  is declared in the common block  ");
		strcat(str, curr->cblock);
	    }
	    return(str);
	}
	curr = curr->next;
    }  
    strcat(str, "No info available on  ");
    strcat(str, var);
    return   (str);
}

char *
el_get_private_info(LI_Instance* LI, char* var, Generic handle, GetTextCallback get_text)
{
    Slist       *curr;
    static char	str[1000];
    char	cvar[30];
    int		rc;
    
    str[0] = '\0';
    
    if (!LI->cur_loop)
	return str;
    
    curr = LI->cur_loop->pvar_list;
    if(var == NULL) return(str);
    
    strcpy(cvar, var);
    
    while (curr != 0)
    {
	rc = strcmp(cvar, curr->name);
	if(rc == 0) {
	    strcat(str, curr->name);
	    if(curr->def_before != AST_NIL) {
		strcat(str, "  is defined before the loop at stmt:\n  ");
		strcat(str, get_text(handle, curr->def_before));
		strcat(str, "\n");
	    }    
	    if(curr->use_after != AST_NIL) {
		if(curr->def_before == AST_NIL) {
		    strcat(str, "  is used after the loop at stmt:\n  ");
		    strcat(str, get_text(handle, curr->use_after));
		    strcat(str, "\n");
		}
		else {
		    strcat(str, "and used after the loop at stmt:\n  ");
		    strcat(str, get_text(handle, curr->use_after));
		    strcat(str, "\n");
		}
	    }    
	    if(strcmp(curr->cblock, "_blank") != 0
	       && strcmp(curr->cblock, "_local_") != 0) {
		strcat(str, "  is declared in the common block  ");
		strcat(str, curr->cblock);
	    }
	    return(str);
	}
	curr = curr->next;
    }  
    strcat(str, "No info available on  ");
    strcat(str, var);
    return   (str);
    
}
/* 
 * el_get_private_list():  return list of private vars in a string.
 */

char *
el_get_private_list(LI_Instance* LI)
{
    Slist       *curr;
    static char	str[2000];
    int		i = 0;
    
    str[0] = '\0';
    
    if (!LI->cur_loop)
	return str;
    
    curr = LI->cur_loop->pvar_list;
    
    while (curr != 0)
    {
        if(strlen(str) > 0)
	    strcat(str, ", ");
	
	strcat(str, curr->name);
	curr = curr->next;
    }
    
    /* kluge to remove mysterious trailing comma that appears only
       for nested loops */
    i = strlen(str) - 1;
    while(i >= 0 && str[i] == ' ')
	i--;
    if(str[i] == ',')
	str[i] = '\0';
    
    return   (str);
}

/* 
 * el_get_num_shared_vars(): return the number of shared vars.
 */

int
el_get_num_shared_vars(LI_Instance* LI)
{
     Slist	*s;
     int	 i=0;
    
     if (!LI->cur_loop)
    	return (0);
    
    s = LI->cur_loop->shvar_list;
    if (s == NULL) 
	return(0);
    
    while(s != NULL) {
	i++;
	s = s->next;
    }
    
    return(i);
    
}

/* 
 * el_get_num_shared_vars(): return the number of shared vars.
 */

int
el_get_num_private_vars(LI_Instance* LI)
{
    Slist	*s;
    int		 i=0;
    
    if (!LI->cur_loop)
    	return (0);
    
    if ((s = LI->cur_loop->pvar_list) == NULL) 
	return(0);
    
    while(s != NULL) {
	i++;
	s = s->next;
    }
    
    return(i);
    
}

/* 
 * el_get_first_shared_node(): return a ptr to the Slist node that contains
 * info about the first shared var for the current loop. "length" is set
 * to the length of the shared var name. 
 */

Slist *
el_get_first_shared_node(LI_Instance* LI, int* length, int type)
{
    Slist	*s;
    
    if (!LI->cur_loop)
	return(NULL);
    
    s  = LI->cur_loop->shvar_list;
    if(s != NULL) 
    {
	*length = strlen(s->name);
   	if (type == ALL_SHARED)
	    return (s);
	if (type == NO_MOD_DEF)
	{/* variables in this catagory have no use after the loop,
	  * and are defined before the loop.
	  */
	    if ((s->def_before != AST_NIL) && (s->use_after == AST_NIL))
		return (s);		
	}
	if (type == MAY_MOD)
	{/* variables of this catagory are defined before and used after the loop*/
	    if ((s->def_before != AST_NIL) && (s->use_after == AST_NIL))
		return (s);	
	}
	if (type == MAY_USE)
	{/* variables of this catagory are defined before and used after the loop*/
	    if ((s->def_before != AST_NIL) && (s->use_after != AST_NIL))
		return (s);	
	}
	if (type == COMMON)
	{/*variables of this catagory have something other than _blank
	  * and _local_ in their cblock
	  */
	    if(strcmp(s->cblock, "_blank") != 0 && strcmp(s->cblock, "_local_") != 0) 	
		return (s);
	}
	if (type == USER_SPEC_SHAR)
	{
	    if (s->user)
	   	return s;
	}
	return (el_get_next_shared_node (LI, s, length, type));
    }
    return(s);
}

/* 
 * el_get_next_shared_node(): return a ptr to the next Slist node. The routine
 * "el_get_first_node" must be called before this one. "s" is a ptr to the
 * current Slist node. "length" is set to the length of the shared var name. 
 */

Slist *
el_get_next_shared_node(LI_Instance*  LI, Slist* s, int* length, int type)
{
    if(LI->cur_loop == NULL) return(NULL);
    if(s == NULL) return(NULL);
    
    for (s = s->next; s != NULL; s = s->next)
    {
	*length = strlen(s->name);
   	if(type == ALL_SHARED)
	    break;
	if (type == NO_MOD_DEF)
	{/* variables in this catagory have no use after the loop,
	  * and are defined before the loop.
	  */
	    if ((s->def_before != AST_NIL) && (s->use_after == AST_NIL))
		break;		
	}
	if (type == MAY_MOD)
	{/* variables of this catagory are defined before and used after the loop*/
	    if ((s->def_before != AST_NIL) && (s->use_after == AST_NIL))
		break;	
	}
	if (type == MAY_USE)
	{/* variables of this catagory are defined before and used after the loop*/
	    if ((s->def_before != AST_NIL) && (s->use_after != AST_NIL))
		return (s);	
	}
	if (type == COMMON)
	{/*variables of this catagory have something other than _blank
	  * and _local_ in their cblock
	  */
	    if(strcmp(s->cblock, "_blank") != 0 && strcmp(s->cblock, "_local_") != 0) 	
	      	break;
	}
	if (type == USER_SPEC_SHAR)
	{
	    if (s->user)
	   	break;
	}
    }
    return(s);
}

/*--------------------------------------------------------------------

  is_private_var()  Whether variable is private with respect to loop

*/

Boolean
is_private_var(LI_Instance* LI, AST_INDEX var, AST_INDEX loop)
{
  Loop_info *li;
  Slist *p;
  char *name;

  if (is_subscript(var))
    var = gen_SUBSCRIPT_get_name(var);

  li = find_loop(LI, loop);
  if (li != (Loop_info *) NO_LEVELV)
  {
    name = gen_get_text(var);
    for (p = li->pvar_list; p != NULL; p = p->next)
    {
      if (!strcmp(name, p->name))
        return true;
    }
  }

  return false;
}


/* 
 * el_get_first_private_node(): return a ptr to the Slist node that contains
 * info about the first private var for the current loop. "length" is set
 * to the length of the private var name.
 */

Slist *
el_get_first_private_node(LI_Instance*  LI, int* length, int type)
{
    Slist	*p;
    
    if(LI->cur_loop == NULL) return(NULL);
    p  = LI->cur_loop->pvar_list;
    if(p != NULL) 
    { 
	*length = strlen(p->name);
	if (type != USER_SPEC_PRIV)
	{
	    return (p);
	}
	else /* type == USER_SPEC_PRIV */
	{
	    if (p->user)
		return (p);
	}
	
	return (el_get_next_private_node(LI, p, length, type));
    }   
    
    return(p);
}

/* 
 * el_get_next_private_node(): return a ptr to the next Slist node. The routine
 * "el_get_first_private_node" must be called before this one. "p" is a ptr
 * to the current Slist node. "length" is set to the length of the shared 
 * var name. 
 */

Slist *
el_get_next_private_node(LI_Instance*  LI, Slist* p, int* length, int type)
{
    if(LI->cur_loop == NULL) return(NULL);
    if(p == NULL) return(NULL);
    for (p = p->next; p != NULL; p = p->next)
    {
	*length = strlen(p->name);
	if (type != USER_SPEC_PRIV)
	    break;
	if (type == USER_SPEC_PRIV)
	{
	    if (p->user)
		break;
	}
    }
    return(p);
}


/*------------------------------------------------------------------

	li_free()		Free LI 

*/

void
li_free(LI_Instance* LI)
{
    Loop_info   *ptr;
    
    /* start from head of list, let el_remove_loop() do work 	*/
    
    while ((ptr = LI->Linfo) != NULL)
	el_remove_loop(LI, ptr->loop_hdr_index);
    
    free_mem((void *)LI);
}
