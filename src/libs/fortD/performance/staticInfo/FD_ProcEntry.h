/* $Id: FD_ProcEntry.h,v 1.1 1997/03/11 14:28:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// Copied from above file in /rn/rn2/src/include/fort_d
// That file cannot be included after the Pablo include files because
// class Array is multiply defined. Fortunately, only the ipanalysis
// seems to use our class Array (that's in rarray.h).

#ifndef fort_d_driver_h

#include <libs/frontEnd/fortTree/fortsym.h>

class FD_Reach_Annot;			/* Cannot include header files */
class FD_Overlap_Annot;			/* because of Array name clash */

class FD_ProcEntry
{
  public:
      // a pointer to its ast number
    AST_INDEX      ast;

      // function, procedure or callsite
    unsigned int   type;

      // index of the module array that contains info for this proc
    int mcount;

    SymDescriptor   proc_sym_table;

      // annotation that contains all the decomposition info for the proc
    FD_Reach_Annot* proc_annot; 

    unsigned int hash(unsigned int size) ;

    int compare(FD_ProcEntry *e1);

    char* name(void) { return nm; };	/* defs in driver.h are local */

    void add_name(char *name) ;

    int mdcount(void) ;

      // store the read annotation
    void PutReadAnnotation(FD_Reach_Annot *annot, FD_Overlap_Annot *overlap_a);

    void PutReadAnnotation(FD_Reach_Annot *annot);

    SymDescriptor symtable(void) { return proc_sym_table; };

    AST_INDEX get_ast(void);

  private:
    char*  nm;    // name of procedure, function or callsite with c_id appended
};

#endif
