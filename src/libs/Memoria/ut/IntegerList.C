#include <stddef.h>
#include <general.h>
#include <IntegerList.h>

Boolean IntegerList::QueryEntry(int i)
{
  int j;  
  
      for (IntegerListIter ILIter(this);
	   j = ILIter()->GetValue();)
	if (i == j) return true;
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
