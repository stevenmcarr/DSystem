/* $Id: NamedGeneric.h,v 1.1 1997/03/11 14:37:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef NamedGeneric_h
#define NamedGeneric_h

/**********************************************************************
 *
 * NamedGeneric.h    Map strings to Generic's
 *
 */

/**********************************************************************
 * Revision History:
 * $Log: NamedGeneric.h,v $
 * Revision 1.1  1997/03/11 14:37:36  carr
 * newly checked in as revision 1.1
 *
 * Revision 1.1  94/02/27  19:23:29  reinhard
 * Initial revision
 * 
 */

#include <libs/support/tables/namedObject/NamedObject.h>
#include <libs/support/tables/namedObject/NamedObjectTable.h>

/**********************************************************************
 * class NamedGeneric
 */
class NamedGeneric : public NamedObject {
public:
  //virtual ~NamedGeneric();                       // Virtual Destructor
  NamedGeneric(const char *const myObjectName,   // Constructor
	       Generic           myObject)
    : NamedObject(myObjectName),
      object(myObject) {};
  Generic getObject() const { return object; }
  
private:
  Generic object;
};


/**********************************************************************
 * class NamedGenericTable
 */
class NamedGenericTable : public NamedObjectTable {
public:
  ~NamedGenericTable()          // Destructor
  {
    Destroy();
  }
    
  void AddEntry(const char *const myObjectName,
		Generic           myObject)
  {
    NamedGeneric *entry = new NamedGeneric(myObjectName, myObject);
    NamedObjectTable::AddEntry(entry);
  }

  Generic QueryEntry(const char *const name)
  {
    NamedGeneric *entry = (NamedGeneric *) NamedObjectTable::QueryEntry(name);
    Generic      object = (entry) ? (entry->getObject()) : 0;

    return object;
  }
};


/**********************************************************************
 * class NamedGenericTableIterator
 */
class NamedGenericTableIterator : public NamedObjectTableIterator {
public:
  NamedGenericTableIterator(const NamedGenericTable *theTable)
    : NamedObjectTableIterator(theTable) {};

  Generic Current()
  {
    NamedGeneric *entry = (NamedGeneric *) NamedObjectTableIterator::Current();
    Generic      object = (entry) ? (entry->getObject()) : 0;

    return object;
  }

  const char *CurrentName()
  {
    NamedGeneric *entry    = (NamedGeneric *) NamedObjectTableIterator::Current();
    const char *const name = (entry) ? (entry->name) : 0;

    return name;
  }
};
#endif

