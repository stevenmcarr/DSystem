/* $Id: dt_info.C,v 1.1 1997/06/25 15:08:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt_info.c		Handle info structures for dependence tests

	Don't bother freeing individual elements, 
	free everything	at the end of the session.

	History
	~~~~~~~
	25 May 91  cwt  Created

*/

#include <string.h>
#include <memory.h>

#include <include/bstring.h>

#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/rsd.h>
#include <libs/support/memMgmt/mem.h>

/*--------------------- local definitions ---------------------*/
		/* initial # of Loop_list structs   */
#define LOOP_BLOCK_SIZE 100
		/* initial # of Subs_list structs   */
#define REF_BLOCK_SIZE 2000
		/* initial # of Rsd_section structs */
#define RSD_BLOCK_SIZE 2000
		/* initial # of Rsd_vector structs  */
#define RVEC_BLOCK_SIZE 2000
		/* initial # of dt_str characters   */
#define STR_BLOCK_SIZE 4000





/*-----------------------------------------------------------------------

	dt_create_info()	Allocate dependence test data structures 

	Leave 0th element free

*/

DT_info *	dt_create_info( )
{
	DT_info *dt;

	dt = (DT_info *) get_mem(sizeof(DT_info), "dt");

	dt->loop_free = 0;
	dt->loop_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
	dt->loop_mem->buf = 
			(char *) get_mem(sizeof(Loop_list) * LOOP_BLOCK_SIZE, "dt");
	dt->loop_mem->next = NULL;

	dt->ref_free = 0;
	dt->ref_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
	dt->ref_mem->buf = 
			(char *) get_mem(sizeof(Subs_list) * REF_BLOCK_SIZE, "dt");
	dt->ref_mem->next = NULL;

	dt->rsd_free = 0;
	dt->rsd_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
	dt->rsd_mem->buf = 
			(char *) get_mem(sizeof(Rsd_section) * RSD_BLOCK_SIZE, "dt");
	dt->rsd_mem->next = NULL;

	dt->rvec_free = 0;
	dt->rvec_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
	dt->rvec_mem->buf = 
			(char *) get_mem(sizeof(Rsd_vector) * RVEC_BLOCK_SIZE, "dt");
	dt->rvec_mem->next = NULL;

	dt->str_free = 0;
	dt->str_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
	dt->str_mem->buf = 
			(char *) get_mem(sizeof(char) * STR_BLOCK_SIZE, "dt");
	dt->str_mem->next = NULL;

	return	dt;
}




/*-----------------------------------------------------------------------

	dt_finalize_info()	Free dependence test data structures 

*/

void dt_finalize_info(DT_info *dt)
{
	Mem_list *mem;

	while (dt->loop_mem)	/* free loops	*/
	{
		mem = dt->loop_mem->next;
		free_mem((void *)dt->loop_mem->buf);
		free_mem((void *)dt->loop_mem);
		dt->loop_mem = mem;
	}

	while (dt->ref_mem)		/* free refs	*/
	{
		mem = dt->ref_mem->next;
		free_mem((void *)dt->ref_mem->buf);
		free_mem((void *)dt->ref_mem);
		dt->ref_mem = mem;
	}

	while (dt->rsd_mem)		/* free rsds	*/
	{
		mem = dt->rsd_mem->next;
		free_mem((void *)dt->rsd_mem->buf);
		free_mem((void *)dt->rsd_mem);
		dt->rsd_mem = mem;
	}

	while (dt->rvec_mem)		/* free rsd vectors	*/
	{
		mem = dt->rvec_mem->next;
		free_mem((void *)dt->rvec_mem->buf);
		free_mem((void *)dt->rvec_mem);
		dt->rvec_mem = mem;
	}

	while (dt->str_mem)		/* free dt_str	*/
	{
		mem = dt->str_mem->next;
		free_mem((void *)dt->str_mem->buf);
		free_mem((void *)dt->str_mem);
		dt->str_mem = mem;
	}

	free_mem((void *)dt);
}


/*-----------------------------------------------------------------------

	dt_reset_info()		Reset dependence test data structures 
						Equivalent to free & recreate data structs 
*/

void dt_reset_info(DT_info *dt)
{

	dt_finalize_info( dt );
	dt_create_info(/* dt */);
}



/*-----------------------------------------------------------------------

	Routine to allocate loop info 

*/

