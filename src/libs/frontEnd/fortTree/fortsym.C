/* $Id: fortsym.C,v 1.2 2001/09/17 00:27:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

#include <libs/frontEnd/ast/forttypes.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>

#define DEBUG 1

typedef struct 
  {
     SymDescriptor d;
     Generic extra_arg;
     fst_ForAllCallback func;
  } fst_ForAll_arg_wrapper;

typedef struct 
  {
     SymDescriptor d;
     va_list extra_arg;
     fst_ForAllCallbackV func;
  } fst_ForAll_arg_wrapperV;

/* forward declarations */
/*    SUSPECT STATICS, NEED TO BE EXTERN'D */

STATIC(void, fst_ForAll_func, (SymTable t, int i, fst_ForAll_arg_wrapper* w));
STATIC(void, free_if_needed, (void* p));
STATIC(void, htfree_if_nonnull, (cNameValueTable ht));
STATIC(void, FreeSymDesc, (char* name, SymDescriptor d, cNameValueTable ht));
STATIC(void, DumpCommonBlock, (SymTable t, int i, SymDescriptor d));
STATIC(void, DumpCommonTable, (SymDescriptor d));
STATIC(void, DumpEquivalences, (SymTable t, int i, SymDescriptor d));
STATIC(void, DumpEquivTable, (SymDescriptor d));
STATIC(void, DumpSymDescriptorInfo, (char* name, SymDescriptor d, int dummy));
STATIC(void, dump_pair, (char* name, int value, int extra));
STATIC(void, DumpHashTable, (cNameValueTable SubPgmHt));


static int fst_DoModule_DumpFlag = 0;

void 
fst_Init(void)
{
  /* do nothing */
  return;
}

void 
fst_Fini(void)
{
  /* do nothing */
  return;
}

TableDescriptor 
fst_Open(Generic ft)
{
	TableDescriptor td;
	td = (TableDescriptor)get_mem(sizeof(struct TableDescriptor_t),
				      "TableDescriptor");

	assert(td != 0);

	td->ModuleSymsComputed = false;
	td->Errors = 0;
	td->NumberOfModuleEntries = 0; /* initialize count of entry points in module */
	td->FortTreePtr  = ft;

	td->SubPgmHt = 0;

	return td;
}


void fst_Recompute(TableDescriptor td)
{
  FortTreeTypeCheck(td, (FortTree)td->FortTreePtr);
  td->ModuleSymsComputed = true;
  if (fst_DoModule_DumpFlag) fst_DumpModuleTables(td);
}

void 
fst_Close(TableDescriptor td)
{
	if (td != (TableDescriptor) 0) {
		TableDescriptorHashTableFree(td);
		free_mem((void *) td);
	}
}


void 
fst_Save(TableDescriptor td, Context context, DB_FP* fp)
{
	/* do nothing - it's cheaper to recompute */
}

unsigned int 
fst_NumberOfModuleEntries(TableDescriptor td)
{
	/* make sure that symbols exist for the module */
	if (td->ModuleSymsComputed == false)
		fst_Recompute(td); /* compute if none already exist */
	return td->NumberOfModuleEntries;
}

SymDescriptor 
fst_GetTable(TableDescriptor td, char* name)
{
	SymDescriptor d;

	/* make sure that symbols exist for the module */
	if (td->ModuleSymsComputed == false)
		fst_Recompute(td); /* compute if none already exist */

	if (NameValueTableQueryPair(td->SubPgmHt, (Generic)name, (Generic*)&d)) return d;
	else return (SymDescriptor) 0;
}

fst_index_t 
fst_Index(SymDescriptor d, char* name)
{
	return (fst_index_t)SymIndex(d->Table, name);
}


fst_index_t 
fst_QueryIndex(SymDescriptor d, char* name)
{
	return (fst_index_t)SymQueryIndex(d->Table, name);
}

fst_index_t 
fst_MaxIndex(SymDescriptor d)
{
	return (fst_index_t)SymMaxIndex(d->Table);
}


Generic 
fst_GetFieldByIndex(SymDescriptor d, fst_index_t index, char* field)
{
	return (Generic)SymGetFieldByIndex(d->Table, (int)index, field);
}


Generic 
fst_GetField(SymDescriptor d, char* name, char* field)
{
	return (Generic)SymGetField(d->Table, name, field);
}


void 
fst_PutFieldByIndex(SymDescriptor d, fst_index_t index, char *field, Generic value)
{
	SymPutFieldByIndex(d->Table, (int)index, field, (int)value);
}


void 
fst_PutField(SymDescriptor d, char *name, char *field, Generic value)
{
	SymPutField(d->Table, (char*)name, field, (int)value);
}


