#ifndef ASTToDDGNodeMap_h  
#define ASTToDDGNodeMap_h  

#include <libs/Memoria/include/mh_ast.h>
#include <libs/support/tables/HashTable.h>

class DDGNode;

class ASTToDDGNodeMap : public HashTable
{
private:

  DDGNode **GraphNodes;

public:
  
  ASTToDDGNodeMap(void) : HashTable() {}
  ~ASTToDDGNodeMap(void)
    {
      delete GraphNodes;
    }

  void MapCreate(const uint initialSlots);
  void MapAddEntry(AST_INDEX node,DDGNode *Node);
  DDGNode *MapToDDGNode(AST_INDEX node);

};

#endif
  
  
