/* $Id: FortD_Sets.h,v 1.3 1997/03/11 14:34:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _FortD_Sets_
#define _FortD_Sets_

#include <libs/fortD/misc/FortD.h>
#include <string.h>
#include <libs/support/lists/IOSinglyLinkedList.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/fortD/localInfo/CommBlock.h>
#include <libs/ipAnalysis/problems/fortD/CommonBlockAnnot.h>

//---------------------------------------------------------------
//---------------------------------------------------------------
class FD_SetItem: public SinglyLinkedListEntry 
{
 public:
 FortranDHashTableEntry *f, *array_ht; // a pointer to the hashtable entry
 FDSetEntry *fd;         // decomposition name, align_index, distrib_index
 common_block_ent *c;    // common block information for array entry

//------------------------
// FD_SetItem constructor

  FD_SetItem(FortranDHashTableEntry *ff = NULL, 
             FortranDHashTableEntry *a_ht = NULL,
             FDSetEntry *ffd = NULL, common_block_ent *cc = NULL)
  {
   array_ht = a_ht;
   f =  ff;
   fd = ffd; 
   c  = cc;
  };

//------------------------
// copy the set item;

  FD_SetItem* copy()
  {
   FD_SetItem *item = new FD_SetItem(this->f,this->array_ht,this->fd,this->c);
   return (item);
  };

  FD_SetItem* copy(common_block_ent *c)
		{
    FD_SetItem *item = new FD_SetItem(this->f, this->array_ht, this->fd, c);
    return (item);
  	};
};

//---------------------------------------------------------------
//---------------------------------------------------------------
class FD_Set: public SinglyLinkedList
{
 private:
  FD_SetItem *current;

 public:
  
  char *nme;

  FD_Set() {current=0; nme = NULL;}
  
  void append_entry(FortranDHashTableEntry *f, FortranDHashTableEntry *array_ht,                   FDSetEntry *fd, common_block_ent *cc = NULL)
	{
    FD_SetItem *s = new FD_SetItem(f, array_ht, fd, cc);
  	SinglyLinkedList::Append((SinglyLinkedListEntry*)s);
	};

  void append_entry(FD_SetItem *item)
	{
  	SinglyLinkedList::Append((SinglyLinkedListEntry*)item);
	};

  FD_SetItem *first_entry()
  {
   return current = (FD_SetItem *) SinglyLinkedList::First();
  };

  FD_SetItem *next_entry()
  { 
	  return current = 
  	(current ? (FD_SetItem *) current->Next() : 0);
  };

 
  void save_name(char *nme1) 
  {
  nme = ssave(nme1);
  };
 
  char *name()
  {
  return nme;
  };
 
  void Union(FD_Set *set1, FD_Set *set2);
  void Union(FD_Set *set1);
  void Union(FD_Set*, common_block_ent*);
  Boolean Diff(FD_Set *set1);
  void write(FormattedFile &port);  
  void map_set_info(FortranDInfo *f);
};


#endif _FortD_Sets_