void 
fst_InitField(SymDescriptor d, char* field, Generic init_value, SymCleanupFunc cleanup_fn)
{
	SymInitField(d->Table, field, (int)init_value, cleanup_fn);
}


void 
fst_KillField(SymDescriptor d, char* field)
{
	SymKillField(d->Table, field);
}


void 
fst_ForAll(SymDescriptor d, fst_ForAllCallback func, Generic extra_arg)
{
	fst_ForAll_arg_wrapper w;

	w.d = d;
	w.func = func;
	w.extra_arg = extra_arg;

	SymForAll(d->Table, (SymIteratorFunc)fst_ForAll_func, (Generic)&w);

  return;
}


void 
fst_ForAllV(SymDescriptor d, fst_ForAllCallbackV func, ...) 
{
  va_list argList;
  fst_ForAll_arg_wrapperV w;
  
  va_start(argList, func);
    {
	w.d = d;
	w.func = func;
	w.extra_arg = argList;

	SymForAll(d->Table, (SymIteratorFunc)fst_ForAll_func, (Generic)&w);
    }
  va_end(argList);

  return;
}


void
fst_Symbol_To_EquivLeader_Offset_Size(SymDescriptor d, fst_index_t index, 
                                      fst_index_t* leader, int* offset, 
                                      unsigned int* size)
{

	if (index == SYM_INVALID_INDEX || index >= d->EQmap.tablesize) { 
		/* not a valid symbol index, or refers to a symbol added to the
		 * table after equivalence processing 
		 */
		*leader = SYM_INVALID_INDEX;
		return;
	} else {
		fst_index_t lead = d->EQmap.LeaderTable[index].leader;

		/* N.B. assumes flattened equiv tree
		 * SYMTAB_EQ_OFFSET is 0 if not leaf in equiv tree.
		 */
		*offset = SymGetFieldByIndex(d->Table, index, SYMTAB_EQ_OFFSET);
		*leader = lead;
	}
	*size = (unsigned int) SymGetFieldByIndex(d->Table, index, SYMTAB_SIZE);
}

/* allocate and fill a nametable that contains a symbol table indices for
 * all symbols in the same equivalence class that overlap the specified
 * region 
 *
 * NOTE: size == FST_ADJUSTABLE_SIZE means size < 0; this is legal here.
 *       adjustable arrays are only legal as dummy arguments to a 
       fst_ForAll(d, func, (Generic)argList);
 *       function or entry which can only be in a singleton equivalence 
 *       class. the singleton will be correctly be returned as 
 *       overlapping. -- JMC 3/93
 */
