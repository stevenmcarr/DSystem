#ifndef ScalarReplaceMap_h  
#define ScalarReplaceMap_h  

#include <libs/Memoria/include/mh_ast.h>
#include <libs/support/tables/HashTable.h>

class DDGNode;

typedef struct scalarreplacmentdata{
  Boolean IsGenerator;
  Boolean IsReplaced;
} ScalarReplacementStatus;

class ScalarReplaceMap : public HashTable
{
private:

  ScalarReplacementStatus *ScalarReplacedStatus;

public:
  
  ScalarReplaceMap(void) : HashTable() {}
  ~ScalarReplaceMap(void)
    {
      delete ScalarReplacedStatus;
    }

  void MapCreate(const uint initialSlots);
  void MapAddEntry(AST_INDEX node,Boolean IsGenerator,Boolean IsReplaced);
  ScalarReplacementStatus& operator[](AST_INDEX node);

};

#endif
  
  
