/* $Id: FortD.h,v 1.13 1997/03/11 14:28:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************
* Fortran D specific initial local information                    *
* Author : Seema Hiranandani                                      *
* Date   : 5 Feb 92                                               *
*******************************************************************/
#ifndef fortranD_h
#define fortranD_h

#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef context_h
#include <libs/support/database/context.h>
#endif

#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

#if 0
#include <libs/support/lists/IOSinglyLinkedList.h>
#else
#include <libs/support/lists/IOSinglyLinkedList.h>
#endif

#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/include/expr.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/localInfo/FortDExpr.h>
#include <libs/fortD/localInfo/LocalOverlap.h>
#include <libs/fortD/localInfo/CommBlock.h>

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/StringBuffer.h>

// forward declarations
class FortranDHashTableEntry;
class FortranDInfo;

EXTERN(void, fd_store_param, (AST_INDEX param, FortranDInfo *f));

//----------------------------------------------------------------
// maps the type information stored in the global symbol table to
// the enumeration type FORM stored in the fortran d symbol table
//----------------------------------------------------------------
enum FORM form_type(int type);


struct dc_expr
{
   char str[DCMAX_LIST];
   int val;
   enum dc_expr_type type;
};

struct dc_subs
{
   int num;
   struct dc_expr expr[DCMAX_LIST];
};

struct dc_id
{
   char str[DCMAX_LIST];
   struct dc_subs subs;
   Boolean ident;
};

struct dc_list
{
   int num;
   struct dc_id id[DCMAX_LIST];
};

//-------------------------------------------------------------
// Distribute Information Access and Initialization Routines 
//-------------------------------------------------------------
void Typedescript(                                TYPEDESCRIPT& idtype);
void read_typedescript(FormattedFile& port,       TYPEDESCRIPT& idtype);
void write_typedescript(FormattedFile& port,      TYPEDESCRIPT& idtype);
void write_typedescript(FormattedFile& port, enum FORM, int,
                                                  TYPEDESCRIPT& idtype);
void copy_typedescript(TYPEDESCRIPT& copy_into,TYPEDESCRIPT& copy_from);
void typedescript_str(StringBuffer*, TYPEDESCRIPT&);
void typedescript_string(StringBuffer*, TYPEDESCRIPT*);
void Distinfo(DIST_INFO& distinfo);

//--------------------------
// forward declarations
//--------------------------
class AlignList;
class AlignEntry;

//--------------------------------------------------------------
// copy routines
//--------------------------------------------------------------
void copy_align_list(AlignList *copy_into, AlignList *copy_from);
// void copy_align_entry(AlignEntry *copy_into, AlignEntry *copy_from);

//---------------------------------------------------------------
// FDSetEntry:
// store decomposition entries for procedure parameters as sets
// ((decomp_name, align_index, distrib_index),(......), ......)
//---------------------------------------------------------------
class FDSetEntry : public SinglyLinkedListEntryIO
{
 char*  decomp_name;
 int align_index, distrib_index;

 public:
 int a_index(){
 return align_index;
 };

 int d_index() {
 return distrib_index;
 };
 
 char *name(){
 return decomp_name;
 };

 FDSetEntry(char* d_name=NULL, int a_indx=0, int d_indx=0)
 {
 decomp_name = ssave(d_name);
 align_index = a_indx;
 distrib_index = d_indx;
 }; 
 
 int WriteUpCall(FormattedFile& port);
 int ReadUpCall(FormattedFile& port);

};
 
//---------------------------------------------------------------
// FDSetEntryList:
// the singly linked list for decomposition set entries
//---------------------------------------------------------------
class FDSetEntryList: public SinglyLinkedListIO
{
  FDSetEntry *current;
  
public:
  FDSetEntryList() {current = 0;};
  virtual SinglyLinkedListEntryIO *NewEntry();
  
  void 
    append_entry (char *d, int a_indx, int d_indx) 
    {
      FDSetEntry *n = new FDSetEntry(d, a_indx, d_indx);
      SinglyLinkedList::Append((SinglyLinkedListEntry *)n);
    };
  
  FDSetEntry *first_entry();
  FDSetEntry *next_entry();
  int Read(FormattedFile&);
};



