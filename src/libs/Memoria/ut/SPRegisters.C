#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/Memoria/ut/SPRegisters.h>
#include <libs/Memoria/ut/DDG.h>
#include <libs/Memoria/ut/MinDist.h>
#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/Memoria/annotate/AddressEquivalenceClassSet.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/ut/ScalarReplaceMap.h>
#include <libs/frontEnd/fortTree/fortsym.h>

extern Boolean DDGDebugFlag;
extern Boolean MinDistDebugFlag;

SPRegisterPrediction::SPRegisterPrediction(AST_INDEX Loop,
					   PedInfo p,
					   int l,
					   char **IVar,
					   ScalarReplaceMap *Map,
					   SymDescriptor SymbolTable)
{
  Ped = p;
  Level = l;
  SRMap = Map;
  LoopBody = gen_DO_get_stmt_LIST(Loop);

  // determine which array references require address registers

  AddressEquivalenceClassSet AECS(Loop,Level,IVar);
    
  for (int i = 0; i < TotalMethods; i++)
  {
    IntegerRegisterPressureComputed[i] = false;
    FloatingPointRegisterPressureComputed[i] = false;
    IntegerRegisterPressure[i] = 0;
    FloatingPointRegisterPressure[i] = 0;
  }

  IntOps = 0;
  Flops = 0;

  II = ComputeResII(AECS);

  // Compute Locality information
 
  la_vect LIS = la_vecNew(Level); // calloc so all values start at
				  // zero
  LIS[Level-1] = 1;
 
 
  UniformlyGeneratedSets UGS(Loop,Level,IVar,LIS);
 

  DataReuseModel ReuseModel(&UGS);
                                                                               

  // Compute MinDist Information for the loop DDG

  ddg = new DDG(LoopBody,ReuseModel,AECS,*SRMap,Ped,Level);

  if (DDGDebugFlag)
    ddg->DumpDDGInDOTForm(SymbolTable);

  Dist = new MinDist(ddg,II);

  if (MinDistDebugFlag)
    Dist->Dump();

  II = Dist->GetFinalII();

  la_vecFree(LIS);

}


int SPRegisterPrediction::ComputeResII(AddressEquivalenceClassSet& AECS)
{
  AST_INDEX ASTNode;
  AstIterAdvanceDirective WalkAdvance;
  

  // walk statements and compute loads and floating-point ops
  
  for (AstIterator ASTIter(LoopBody);
       ASTNode = ASTIter.Current();
       ASTIter.Advance(WalkAdvance))
    WalkAdvance = ComputeFlopsAndIntOps(ASTNode);
      
      
  if (!((config_type *)PED_MH_CONFIG(Ped))->AutoIncrement)
    IntOps += (AECS.GetSize()); 
    

  return (MAX(ceil_ab(IntOps,((config_type*)PED_MH_CONFIG(Ped))->IntegerUnits),
	      ceil_ab(Flops,((config_type*)PED_MH_CONFIG(Ped))->FPUnits)));    
}


//
//  Method: ComputeFlopsAndIntOps
//
//  Input: Node - AST node of operation on which to check the resource
//                requirements.
//
//  Output: Increase in number of issue slots required by operation
//
//  Description: Add to memory issue for array references and flops
//               for operators
//

AstIterAdvanceDirective 
SPRegisterPrediction::ComputeFlopsAndIntOps(AST_INDEX Node)
{

  if (is_subscript(Node))
  {
    // determine whether this is not scalar replaced to get cycle time
    
    if (!(*SRMap)[Node].IsReplaced)
	IntOps += 
	  (((config_type *)PED_MH_CONFIG(Ped))->hit_cycles /
	   ((config_type *)PED_MH_CONFIG(Ped))->IntegerPipeLength);

    return (AST_ITER_SKIP_CHILDREN);
  }
  else if (is_operator(Node))
    if (gen_get_converted_type(Node) == TYPE_REAL ||
        gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION)
      Flops += OperationIssueSlots(Node);
    else
      IntOps++;

  return(AST_ITER_CONTINUE);
  }   

//
//  Method: OperationIssueSlots
//
//  Input: Node - binary operator AST
//
//  Output: number of issue slots for an operator
//
//  Description: look up in configuration info how long a particular 
//               operation takes relative to the pipe length
//

int SPRegisterPrediction::OperationIssueSlots(AST_INDEX Node)
					  