Loop_list *
dt_alloc_loop(DT_info *dt)
{
	Loop_list *loop;
	Mem_list *old_mem;

	/* if array is full, add another block */

	if (dt->loop_free >= LOOP_BLOCK_SIZE)
	{
		dt->loop_free = 0;
		old_mem = dt->loop_mem;
		dt->loop_mem = (Mem_list *)	get_mem(sizeof(Mem_list), "dt");
		dt->loop_mem->buf = 
			(char *) get_mem(sizeof(Loop_list) * LOOP_BLOCK_SIZE, "dt");
	        bzero(dt->loop_mem->buf, sizeof(Loop_list) * LOOP_BLOCK_SIZE);
		dt->loop_mem->next = old_mem;
	}

	loop = (Loop_list *) dt->loop_mem->buf;

	return loop + dt->loop_free++;	/* point to internal element	*/
}



/*-----------------------------------------------------------------------

	Routine to allocate reference info 

*/

Subs_list *
dt_alloc_ref(DT_info *dt)
{
	Subs_list *ref;
	Mem_list *old_mem;

	/* if array is full, add another block */

	if (dt->ref_free >= REF_BLOCK_SIZE)
	{
		dt->ref_free = 0;
		old_mem = dt->ref_mem;
		dt->ref_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
		dt->ref_mem->buf = 
			(char *) get_mem(sizeof(Subs_list) * REF_BLOCK_SIZE, "dt");
	        bzero(dt->ref_mem->buf, sizeof(Subs_list) * REF_BLOCK_SIZE);
		dt->ref_mem->next = old_mem;
	}

	ref = (Subs_list *) dt->ref_mem->buf;

	return ref + dt->ref_free++;	/* point to internal element	*/
}



/*-----------------------------------------------------------------------

	Routine to allocate rsd info 

*/

Rsd_section *
dt_alloc_rsd(DT_info *dt)
{
	Rsd_section *rsd;
	Mem_list *old_mem;

	/* if array is full, add another block */

	if (dt->rsd_free >= RSD_BLOCK_SIZE)
	{
		dt->rsd_free = 0;
		old_mem = dt->rsd_mem;
		dt->rsd_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
		dt->rsd_mem->buf = 
			(char *) get_mem(sizeof(Rsd_section) * RSD_BLOCK_SIZE, "dt");
	        bzero(dt->rsd_mem->buf, sizeof(Rsd_section) * RSD_BLOCK_SIZE);
		dt->rsd_mem->next = old_mem;
	}

	rsd = (Rsd_section *) dt->rsd_mem->buf;

	return rsd + dt->rsd_free++;	/* point to internal element	*/
}



/*-----------------------------------------------------------------------

	Routine to allocate rsd vector info 

*/

Rsd_vector *
dt_alloc_rsd_vector(DT_info *dt)
{
	Rsd_vector	*rvec;
	Mem_list	*old_mem;

	/* if array is full, add another block */

	if (dt->rvec_free >= RVEC_BLOCK_SIZE)
	{
		dt->rvec_free = 0;
		old_mem = dt->rvec_mem;
		dt->rvec_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
		dt->rvec_mem->buf = 
			(char *) get_mem(sizeof(Rsd_vector) * RVEC_BLOCK_SIZE, "dt");
		bzero(dt->rvec_mem->buf, sizeof(Rsd_vector) * RVEC_BLOCK_SIZE);
		dt->rvec_mem->next = old_mem;
	}

	rvec = (Rsd_vector *) dt->rvec_mem->buf;

	return rvec + dt->rvec_free++;	/* point to internal element	*/
}


/*-----------------------------------------------------------------------

	Routine to allocate dt_str info 

*/

char *
dt_ssave(char *str, DT_info *dt)
{
	Mem_list *old_mem;
	int len;
	char *newstr;

	/* if string array is full, add another block */

	len = strlen(str) + 1;
	if (dt->str_free + len >= STR_BLOCK_SIZE)
	{
		dt->str_free = 0;
		old_mem = dt->str_mem;
		dt->str_mem = (Mem_list *) get_mem(sizeof(Mem_list), "dt");
		dt->str_mem->buf = 
			(char *) get_mem(sizeof(char) * STR_BLOCK_SIZE, "dt");
	        bzero(dt->str_mem->buf, sizeof(char) * STR_BLOCK_SIZE);
		dt->str_mem->next = old_mem;
	}

	newstr = dt->str_mem->buf + dt->str_free;
	dt->str_free += len;
	memcpy(newstr, str, len);

	return newstr;	/* point to internal element	*/

}


/* eof */