//---------------------------------------------------------
// DistribEntryArray:
// contains distribution information for a single entry
//---------------------------------------------------------
class DistribEntryArray {
 public:
  
  DIST_INFO distinfo[DC_MAXDIM];
  AST_INDEX node;
  enum ALIGN_DISTRIB_STATE state;
  Boolean marked;
  int number;
  int id_number;

  DistribEntryArray(){

  node = AST_NIL;
  state = ACTIVE;
  marked = false;
  number = 0;
  int i;
   for(i=0;i<DC_MAXDIM;++i){
    Distinfo(distinfo[i]);
	}
 };  

 void status(enum ALIGN_DISTRIB_STATE st){
 state = st;
 };
 
  int write(FormattedFile& port, int numdim);
  Boolean isEqual(DistribEntryArray *d, int numdim);
};

//---------------------------------------------------------
// AlignEntry:
// contains  alignment information for each dimension
//---------------------------------------------------------
class AlignEntry: public SinglyLinkedListEntryIO {
 SUBSCR_INFO s;
public: 

  AlignEntry
  (int indx=0, int off=0, int coef=0, 
   enum ALIGNTYPE type=ALIGN_UNKNOWN) 
	{ 

    s.stype = type;
    s.index = indx;
    s.offset = off;
    s.coeff = coef;
 	};

  SUBSCR_INFO* sub_info()
  {
  return &s;
  };

  Boolean isEqual(AlignEntry *e)
	{
   if ((s.stype == e->s.stype) &&
       (s.index == e->s.index) &&
       (s.offset == e->s.offset) &&
       (s.coeff == e->s.coeff))
    return true;
  else
    return false;
  }
  enum ALIGNTYPE stype()
	{ return s.stype; }
  int index()
 	{ return s.index; }
  int val()
	{ return s.offset; }

  int ReadUpCall(FormattedFile& port);
  int WriteUpCall(FormattedFile& port);
  friend void copy_align_entry(AlignEntry* copy_into, AlignEntry* copy_from);

};

//---------------------------------------------------------
// AlignList:
// contains alignment information for the array
// stored as a linked list. Each entry in the linked list
// is of class AlignEntry
//---------------------------------------------------------
class AlignList: public SinglyLinkedListIO {
 public:
 AlignEntry *current;
 Boolean perfect_align;
 int ndim;
 int distrib_index;
 enum ALIGN_DISTRIB_STATE state;
 AST_INDEX node; /* store the AST_INDEX of the align stmt */
 Boolean marked;
 int number;
 int id_number;

 AlignList() 
  { 
   current = 0;
   perfect_align = false;
   ndim = 0;
   distrib_index = 0; 
   state = ACTIVE; 
   node = AST_NIL; 
   marked = false; 
   number = 0;
  };

 void status(enum ALIGN_DISTRIB_STATE st){ state = st;};
 virtual SinglyLinkedListEntryIO *NewEntry();

 void 
 append_entry(int index, int offset, int coeff, enum ALIGNTYPE stype)
  {
  AlignEntry *e = new AlignEntry(index, offset, coeff, stype);
  SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
  };

 void
 append_entry(AlignEntry *a)
  {
	SinglyLinkedList::Append((SinglyLinkedListEntry *) a);
	}

  AlignEntry *first_entry() 
  { 
	return current = (AlignEntry *) SinglyLinkedList::First(); 
	};

  AlignEntry *next_entry() 
  { 
	return current = 
	(current ? (AlignEntry *) current->Next() : 0);
	};

 int Read(FormattedFile& port, int numdim);
 int Write(FormattedFile& port);
 Boolean isEqual(AlignList *a);
 void align_str(StringBuffer*, int);
};

void store_decomp_sp(FortranDHashTableEntry *d_entry);
//--------------------------------------------------------- 
// a name entry 
//---------------------------------------------------------
class NameEntry: public SinglyLinkedListEntry /* IO  */{
 public:
 char *array_name;

 NameEntry(char *a = NULL)
 {
  array_name = ssave(a);
 };
 
 char *name() {return array_name;};

#if 0
 int ReadUpCall(FormattedFile &);
 int WriteUpCall(FormattedFile &);
#endif

};

