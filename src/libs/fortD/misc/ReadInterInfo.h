/* $Id: ReadInterInfo.h,v 1.5 2001/10/12 19:31:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/fortD/misc/FortD.h>

#if 0
#include <libs/ipAnalysis/ipInfo/annot_table.h>
#endif

#undef is_open
#include <iostream.h>

EXTERN(void, map_set_to_sp, (SNODE *sp, FortranDHashTableEntry *a,
                FortranDHashTableEntry *d, int align_index, int dist_index));

EXTERN (void, map_decomp_to_sp, (FortranDHashTableEntry *decomp_entry,
                 SNODE *sp, int d_index));

EXTERN(void, map_decomp,(SNODE *sp));

#if 0
class ReadAnnotation : public Annotation 
{
public:
   FortranDInfo  *f;
 
   ReadAnnotation(char *name) : Annotation (name, "ReadAnnotation")
      { f = new FortranDInfo(); };
};
#endif
