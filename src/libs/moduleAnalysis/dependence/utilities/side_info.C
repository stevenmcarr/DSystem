/* $Id: side_info.C,v 1.1 1997/06/25 15:10:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/utilities/side_info.c					*/
/*									*/
/*	Routines to handle the Info Array				*/
/*									*/
/*									*/
/************************************************************************/

/*	This File Defines the Following Functions for External Use:
 *
 * SideInfo *	create_side_info( FortTree ft );
 * void		destroy_side_info( FortTree ft, SideInfo * infoPtr );
 * void		clear_info_array( SideInfo * infoPtr );
 *
 * void		create_info( SideInfo *infoPtr, AST_INDEX astindex);
 		920802: only used by ped_cp/memory/uj/do_unroll.C:
 
 * void		dg_put_info( SideInfo * infoPtr, AST_INDEX astindex,
			Info_type infotype, Generic value );
 * Generic	dg_get_info( SideInfo * infoPtr, AST_INDEX astindex,
			Info_type infotype );
 *
 */

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>

#include <libs/moduleAnalysis/dependence/utilities/side_info.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>


static Generic     INFO_SIDE_ARRAY_INITIALS = {-1};



/****************************/
/* Info Side Array Routines */
/****************************/

SideInfo*
create_side_info(FortTree  ft)
{
	SideInfo	*infoPtr;
	int		 info_side_array;
	Info_elem	*info_array;
	int		 first_free;
	int		 count;

	infoPtr	= (SideInfo *) get_mem(sizeof(SideInfo), "SideInfo structure");

	/*--------------------------------------*/
	/* create and attach side array to AST	*/
	
	info_side_array = 
	  ft_AttachSideArray(ft, 1, &INFO_SIDE_ARRAY_INITIALS);
	
	/*----------------------------------------------*/
	/* allocate space for the information array  */
	
	info_array	= (Info_elem *) xalloc(INFO_ARRAY_BLOCK_SIZE, sizeof(Info_elem));
	
	/* mark info array as unused */
	for (count=0; count < INFO_ARRAY_BLOCK_SIZE; count++)
	  {
	    info_array[count].free_list       = count+1;
	    info_array[count].levelv_index    = NO_LEVELV;
	    info_array[count].subscript_index = NO_SUBSCRIPT;
	    info_array[count].ref_index       = NO_REF;
	    info_array[count].dc_index        = NO_DC_INFO;
	    info_array[count].cd_map          = NO_CD_MAP;
	    info_array[count].fd_index        = NO_FD_INFO;
	    info_array[count].cfg_inst        = NO_CFG_INST;
	  };
	info_array[INFO_ARRAY_BLOCK_SIZE - 1].free_list = NO_MORE;
	
	SI_INFO_SIDE_ARRAY(infoPtr)	= info_side_array;
	SI_INFO_ARRAY(infoPtr)		= info_array;
	SI_INFO_ARRAY_FIRST_FREE(infoPtr)	= 1;	/* init first free pointer */

	return( infoPtr );
}    


void
destroy_side_info(FortTree ft, SideInfo* infoPtr )
{

	/*------------------------------*/
	/* detach side array from AST	*/
	
	ft_DetachSideArray( ft, SI_INFO_SIDE_ARRAY(infoPtr) );

	/*----------------------------------------------*/
	/* deallocate space for the information array	*/
	
	xfree( (int *)SI_INFO_ARRAY(infoPtr) );

	/*----------------------------------------------*/
	/* deallocate space for the SideArray struct	*/

	free_mem((char *) infoPtr );

} /* end_destroy_side_array */


/*------------------*/
/* clear info array	*/
void
clear_info_array(SideInfo* infoPtr)
{
	int		 info_elem_quantity;
	int		 count;
	Info_elem	*info_array;

	info_array	= SI_INFO_ARRAY(infoPtr);
	/* the following trick works because info_array is "xalloc"ed	*/
	info_elem_quantity = ((int *) info_array)[-1];

	for (count=0; count < info_elem_quantity; count++)
	  {
	    info_array[count].free_list       = count+1;
	    info_array[count].levelv_index    = NO_LEVELV;
	    info_array[count].subscript_index = NO_SUBSCRIPT;
	    info_array[count].ref_index       = NO_REF;
	    info_array[count].dc_index        = NO_DC_INFO;
	    info_array[count].cd_map          = NO_CD_MAP;
	    info_array[count].fd_index        = NO_FD_INFO;
	    info_array[count].cfg_inst        = NO_CFG_INST;
	  };
}