EquivalenceClass* 
fst_Symbol_Offset_Size_To_VarSymbols(SymDescriptor d, fst_index_t leader, 
                                     int offset, int size)
{
	int eqvars = 0;
	int lb, ub;
	fst_index_t k;
	SymTable t = d->Table;

	EquivalenceClass* EqNTable;
	int eqntable_size;

	LeaderTable_t* LTable = d->EQmap.LeaderTable;
	int* EqNext = d->EQmap.EqChainNext;

	if (leader == SYM_INVALID_INDEX || leader >= d->EQmap.tablesize)
		return (EquivalenceClass* ) 0; /* not a valid index */


	k = LTable[leader].leader;

	assert(k != SYM_INVALID_INDEX);

	/* max # symbols we can report is the size of the equivalence class +
	 * the leader 
	 */
	eqntable_size = d->EQmap.LeaderTable[k].varcount + 1;
	EqNTable = (EquivalenceClass* ) get_mem(sizeof(EquivalenceClass) + 
		sizeof(fst_index_t) * (eqntable_size - 1), "EqNTable");
	
	/* adjust offset so it is with respect to "k" rather than "leader" */
	if (k != leader) 
		offset += SymGetFieldByIndex(t, leader, SYMTAB_EQ_OFFSET);

	lb = offset;
	ub = lb + size;

	/* report equivalences using equivalence chain table
	 *
	 * if leader is common block name, don't report it since it isn't a
	 * variable
	 */

	if (SymGetFieldByIndex(t, k, SYMTAB_OBJECT_CLASS) & 
		(OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) {
			EqNTable->member[eqvars++] = k;
	} else {
		if (SymGetFieldByIndex(t, k, SYMTAB_OBJECT_CLASS) & OC_IS_COMMON_NAME)
			k = EqNext[k];
		for(; k != SYM_INVALID_INDEX; k = EqNext[k]) {
			/* test for overlap ... i's lb < k's hb && k's lb < i's hb	*/
			/* 		assumes non-zero length for i and k		*/
			int eq_offset_for_k = SymGetFieldByIndex(t, k, SYMTAB_EQ_OFFSET);
			if ((eq_offset_for_k < ub) && 
					(lb < (eq_offset_for_k + 
						SymGetFieldByIndex(t, k, SYMTAB_SIZE)))) { 
				/* They overlap, so add it to the list */
				EqNTable->member[eqvars++] = k;
			}
		}
	}

	assert(eqvars <= eqntable_size);

	if (eqvars == 0) {
		/* no symbols in the equivalence class that overlap the offset, span
		 * pair. strange, but not always erroneous. 
		 */
		free_mem((void *)EqNTable);
		return 0;
	} else {
		EqNTable->members = eqvars;
		return EqNTable;
	}
}

/*********************************************************************/
/*  For a specific parameter or common block entry compute           */
/*  offset, type and name of the leader                              */
/*********************************************************************/
Boolean 
entry_name_symdesc_name_To_leader_offset_size_vtype(char* entry_name, SymDescriptor d,
                                                    char* name, char** leader, 
                                                    int* offset, int* size, 
						    unsigned int* vtype)
{
  fst_index_t name_index, leader_index;
  unsigned int type = 0;
  Boolean leader_set = false;
  int oc, sc, ocl, param_position, entry_index;
  cNameValueTable ht;

  name_index = fst_QueryIndex(d, name);
  fst_Symbol_To_EquivLeader_Offset_Size(d, name_index, &leader_index,
					offset, (unsigned int*)size);
  assert(fst_index_is_valid(leader_index));
  *leader = (char* ) fst_GetFieldByIndex(d, leader_index, SYMTAB_NAME);
  
   oc = fst_GetFieldByIndex(d, name_index, SYMTAB_OBJECT_CLASS);
  
      /* if symbol is a parameter, it must be a parameter of the current
       entry point, otherwise it is not included in mod/ref information
       (it is an error to reference a parameter of entry e1 in an 
       invocation of entry e2)
      */
  
  if (oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) {
    entry_index = fst_QueryIndex(d, entry_name);
    assert(fst_index_is_valid(entry_index));
    ht = (cNameValueTable) fst_GetFieldByIndex(d, entry_index, SYMTAB_FORMALS_HT);
    if (ht && NameValueTableQueryPair(ht, (Generic)name, (Generic*)&param_position)) {
      type |= VTYPE_FORMAL_PARAMETER;
    }
    else return false;
  }
  
  sc = fst_GetFieldByIndex(d, name_index, SYMTAB_STORAGE_CLASS);
  if (sc & SC_EXTERNAL) type |= VTYPE_PROCEDURE;
  else {
    ocl = fst_GetFieldByIndex(d, leader_index, SYMTAB_OBJECT_CLASS);
    if (ocl & OC_IS_COMMON_NAME) type |= VTYPE_COMMON_DATA;
    else type |= VTYPE_LOCAL_DATA;
  }
  *vtype = type;
  return true;
}

void 
fst_Dump(SymDescriptor d)
{
	DumpSymDescriptorInfo(d->name, d, 0);
}

void 
fst_Dump_Symbol(SymDescriptor d, fst_index_t index)
{
	SymDumpEntryByIndex(d->Table, index);
}

void
fst_DumpModuleTables(TableDescriptor td)
{
	NameValueTableForAll(td->SubPgmHt, 
                             (NameValueTableForAllCallback)DumpSymDescriptorInfo, 
                             (Generic)0);
	DumpHashTable(td->SubPgmHt);
}

unsigned int fst_NumEntryPoints(TableDescriptor td)
{
	return NameValueTableNumPairs(td->SubPgmHt);
}

/******************************/
/* local functions            */
/******************************/

static void 
fst_ForAll_func(SymTable t, int i, fst_ForAll_arg_wrapper* w)
{
	(*(w->func))(w->d, i, w->extra_arg);
}

static void 
free_if_needed(void* p)
{
	if (p) free_mem((void *)p); /* free structure */
}

static void 
htfree_if_nonnull(cNameValueTable ht)
{
	if (ht) NameValueTableFree(ht); /* free hash table */
}

/* boolean return code indicates name collision in hashtable */
Boolean
SymDescriptorHashTableAddEntryPoint(cNameValueTable SubPgmHt, char* proc, SymDescriptor d)
{
	Boolean collision = false;
	SymDescriptor old;

	/* increment reference count -- important for deallocation since
	 * entry points will have references to the same symbol table as the 
	 * name for their enclosing  procedure body
	 */
	d->reference_cnt++;
	if (NameValueTableAddPair(SubPgmHt, (Generic)proc, (Generic)d, (Generic*)&old)) {
		/* collision! -- two sub programs with same name --> error */
		/* free table from previous subprogram with this name */
		SymDescriptorFree(old);
		return true;
	}
	return false;
}

Boolean
SymDescriptorAlloc(TableDescriptor td, char* proc, SymDescriptor* d)
{
	SymDescriptor p;
	SymDescriptor old;

	p = (SymDescriptor) get_mem(sizeof(struct SymDescriptor_t), 
	"SymDescriptorAlloc(): SymDescriptor");
	assert (p != 0);

	p->name	= proc; /* no save; tree outlasts Symbol Table! */
	p->Errors = 0;
	p->parent_td = td;

	p->Table = SymInit(ST_INITIAL_NUM_SLOTS);
	assert (p->Table != 0);

	/* specify cleanup for field of dimension arrays */  
	SymInitField(p->Table, SYMTAB_DIM_BOUNDS, 0, (SymCleanupFunc)free_if_needed);
	/* initially nothing is in common */
	SymInitField(p->Table, SYMTAB_PARENT, SYM_INVALID_INDEX, (SymCleanupFunc)0);
	SymInitField(p->Table, SYMTAB_FIRST_NAME, SYM_INVALID_INDEX, (SymCleanupFunc)0);
	SymInitField(p->Table, SYMTAB_LAST_NAME, SYM_INVALID_INDEX, (SymCleanupFunc)0);
	SymInitField(p->Table, SYMTAB_FORMALS_HT, 0, (SymCleanupFunc)htfree_if_nonnull);

	p->implicits.IList = (ImplicitList) 0;
	p->reference_cnt = 0;
	p->dataIsStatic = false;
	SymDescriptorInitializeImplicits(p);	/* fill in the implicit table */

	/* initialize these fields here, for some odd reason */
	p->EQmap.LeaderTable = 0;
	p->EQmap.EqChainNext = 0;
	p->EQmap.tablesize = 0;

	*d = p;

	return SymDescriptorHashTableAddEntryPoint(td->SubPgmHt, proc, p);
}

void 
SymDescriptorFree(SymDescriptor p)
{
	ImplicitList first;

	p->reference_cnt--;
	if (p->reference_cnt > 0) return;
	SymKill(p->Table); /* free the Symbol Table */
	first = p->implicits.IList;	/* if it exists, walk it and free it ! */
	while(first != (ImplicitList)0) {
		ImplicitList second = first;
		first  = first->next;
		free_mem((void*)second);
	}
	free_if_needed((void*)p->EQmap.LeaderTable);
	free_if_needed((void*)p->EQmap.EqChainNext);
	free_mem((void*)p);
}

static void
FreeSymDesc(char* name, SymDescriptor d, cNameValueTable ht)
{
	SymDescriptorFree(d);
}

void 
TableDescriptorHashTableFree(TableDescriptor td)
{
	if (td->SubPgmHt != 0) {
		NameValueTableForAll(td->SubPgmHt, 
                                     (NameValueTableForAllCallback)FreeSymDesc, 
                                     (Generic)td->SubPgmHt);
		NameValueTableFree(td->SubPgmHt);
		td->SubPgmHt = 0;
	}
}

void
TableDescriptorHashTableClear(TableDescriptor td)
{
	TableDescriptorHashTableFree(td);
	td->SubPgmHt = NameValueTableAlloc(16, 
                                           (NameCompareCallback)strcmp, 
                                           (NameHashFunctCallback)hash_string);
}

void SymDescriptorInitializeImplicits(SymDescriptor p)
{
  register int  i;
  register char c;

  /* initialize the Implicit Table to default values */

	for (i=0;i<256;i++) {
		p->implicits.typedecl[i].seen = false;
		p->implicits.typedecl[i].type = TYPE_UNKNOWN;
	}

	for (c='a'; c<='h'; c++)
		p->implicits.typedecl[c].type = TYPE_REAL;

	for (c='A'; c<='H'; c++)
		p->implicits.typedecl[c].type = TYPE_REAL;

	for (c='i'; c<='n'; c++)
		p->implicits.typedecl[c].type = TYPE_INTEGER;

	for (c='I'; c<='N'; c++)
		p->implicits.typedecl[c].type = TYPE_INTEGER;

	for (c='o'; c<='z'; c++)
		p->implicits.typedecl[c].type = TYPE_REAL;

	for (c='O'; c<='Z'; c++)
		p->implicits.typedecl[c].type = TYPE_REAL;

	p->implicits.typedecl['$'].type = TYPE_REAL; 	
}

/********************************************/
/* Sym Descriptor Information Dump Routines */
/********************************************/


static void
DumpCommonBlock(SymTable t, int i, SymDescriptor d)
{
	int j;

	/* if symbol is not name of common block  we are done */
	if (!(SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS) & 
			OC_IS_COMMON_NAME)) 
		return;

	(void) fprintf(stderr,"Common block `%s' size = %d\n",
		SymGetFieldByIndex(d->Table, i, SYMTAB_NAME),  
		SymGetFieldByIndex(d->Table, i, SYMTAB_SIZE));

	/* print all variables in common block */
	j = SymGetFieldByIndex(d->Table, i, SYMTAB_FIRST_NAME);
	while (j != SYM_INVALID_INDEX) {
		(void) fprintf(stderr, "\t`%s' off = %d len = %d\n", 
			SymGetFieldByIndex(d->Table, j, SYMTAB_NAME), 
			SymGetFieldByIndex(d->Table, j, SYMTAB_EQ_OFFSET),
			SymGetFieldByIndex(d->Table, j, SYMTAB_SIZE));
		j = SymGetFieldByIndex(d->Table, j, SYMTAB_NEXT_COMMON);
	}
}


