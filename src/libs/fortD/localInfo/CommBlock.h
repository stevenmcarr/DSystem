/* $Id: CommBlock.h,v 1.5 1997/03/11 14:28:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef CommBlock_h
#define CommBlock_h

#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/support/lists/IOSinglyLinkedList.h>

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#if 0
#ifndef sllist_h
#include <libs/support/lists/SinglyLinkedList.h>
#endif
#endif

#ifndef SinglyLinkedList_h
#include <libs/support/lists/SinglyLinkedList.h>
#endif

#ifndef FormattedFile_h
#include <libs/support/file/FormattedFile.h>
#endif

//-------------------------------------------------------------------------
class common_block_ent: public SinglyLinkedListEntryIO
{
public:
  char *name; 
  char *leader;
  int offset;
  unsigned int size;
  
  
  //-------------------------------------------------------------------------
  // constructors for overlap entry, assigns values to the data object.
  //-------------------------------------------------------------------------
  common_block_ent(char *nme, char *leadre, int off, unsigned int sze) {
    name = ssave(nme);
    leader = ssave(leadre);
    offset = off;
    size = sze;
  };
  
  common_block_ent() {
    name = NULL;
    leader = NULL;
    offset = 0;
    size = 0;
  };
  
  //-------------------------------------------------------------------------
  // read and write an entry to the database
  
  int WriteUpCall(FormattedFile &port);
  int ReadUpCall(FormattedFile &port);
};

//-------------------------------------------------------------------------
// a list of overlap entries for each procedure
//-------------------------------------------------------------------------
class CommonBlockList:  public SinglyLinkedListIO
{
  common_block_ent *current;

  public :

  SinglyLinkedListEntryIO *NewEntry();

  void append(common_block_ent *a)
  {
	SinglyLinkedList::Append((SinglyLinkedListEntry *) a);
	};

  void append_entry(char *nme, char* leadre, int off, unsigned int sze)
	{
  if(!get_entry(nme))
	 {
   common_block_ent *entry = new common_block_ent(nme, leadre, off, sze);
   append(entry);
   }
  };

  common_block_ent *first_entry() 
  { 
	return current = (common_block_ent *) SinglyLinkedList::First(); 
	};

  common_block_ent *next_entry() 
  { 
	return current = 
	(current ? (common_block_ent *) current->Next() : 0);
	};
 
  common_block_ent *get_entry(char *name)
  {
   common_block_ent *i;
   for(i = first_entry(); i != 0; i = next_entry())
    if(strcmp(i->name, name) == 0)
    return(i);
// if not found return 0;
   return(0);
  };

};

//-----------------------------------------------------------------
// common block entry
//-----------------------------------------------------------------
class common_block_entry_list : public SinglyLinkedListEntryIO
{
 public:
 char *name;           // common block name
 CommonBlockList *common_list; 
                      // list of entries belonging to common block name
 
 common_block_entry_list() {
   name = 0;
   common_list = new CommonBlockList;
 };
 common_block_entry_list(char* nme) {
   name = ssave(nme);
   common_list = new CommonBlockList();
  };	 

 int ReadUpCall(FormattedFile&);
 int WriteUpCall(FormattedFile&);

};

//-----------------------------------------------------------------
// contains a list of common block entries. Each entry contains
// a list of entries containing the elements in the common block 
//-----------------------------------------------------------------
class common_block_list:	public SinglyLinkedListIO
{
 public:
 common_block_entry_list *current;

 SinglyLinkedListEntryIO *NewEntry();

//---------------------------------
// append entry operations

  void append(common_block_entry_list *a)
  {
	SinglyLinkedList::Append((SinglyLinkedListEntry *) a);
	};

//---------------------------------- 
  common_block_entry_list* append_entry(char *name)
	{
   common_block_entry_list *entry = 
                  new common_block_entry_list(name);
   append(entry);
   return(entry);
  };

//------------------------------------
  common_block_entry_list *first_entry() 
  { 
	return current = (common_block_entry_list *) SinglyLinkedList::First();  };

//------------------------------------
  common_block_entry_list *next_entry() 
  { 
	return current = 
	(current ? (common_block_entry_list *) current->Next() : 0);
	};
 
//---------------------------------- 
  common_block_entry_list *get_entry(char *name)
  {
   common_block_entry_list *i;
   for(i = first_entry(); i != 0; i = next_entry())
    if(strcmp(i->name, name) == 0)
    return(i);
// if not found return 0;
   return(0);
  };

};

#endif
