#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/ut/ScalarReplaceMap.h>
#include <libs/Memoria/ut/DDG.h>

STATIC(uint,Hash,(const void* entry, const uint size));
STATIC(int,Compare,(const void* e1, const void* e2));

void ScalarReplaceMap::MapCreate(const uint initialSlots)
{
  HashTable::Create(sizeof(AST_INDEX),initialSlots,Hash,NULL,
		    Compare,NULL);
  ScalarReplacedStatus = new ScalarReplacementStatus[initialSlots];
}

void ScalarReplaceMap::MapAddEntry(AST_INDEX node,
				   Boolean   IsGenerator,
				   Boolean   IsReplaced)
{
  HashTable::AddEntry((void *)&node,NULL);
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  ScalarReplacedStatus[Index].IsGenerator = IsGenerator;
  ScalarReplacedStatus[Index].IsReplaced = IsReplaced;
}
  
ScalarReplacementStatus& ScalarReplaceMap::operator[](AST_INDEX node)
{
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  return ScalarReplacedStatus[Index];
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