{
  int ops;  // complex operations take more than 1 instruction
  
  if (!is_binary_times(Node) || 
      (!is_binary_plus(tree_out(Node)) && 
       !is_binary_minus(tree_out(Node))) ||
      !((config_type *)PED_MH_CONFIG(Ped))->mult_accum)
    if (gen_get_converted_type(Node) == TYPE_DOUBLE_PRECISION ||
	gen_get_converted_type(Node) == TYPE_COMPLEX ||
	gen_get_converted_type(Node) == TYPE_REAL)
    {
      if (gen_get_converted_type(Node) == TYPE_COMPLEX)
	ops = 2;
      else
	ops = 1;
      if (is_binary_times(Node))
	
	return((((config_type *)PED_MH_CONFIG(Ped))->mul_cycles * ops) /
	       ((config_type *)PED_MH_CONFIG(Ped))->FloatPipeLength);
      
      else if (is_binary_plus(Node) || is_binary_minus(Node))
	
	return((((config_type *)PED_MH_CONFIG(Ped))->add_cycles * ops) /
	       ((config_type *)PED_MH_CONFIG(Ped))->FloatPipeLength);

      else if (is_binary_divide(Node)) // not usually pipelined

	return(((config_type *)PED_MH_CONFIG(Ped))->div_cycles * ops);

      else
	return(1); 
    }
  return(0);
}

int
SPRegisterPrediction::GetIntegerRegisterPressure(RegisterPressureMethod Method)
{
  if (IntegerRegisterPressureComputed[(int)Method])
    return IntegerRegisterPressure[(int)Method];
  else
  {
    switch(Method)
    {
      case MinDistMethod:
	IntegerRegisterPressure[(int)Method] = ComputeMinDist(TYPE_INTEGER);
	break;
      
      case MinAvgMethod:
	IntegerRegisterPressure[(int)Method] = ComputeMinAvg(TYPE_INTEGER);
	break;

      case LiveAcrossMethod:
	IntegerRegisterPressure[(int)Method] = ComputeLiveAcross(TYPE_INTEGER);
	break;
    }
    IntegerRegisterPressureComputed[(int)Method] = true;

    return IntegerRegisterPressure[(int)Method];
  }
}

int
SPRegisterPrediction::GetFloatingPointRegisterPressure(RegisterPressureMethod
						       Method)
{
  if (FloatingPointRegisterPressureComputed[(int)Method])
      return FloatingPointRegisterPressure[(int)Method];
  else
  {
    switch(Method)
    {
      case MinDistMethod:
	FloatingPointRegisterPressure[(int)Method] = 
	  ComputeMinDist(TYPE_DOUBLE_PRECISION) +
	  ComputeMinDist(TYPE_REAL);
        break;
      
      case MinAvgMethod:
	FloatingPointRegisterPressure[(int)Method] = 
	  ComputeMinAvg(TYPE_DOUBLE_PRECISION) +
	  ComputeMinAvg(TYPE_REAL);
	FloatingPointRegisterPressureComputed[(int)Method] = true;
	break;

      case LiveAcrossMethod:
	FloatingPointRegisterPressure[(int)Method] = 
	  ComputeLiveAcross(TYPE_DOUBLE_PRECISION) +
	  ComputeLiveAcross(TYPE_REAL);
	break;
    }

    FloatingPointRegisterPressureComputed[(int)Method] = true;

    return FloatingPointRegisterPressure[(int)Method];
  }
  
}

