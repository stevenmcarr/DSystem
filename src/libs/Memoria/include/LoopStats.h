/* $Id: LoopStats.h,v 1.11 1994/11/21 14:56:15 qwu Exp $ */
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
  int     ContainsExit,
	  ContainsIO,
	  ContainsCall,
	  ContainsConditionalDO;
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
          Reversed,
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
          Normalized,
          AlreadyBalanced,
          InterlockCausedUnroll;

	  /* ADD FIELDS FOR SR STATS HERE */

  int     NumLoop_badexit,     /* number of loops having bad exit */
          NumLoop_backjump,    /* number of loops having back jump */
	  NumLoop_illjump,     /* number of loops having illegal jump */
	  Numbadflow,          /* number of loops having bad flow */
          FPRegisterPressure,  /* float point register pressure */
	  SRRegisterPressure,  /* number of registers used for scalar replacement */
	  NumLoopReplaced,     /* number of loops replaced */
	  NumLoopSpilled,      /* number of loops spilled  */
          NumRefRep,           /* number of references replaced */
          NumBasicBlock,       /* number of basic blocks */
	  NumZeroFPLoop,       /* number of loops w/ free FP pressure */ 
          NumInnermostLoop,    /* number of innermost loops */

	  /* QUNYAN 0001 */
	  /* Add in new variables for LIAV,LCAV ... */
	  NumLIAV,             /* number of loop independent dependency */
	  NumLCAV,             /* number of loop carry dependency */
	  NumLIPAV,            /* number of partial loop indenpendent dependency*/
	  NumLCPAV;            /* number of partial loop carry dependency */
          /* QUNYAN 0001*/
  
  float   LoopBal;             /* Loop Balance */
  
 } LoopStatsType;

#endif
