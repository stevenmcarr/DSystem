/* $Id: side_info.h,v 1.9 1997/03/11 14:36:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef	side_info_h
#define	side_info_h

/*---------------------- Include Files -------------------------*/

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif

#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

#ifndef	FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif


/*------------------------------------------------------*/
/* definitions  for fields within the side array	*/

#define	INFO_ARRAY_BLOCK_SIZE	2048 	/* initial size of info array	*/

#define	CAN_CHANGE		40
#define	CANNOT_CHANGE	41
#define	USED			-2
#define	NO_MORE			-1
#define	NO_LEVELV		-1
#define	NO_SUBSCRIPT	 0
#define	NO_REF			 0
#define NO_DC_INFO		-1
#define NO_CD_MAP		-1
#define NO_FD_INFO  	-1
#define	NO_CFG_INST		 0


/*------------------------------------------------------*/
/* info_type is used by get_info() and put_info() to  	*/
/* determine the component desired from the info array	*/

typedef enum info_type 
	{ type_levelv, type_subscript, type_ref, type_dc, type_cd_map, type_fd, type_cfg_inst} Info_type;


/*--------------------------------------------------------------*/
/* info_elem unifies all PED side arrays into one side array	*/
/* handles to components of the info array are stored here		*/

typedef struct Info_elem
{
    Generic levelv_index;		/* index into level vector array	*/
    Generic subscript_index;	/* index into subscript text array	*/
    Generic ref_index;			/* index into reference info array	*/
    Generic dc_index;			/* index into dc info array        	*/
    Generic fd_index;           /* index into fd infor array        */
    Generic cd_map;				/* maps from stmts into cdg nodes  	*/
    Generic cfg_inst;			/* gives cfg_instance for function  */
    int free_list;				/* USED, NO_MORE, or next free elem	*/
} Info_elem;


/*------------------------------------------------------*/
/* info/side array access and update information	*/

typedef struct struct_SideInfo
{
    int     	 info_side_array;		/* handle to side array	*/
    Info_elem   *info_array;			/* handle to info array	*/
    int		 info_array_first_free;		/* first free element	*/
} *SideInfoPtr, SideInfo;    




EXTERN( SideInfo *,  create_side_info, ( FortTree ft )	);

EXTERN( void,	destroy_side_info, ( FortTree ft, SideInfo * infoPtr ) );

EXTERN( void,	clear_info_array, ( SideInfo * infoPtr ) );

/*	920802: only used by ped_cp/memory/uj/do_unroll.C:		*/
EXTERN( void,	create_info, ( SideInfo * infoPtr, AST_INDEX astindex) );


EXTERN( void,	dg_put_info, ( SideInfo * infoPtr, AST_INDEX astindex,
			Info_type infotype, Generic value ) );

EXTERN( Generic, dg_get_info, ( SideInfo * infoPtr, AST_INDEX astindex,
			Info_type infotype ) );


#define		SI_INFO_SIDE_ARRAY(infoPtr)	(infoPtr->info_side_array)
#define		SI_INFO_ARRAY(infoPtr)		(infoPtr->info_array)
#define		SI_INFO_ARRAY_FIRST_FREE(infoPtr)	(infoPtr->info_array_first_free)


#endif	/* side_info_h */
