/* $Id: QuickSort.h,v 1.3 1997/03/11 14:37:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*******************************************************************************
 * File:   QuickSort Class                                                     *
 * Author: Christine Patton                                                    *
 * Date:   June, 1993                                                          *
 *                                                                             *
 ***************************** QuickSort Public ********************************
 *                                                                             *
 * Constructor:                                                                *
 *   Nothing.                                                                  *
 *                                                                             *
 * Destructor:                                                                 *
 *   Nothing.                                                                  *
 *                                                                             *
 * Create:                                                                     *
 *   This function must be called after creating the object as it allocates    *
 *   space for a pointer to an array of void pointers, initializes the number  *
 *   of slots in the array, and initializes a pointer to the comparison funct. *
 *   The comparison function should take two arguments and return a positive   *
 *   integer if the first argument is greater, a negative one if the second    *
 *   argument is greater.                                                      *
 *                                                                             *
 * Destroy:                                                                    *
 *   deallocates the space for the pointers initialized in Create.             *
 *                                                                             *
 * Sort:                                                                       *
 *   recursivley sorts the section of the array from minEntry to maxEntry.     *
 *                                                                             *
 **************************** QuickSort Private ********************************
 *                                                                             *
 * Partition:                                                                  *
 *   returns the rank of element q int the array (all elements less than       *
 *   element q are to the left, all elements greater than element q are        *
 *   to the right)                                                             *
 *                                                                             *
 * Swap:                                                                       *
 *   pretty much self-explanatory                                              *
 *                                                                             *
 * ArrayPtr:                                                                   *
 *   pointer to the array that will be sorted                                  *
 *                                                                             *
 * CompareFunc:                                                                *
 *   pointer to the comparison function provided by the user                   *
 *                                                                             *
 * QuickSortCreated:                                                           *
 *   set to true in Create (must have value true before Sort, Partition,       *
 *   or Swap can be executed)                                                  *
 *                                                                             *
 ******************************************************************************/

#ifndef QuickSort_h
#define QuickSort_h

/***************************** include files **********************************/

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

/************************ QuickSort function prototypes ***********************/

typedef FUNCTION_POINTER (int, EntryCompareFunctPtr, (const void*, const void*));

/************************** QuickSort class definition ************************/

class QuickSort
{
  public:
    QuickSort ();
    virtual ~QuickSort ();

    void Create (void** UserArrayPtr, const EntryCompareFunctPtr _CompareFunct);
    void Destroy ();

    void Sort (const int minEntryIndex, const int maxEntryIndex);

  private:
    void** ArrayPtr;                 
    EntryCompareFunctPtr CompareFunct;   

    Boolean QuickSortCreated;         

    void Swap (int a, int b);
    int  Partition (const int min, const int max, const int q);
};

#endif
