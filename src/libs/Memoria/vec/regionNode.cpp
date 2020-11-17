#include <iostream>
#include <libs/Memoria/vec/regionNode.h>
#include <libs/Memoria/vec/scc.h>

RegionNode::RegionNode()
{

    regionNum = numRegions++;
}

RegionNode::RegionNode(RegionNode &r)
{
    regionNum = numRegions++;
    addStmts(r.getStmts());
}

RegionNode::RegionNode(SCC* scc,int level)
{
    regionNum = numRegions++;
    for (std::list<AST_INDEX>::iterator it = scc->getNodes().begin();
        it != scc->getNodes().end();
        it++)
    {
        addStmt(*it);
    }
}

void RegionNode::addStmt(AST_INDEX stmt)
{

    stmts.push_back(stmt);
    get_stmt_info_ptr(stmt)->R = this;
}

void RegionNode::addStmts(std::list<AST_INDEX>& s)
{

    for (std::list<AST_INDEX>::iterator it = s.begin();
         it != s.end();
         it++)
        addStmt(*it);
}

void RegionNode::dumpRegion()
{
    cout << " Region " << regionNum << "\n\t Statements = ";
    for (std::list<AST_INDEX>::iterator it = stmts.begin();
         it != stmts.end();
         it++)
    {
        cout << get_stmt_info_ptr(*it)->stmt_num << " ";
    }

    cout << "\n";
}

std::list<AST_INDEX> *RegionNode::getStatementGroup(int level)
{
    std::list<AST_INDEX> *stmt_group = new std::list<AST_INDEX>();
    std::list<AST_INDEX>::iterator it = stmts.begin();
    bool this_level = get_stmt_info_ptr(*it)->level == level;
    while(it != stmts.end())
    {
        if (this_level)
            if (get_stmt_info_ptr(*it)->level == level)
            {
                stmt_group->push_back(*it);
                it = stmts.erase(it);
            }
            else
                break;
        else
            if (get_stmt_info_ptr(*it)->level > level)
            {
                stmt_group->push_back(*it);
                it = stmts.erase(it);
            }
            else
                break;
    }

    return stmt_group;
}