/* $Id: SPRegisters.h,v 2.1 2001/09/14 17:08:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef SPRegisters_h
#define SPRegisters_h

#include <stdio.h>

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/Memoria/ut/DDG.h>
#include <libs/Memoria/ut/MinDist.h>
#include <libs/Memoria/ut/ScalarReplaceMap.h>
#include <libs/frontEnd/fortTree/fortsym.h>

static const int TotalMethods = 3;
typedef enum RegisterPressureMethod {MinDistMethod = 0,MinAvgMethod = 1,
                                     LiveAcrossMethod = 2};

class SPRegisterPrediction {

  int Flops;
  int IntOps;
  AST_INDEX LoopBody;
  PedInfo Ped;
  int Level;
  int II;
  char **IVar;
  DDG *ddg;
  MinDist *Dist;
  ScalarReplaceMap *SRMap;

  Boolean IntegerRegisterPressureComputed[TotalMethods];
  Boolean FloatingPointRegisterPressureComputed[TotalMethods];

  int IntegerRegisterPressure[TotalMethods];
  int FloatingPointRegisterPressure[TotalMethods];

  int ComputeResII(AddressEquivalenceClassSet& AECS);
  AstIterAdvanceDirective ComputeFlopsAndIntOps(AST_INDEX Node);
  int OperationIssueSlots(AST_INDEX Node);
  int ComputeMinAvg(int DataType);
  int ComputeLiveAcross(int DataType);
  int ComputeMinDist(int DataType);

public:

  SPRegisterPrediction(AST_INDEX,PedInfo,int,char**,
		       ScalarReplaceMap *Map,
		       SymDescriptor SymbolTable);

  ~SPRegisterPrediction()
    {
      delete ddg;
      delete Dist;
    }

  int GetFloatingPointRegisterPressure(RegisterPressureMethod Method);
  int GetIntegerRegisterPressure(RegisterPressureMethod Method);
  int GetII()
    {
      return II;
    }
};

#endif