/*---------------------------------------------------------------------

	create_info()		create an entry in the info side array
				for astindex.  This does not check the
				current side array pointer, so if it
				is valid, it will be overwritten.

*/
void 
create_info(SideInfo* infoPtr, AST_INDEX astindex)
{
    Info_elem	        *info_array;
    int			info_array_index;
    int			count;
    int			info_elem_quantity;
   
    /* get pointer to the beginning of the info array */
    info_array = SI_INFO_ARRAY(infoPtr);
    
    /* get the first free element of the info array */
    info_array_index = SI_INFO_ARRAY_FIRST_FREE(infoPtr);
    
    /*-----------------------------------------------*/
    /* Enlarge info array by 50% if no free elements */
    
    if (info_array_index == NO_MORE)
    {
	/* get the number of elements of the info array */
	info_elem_quantity = ((Generic *) info_array)[-1];
	
	/* enlarge the info array by 50% */
	info_array = (Info_elem *)(xrealloc((int *)info_array,
					    ((info_elem_quantity*3)>>1)));
	
	SI_INFO_ARRAY(infoPtr) = info_array;
	
	/* make side array index first element of the enlarged info array */
	info_array_index = info_elem_quantity;
	
	/*------------------------------------------*/
	/* initialize the new section of info array */
	
	info_elem_quantity =((Generic *) info_array)[-1];	/* new size */
	
	for (count = info_array_index; count < info_elem_quantity; count++)
	{
	    info_array[count].free_list		= count+1;
	    info_array[count].levelv_index	= NO_LEVELV;
	    info_array[count].subscript_index	= NO_SUBSCRIPT;
	    info_array[count].ref_index		= NO_REF;
	    info_array[count].dc_index		= NO_DC_INFO;
	    info_array[count].fd_index		= NO_FD_INFO;
	    info_array[count].cfg_inst      = NO_CFG_INST;
	};
	
	info_array[info_elem_quantity - 1].free_list = NO_MORE;
    }
    
    /* store handle for info array in side array */
    ast_put_side_array (SI_INFO_SIDE_ARRAY(infoPtr), astindex, 0, info_array_index);
    
    /* make the next free element the new first free element */
    SI_INFO_ARRAY_FIRST_FREE(infoPtr) = info_array[info_array_index].free_list;
    
    /* mark the element where the value is going to go as used */
    info_array[info_array_index].free_list = USED;
}



