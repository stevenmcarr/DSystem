/* $Id: align.h,v 1.6 1997/03/11 14:31:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*********************************/
/* Information structure passed  */
/* to the dialog                 */
/*********************************/

typedef struct Align_Info
{
	int		degree;
    int     loop_depth;
	Boolean		danger;
	Boolean 	other_dep;
}  align_info;

EXTERN(align_info, pt_align_degree,(PedInfo ped));

EXTERN(void, pt_align,(PedInfo ped, char *arg));