//---------------------------------------------------------
// a linked list of name entries
//---------------------------------------------------------

class NameList: public SinglyLinkedList /* IO */ {
 NameEntry *current;

public:
 NameList() {current = 0;};

 virtual SinglyLinkedListEntryIO *NewEntry();

 void append_entry(char *d) 
  {
  NameEntry *n = new NameEntry(d);
  SinglyLinkedList::Append((SinglyLinkedListEntry *)n);
  };

 NameEntry *first_entry() 
  { 
	return (current = ((NameEntry *)SinglyLinkedList::First())); 
	};

 NameEntry *next_entry() 
  { 
	return current = 
	(current ? (NameEntry *)current->Next() : 0);
	};


 Boolean final_entry()
  {
  if (current == 0)
   return true;
  else
   return false;
  };  

 void delete_entry(NameEntry* n);

 virtual void read(FormattedFile& ){};
 virtual void write(FormattedFile& ){};
};

//------------------------------------------------------------------------
// For each DECOMPOSITION store information about the decomposition     
// a list of arrays mapped to the decomposition                         
// For each ARRAY store information about the decomposition it is mapped
// to and specifics on the alignment mapping                            
//------------------------------------------------------------------------
class DecEntry : public SinglyLinkedListEntryIO {

public :
 int current_index;
 int dist_index, d_index, a_index, n_index;
 int output_number;  /* used during interprocedural propagation during write */
 char *dec_name;    /* name of the decomposition if array */
 char *unique_name; /* used during interprocedural propagation during write */
 TYPEDESCRIPT idtype[DC_MAXDIM];
 DistribEntryArray *distrib_info[DCMAX];
 AlignList  *align_info[DCMAX];
 NameList *name_info;
 int distrib_index[DCMAX];
 SNODE sp[DCMAX][DCMAX];
 int decomp_id_number;
#if 0
 Context decomp_context;
#endif
 DecEntry(enum FORM formm=UNKNOWN); 

 void AddAlignIdNumber(AST_INDEX, FortTree);
 void AddDistribIdNumber(AST_INDEX, FortTree);
 void AddDecompIdNumber(AST_INDEX, FortTree);
#if 0
 void AddDecompContext(Context);
#endif
 void AddDecEntry(struct dc_id id, int numdim);

 void  StoreDistribPattern(struct dc_subs *subs, AST_INDEX node);

 void  InitAlignInfo(int numdim, AST_INDEX node);

 void  AddAlignInfo(int numdim, int dim_num, int index, int offset, int coeff, 
                    enum ALIGNTYPE stype, AST_INDEX node);

 void  AddAlignInfo(int numdim, AST_INDEX node);

 void  AddAlignInfo(int dim_num, int index, int offset, int coeff, enum
                    ALIGNTYPE stype);

 void  AddDecompNameList(struct dc_list *arrays);
 void  AddDistribIndex(FortranDInfo *fd);
 void AddBounds(int dim, int up, int lb, Expr_type e_lb, Expr_type e_up );
 AlignList *get_align_list() { return align_info[0]; };

 int ReadUpCall(FormattedFile&) { return 0; };
 int WriteUpCall(FormattedFile&) { return 0; };

 void read(FormattedFile& port, enum FORM fform, int numdim);
 void write(FormattedFile& port, enum FORM fform, int);
 void write(FortranDInfo*, FormattedFile& port, enum FORM fform);
};

//---------------------------------------------------------
// DecList:
// a list of decompositions                                
//---------------------------------------------------------
class DecList: public  SinglyLinkedListIO {
DecEntry *current;
public:

DecEntry *d;
  
 SinglyLinkedListEntryIO *NewEntry(); 

 DecEntry*  append_entry(enum FORM formm) {
  DecEntry *e = new DecEntry(formm);
  SinglyLinkedList::Append(e);
  return(e);
  };

 DecEntry *first_entry() {
   return (d = (DecEntry *) SinglyLinkedList::First());
 }; 

 DecEntry *next_entry(){
  return d = 
  (d ? (DecEntry *)d->Next() : 0);
 };

 virtual void read(FormattedFile& ) {};
 virtual void write(FormattedFile& ){};
};

