#include <scc.h>
#include <iostream>

#include <libs/Memoria/include/mh.h>

using namespace std;

SCC::SCC() {

}

void SCC::dumpSCC() {
    cout << "SCC: ";
    for (std::list<AST_INDEX>::iterator it = nodes.begin();
         it != nodes.end();
         it++)
        cout << get_stmt_info_ptr(*it)->stmt_num << " " ;

    cout << "\n";
}