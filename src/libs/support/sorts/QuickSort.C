/* $Id: QuickSort.C,v 1.2 1997/03/11 14:37:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>

#include <libs/support/misc/general.h>
#include <libs/support/sorts/QuickSort.h>

QuickSort::QuickSort ()
{
  return;
}

QuickSort::~QuickSort ()
{
  return;
}

void QuickSort::Create (void** UserArrayPtr, const EntryCompareFunctPtr _CompareFunct)
{
  ArrayPtr = UserArrayPtr;
  CompareFunct = _CompareFunct;

  QuickSortCreated = true;

  return;
}

void QuickSort::Destroy ()
{
  QuickSortCreated = false;

  return;
}

void QuickSort::Swap (int a, int b)
{
  if (QuickSortCreated)
    {
      void* hold;
      hold = ArrayPtr[a];
      ArrayPtr[a] = ArrayPtr[b];
      ArrayPtr[b] = hold;
    }

  return;
}

int QuickSort::Partition (const int min, const int max, const int q)
{
  if (QuickSortCreated)
    {
      Swap (min, q);
      void* x = ArrayPtr[min];
      int j = min - 1;
      int k = max + 1;
      Boolean ExitFlag = false;
      while (!ExitFlag)
        {
          do
            k--;
          while ((CompareFunct (ArrayPtr[k], x) > 0) && (k>min));
          do
            j++;
          while ((CompareFunct (ArrayPtr[j], x) < 0) && (j<max));
          if (j < k)
             Swap (j, k);
           else
            ExitFlag = true;
        }
      return k;
    }
  else return 0;
}

void QuickSort::Sort (const int minEntryIndex, const int maxEntryIndex)
{
  if (QuickSortCreated)
    {
      if (minEntryIndex < maxEntryIndex)
        {
          int index = rand () % (maxEntryIndex-minEntryIndex+1) + minEntryIndex;
          int mid = Partition (minEntryIndex, maxEntryIndex, index);
          Sort (minEntryIndex, mid);
          Sort (mid+1, maxEntryIndex);
        }
    }
 
  return;
}