//---------------------------------------------------------
// FortranDHashTableEntry:
// contains information about an array or decomposition
//---------------------------------------------------------
class FortranDHashTableEntry {
  private:
       
    char* nameD;        // name of either array or decomposition

  public:
    enum FORM fform;    // if decomp then = Decomptype
    enum FORM type;     // type = integer, real, double precision ....

    int numdim;         // previously not commented
    char* name_unique;  // previously not commented

       // variables used for the interprocedual phase
    DecEntry* d;        // previously not commented
    Boolean marked;     // used during the interprocedural phase
    int number;         // previously not commented

    FortranDHashTableEntry();
    ~FortranDHashTableEntry();

    void init(enum FORM form, char *name, int dim);
    unsigned int hash(unsigned int size);
    int compare(FortranDHashTableEntry  *f);

    char* name();
    void add_name(char* name);
    int getdim();

    SNODE* sp(int align_index, int dist_index);

       // store the array type */
    void  put_form(enum FORM fform1);
 
    void write(FortranDInfo* , FormattedFile& port);
    void read(FormattedFile& ){};

    StringBuffer* typedescript_string(int dim);
    StringBuffer* align_with_string(int, FortranDHashTableEntry*);
    StringBuffer* align_string(int, char*);
    StringBuffer* distrib_string(int);
};

// forward declaration
class FortranDInfo;
class CallSite;
class CallSiteList;
/*********************************************************/
/* DecompList derived from SinglyLinkedList base class   */
/*********************************************************/

class DecompListEntry: public SinglyLinkedListEntry {
private:
 AST_INDEX invocation_node;
 enum FDtype d_type;
 NameList *nlist;
 AST_INDEX if_stmt;
 char *decomp_name;


public:
 int id_number;
 DecompListEntry(AST_INDEX node = AST_NIL, enum FDtype dt = NODEC)
 {
 invocation_node = node;
 d_type = dt;
 if_stmt = get_if(node);
 nlist = new NameList();
 decomp_name = NULL;
 };

   AST_INDEX node() {return invocation_node;};
   enum FDtype type() {return d_type;};
   AST_INDEX if_st() {return if_stmt;};
   AST_INDEX get_if(AST_INDEX node);
   void add_namelist(char *str);
   void add_decomp_name(char *nme);
   NameList* intersect(NameList *n);

   NameList* get_namelist(){
    return(nlist);
   };

   char *name() {return decomp_name;};

   void invalidate(FortranDInfo *f, NameList* n);
   void invalidate(FortranDInfo *f);

   void delete_namelist(FortranDInfo *fd, NameList*);

   Boolean compare(char* nme){
   if (!strcmp(decomp_name, nme))
    return true; 
   else
    return false;
 };
};
class DecompList: public SinglyLinkedList {
public:
   DecompListEntry *current;

   DecompList(){ current = 0;};

   virtual SinglyLinkedListEntry* NewEntry() {
	 return (SinglyLinkedListEntry*) new DecompListEntry;
   };

   void append_entry(AST_INDEX node, enum FDtype c_type){
   DecompListEntry *e = new DecompListEntry(node,c_type);
   SinglyLinkedList::Append(e);
   };

   void append_entry(DecompListEntry *d){
   SinglyLinkedList::Append(d);
	 };

   DecompListEntry *first_entry(){
   return current = (DecompListEntry *) SinglyLinkedList::First();
   };

   DecompListEntry *next_entry(){
     return current = (current ? (DecompListEntry*)current->Next() : 0);
   };

   void delete_entry(DecompListEntry* d){
   if (current==d){
    current = (current ? (DecompListEntry*)current->Next() : 0); }
	  SinglyLinkedList::Delete(d);
	};

};

//---------------------------------------------------------
// A hash table for storing DECOMPOSITION, DISTRIBUTE  
// ALIGNMENT and overlap information                           
//---------------------------------------------------------
class FortranDInfo : private HashTable {
  private:
    int current_index;
    DecompList *dlist;
    char *str_t;

