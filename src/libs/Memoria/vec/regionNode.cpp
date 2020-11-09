#include <iostream>
#include <regionNode.h>

RegionNode::RegionNode()
{

    regionNum = numRegions++;
    stmts = new std::list<pair<AST_INDEX, int> >();
}

RegionNode::RegionNode(RegionNode &r)
{

    addStmts(r.getStmts());
}

void RegionNode::addStmt(AST_INDEX stmt, int level)
{

    numStmts++;
    stmts->push_back(pair<AST_INDEX, int>(stmt, level));
    get_stmt_info_ptr(stmt)->R = this;
}

void RegionNode::addStmts(std::list<pair<AST_INDEX, int> > *s)
{

    stmts = new std::list<pair<AST_INDEX, int> >();

    for (std::list<pair<AST_INDEX, int> >::iterator it = s->begin();
         it != s->end();
         it++)
        addStmt(it->first, it->second);
}

void RegionNode::dumpRegion()
{
    cout << " Region " << regionNum << "\n\t Statements = ";
    for (std::list<pair<AST_INDEX,int> >::iterator it = stmts->begin();
         it != stmts->end();
         it++)
    {
        cout << get_stmt_info_ptr(it->first)->stmt_num << " ";
    }
}