/* $Id: LoopStats.h,v 1.3 1993/07/20 15:28:23 carr Exp $ */
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
  float   PredictedFinalBalance,
          PredictedInitialBalance,
          InitialBalanceWithInterlock,
          ActualFinalBalance,
          FinalBalanceWithInterlock,
          InitialInterlock,
          FinalInterlock;
  int     PredictedFPRegisterPressure,
          ActualFPRegisterPressure,
          UnrolledLoops,
          NotUnrolled,
          SingleDepth;
  float   NotUnrolledBalance,
          NotUnrolledBalanceWithInterlock,
          NotUnrolledInterlock;
  int     NotUnrolledFPRegisterPressure;
  float   SingleDepthBalance,
          SingleDepthBalanceWithInterlock,
          SingleDepthInterlock;
  int     SingleDepthFPRegisterPressure;
  int     Distribute,
          Interchange,
          NoImprovement,
          AlreadyBalanced,
          InterlockCausedUnroll;
 } LoopStatsType;

#endif