int SPRegisterPrediction::ComputeMinAvg(int DataType)
{
  int SumMinLT = 0;
  DDGNode *ddgNode;
  DDGEdge *ddgEdge;

  for (DDGNodeIterator ddgNodeIter(ddg);
       ddgNode = ddgNodeIter.Current();
       ++ddgNodeIter)
  {
    AST_INDEX ASTNode = ddgNode->GetASTNode();
    Boolean Compute = false;

    if (is_subscript(ASTNode) && 
	gen_get_converted_type(ASTNode) == DataType)
      if ((*SRMap)[ASTNode].IsGenerator ||
	  NOT((*SRMap)[ASTNode].IsReplaced))
	Compute = true;
      else;

    // 
    // The ddg nodes for the name of an array are there to represent
    // address register pressure. So, we only count the register
    // pressure for TYPE_INTEGER. There will only be edges for the
    // first reference in the loop in the DDG (so address registers
    // are counted only once)
    //

    else if (is_subscript(tree_out(ASTNode)) && 
	     (DataType == TYPE_INTEGER))
      Compute = true;
    else if ((NOT(is_subscript(tree_out(ASTNode))) &&
	      gen_get_converted_type(ASTNode) == DataType))
      Compute = true;
    if (Compute)
    {
      int MinLT = 0;
      for (DDGEdgeIterator ddgEdgeIter(ddgNode,DirectedEdgeOut);
	   ddgEdge = ddgEdgeIter.Current();
	   ++ddgEdgeIter)
      {
	int LT = ddgEdge->GetOmega()*II +
	         (*Dist)[ddgNode->GetId()][ddgEdge->GetSink()->GetId()];
	MinLT = MAX(MinLT,LT);
      }
      SumMinLT += MinLT;
    }
  }
  
  return (ceil_ab(SumMinLT,II));
}
  
		    
int SPRegisterPrediction::ComputeLiveAcross(int DataType)
{
  int NumberLiveAcross = 0;
  DDGNode *ddgNode;
  DDGEdge *ddgEdge;

  for (DDGNodeIterator ddgNodeIter(ddg);
       ddgNode = ddgNodeIter.Current();
       ++ddgNodeIter)
  {
    AST_INDEX ASTNode = ddgNode->GetASTNode();
    Boolean Compute = false;

    if (is_subscript(ASTNode) && 
	gen_get_converted_type(ASTNode) == DataType)
      if ((*SRMap)[ASTNode].IsGenerator ||
	  NOT((*SRMap)[ASTNode].IsReplaced))
	Compute = true;
      else;

    // 
    // The ddg nodes for the name of an array are there to represent
    // address register pressure. So, we only count the register
    // pressure for TYPE_INTEGER. There will only be edges for the
    // first reference in the loop in the DDG (so address registers
    // are counted only once)
    //

    else if (is_subscript(tree_out(ASTNode)) && 
	     (DataType == TYPE_INTEGER))
      Compute = true;
    else if ((NOT(is_subscript(tree_out(ASTNode))) &&
	      gen_get_converted_type(ASTNode) == DataType))
      Compute = true;
    if (Compute)
    {
      int MinLT = 0;
      for (DDGEdgeIterator ddgEdgeIter(ddgNode,DirectedEdgeOut);
	   ddgEdge = ddgEdgeIter.Current();
	   ++ddgEdgeIter)
      {
	int LT = ddgEdge->GetOmega()*II +
	         (*Dist)[ddgNode->GetId()][ddgEdge->GetSink()->GetId()];
	MinLT = MAX(MinLT,LT);
      }
      if (MinLT >= II)
	NumberLiveAcross += ceil_ab(MinLT,II);
    }
  }
  
  return (NumberLiveAcross);
}


int SPRegisterPrediction::ComputeMinDist(int DataType)
{
  int SumOverlapLT = 0;
  int SumMinLT = 0;
  DDGNode *ddgNode;
  DDGEdge *ddgEdge;

  for (DDGNodeIterator ddgNodeIter(ddg);
       ddgNode = ddgNodeIter.Current();
       ++ddgNodeIter)
  {
    AST_INDEX ASTNode = ddgNode->GetASTNode();
    Boolean Compute = false;
    Boolean ComputeMinAvg = true; // use MinAvg for AST edges
                                  // as the register estimation

    if (is_subscript(ASTNode) && 
	gen_get_converted_type(ASTNode) == DataType)
      if ((*SRMap)[ASTNode].IsGenerator)
      {
	Compute = true;
	ComputeMinAvg = false;
      }
      else if (NOT((*SRMap)[ASTNode].IsReplaced))
	Compute = true;
      else;

    // 
    // The ddg nodes for the name of an array are there to represent
    // address register pressure. So, we only count the register
    // pressure for TYPE_INTEGER. There will only be edges for the
    // first reference in the loop in the DDG (so address registers
    // are counted only once)
    //

    else if (is_subscript(tree_out(ASTNode)) && 
	     (DataType == TYPE_INTEGER))
      Compute = true;
    else if ((NOT(is_subscript(tree_out(ASTNode))) &&
	      gen_get_converted_type(ASTNode) == DataType))
    {
      if (is_identifier(ASTNode))
	ComputeMinAvg = false;
      Compute = true;
    }
    if (Compute)
    {
      int MinLT = 0;
      for (DDGEdgeIterator ddgEdgeIter(ddgNode,DirectedEdgeOut);
	   ddgEdge = ddgEdgeIter.Current();
	   ++ddgEdgeIter)
      {
	int LT = ddgEdge->GetOmega()*II +
	         (*Dist)[ddgNode->GetId()][ddgEdge->GetSink()->GetId()];
	MinLT = MAX(MinLT,LT);
	  
      }

      if (ComputeMinAvg)

	// Use the MinAvg calculation for references that are not
	// scalar replaced
	
	SumMinLT += MinLT;
      else

	// compute the number of registers required due to overlapping of
	// software pipelining. We'll add in the number of scalar
	// replacement registers later.
	

      SumOverlapLT += (MAX(ceil_ab(MinLT,II) - 1,0));

    }
  }
  
  return (SumOverlapLT + ceil_ab(SumMinLT,II));
}
