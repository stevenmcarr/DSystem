/* $Id: LoopStats.h,v 1.18 2001/09/14 17:00:57 carr Exp $ */
/* $Id: */
#ifndef LoopStats_h
#define LoopStats_h

#include <libs/support/misc/general.h>        /* for Boolean */

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
          PredictedP_L,
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
	  NumLCPAV,            /* number of partial loop carry dependency */
	  NumInv,              /* number of loop carried invariant */
	  NumLC1,              /* number of loop carried distance 1 */
          /* QUNYAN 0001*/
  
	  UniformRefs,         /* number of uniformly generated references */
	  NonUniformRefs,      /* number of non-uniformly generated references */
          NonUniformLoops,     /* number of loops with non-UGR's */
	  NonUniformLoopsReplaced,     /* number of loops w/ non-uniformly generated references and potential for SR */
	  NonUniformLoopsZeroFP;     /* number of loops w/ non-uniformly generated references and no register pressure */
  float   LoopBal;             /* Loop Balance */

  int     NumKilledGenerators;
  int     NumNoConsistentDependence;
  

  int 	  NumberOfTrueDependences;
  int     NumberOfLoopCarriedTrueDependences;
  int 	  NumberOfAntiDependences;
  int     NumberOfLoopCarriedAntiDependences;
  int 	  NumberOfOutputDependences;
  int     NumberOfLoopCarriedOutputDependences;
  int 	  NumberOfInputDependences;
  int     NumberOfLoopCarriedInputDependences;

  int	  IntRegsMinAvg;
  int	  IntRegsMinDist;
  int	  IntRegsLiveAcross;
  int	  FloatRegsMinAvg;
  int	  FloatRegsMinDist;
  int	  FloatRegsLiveAcross;
  int     II;
 } LoopStatsType;

#endif
