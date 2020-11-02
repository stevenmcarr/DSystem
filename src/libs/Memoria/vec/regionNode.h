#ifndef region_node_h
#define region_node_h

#include <list> 
#include <libs/frontEnd/ast/ast.h>

using namespace std;

class RegionNode 
{

    list<AST_INDEX> *stmts;

    public:
        RegionNode();
        void addStmt(AST_INDEX stmt);
        const list<AST_INDEX> *getStmts() { return stmts; }
};


#endif