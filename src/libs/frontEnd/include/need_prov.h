/* $Id: need_prov.h,v 1.3 1997/03/11 14:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef NeedProv_h
#define NeedProv_h

#include <sys/types.h>


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifndef gi_h
#include <libs/frontEnd/include/gi.h>
#endif

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif


#define UNKNOWN_MODULE_NAME "_unknown_module_name_"

class NeedProvEntry 
{
  public:
    void init_entry (GI_ENT *ent = 0, char *name = 0);
    void delete_entry (void);

    void    dump (void);
    GI_ENT* get_gi_ent (void);
    char*   get_gi_ent_name (void);
    char*   get_proc_name (void);

    uint hash (uint size);
    int  compare (NeedProvEntry* e);

    void read (DB_FP* port);
    void write (DB_FP* port);

  private:
    GI_ENT* g;
    char*   proc_name; 
};

class NeedProvs : private HashTable 
{
  public:
    NeedProvs (char* name = 0);
   ~NeedProvs (void);

    char* get_module_name (void);
    void dump (void);

    virtual void write (DB_FP* port);
    virtual void read (DB_FP* port);


    void add_entry(NeedProvEntry* entry);
    NeedProvEntry* query_entry(NeedProvEntry* entry);
    NeedProvEntry* get_entry_by_index(int index);
    int count ();

  protected:
    virtual uint HashFunct (const void* entry, const uint size);
    virtual int  EntryCompare (const void* entry1, const void* entry2);
    virtual void EntryCleanup (void* entry);

  private:
    char* module_name;
};

#endif
