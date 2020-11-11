#ifndef region_node_h
#define region_node_h

#include <list> 
#include <libs/frontEnd/ast/ast.h>
#include <libs/Memoria/vec/scc.h>

class RegionNode;

#include <libs/Memoria/include/mh.h>

using namespace std;

static int numRegions = 0;

class RegionNode 
{

    std::list<AST_INDEX> stmts;
    int sccNum = -1;
    bool visited = false;
    int regionNum=0;

    public:
        RegionNode();
        RegionNode(RegionNode &r);
        RegionNode(SCC* scc, int level);
        int getNumStmts() { return stmts.size(); }
        int getRegionNum() { return regionNum; }
        void addStmt(AST_INDEX stmt);
        void addStmts(std::list<AST_INDEX>& s);
        std::list<AST_INDEX>& getStmts() { return stmts; }
        bool isVisited() { return visited; }
        void markVisited() { visited = true; }
        void clearVisited() { visited = false; }
        void setSCCNum(int n) { sccNum = n; }
        int getSCCNum() { return sccNum; }
        void updateRegion(int level);
        void dumpRegion();
};


#endif