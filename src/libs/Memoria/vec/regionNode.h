#ifndef region_node_h
#define region_node_h

#include <list> 
#include <libs/frontEnd/ast/ast.h>
#include <libs/Memoria/include/mh.h>

using namespace std;

class RegionNode 
{

    std::list< pair<AST_INDEX,int> > *stmts;

    public:
        RegionNode();
        RegionNode(RegionNode &r);
        void addStmt(AST_INDEX stmt,int level);
        void addStmts(std::list< pair<AST_INDEX,int> > *s);
        std::list< pair<AST_INDEX,int> > *getStmts() { return stmts; }
};


#endif