static void
DumpCommonTable(SymDescriptor d)
{
	(void) fprintf(stderr, "\nCOMMON BLOCKS:\n");
	SymForAll(d->Table, (SymIteratorFunc)DumpCommonBlock, (Generic)d);
}

static void
DumpEquivalences(SymTable t, int i, SymDescriptor d)
{
	int j;
	int oc = SymGetFieldByIndex(t, i, SYMTAB_OBJECT_CLASS);
	int sc = SymGetFieldByIndex(t, i, SYMTAB_STORAGE_CLASS);

	if (((oc & (OC_IS_DATA | OC_IS_COMMON_NAME)) && 
			(!(oc & (OC_IS_FORMAL_PAR | OC_IS_ENTRY_ARG)) && 
			!(sc & SC_CONSTANT))) ||
		((sc & SC_FUNCTION) && ((sc & SC_ENTRY) || 
                   (strcmp(d->name, (char*)SymGetFieldByIndex(t,i,SYMTAB_NAME)) == 0)))){ 
		EquivalenceClass *eqtable = 
			fst_Symbol_Offset_Size_To_VarSymbols(d, i, 0, 
				SymGetFieldByIndex(t,i,SYMTAB_SIZE));
		fprintf(stderr, "Symbol `%s': ", SymGetFieldByIndex(t,i,SYMTAB_NAME));

		if (eqtable == 0) fprintf(stderr, "no overlapping variables");
		else {
			int size = eqtable->members;
			for(j = 0; j < size; j++) {
				fprintf(stderr, "(`%s',%d,%d) ",
					SymGetFieldByIndex(t, eqtable->member[j], SYMTAB_NAME),
					SymGetFieldByIndex(t, eqtable->member[j], 
						SYMTAB_EQ_OFFSET),
					SymGetFieldByIndex(t, eqtable->member[j], 
						SYMTAB_SIZE));
			}
			free_mem((void *)eqtable);
		}
		fprintf(stderr, "\n");
	}
}


