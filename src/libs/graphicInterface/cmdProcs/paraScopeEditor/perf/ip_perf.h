/* $Id: ip_perf.h,v 1.5 1997/03/11 14:32:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
EXTERN(void, perf_local_phase_walkcd, 
		(Perf_data *pdata,
		 FortTree ft,
		 Context context,
		 DG_Instance *dg,
		 SideInfo *infoPtr,
		 FILE *dbgfp,
		 Boolean *single,
		 char *tmodname));


