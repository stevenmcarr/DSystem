/* $Id: local_decomp2.C,v 1.5 1997/03/11 14:28:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//------------------------------------------------------------------------
// author : Seema Hiranandani
// contents : The functions in this file will be used to perform
//            local decomposition analysis when the fortran D compiler
//            is invoked from within PED on a single procedure
// date     : September '92
//------------------------------------------------------------------------

#include <stdio.h>
#include <libs/fortD/driver/driver.h>
#include <libs/fortD/misc/fd_code.h>

#if 0
#include <libs/support/database/OBSOLETE/AsciiDbioPort.h>
#endif

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
// #include <util++.h>
#include <libs/frontEnd/ast/treeutil.h>
// #include <DFFortD.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>

#define FD_TEMP "temp annotation"

//----------------------
// forward declarations

#if 0
static int dc_compute_local_decomp(AST_INDEX, int, Generic);

//---------------------------------------------------------------------
// this function may be called from C code. It performs local
// decomposition analysis for a program that contains a single
// procedure
// save the procedure information in the FD_Composition_HT hash table
//---------------------------------------------------------------------
void dc_compute_local(AST_INDEX node, PedInfo ped)
{
 walk_statements(node, LEVEL1, dc_compute_local_decomp, NULL, Generic(ped));
}

static int dc_compute_local_decomp(AST_INDEX node, int level, Generic(ped))
{

 AST_INDEX pnode;
 struct proc_in_module p;
 FD_Composition_HT *fortd_ht;
 FD_Reach_Annot *read_annot;

 if ((gen_get_node_type(node)) == GEN_PROGRAM)
 {
 fortd_ht = new FD_Composition_HT();
 fortd_ht->record(FortTree(PED_FT(PedInfo(ped))), 0, node, 0, PedInfo(ped));
 p.proc = node;
 p.name = ssave(gen_get_text(get_name_in_entry(node)));
 fortd_ht->add(p.name, p.proc, GEN_PROGRAM);

//-------------------------------------------------
// get the node that corresponds to the procedure

 FD_ProcEntry *entry = fortd_ht->GetEntry(p.name);
 pnode = entry->ast;

//-------------------------------------------------------
// add the read annotation to the FD_ProcEntry structure

 read_annot = new FD_Reach_Annot();
 entry->PutReadAnnotation(read_annot);
 
 fortd_ht->put_proc(entry);

// perform local decomposition analysis
// store the sp in the side array
 
 dc_compute_local_decomp(node, fortd_ht);
// dc_compile_proc(ped, node, 4);

 return(WALK_ABORT);
}

 return(WALK_CONTINUE);
}
#endif




