#ifndef ASTToIntMap_h  
#define ASTToIntMap_h  

#include <libs/Memoria/include/mh_ast.h>
#include <libs/support/tables/HashTable.h>

class ASTToIntMap : public HashTable
{
private:

  int *Values;
  int MaxIndex;

public:
  
  ASTToIntMap(void) : HashTable() {}
  ~ASTToIntMap(void)
    {
      delete Values;
    }

  void MapCreate(const uint initialSlots);
  void MapAddEntry(AST_INDEX node,int val);
  int MapToValue(AST_INDEX node);

};

#endif
  
  
