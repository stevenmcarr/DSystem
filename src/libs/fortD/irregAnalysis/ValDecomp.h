/* $Id: ValDecomp.h,v 1.3 1997/03/11 14:28:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef ValDecomp_h
#define ValDecomp_h

/**********************************************************************
 * Information associated with value based decompositions.
 *
 * $Log: ValDecomp.h,v $
 * Revision 1.3  1997/03/11 14:28:36  carr
 * newly checked in as revision 1.3
 *
 * Revision 1.3  94/03/21  12:56:24  patton
 * fixed comment problem
 * 
 * Revision 1.2  94/02/27  20:15:07  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.2  1994/02/27  19:45:56  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.1  1994/01/18  19:54:31  reinhard
 * Initial revision
 *
 */

/**********************************************************************
 * Revision History:
 * $Log: ValDecomp.h,v $
 * Revision 1.3  1997/03/11 14:28:36  carr
 * newly checked in as revision 1.3
 *
 * Revision 1.3  94/03/21  12:56:24  patton
 * fixed comment problem
 * 
 * Revision 1.2  94/02/27  20:15:07  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.2  1994/02/27  19:45:56  reinhard
 * Tweaks to make CC happy.
 *
 * Revision 1.1  1994/01/18  19:54:31  reinhard
 * Initial revision
 *
 */

#ifndef fortranD_h
#include <libs/fortD/misc/FortD.h>
#endif

class NamedGenericTable;             // minimal external declaration
class IrrSymTab;                     // minimal external declaration
//#ifndef Str_ht_h
//#include <libs/fort_d/irreg/Str_ht.h>
//#endif

//#ifndef IrrSymTab_h
//#include <libs/fort_d/irreg/IrrSymTab.h>
//#endif

#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif

// Forward declarations
class ValDecomp;
class ValDecompInfo;

/**********************************************************************
 * Class ValDecompInfo declaration
 */
class ValDecompInfo
{
public:
  ValDecompInfo(IrrSymTab    *my_st,     // Constructor
		FortranDInfo *my_fd);
  ~ValDecompInfo();                      // Destructor

  // Access functions
  IrrSymTab   *getSt() const { return st; }
  CfgInstance getCfg() const { return cfg; }
  void putCfg(CfgInstance my_cfg) { cfg = my_cfg; }

  void       add_directive(DecEntry     *d, // Collect a directive
			   struct dc_id *id,
			   AST_INDEX    node);
  void       gencode_redists();                 // Generate remap code
  ValDecomp  *decompName2ValDecomp(const char* decomp_name);
  ValDecomp  *name2ValDecomp(const char* array_name);
  ValDecomp  *snode2ValDecomp(SNODE *sp);
  Boolean    is_reg_dist(AST_INDEX array_node);
  ValDecomp  *ref2ValDecomp(AST_INDEX node);

private:

  // Private fields:
  CfgInstance       cfg;
  FortranDInfo      *fd;
  IrrSymTab         *st;
  NamedGenericTable *str2vd;  // Value-based distributions, hashed by name
  //Str_ht       str2vd;  // Value-based distributions, hashed by name
};


/**********************************************************************
 * Class ValDecomp declaration
 */
class ValDecomp
{
public:
  ValDecomp(ValDecompInfo  *my_vdi,            // Constructor
	    const char     *my_decomp_name,
	    DecEntry       *my_d,
	    struct dc_id   *my_id,
	    AST_INDEX      my_directive_node);
  ~ValDecomp();                                // Destructor

  // Access functions:
  const char *getCnt_name()      const { return cnt_name; }
  const char *getLoc2glob_name() const { return loc2glob_name; }

  void       gencode_redist();         // Generate remap code
  void       parse_directive();        // Parse distribute directive
  void       decl_vars();              // Declare variables
  Boolean    reaches_ref(AST_INDEX node);

private:
  Boolean   needs_remap(fst_index_t index);
  void      gen_names();               // Generate names
  AST_INDEX gencode_map();             // Generate code for creating map
  AST_INDEX gencode_moves();           // Generate code for moving data

  // Private fields:
  CfgNodeId     cn;
  const char    *cnt_name;        // # of local elements
  DecEntry      *d;
  const char    *decomp_name;
  AST_INDEX     directive_node;
  struct dc_id  *id;
  const char    *loc2glob_name;   // local -> global index
  const char    *loc2proc_name;   // Local index -> proc # (before map)
  int           ndim;             // # of value dimensions
  int           orig_decomp_type; // Previous decomposition (1 = BLOCK)
  int           reg_elmnt_cnt;
  AST_INDEX     reg_elmnt_cnt_node;
  const char    *sched_name;      // Name of remapping schedule
  const char    *tab_name;        // Name of remapping table
  char          **val_names;      // Names of value arrays
  ValDecompInfo *vdi;
  Boolean       weighed;          // With workload info ?
  char          *weight_name;     // Name of weight array
};
#endif
