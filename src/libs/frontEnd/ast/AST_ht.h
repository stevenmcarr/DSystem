/* $Id: AST_ht.h,v 1.6 1997/03/11 14:29:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AST_ht_h
#define AST_ht_h

/**********************************************************************
 * This hash table class matches AST_INDEX's to arbitrary pointers.
 * When templates are supported, use Type instead of Generic.
 */
/**********************************************************************
 * Revision history:
 * $Log: AST_ht.h,v $
 * Revision 1.6  1997/03/11 14:29:17  carr
 * newly checked in as revision 1.6
 *
 * Revision 1.6  93/12/17  14:25:13  rn
 * made include paths relative to the src directory.
 * 
 * Revision 1.5  93/09/30  14:25:13  curetonk
 * changes to make AST_ht.h ANSI-C compliant
 * 
 * Revision 1.4  93/08/19  14:24:07  curetonk
 * forgot the misc path on rn_varargs.h.
 * 
 * Revision 1.3  93/08/19  11:57:56  curetonk
 * Change include stdarg.h to include rn_varargs.h.
 * 
 * Revision 1.2  93/06/23  10:31:26  reinhard
 * Updated comments.
 * 
 */

#undef is_open

#include <sys/types.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/*------------------- FORWARD DECLARATIONS ------------------*/

class AST_ht;
class AST_htEntry;

/*------------------- TYPES ---------------------------------*/

typedef FUNCTION_POINTER (void, Action_ftype, ());
typedef FUNCTION_POINTER (void, Apply_ftype, (AST_htEntry *entry, va_list ap));
typedef FUNCTION_POINTER (Generic, Data_gen_ftype, (AST_INDEX node, Generic gen_arg));
typedef FUNCTION_POINTER (void, Forall_ftype, (Generic data));

/*********************************************************************/
/*** Declaration of class AST_htEntry ********************************/
/*********************************************************************/
class AST_htEntry {
  public:
    AST_htEntry(AST_INDEX my_key_node, Generic my_data);
   ~AST_htEntry();

    Generic getData() { return data; };     // Access function
    uint    hash(uint table_size);          // Hash function
    int     compare(AST_htEntry *e);        // Equality predicate

  private:
    AST_INDEX key_node;     // The hash key
    Generic   data;         // The data associated with <key_node>

    friend class AST_ht;
};


/*********************************************************************/
/*** Declaration of class AST_ht *************************************/
/*********************************************************************/
class AST_ht : private HashTable {
  public:
    AST_ht();
    AST_ht(Data_gen_ftype my_gen_func);
    AST_ht(Data_gen_ftype my_gen_func, Generic my_gen_arg);
   ~AST_ht();

    uint    count() const;
    void    add_entry(AST_INDEX my_key_node, Generic my_data);
    Boolean query_entry(AST_INDEX query_node) const;
    Generic get_entry_by_index(int index) const;
    Generic get_entry_by_AST(AST_INDEX query_node) const;
    Generic gen_entry_by_AST(AST_INDEX query_node);
    void    forall(Apply_ftype apply_func, Action_ftype action, ...);

  protected:
    Generic gen_arg;   // Argument to gen_func

  private:
    Data_gen_ftype gen_func;  // Function generating new entries

    virtual uint HashFunct(const void* entry, const uint table_size);
    virtual int  EntryCompare(const void* e1, const void* e2);

    friend class AST_ht_Iter;   // Iterator
};


/**********************************************************************
 * An Iterator.  Usage:
 *
 *    AST_ht  *ht;
 *    Generic elmt;
 *    ...
 *    for (AST_ht_Iter iter(ht); elmt = iter();) { ... }
 *
 */
class AST_ht_Iter {
public:
  AST_ht_Iter(AST_ht *my_ht)
    : ht(my_ht), index(0), count(ht->count()) {}

  Generic operator() () {
    return (index < count) ? (ht->get_entry_by_index(index++)) : 0;
  }

  void reset() { index = 0; }

private:
  AST_ht *ht;
  int    index;
  int    count;
};

#endif