/*---------------------------------------------------------------------

	dg_put_info()		Put information into info side array

	Puts info into one of the components of the info side array
	(e.g. the subscript or level vector array) as specified by 
	the type and the index provided

	NOTE:	currently no nodes are ever freed!
*/
void
dg_put_info(SideInfo* infoPtr, AST_INDEX astindex, Info_type infotype, Generic value)
{
    Info_elem	        *info_array;
    int			info_array_index;
    int			count;
    int			info_elem_quantity;
        
    /* get pointer to the beginning of the info array */
    info_array = SI_INFO_ARRAY(infoPtr);
    
    /* get index to the info array from the side array */
    info_array_index = ast_get_side_array (SI_INFO_SIDE_ARRAY(infoPtr), astindex, 0);
    
    /*----------------------------------------------------------*/
    /* if the node of the side array isn't indexing an			*/
    /* element of the info array, then allocate a new element	*/
    
    if (!info_array_index || (info_array_index == UNUSED))
    {
	/* get the first free element of the info array */
	info_array_index = SI_INFO_ARRAY_FIRST_FREE(infoPtr);
	
	/*-----------------------------------------------*/
	/* Enlarge info array by 50% if no free elements */
	
	if (info_array_index == NO_MORE)
	{
	    /* get the number of elements of the info array */
	    info_elem_quantity = ((Generic *) info_array)[-1];
	    
	    /* enlarge the info array by 50% */
	    info_array = (Info_elem *)(xrealloc((int *)info_array,
						((info_elem_quantity*3)>>1)));
	    
	    SI_INFO_ARRAY(infoPtr) = info_array;
	    
	    /* make side array index first element of the enlarged info array */
	    info_array_index = info_elem_quantity;
	    
	    /*------------------------------------------*/
	    /* initialize the new section of info array */
	    
	    info_elem_quantity =((Generic *) info_array)[-1];	/* new size */
	    
	    for (count = info_array_index; count < info_elem_quantity; count++)
	    {
		info_array[count].free_list        = count+1;
		info_array[count].levelv_index     = NO_LEVELV;
		info_array[count].subscript_index  = NO_SUBSCRIPT;
		info_array[count].ref_index        = NO_REF;
		info_array[count].dc_index         = NO_DC_INFO;
		info_array[count].fd_index         = NO_FD_INFO;
		info_array[count].cfg_inst         = NO_CFG_INST;
	    };
	    
	    info_array[info_elem_quantity - 1].free_list = NO_MORE;
	};
	
	/* store handle for info array in side array */
	ast_put_side_array (SI_INFO_SIDE_ARRAY(infoPtr), astindex, 0, info_array_index);
	
	/* make the next free element the new first free element */
	SI_INFO_ARRAY_FIRST_FREE(infoPtr) = info_array[info_array_index].free_list;
	
	/* mark the element where the value is going to go as used */
	info_array[info_array_index].free_list = USED;
    };
    
    /*----------------------------------------------------------------*/
    /* place the value in the appropriate component of the info array  */
    
    switch (infotype)
    {
    case (type_levelv):
	info_array[info_array_index].levelv_index = value;
	break;
    case (type_subscript):
	info_array[info_array_index].subscript_index = value;
	break;
    case (type_ref):
	info_array[info_array_index].ref_index = value;
	break;
    case (type_dc):
	info_array[info_array_index].dc_index = value;
	break;
    case (type_cd_map):
	info_array[info_array_index].cd_map = value;
	break;
    case (type_fd):
        info_array[info_array_index].fd_index = value;
	break;
    case (type_cfg_inst):
        info_array[info_array_index].cfg_inst = value;
	break;

    default: 
	die_with_message("put_info(): invalid info type %d", infotype);
	break;
    };
}



/*---------------------------------------------------------------------

  dg_get_info()		Get information from info side array

  Returns:	handle to one of the components of the info side array
                (e.g. the subscript or level vector array) as specified by 
		the type and the index to the side array given 

*/
Generic
dg_get_info(SideInfo* infoPtr, AST_INDEX astindex, Info_type infotype)
  /* infoPtr - info for side array access	*/
  /* astindex - index of AST & side array	*/
  /* infotype - type of info desired		*/
{
    Info_elem	        *info_array;
    Generic		info_array_index;
    
    
    /* get index to the info array from the side array */
    info_array_index = ast_get_side_array (SI_INFO_SIDE_ARRAY(infoPtr), astindex, 0);
    
    /* if the side array node is unused, then return unused to the caller */
    
    if (!info_array_index || (info_array_index == UNUSED))
    {
	switch (infotype)
	{
	case (type_levelv):
	    return(NO_LEVELV);
	case (type_subscript):
	    return(NO_SUBSCRIPT);
	case (type_ref):
	    return(NO_REF);
	case (type_dc):
	    return(NO_DC_INFO);
	case (type_cd_map):
	    return (NO_CD_MAP);
	case (type_fd):
	    return (NO_FD_INFO);
	case (type_cfg_inst):
	    return (NO_CFG_INST);
	break;
	};
    }
    
    /* else return the desired component of the info array   */
    
    else
    {
	/* get pointer to the beginning of the info array */
	info_array = SI_INFO_ARRAY(infoPtr);
	
	switch (infotype)
	{
	case (type_levelv):
	    return (info_array[info_array_index].levelv_index);
	case (type_subscript):
	    return (info_array[info_array_index].subscript_index);
	case (type_ref):
	    return (info_array[info_array_index].ref_index);
	case (type_dc):
	    return (info_array[info_array_index].dc_index);
	case (type_cd_map):
	    return (info_array[info_array_index].cd_map);
	case (type_fd):
	    return (info_array[info_array_index].fd_index);
	case (type_cfg_inst):
	    return (info_array[info_array_index].cfg_inst);
	break;
	};
    }
    
    die_with_message("get_info(): invalid info type %d", infotype);
    return(UNUSED);
}
