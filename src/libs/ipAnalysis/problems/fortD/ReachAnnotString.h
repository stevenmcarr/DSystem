/* $Id: ReachAnnotString.h,v 1.2 1997/03/11 14:35:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _ReachAnnotString_
#define _ReachAnnotString_

#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>

class FD_Reach_Annot_String 
{
 FD_Reach_Annot *reach_annot;

 public:
 FD_Reach_Annot_String(FD_Reach_Annot *annot)
 {
   reach_annot = annot;
 }
 void  CreateDecompositionString(int index, StringBuffer *s);
 void  CreateAlignString(int index, StringBuffer *s);
 void  CreateDistributeString(int index, StringBuffer *s);
}; 

#endif _ReachAnnotString_
