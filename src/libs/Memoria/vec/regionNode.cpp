#include <regionNode.h>

RegionNode::RegionNode() {

    stmts = new std::list< pair<AST_INDEX,int> >();
}

RegionNode::RegionNode(RegionNode &r) {

    addStmts(r.getStmts());
}

void RegionNode::addStmt(AST_INDEX stmt,int level) {

    stmts->push_back(pair<AST_INDEX,int>(stmt,level));
}

void RegionNode::addStmts(std::list< pair<AST_INDEX,int> > *s) {

    stmts = new std::list< pair<AST_INDEX,int> >(*stmts);
}
