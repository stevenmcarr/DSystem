/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stddef.h>
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/GenericList.h>

Boolean GenericList::QueryEntry(Generic i)
{
  GenericListEntry* GLEntry;  
  
      for (GenericListIter GLIter(this);
	   GLEntry = GLIter();)
	if (i == GLEntry->GetValue()) return true;
      return false;
} 

GenericListEntry* GenericList::GetEntry(Generic i)
{
  GenericListEntry* GLEntry;  

      for (GenericListIter GLIter(this);
	   GLEntry = GLIter();)
	if (i == GLEntry->GetValue()) return GLEntry;
      return (GenericListEntry*)NULL;
}
