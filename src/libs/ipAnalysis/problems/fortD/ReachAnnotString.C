/* $Id: ReachAnnotString.C,v 1.4 1997/03/11 14:35:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/problems/fortD/FortD_Sets.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnotString.h>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>
#include <libs/fortD/misc/FortDStr.h>

//-------------------------------------------------------------------------
// create the string DECOMPOSITION <decomp name>(dim_size, dim_size, ...)
//------------------------------------------------------------------------
void
FD_Reach_Annot_String::CreateDecompositionString(int index, StringBuffer *s)
{
 FD_Set *fortd_set = reach_annot->fortd_set[index];
 FortranDHashTableEntry *decomp_entry;

 if (fortd_set->Count() != 0)
 s->Append("%s ", DecompKeyWord);

 for(FD_SetItem *set_item = fortd_set->first_entry(); set_item != 0; 
                 set_item = fortd_set->next_entry())
   {
   decomp_entry = set_item->f;  
   s->Append("%s", set_item->f->name());
   
   for (int i = 0; i< decomp_entry->numdim; ++i)
   {
    if (i == 0) 
     {
     s->Append("%s", LeftBracket); 
     }
    s->Append(decomp_entry->typedescript_string(i));
    if(i != (decomp_entry->numdim -1))
      s->Append(','); 
   }
   s->Append("%s \n", RightBracket);
  }
}

//-------------------------------------------------------------------------
// create the string ALIGN <array string> with <decomp string>
//------------------------------------------------------------------------
void 
FD_Reach_Annot_String::CreateAlignString(int index, StringBuffer *s)
{
 FortranDHashTableEntry *decomp_entry, *array_entry;
 int align_index;

 FD_Set *fortd_st = reach_annot->fortd_set[index];

 if (fortd_st->Count() != 0)
 s->Append("%s ", AlignKeyWord);

 for(FD_SetItem *set_item = fortd_st->first_entry(); set_item != 0;
                 set_item = fortd_st->next_entry())
  {
   decomp_entry = set_item->f;
   array_entry  = set_item->array_ht;
   align_index  = set_item->fd->a_index();

  s->Append(array_entry->align_string(align_index, fortd_st->name()));
  s->Append("%s", "with");
  s->Append(array_entry->align_with_string(align_index,decomp_entry));
  s->Append("%c", '\n');
  }

}

//-------------------------------------------------------------------------
// create the string DISTRIBUTE <decomp name> (<distrib type>)
//------------------------------------------------------------------------
void
FD_Reach_Annot_String::CreateDistributeString(int index, StringBuffer *s)
{
 FortranDHashTableEntry *decomp_entry;
 int distrib_index;

 FD_Set *fortd_set = reach_annot->fortd_set[index];
  for(FD_SetItem *set_item = fortd_set->first_entry(); set_item != 0;
                  set_item = fortd_set->next_entry())
   {
   decomp_entry = set_item->f;
   distrib_index = set_item->fd->d_index();
   s->Append("%s %s", DistribKeyWord, decomp_entry->name());
   s->Append("%s",LeftBracket);
   s->Append(decomp_entry->distrib_string(distrib_index));
   s->Append("%s \n",RightBracket);
   }
 }

