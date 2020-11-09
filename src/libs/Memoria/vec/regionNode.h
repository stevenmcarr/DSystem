#ifndef region_node_h
#define region_node_h

#include <list> 
#include <libs/frontEnd/ast/ast.h>

class RegionNode;

#include <libs/Memoria/include/mh.h>

using namespace std;

class RegionNode 
{

    std::list< pair<AST_INDEX,int> > *stmts;
    int sccNum = -1;
    bool visited = false;
    int numStmts = 0;

    public:
        RegionNode();
        RegionNode(RegionNode &r);
        int getNumStmts() { return numStmts; }
        void addStmt(AST_INDEX stmt,int level);
        void addStmts(std::list< pair<AST_INDEX,int> > *s);
        std::list< pair<AST_INDEX,int> > *getStmts() { return stmts; }
        bool isVisited() { return visited; }
        void markVisited() { visited = true; }
        void clearVisited() { visited = false; }
        void setSCCNum(int n) { sccNum = n; }
        int getSCCNum() { return sccNum; }
};


#endif