static void 
DumpEquivTable(SymDescriptor d)
{
	(void) fprintf(stderr, "\nEQUIVALENCE (symbol,offset,size) pair list \n");
	SymForAll(d->Table, (SymIteratorFunc)DumpEquivalences, (Generic)d);
}


static void
DumpSymDescriptorInfo(char* name, SymDescriptor d, int dummy)
{
	fprintf(stderr, "SYMBOL TABLE for %s\n", name);
	SymDump(d->Table);
	DumpCommonTable(d);
	DumpEquivTable(d);
}


static void
dump_pair(char* name, int value, int extra)
{
	fprintf(stderr,"\t%-20s\t0x%-8x\n",name,value);
}

static void
DumpHashTable(cNameValueTable SubPgmHt)
{
	fprintf(stderr,"\nMODULE HASH TABLE DUMP\n");
	fprintf(stderr,"\t%-20s\t%-10s\n","Entry Name","SymDescriptor");
	NameValueTableForAll(SubPgmHt, (NameValueTableForAllCallback)dump_pair,
                             (Generic)SubPgmHt);
}

Boolean 
fst_bound_is_const_lb(ArrayBound a)
{
  if (a.lb.type == constant)
  return true;

  else
  return false;

}

Boolean
fst_bound_is_const_ub(ArrayBound a)
{
  if(a.ub.type == constant)
  return true;

  else
  return false;
}
