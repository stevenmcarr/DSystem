/* $Id: LoopStats.h,v 1.2 1992/12/11 11:19:40 carr Exp $ */
/* $Id: */
#ifndef LoopStats_h
#define LoopStats_h

#ifndef general_h
#include <general.h>        /* for Boolean */
#endif

#define NESTING_DEPTH 10

typedef struct localityrow {
  int Invariant,
      Spatial,
      None;
 } LocalityRowType;

typedef struct localitystruct {
  LocalityRowType SingleGroups,
                  MultiGroups,
                  MultiRefs;
 } LocalityMatrixType;

typedef struct loopstatstype {
  Boolean Perfect;
  int     TotalLoops,
          InMemoryOrder,
          NotInMemoryOrder,
          NearbyPermutationAttained,
          InterchangedIntoMemoryOrder,
          ObtainedInnerLoop,
          InnerLoopAlreadyCorrect,
          TimeStepPreventedMemoryOrder,
          DesiredInnerTimeStep,
          WrongInnerLoop,
          NextInnerLoop,
          UnsafeInterchange,
          DistributionUnsafe,
          NeedsScalarExpansion,
          TooComplex,
          Imperfect,
          Nests,
          NestingDepth[NESTING_DEPTH];
  LocalityMatrixType OriginalLocalityMatrix,
                     FinalLocalityMatrix,
                     MemoryLocalityMatrix;
  int     OriginalOtherSpatialGroups,
          FinalOtherSpatialGroups,
          MemoryOtherSpatialGroups;
  float   FinalRatio[NESTING_DEPTH],
          MemoryRatio[NESTING_DEPTH];
 } LoopStatsType;

#endif
