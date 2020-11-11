#ifndef scc_h
#define scc_h

#include <list>

#include <libs/frontEnd/ast/ast.h>

class SCC {
    std::list<AST_INDEX> nodes;

    public:

        SCC();
        void addNode(AST_INDEX stmt) { nodes.push_back(stmt); }
        std::list<AST_INDEX>& getNodes() { return nodes; }
        int size() {return nodes.size(); }
        void dumpSCC();
};

#endif