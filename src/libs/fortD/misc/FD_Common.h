/* $Id: FD_Common.h,v 1.4 1997/03/11 14:28:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef _FD_Common_
#define _FD_Common_

#include <libs/ipAnalysis/ipInfo/annot_table.h>
#include <libs/fortD/localInfo/CommBlock.h>

#define FD_COMMON_BLOCK "FortranDCommonBlkInfo"
//---------------------------------------------------------------------
// initialize fortranD annotation 
// the node annotation contains common block declaration information
//---------------------------------------------------------------------
class FD_CommBlkNodeAnnotation : public Annotation 
{
 public:
 common_block_list *common_blks_decl;
 
 FD_CommBlkNodeAnnotation(char *name, common_block_list *c) : 
 Annotation(name, "FD_CommBlkAnnotation") // initialize parent class
 {
  common_blks_decl = c;
 };

//-------------------------------------------------
// utilities to look up information on common 
// block declarations stored at the nodes

 Boolean fort_d_is_common(char *nme);
 char* fort_d_get_common_entry_nme(char *leader, int offset, int size);

//-----------------------------------------------------------------------
// look up the annotation to see if the string nme is a global variable
//-----------------------------------------------------------------------
common_block_ent *fortd_is_global(char* nme)
{
     for(common_block_entry_list *clist =
      common_blks_decl->first_entry();
      clist != 0;
      clist = common_blks_decl->next_entry())
    {
     for(common_block_ent *e = clist->common_list->first_entry(); e != 0;
         e = clist->common_list->next_entry())

// for each entry check if the size and starting position are the
// same. If so then return the comon_block_entry

     if (strcmp(e->name, nme) == 0)
       return (e);
		}	
   return(0);
};

//---------------------------------------------------------------------
// look up the annotation to find a common_block_ent that has the same
// common block leader name, offset, size as that stored in the
// common block entry c
//---------------------------------------------------------------------
common_block_ent *translate_global(common_block_ent *c)
{
 for(common_block_entry_list *clist =
      common_blks_decl->first_entry();
      clist != 0;
      clist = common_blks_decl->next_entry())
    {
     for(common_block_ent *e = clist->common_list->first_entry(); e != 0;
         e = clist->common_list->next_entry())

// for each entry check if the size and starting position are the
// same. If so then return the comon_block_entry

     if (strcmp(e->leader, c->leader) == 0)
			{
      if ((e->offset == c->offset) && (e->size == c->size))
       return (e);
//      else
//				{
//         cout<<form("CommonBlock %s not aligned correctly \n", e->leader);
//         exit(0);
//        }
      }
	 }
   return(0);
};

};


#endif _FD_Common_
