#ifndef DDG_h
#define DDG_h

#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/support/graphs/directedGraph/DirectedGraph.h>
#include <libs/support/graphs/directedGraph/DirectedGraphIterators.h>
#include <libs/Memoria/include/la.h>
#include <libs/Memoria/annotate/AddressEquivalenceClassSet.h>
#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/Memoria/ut/ASTToDDGNodeMap.h>
#include <libs/Memoria/ut/ScalarReplaceMap.h>
#include <libs/frontEnd/fortTree/fortsym.h>


class DDG : public DirectedGraph {
  ASTToDDGNodeMap *ASTMap;
  
  int GetOperationLatency(AST_INDEX Node,DataReuseModel& ReuseModel,
			  ScalarReplaceMap& SRMap, PedInfo ped);

  void BuildDDG(AST_INDEX InnermostLoopStatementList,
		DataReuseModel& ReuseModel,
		AddressEquivalenceClassSet& AECS,
		ScalarReplaceMap& SRMap,
		PedInfo ped,
		int level);

  int AddGraphNodes(AST_INDEX Node, DDG *Graph);
  AstIterAdvanceDirective CountNodes(AST_INDEX Node, int& size);
  AstIterAdvanceDirective InitializeNodeMapping(AST_INDEX Node);
  void AddASTAndDependenceEdgesToDDG(AST_INDEX LoopBody,
				     PedInfo ped,
				     DataReuseModel& ReuseModel,
				     ScalarReplaceMap& SRMap,
				     int level);
  void AddAddressRegisterEdgesToDDG(AddressEquivalenceClassSet& AECS,
				    PedInfo ped);
  void CheckForIndexArrays(AST_INDEX Node);
  AstIterAdvanceDirective ProcessSubscripts(AST_INDEX Node);
  void AddArrayToMap(AST_INDEX Node);

public:

  DDG(AST_INDEX InnermostLoopStatementList,
      DataReuseModel& ReuseModel,
      AddressEquivalenceClassSet& AECS,
      ScalarReplaceMap& SRMap,
      PedInfo ped,
      int level) : DirectedGraph() 
    {
      Create();

      BuildDDG(InnermostLoopStatementList,ReuseModel,AECS,SRMap,ped,level);
    }

  ~DDG() 
    {
      ASTMap->Destroy();
      delete ASTMap;
    }


  ASTToDDGNodeMap *GetASTMap()
    {
      return ASTMap;
    }

  void DumpDDGInDOTForm(SymDescriptor SymbolTable);

};


class DDGNode : public DirectedGraphNode {

  AST_INDEX ASTNode;
  int Id;
  
public:
  
  DDGNode(AST_INDEX Node,DDG *Graph) :
    DirectedGraphNode(Graph)
    {
      ASTNode = Node;
      Id = Graph->NumberOfNodes()-1;
    }

  AST_INDEX GetASTNode() 
    {
      return ASTNode;
    }

  void SetASTNode(AST_INDEX Node)
    {
      ASTNode = Node;
    }

  int GetId()
    {
      return Id;
    }

  void GetText(char *Text,SymDescriptor SymbolTable);

};

typedef enum DDGEdgeType {TrueDep,AntiDep,OutputDep};

class DDGEdge: public DirectedGraphEdge {
  int Omega;
  int Latency;
  DDGEdgeType Type;

public:
  
  DDGEdge(DDG *Graph, DDGNode *Source, DDGNode *Sink,int w, int l,
	  DDGEdgeType t) :
    DirectedGraphEdge(Graph,Source,Sink) 
    {
      Omega = w;
      Latency = l;
      Type = t;
    }

  int GetOmega()
    {
      return Omega;
    }

  int GetLatency()
    {
      return Latency;
    }

  void SetOmega(int w)
    {
      Omega = w;
    }
  
  void SetLatency(int l)
    {
      Latency = l;
    }

  DDGNode *GetSource()
    {
      return (DDGNode*)DirectedGraphEdge::Src();
    }

  DDGNode *GetSink()
    {
      return (DDGNode*)DirectedGraphEdge::Sink();
    }

  DDGEdgeType GetType()
    {
      return Type;
    }
};


class DDGNodeIterator : public DirectedGraphNodeIterator
{
 public:

  DDGNodeIterator(DirectedGraph *dg, const 
		  TraversalOrder torder = Unordered, 
		  const DirectedEdgeDirection direction = DirectedEdgeOut) :
    DirectedGraphNodeIterator(dg,torder,direction)
    {}
  
  DDGNode *Current()
   {
     return (DDGNode *)DirectedGraphNodeIterator::Current();
   }
};

class DDGEdgeIterator : public DirectedGraphEdgeIterator
{
 public:

  DDGEdgeIterator(const DirectedGraphNode *node, 
		  DirectedEdgeDirection et) :
    DirectedGraphEdgeIterator(node,et)
    {}

  DDGEdge *Current()
    {
      return (DDGEdge *)DirectedGraphEdgeIterator::Current();
    }

};
  

#endif
