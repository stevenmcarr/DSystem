/* $Id: FortDNumProcsAnnot.h,v 1.1 1997/03/11 14:34:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortDNumProcsAnnot_h
#define FortDNumProcsAnnot_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef OrderedSetOfStrings_h
#include <libs/support/strings/OrderedSetOfStrings.h>
#endif

#ifndef Annotation_h
#include <libs/support/annotation/Annotation.h>
#endif

class FortDNumProcsAnnot : public Annotation 
{
  public:
    const int numberProcs;

    // create a NumProcs annotation
    FortDNumProcsAnnot(int _numberProcs = 4);

    ~FortDNumProcsAnnot();

    // upcall to read an annotation
    int ReadUpCall(FormattedFile* file);

    // upcall to write an annotation
    int WriteUpCall(FormattedFile* file);

    // generate printable version of the annotation
    OrderedSetOfStrings* CreateOrderedSetOfStrings();
};

extern char* FORTD_NUM_PROCS_ANNOT;

#endif /* FortDNumProcsAnnot_h */
