#ifndef AliasQuery_h
#define AliasQuery_h

/******************************************************************************
 * AliasQuery.h
 *
 *
 ******************************************************************************/


#ifndef general_h
#include <general.h>
#endif


/******************************************************************************
 * Boolean IPQuery_IsAliased
 * 
 * return true if the variable identified by the triple (vname, offset, length)
 * is in the aliased in procedure proc_name
 *
 ******************************************************************************/
EXTERN(Boolean, IPQuery_IsAliased, (Generic callGraph, char *proc_name, 
				    char *vname, int length, int offset,
				    Boolean IsGlobal));


#endif /* AliasQuery_h */
