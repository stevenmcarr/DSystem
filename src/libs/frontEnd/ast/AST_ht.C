/* $Id: AST_ht.C,v 1.5 2001/10/12 19:37:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/***********************************************************************
 * This class matches AST_INDEX's to arbitrary pointers.
 */
/***********************************************************************
 * Revision history:
 * $Log: AST_ht.C,v $
 * Revision 1.5  2001/10/12 19:37:04  carr
 * updated for Solaris 2.8 and RedHat 7.1
 *
 * Revision 1.4  1997/03/27 20:34:58  carr
 * Alpha
 *
 * Revision 1.3  1997/03/11  14:29:17  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  93/12/17  14:46:13  rn
made include paths relative to the src directory. -KLC

Revision 1.2  93/09/30  13:46:26  curetonk
changes to make AST_ht.C ANSI-C compliant

 * Revision 1.1  93/06/22  18:01:11  reinhard
 * Initial revision
 * 
 * Revision 1.6  1993/06/15  16:01:41  reinhard
 * Added constructor w/o arguments.
 * const'ed some methods.
 *
 * Revision 1.5  1993/06/09  23:43:17  reinhard
 * Cleaned up include hierarchy.
 *
 */

#include <assert.h>
#include <stdarg.h>
#include <iostream.h>

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif

/**********************************************************************/
/*** Definitions for class AST_htEntry ********************************/
/**********************************************************************/

/**********************************************************************
 * Constructor
 */
AST_htEntry::AST_htEntry(AST_INDEX my_key_node, Generic my_data)
{
  key_node = my_key_node;
  data     = my_data;
}


/**********************************************************************
 * Destructor
 */
AST_htEntry::~AST_htEntry()
{
}


/**********************************************************************
 * hash()  Hash on <key_node>.
 */
uint AST_htEntry::hash(unsigned int table_size)
{
  return ((unsigned int) key_node) % table_size;
}


/**********************************************************************
 * compare()  Compare <key_node>'s
 */
int AST_htEntry::compare(AST_htEntry *e)
{
  return key_node - e->key_node;
}


/**********************************************************************/
/*** Definitions for class AST_ht *************************************/
/**********************************************************************/

/**********************************************************************
 * Constructor w/o arguments
 */
AST_ht::AST_ht() 
   : HashTable(), gen_func(0), gen_arg(0)
{
  HashTable::Create(sizeof(AST_htEntry), 8);
};


/**********************************************************************
 * Constructor w/ gen_func
 */
AST_ht::AST_ht(Data_gen_ftype my_gen_func) 
   : HashTable(), gen_func(my_gen_func), gen_arg(0)
{
  HashTable::Create(sizeof(AST_htEntry), 8);
};


/**********************************************************************
 * Constructor w/ gen_func, gen_arg
 */
AST_ht:: AST_ht(Data_gen_ftype my_gen_func, Generic my_gen_arg)
   : HashTable(), gen_func(my_gen_func), gen_arg(my_gen_arg)
{
  HashTable::Create(sizeof(AST_htEntry), 8);
};


/**********************************************************************
 * Destructor
 */
AST_ht::~AST_ht()
{
  HashTable::Destroy();
}


/**********************************************************************
 * Virtual function for hashing entries in the table
 * (this definition overrides the virtual function of the base class).
 */
uint AST_ht::HashFunct(const void* entryV, const uint table_size) 
{
  AST_htEntry* entry = (AST_htEntry*)entryV;

  return entry->hash(table_size);
}


/**********************************************************************
 * Virtual function for comparing entries in the table
 * (this definition overrides the virtual function of the base class).
 */
int AST_ht::EntryCompare(const void* entryV1, const void* entryV2)
{
  AST_htEntry* entry1 = (AST_htEntry*)entryV1;
  AST_htEntry* entry2 = (AST_htEntry*)entryV2;

  return entry1->compare(entry2);
}


/**********************************************************************
 * count()  Returns # of entries.
 */
uint AST_ht::count() const
{
  return HashTable::NumberOfEntries();
}


/**********************************************************************
 * add_entry()
 */
void AST_ht::add_entry(AST_INDEX key_node, Generic data)
{
  AST_htEntry e(key_node, data);

  HashTable::AddEntry((void*)&e);
}


/**********************************************************************
 * query_entry()  Check whether an entry with <query_node> exists.
 */
Boolean AST_ht::query_entry(AST_INDEX query_node) const
{
  AST_htEntry   e(query_node, 0);
  AST_htEntry*  found;

  found = (AST_htEntry*)HashTable::QueryEntry((const void*)&e);

  return (Boolean)(found != 0);
}


/**********************************************************************
 * get_entry_by_AST()  Map <query_node> to entry.
 *                     Not found => return NULL.
 */
Generic AST_ht::get_entry_by_AST(AST_INDEX query_node) const
{
  AST_htEntry   e(query_node, 0);
  AST_htEntry*  found_entry;
  Generic       found_data;

  found_entry = (AST_htEntry*)HashTable::QueryEntry((const void*)&e);
  found_data  = (found_entry == 0) ? 0 : found_entry->data;

  return found_data;
}


/**********************************************************************
 * get_entry_by_index()  Map <index> to entry.
 *                       Not found => return NULL.
 */
Generic AST_ht::get_entry_by_index(int index) const
{
  AST_htEntry*  found_entry;
  Generic       found_data;

  found_entry = (AST_htEntry*)HashTable::GetEntryByIndex((const uint)index);
  found_data  = (found_entry == 0) ? 0 : found_entry->data;

  return found_data;
}


/**********************************************************************
 * gen_entry_by_AST()  Map <query_node> to entry.
 *                     Not found => return NULL.
 */
Generic AST_ht::gen_entry_by_AST(AST_INDEX query_node)  // Where to attach data
{
  Generic data;

  data = get_entry_by_AST(query_node);
  if (data == 0) {                            // No data found ?
    if (gen_func != 0)
    {
      data = (*gen_func)(query_node, gen_arg);  // Generate new data
      add_entry(query_node, data);
    }
    else
    {
      cout << "WARNING: AST_ht::gen_entry_by_AST(): " <<
	"gen_func undefined !\n";
    }
  }

  return data;
} 


/**********************************************************************
 * forall()  Apply forall_func to the data of all hash table entries.
 */
void AST_ht::forall(Apply_ftype apply_func, Action_ftype action, ...)
{
  va_list argList;
  HashTableIterator anIterator((HashTable*)this);

  anIterator.Reset();

  va_start(argList, apply_func);
    {
       for (int i = 0; i < HashTable::NumberOfEntries(); i++, ++anIterator)
         {
         apply_func((AST_htEntry*)anIterator.Current(), argList);
         }
    }
  va_end(argList);

  return;
}
