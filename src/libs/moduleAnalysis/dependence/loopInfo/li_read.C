/* $Id: li_read.C,v 1.1 1997/06/25 15:09:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/li/li_read.c					        */
/*									*/
/*	li_read.c -- reads the loop information and shared/private	*/
/*		variable designations from file				*/
/*									*/
/*	readindex()							*/
/*									*/
/*	Strings for Slist structures must be duplicated using ssave()	*/
/*	instead of dt_ssave() since the fields will be freed using	*/
/*	free_mem() during some transformations.	mpal:910712		*/
/*									*/
/************************************************************************/



/*--------------------------------------------------------------------------
  readindex() - Reads the index file to set up the loop and variable 
  structures for the LI structure.
  -------------------------------------------------------------------------- */

#include <string.h>
#include <ctype.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>

#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>
#include <libs/moduleAnalysis/dependence/utilities/read_util.h>


/*
 * This should be unnecessary 
 */
#define MAXLINE    10000
#define EDGE_NUM    2000		/* # of initial DG edges	*/


/*
 *  readindex() - read in the index file and store the shared variables info
 *  in internal data structure. 
 */

LI_Instance *
readindex(FILE* ifile_ptr, MapInfoOpaque map)
{
    int      	loop_hdr_line, loop_level;
    int      	pos, i;
    int  	def_before, use_after;
    Slist    	*sptr, *savesptr;
    Slist    	*savepptr;
    char     	foo[MAXLINE];
    char     	*buf;
    char     	*fields;
    LI_Instance	*LI;
    Loop_info	*Lptr, *saveLptr;
    
    LI = (LI_Instance *) get_mem(sizeof(LI_Instance), "readindex");
    LI->Linfo = NULL;
    LI->cur_loop = NULL;
    LI->num_loops  =  0;
    
    Lptr = NULL;
    
    while(fgets(foo, MAXLINE, ifile_ptr) != NULL) {
	/* check if the first field of foo is an integer */
	if(isdigit(foo[0])) {
	    /* this line contains loop hdr info */
	    (void) sscanf(foo, "%d %*d %d", &loop_hdr_line, &loop_level);
	    saveLptr = Lptr;
	    Lptr = (Loop_info *) get_mem(sizeof(Loop_info), "readindex");
	    
	    Lptr->loop_hdr_index = MapLineToIndex(map,loop_hdr_line);
	    Lptr->loop_level     = loop_level;
	    Lptr->parallelized   = false;
	    Lptr->shvar_list     = NULL;
	    Lptr->pvar_list      = NULL;
	    Lptr->next           = NULL;
	    Lptr->prev 		 = saveLptr;
	    
	    if (saveLptr == NULL) {
		LI->Linfo = Lptr;
		LI->cur_loop = NULL;
	    }
	    else saveLptr->next = Lptr;
	    
	    LI->num_loops++;
	}
	else {
	    /* this line contains variable info */
	    
	    /* convert into lower case for compatibility with NED */
	    for(i=0; i<strlen(foo); i++) {
		if(isalpha(foo[i]))
		    if(isupper(foo[i]))
			foo[i] = tolower(foo[i]);
	    }
	    
	    sptr = (Slist *) get_mem(sizeof(Slist), "readindex");
	    sptr->name = (char *)NULL;
	    pos	       = 0;
	    def_before = -1;
	    use_after  = -1;
	    
	    fields = foo;
	    fields = get_field_s(fields, &buf );		/* field  1 */
	    sptr->name = (char *)ssave( buf );			/* must be ssave, mpal:910711	*/
	    /* private and shared variable lists are modified using free_mem		*/
	    
	    fields = get_field_d(fields, &def_before );		/* field  2 */
	    if (def_before >= 0)
		sptr->def_before = MapLineToIndex(map,def_before);
	    else
		sptr->def_before = AST_NIL;
	    
	    fields = get_field_d(fields, &use_after );		/* field  3 */
	    if (use_after >= 0)
		sptr->use_after = MapLineToIndex(map,use_after);
	    else
		sptr->use_after = AST_NIL;
	    
	    fields = get_field_d(fields, (int *)&sptr->why );		/* field  4 */
	    sptr->user   = false;
	    fields = get_field_s(fields, &buf );		/* field  5 */
	    sptr->cblock = (char *)ssave( buf );		/* must be ssave, mpal:910711	*/
	    /* private and shared variable lists are modified using free_mem		*/
	    fields = get_field_d(fields, &sptr->dim );		/* field  6 */	    
	    
	    
	    sptr->next = NULL;
	    
	    /* which list to add this var to? */
	    
	    /* Check to see if this is a spurious var from PFC.
	       If it is, do not add it to the list.            */
	    
	    if (pfc_bogus(sptr->name))
	    {
		sfree((char *)sptr);
		continue;
	    }
	    
	    if(def_before == -1 && use_after == -1 
	       && strcmp(sptr->cblock, "_local_") == 0
	       && (sptr->dim == 0 || sptr->why == var_private) ) {
		/* this is a local variable, so add it to the
		   the pvar list */
		if(Lptr->pvar_list == NULL)
		    Lptr->pvar_list = sptr;
		else savepptr->next = sptr;
		
		savepptr = sptr;
	    }
	    else { 
		/* this is a shared variable, add it to 
		   the shvar list */
		if(Lptr->shvar_list == NULL)
		    Lptr->shvar_list = sptr;
		else savesptr->next = sptr;
		
		savesptr = sptr;
	    }
	}
    }
    
    return (LI);
}
