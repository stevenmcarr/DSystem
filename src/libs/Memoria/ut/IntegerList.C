/* $Id: IntegerList.C,v 1.3 1997/03/27 20:29:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stddef.h>
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/IntegerList.h>

Boolean IntegerList::QueryEntry(int i)
{
  IntegerListEntry* ILEntry;  
  
      for (IntegerListIter ILIter(this);
	   ILEntry = ILIter();)
	if (i == ILEntry->GetValue()) return true;
      return false;
} 

IntegerListEntry* IntegerList::GetEntry(int i)
{
  IntegerListEntry* ILEntry;  

      for (IntegerListIter ILIter(this);
	   ILEntry = ILIter();)
	if (i == ILEntry->GetValue()) return ILEntry;
      return (IntegerListEntry*)NULL;
}
