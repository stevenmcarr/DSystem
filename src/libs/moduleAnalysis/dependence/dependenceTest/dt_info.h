/* $Id: dt_info.h,v 1.8 1997/06/25 15:08:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef	dt_info_h
#define	dt_info_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif

/*----------------------------------------------------------*/
/* structure to manage loop, subs, and dt_str storage		*/

typedef struct mem_list
{
	struct mem_list	*next;
	char		*buf;
} Mem_list;

typedef struct dt_info_struct
{
	int	  loop_free;
	Mem_list *loop_mem;

	int	  ref_free;
	Mem_list *ref_mem;

	int	  rsd_free;
	Mem_list *rsd_mem;

	int	  rvec_free;
	Mem_list *rvec_mem;

	int	  str_free;
	Mem_list *str_mem;

} *DT_infoPtr, DT_info;

struct Rsd_section_struct;
typedef struct Rsd_section_struct Rsd_section;
struct Rsd_vector_struct;
typedef struct Rsd_vector_struct Rsd_vector;

EXTERN(DT_info *, dt_create_info, (void));
EXTERN(void, dt_finalize_info, (DT_info *dt));
EXTERN(void, dt_reset_info, (DT_info *dt));
EXTERN(Rsd_section*, dt_alloc_rsd, (DT_info *dt));
EXTERN(Rsd_vector*, dt_alloc_rsd_vector, (DT_info *dt));



#endif