  public:
    DecompList *getDlist() const { return dlist; }
    FortTree ft;            
    Context contxt;        
    OverlapList *overlap_info;            // used to store overlap information 
    Boolean  def_numprocs;                // n$proc defined?
    int numprocs;                         // if yes it's value.
    NameList *namealist, *namedlist ;
    CommonBlockList *common_block_info;   // list of arrays in common blocks that
                                          // have been distributed

    common_block_list *common_blks_decl;  // list of all common blocks declared in
                                          // the subroutine

    enum FDtype comment_type;

    uint HashFunct(const void* entry, const unsigned int size);
    int  EntryCompare(const void* entry1, const void* entry2);

    FortranDInfo() : HashTable ()
    {
    HashTable::Create(sizeof(FortranDHashTableEntry), 8);
    current_index = 0;
    def_numprocs = false;
    numprocs = 0;
    dlist = new DecompList();
    namedlist = new NameList();
    namealist = new NameList();
    overlap_info = new OverlapList();          // store the overlap information
    common_block_info = new CommonBlockList(); 
    common_blks_decl = new common_block_list();
    };

    FortranDInfo(FortTree ftree, Context c) : HashTable ()
    {
    HashTable::Create(sizeof(FortranDHashTableEntry), 8);
    current_index = 0;
    def_numprocs = false;
    numprocs = 0;
    ft = ftree;
    contxt = c;
    dlist = new DecompList();
    namedlist = new NameList();
    namealist = new NameList();
    overlap_info = new OverlapList();          // store the overlap information
    common_block_info = new CommonBlockList(); 
    common_blks_decl = new common_block_list();
    };

//----------------------------------------------------------------------
// routines to handle accessing the hash table

  FortranDHashTableEntry *AddDecomp(char *name, enum FORM fform, int dim);
  FortranDHashTableEntry *GetEntry(char* name);
  enum Dist_type GetDistributeType(char *array, int dim);

//----------------------------------------------------------------------
// routines to store decomposition, align and distribute information
   
  Boolean FortranD_dir(AST_INDEX);
  void StoreDecomposition(AST_INDEX node);
  void StoreAlign(SymDescriptor sym_t, AST_INDEX node, AST_INDEX proc_node);
  void StoreDistrib(AST_INDEX node, Boolean in_codegen);
  void StoreReachDecomp(CallSite* c_entry);
  void StoreFortranD(AST_INDEX node, AST_INDEX proc_node, SymDescriptor sym_t,
                      Boolean flag);
  void check_align_array(struct dc_id *id, SymDescriptor sym_d, AST_INDEX proc_node);
  void check_align_decomp(struct dc_id *id, struct dc_list *arrays, AST_INDEX node);

//----------------------------------------------------------------------
// routines to store the decompositions reaching actual parameters at the
// callsites

  void AddSetInfo(FortranDHashTableEntry *fd,
                  FDSetEntryList *fortd_set);
  void WriteCallInfo(IPinfoTreeNode*, FormattedFile& port);
  void ReadCallInfo(IPinfoTreeNode*, FormattedFile& port);

//----------------------------------------------------------------------
// routines to handle common block variables that have been distributed

  void CheckAndStoreCommonBlock(char*, SymDescriptor, char* entry_name);
  void WriteGlobalsCallInfo(CallSite *,  FormattedFile& port);
  void ReadGlobalsCallInfo(CallSite *,  FormattedFile& port);
  void StoreReachDecompGlobals(CallSite *c);


//----------------------------------------------------------------------
// routine to parse and store common block declarations

  void ParseAndStoreCommon(AST_INDEX node, char* entry_name, SymDescriptor d);

//----------------------------------------------------------------------
// routines to handle overlap computation

  void StoreOverlap(AST_INDEX node, OverlapList *info);
  void ComputeOverlapProgram(AST_INDEX node);
  void WriteOverlapInfo(FormattedFile& port)
  {
   overlap_info->Write(port);
  };

  void ReadOverlapInfo(FormattedFile& port)
  {
  overlap_info->Read(port);
  };

 int Read(FormattedFile &);
 int Write(FormattedFile &);

//------------------------------------------------------------------
// routine stores context and fort tree to be used for obtaining 
// unique ids for decomposition statements

  void ContextFt(Context, FortTree);

};

#endif
