#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/ut/ASTToDDGNodeMap.h>
#include <libs/Memoria/ut/DDG.h>

STATIC(uint,Hash,(const void* entry, const uint size));
STATIC(int,Compare,(const void* e1, const void* e2));

void ASTToDDGNodeMap::MapCreate(const uint initialSlots)
{
  HashTable::Create(sizeof(AST_INDEX),initialSlots,Hash,NULL,
		    Compare,NULL);
  GraphNodes = new DDGNode*[initialSlots];
}

void ASTToDDGNodeMap::MapAddEntry(AST_INDEX node,DDGNode *graphNode)
{
  HashTable::AddEntry((void *)&node,NULL);
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  GraphNodes[Index] = graphNode;
}
  
DDGNode *ASTToDDGNodeMap::MapToDDGNode(AST_INDEX node)
{
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  return GraphNodes[Index];
}

uint Hash(const void* entry, const uint size)
{
  return (uint)(((*(AST_INDEX*)entry) >> 3)*7 ^ 
		((*(AST_INDEX*)entry) << 8*sizeof(long)- 3)) % size;
}

int Compare(const void* e1, const void* e2)
{
 return (*((AST_INDEX*)e1) != *((AST_INDEX*)e2));
}

