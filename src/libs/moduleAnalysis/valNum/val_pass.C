/* $Id: val_pass.C,v 1.6 1997/03/11 14:36:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <libs/support/file/FormattedFile.h>

#include <libs/moduleAnalysis/valNum/val_pass.h>

//------------------------------------------------------------------------------
//  ValPassEntry private member functions
//------------------------------------------------------------------------------

ValPassEntry::ValPassEntry(int s, char *n, uint o, ValNumber v)
  : site(s), name(n), offset(o), val(v)
{
  if (name) name = ssave(name);
}

ValPassEntry::~ValPassEntry()
{
  if (name) sfree(name);
}

unsigned int ValPassEntry::hash(unsigned int size)
{
  return (hash_string(name,size) ^ offset ^ (site << 16)) % size;
}

int ValPassEntry::compare(ValPassEntry *e)
{
    // if different, any non-zero value suffices
  return ((this->site != e->site) ||
          (this->offset != e->offset) ||
          strcmp(this->name, e->name));
}

void ValPassEntry::Write(FormattedFile& port)
{
    port.Write("P");
    port.Write(this->site);
    port.Write(this->name);
    port.Write(this->offset);
    port.Write(this->val);
}

void ValPassEntry::Read(FormattedFile& port)
{
    char buff[20];
    if (port.Read(buff) || (buff[0] != 'P'))
	die_with_message("ValPassEntry::Read -- format error\n");

    port.Read(this->site);
    port.Read(buff); 
    this->name = ssave(buff);
    port.Read(this->offset);
    port.Read(this->val);
}

Boolean ValPassMap::Write(FormattedFile& port)
{
    int count = this->count();

    port.Write("ValPassMap");
    port.Write(count);

    for (int i = 0; i < count; i++)
    {
	(*this)[i].Write(port);
    }
    return true;
}
Boolean ValPassMap::Read(FormattedFile& port)
{
    ValPassEntry placeholder(0,NULL,0,-1);
    int count;
    char buff[20];

    if (port.Read(buff, 20) || strcmp(buff, "ValPassMap")) return false;
    port.Read(count);

    for (int i = 0; i < count; i++) 
    {
	placeholder.Read(port);
	add_entry(&placeholder);
    }
    return true;
}

void ValPassEntry::Dump()
{
    fprintf(stderr, "\t%d\t%d\t%d\t%s\n",
	    site, offset, val, name);
}

void ValPassMap::Dump()
{
    int ct = count();

    fprintf(stderr, "ValPassMap\n\tsite\toffset\tval\tname\n");

    for (int i = 0; i < ct; i++)
    {
	(*this)[i].Dump();
    }
}

//------------------------------------------------------------------------------
//  ValPassMap public member functions
//------------------------------------------------------------------------------


ValPassMap::ValPassMap() 
  : HashTable ()
{
  HashTable::Create (sizeof(ValPassEntry), 8);
};

unsigned int ValPassMap::count()
{
  return HashTable::NumberOfEntries();
};

void ValPassMap::add_entry(int s, char *n, uint o, ValNumber v)
{
  ValPassEntry e(s, n, o, v);
  HashTable::AddEntry(&e);
  e.name = (char *) 0;
  return;
};

void ValPassMap::add_entry(ValPassEntry *ep)
{
  add_entry(ep->site, ep->name, ep->offset, ep->val);
};

void ValPassMap::add_entry(int s, uint p, ValNumber v)
{
  add_entry(s, "", p, v);
};

void ValPassMap::add_entry(char * n, uint o, ValNumber v) 
{
  add_entry(UNUSED, n, o, v);
};

void ValPassMap::delete_entry(int s, char *n, uint o) 
{
  ValPassEntry e(s, n, o, VAL_NIL);
  HashTable::DeleteEntry(&e);
};

int ValPassMap::query_entry(int s, char *n, uint o) 
{
  ValPassEntry e(s, n, o, VAL_NIL);
  ValPassEntry *found = (ValPassEntry *) HashTable::QueryEntry(&e);
  if (found) return found->val;
  else       return VAL_NIL;
};

int ValPassMap::query_entry(int s, uint p)
{
  return query_entry(s, "", p);
};

int ValPassMap::query_entry(char *n, uint o)
{
  return query_entry(UNUSED, n, o);
};


//------------------------------------------------------------------------------
//  ValPassMap private member functions
//------------------------------------------------------------------------------

unsigned int ValPassMap::HashFunct(const void *entry, const unsigned int size)
{
  return ((ValPassEntry *)entry)->hash(size);
}

int ValPassMap::EntryCompare(const void *e1, const void *e2)
{
  return ((ValPassEntry *)e1)->compare((ValPassEntry *)e2);
}

void ValPassMap::EntryCleanup(void *entry)
{
  if (((ValPassEntry *)entry)->name) sfree(((ValPassEntry *)entry)->name);

  return;
}

ValPassEntry& ValPassMap::operator[](int index)
{
  return *((ValPassEntry *) HashTable::GetEntryByIndex(index));
}

