/* $Id: val_ht.C,v 1.8 1997/03/27 20:47:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdarg.h>

#include <libs/moduleAnalysis/valNum/val.i>

#include <libs/support/file/FormattedFile.h>

#include <libs/moduleAnalysis/valNum/val_ht.h>

//
//
Values val_Open()
{
    return new ValTable;
}

//
//
ValTable::ValTable() : HashTable()
{

    HashTable::Create(sizeof(ValEntry*), 8);

        //  Structure of value entries assumes...
    if (!((sizeof(ValField) == sizeof(float)) && 
	  (sizeof(ValField) == sizeof(void*))))
      {
  	die_with_message("ValTable: ValFields have different sizes\n");
      }
        //  These should be the first values allocated, so that they
        //  have the same indices for every value table.
    add_entry(val_new(VAL_BOT_TYPE));	// should have index VAL_BOTTOM
    add_entry(val_new(VAL_TOP_TYPE));	// should have index VAL_TOP

        //  Add these values to the table and make sure they get the numbers
        //  we think they're getting.  Since they're added to every table
        //  at the start, in the same order, they should always be the same.
    if ((VAL_ZERO  != val_lookup_const(*this, TYPE_INTEGER, 0)) ||
	(VAL_ONE   != val_lookup_const(*this, TYPE_INTEGER, 1)) ||
	(VAL_M_ONE != val_lookup_const(*this, TYPE_INTEGER, -1)) ||
	(VAL_TRUE  != val_lookup_const(*this, TYPE_LOGICAL, true)) ||
	(VAL_FALSE != val_lookup_const(*this, TYPE_LOGICAL, false)))
      {
	die_with_message("ValTable: failed constants allocation\n");
      }
}

//
//
ValTable::~ValTable()
{
  HashTable::Destroy();
}

//
//
void val_Close(Values Vp)
{
      //  Hope this calls delete_entry when ValTable is destructed
  delete ((ValTable*)Vp);
}

//
//
int ValTable::count() 
{ 
  return HashTable::NumberOfEntries(); 
}

//
//
void ValTable::ForAll(ValTableForAllFunct aFunct, ...)
{
  va_list args;

  va_start(args, aFunct);
    {
      ValEntry* currentValEntry;
      HashTableIterator anIterator((HashTable*)this);

      currentValEntry = (ValEntry*)anIterator.Current();
      while (currentValEntry != NULL)
        {
          aFunct((ValEntry*)currentValEntry, args);
          ++anIterator;
          currentValEntry = (ValEntry*)anIterator.Current();
        }
    }
  va_end(args);

  return;
}

//
//
void ValTable::add_entry(ValEntry *ve)
{
  HashTable::AddEntry((void*)&ve);
}

//
//
int ValTable::operator[](ValEntry *ve)
{
  return HashTable::GetEntryIndex((void*)&ve);
}

//
//
ValEntry& ValTable::operator[](int index)
{
  return **((ValEntry**)HashTable::GetEntryByIndex(index));
}


//---------------------------------------------------------------------------
// val_Dump
//
//    a debugging to show the contents of the value table 
//
//---------------------------------------------------------------------------

//
//
void val_Dump(Values Vp)
{
    int		i,  length;

    length = (*Vp).count();

    for (i=0; i<length; i++)
	val_print_entry(Vp, i);
}

//
//
Boolean ValTable::Write(FormattedFile& port)
{
    int count = this->count();

    port.Write("ValTable");
    port.Write(count);

    for (int i = 0; i < count; i++)
    {
	(*this)[i].Write(port);
    }
    return true;
}

//
//
Boolean ValTable::Read(FormattedFile& port)
{
    ValEntry placeholder;
    int count;
    char buff[20];

    if (port.Read(buff, 20) || strcmp(buff, "ValTable")) return false;
    port.Read(count);

    for (int i = 0; i < count; i++) 
    {
	ValEntry *ve = placeholder.Read(port);
	if (ve == 0) return false;
	this->add_entry(ve);
    }
    return true;
}

//
//
int val_table_max(Values Vp)
{
    return (*Vp).count();
}

//
//
unsigned int ValTable::HashFunct(const void *entry, const unsigned int size)
{
    return (*((ValEntry **)entry))->hash(size);
};

//
//
int ValTable::EntryCompare(const void *e1, const void *e2) /* 0 if equal */
{
    return (*((ValEntry **)e1))->compare(**((ValEntry **)e2));
};

//
//
void ValTable::EntryCleanup(void *entry)
{
    ValEntry *found = *((ValEntry **)entry);
    val_delete(found);
}
