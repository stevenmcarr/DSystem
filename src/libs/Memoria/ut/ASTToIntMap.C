#include <libs/Memoria/include/ASTToIntMap.h>

STATIC(uint,Hash,(const void* entry, const uint size));
STATIC(int,Compare,(const void* e1, const void* e2));

void ASTToIntMap::MapCreate(const uint initialSlots)
{
  HashTable::Create(sizeof(AST_INDEX),initialSlots,Hash,NULL,
		    Compare,NULL);
  Values = new int[initialSlots];
}

void ASTToIntMap::MapAddEntry(AST_INDEX node,int val)
{
  HashTable::AddEntry((void *)&node,NULL);
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  Values[Index] = val;
  MaxIndex++;
}
  
int ASTToIntMap::MapToValue(AST_INDEX node)
{
  int Index = HashTable::GetEntryIndex((const void*)&node); 
  return Values[Index];
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

