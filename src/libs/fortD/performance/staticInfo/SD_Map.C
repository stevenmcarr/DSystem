/* $Id: SD_Map.C,v 1.4 1997/06/24 17:40:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// -*- C++ -*-
static const char * RCS_ID = "$Id: SD_Map.C,v 1.4 1997/06/24 17:40:32 carr Exp $";
#define MKASSERT
#define ASSERT_FILE_VERSION RCS_ID

#include <stream.h>
#include <include/rn_varargs.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/support/msgHandlers/log_msg.h>

#include <libs/frontEnd/ast/map.h>

// Written by Mark Anderson 6/15/94
// This does the mapping of statements to lines. It also adds a
// pointer which points to the instrumentation information for this record.
// most of this code blatantly stolen from john m-c and appropriated
// for my use.  

#include <stdlib.h>
#include <stdio.h>

#include <libs/support/misc/general.h>

#include <libs/support/database/context.h>
#include <libs/support/database/newdatabase.h>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/ipAnalysis/interface/IPQuery.h>

#include <libs/frontEnd/ast/ftxform.h>
#include <libs/frontEnd/ast/ftxannot.h>
#include <libs/frontEnd/ast/ftx_stdgraph.h>



#include <libs/frontEnd/ast/AstIterators.h>
#include <libs/frontEnd/ast/ftExportSimple.h>
#include <libs/frontEnd/prettyPrinter/sigcomments.h>

#include <libs/fortD/performance/staticInfo/SD_Map.h>
#include <libs/fortD/performance/staticInfo/MkAssert.h>

// Do not use this, since we should cleanup and destroy the sidearray
// right around dc_compile  
//static FortTreeSideArray SD_MapData;

static const int SD_SIZE_MAP_SIDEARRAY = 4;

static const int SD_START_LINE_INDEX = 0;
static const int SD_END_LINE_INDEX = 1;
static const int SD_NODE_TEXT_INDEX = 2;
static const int SD_INSTRUMENT_INFO_INDEX = 3;
//static const int SD_START_CHAR_INDEX = 1;
//static const int SD_END_CHAR_INDEX = 3;

///////////////////////////////////////////////////////////////////////
// Add mapping info for one statement
///////////////////////////////////////////////////////////////////////
void SDDF_SideArray::SD_MapNode(FortTreeNode node) {

  int l1, l2, c1, c2;

  ftt_NodeToText(myFtt, node, &l1, &c1, &l2, &c2);
  
  ft_PutToSideArray(md, node, SD_START_LINE_INDEX, l1);
  ft_PutToSideArray(md, node, SD_END_LINE_INDEX, l2);
 
  PabloSideArrayInfo * sideArrayRec = new PabloSideArrayInfo(l1,l2);
  KeepSideArrayRecPointer(sideArrayRec);
  sideArrayRec->SetStartChar(c1);
  sideArrayRec->SetEndChar(c2);
  
  if (mapText && is_statement(node)) {
    char *stmt_text = ftt_GetTextLine(myFtt, l1);
    ft_PutToSideArray(md, node, SD_NODE_TEXT_INDEX, (Generic) stmt_text);
    sideArrayRec->SetTextPtr(stmt_text);
    // WriteNormalizedLineToFile(stmt_text);
  }

  // This is the only non-depricated portion of the set side array
  ft_PutToSideArray(md,node,SD_INSTRUMENT_INFO_INDEX,(Generic) sideArrayRec);
}

void SDDF_SideArray::KeepSideArrayRecPointer(PabloSideArrayInfo* sideArrayRec)
{
    assert(sideArrayRec != (PabloSideArrayInfo*) NULL);
    saRecPtrVector.Append(sideArrayRec);
}
void SDDF_SideArray::FreeSideArrayRecords()
{
    PabloSideArrayInfo* sideArrayRec;
    int vectorSize = saRecPtrVector.NumberOfEntries(); 
    for (int i = 0; i < vectorSize; i++) {
	sideArrayRec = (PabloSideArrayInfo*) saRecPtrVector[i];
	assert (sideArrayRec != (PabloSideArrayInfo *) NULL);
	if (sideArrayRec->GetTextPtr() != (char*) NULL)
	    free_mem(sideArrayRec->GetTextPtr()); // alloc in ftt_GetTextLine
	delete sideArrayRec;
    }
}

///////////////////////////////////////////////////////////////////////
// Build line mapping info for a FortTree
///////////////////////////////////////////////////////////////////////
// very similar to ftx_create_mapData, since I stole all the code from it.
// Runs around and builds the side array for line number mapping data
// for the current fort tree.
SDDF_SideArray::SDDF_SideArray(char* moduleFileName,
			       AST_INDEX root,
			       FortTree ft,
			       FortTextTree ftt, 
			       Boolean MapText)
{
  theRoot 	= root;
  myFt   	= ft;
  myFtt		= ftt;
  mapText       = MapText;
  normOutFile   = (FILE*) NULL;

  // For the curious, in order: beg line, end line,
  // 				text string of node, sddf info record.
  static Generic initial[SD_SIZE_MAP_SIDEARRAY] = {-1,-1, 0, 0}; 
  md = ft_AttachSideArray(myFt, SD_SIZE_MAP_SIDEARRAY, initial);
  
  // Do work of mapping
  
  /* ftt must be updated prior to mapStmt invocations */
  // Is this necessary? Ask JMC
  // ftt_TreeChanged(ftt, ft_Root(ft));

  AST_INDEX node;
  //AstIterator tree_walk(theRoot, PreOrder, AST_ITER_STMTS_ONLY);

  // Walk the full tree, mapping statements and individual identifiers
  // use the same mapping routines for both
  
  AstIterator tree_walk(theRoot, PreOrder, AST_ITER_FULL_WALK);
  for ( ; node = tree_walk.Current(); ++tree_walk) {
    if (is_statement(node) || is_identifier(node))
	SD_MapNode(node);
  }

  WriteNormalizedFile(moduleFileName);
}

///////////////////////////////////////////////////////////////////////
// Cleanup
///////////////////////////////////////////////////////////////////////
SDDF_SideArray::~SDDF_SideArray() {
  FreeSideArrayRecords();
  ft_DetachSideArray(myFt, md);
}

///////////////////////////////////////////////////////////////////////
// Dump linenos and text to stdout. does nothing if no
///////////////////////////////////////////////////////////////////////
void SDDF_SideArray::Dump() {
  int line, linetwo;
  char * text;
  if (mapText) {
    AST_INDEX stmt;
    AstIterator tree_walk(theRoot, PreOrder, AST_ITER_STMTS_ONLY);
    for ( ; stmt = tree_walk.Current(); ++tree_walk) {
      line = (int)(ft_GetFromSideArray(md,stmt,SD_START_LINE_INDEX));
      text = (char *)(ft_GetFromSideArray(md,stmt,SD_NODE_TEXT_INDEX));
      linetwo = ((PabloSideArrayInfo *)
	(ft_GetFromSideArray(md,stmt,SD_INSTRUMENT_INFO_INDEX)))
	->GetStartLine();
      cout << '(' << line << ',' << linetwo << ") :" << text << endl;
    }
  } else {
    cout << "Cannot dump, no text was mapped\n";
  }
}

StaticDescriptorBase* SDDF_SideArray::getSDDFDescriptor(AST_INDEX node) {
  PabloSideArrayInfo * p = (PabloSideArrayInfo *)
    ft_GetFromSideArray(md, node, SD_INSTRUMENT_INFO_INDEX);
  MkAssert(p !=0, "Attempt to reference undefined SideNode",EXIT);
  return p->Get();
}

PabloSideArrayInfo * SDDF_SideArray::getInfo(AST_INDEX node) {
  return (PabloSideArrayInfo *)
    ft_GetFromSideArray(md, node, SD_INSTRUMENT_INFO_INDEX);
}

void SDDF_SideArray::setInfo(AST_INDEX node, PabloSideArrayInfo * p) {
  PabloSideArrayInfo * oldP =  (PabloSideArrayInfo *)
        ft_GetFromSideArray(md, node, SD_INSTRUMENT_INFO_INDEX);
  if (oldP != 0) {
    delete oldP;
  }
  ft_PutToSideArray(md, node, SD_INSTRUMENT_INFO_INDEX, (Generic) p);
}
   
int SDDF_SideArray::getLine(AST_INDEX node) {
  return (int)ft_GetFromSideArray(md,node,SD_START_LINE_INDEX);
}


////////////////////////////////////////////////////////////////////////////
// Routines to open, close and write out the module file in normalized form.
// A normalized file has line numbers and char positions that are consistent 
// with the line number, char position information within the compiler, and
// in particular, does not use continuation lines.
// A RUDE but unavoidable hack, without major changes to the front end.
////////////////////////////////////////////////////////////////////////////

// OpenNormalizedOutFile(moduleFileName)
//
// Compute name of normalized output file and open it for writing
// Currently use: .NORM.moduleFileName as the normalized file corresponding
// to [<some_path>/]moduleFileName.  I think it should be in the current
// directory, not necessary in the same directory (<somepath>) as the 
// original source file because this is more consistent with the FortD
// compiler, which places the output .fortd.f file in the current directory.
// This way, all files output by the Fortran D compiler are in the current
// working directory.

#define NORM_FILE_PREFIX ".NORM."

void SDDF_SideArray::OpenNormalizedOutFile(char* modulePathName)
{
    assert(modulePathName != (char*) NULL);
    assert(normOutFile == (FILE*) NULL); // Check any previous file closed.
    
    int i, fileNameStart, normFileNameLen;
    int moduleNameLen = 1 + strlen(modulePathName);
    
    // Extract start index of trailing path name component
    for (fileNameStart = 0, i=0; i < moduleNameLen-1; i++)
	if ((char) modulePathName[i] == '/')
	    fileNameStart = i+1;
    
    normFileNameLen = strlen(NORM_FILE_PREFIX) + moduleNameLen - fileNameStart;
    normFileName = new char[normFileNameLen];
    
    strcpy(normFileName, NORM_FILE_PREFIX);
    strcat(normFileName, modulePathName + fileNameStart);
    if ((normOutFile = fopen(normFileName, "w")) == (FILE*) NULL) {
	printf("WARNING: Unable to open file %s for writing.\n", normFileName);
	return;
    }
}
#undef NORM_FILE_PREFIX 

// CloseNormalizedOutFile()

void SDDF_SideArray::CloseNormalizedOutFile()
{
    assert(normOutFile != (FILE*) NULL);
    fclose(normOutFile);
    normOutFile = (FILE*) NULL;
    delete normFileName;
}

// WriteNormalizedFile(moduleFileName)
//
// Write out the normalized file. This pretty prints the file with
// indentation, spacing, and enddo where used in the original, but
// without continuation lines.  The filename of the output file is
// computed in OpenNormalizedOutFile(moduleFileName).
//
// Instead of directly invoking ft_export.C, had to copy its body here
// and modify to print without continuations.
// LATER: Check into modifying ft_export(...) to take continuation char
// as optional final argument.

int  SDDF_SideArray::WriteNormalizedFile(char* moduleFileName)
{
  OpenNormalizedOutFile(moduleFileName);
  
  FILE *outf = NormalizedFile();		// 5 original arguments of
  FortTree ft = myFt;				// ft_export(...)
  FortTextTree ftt = myFtt;
  
  //-------- COPIED FROM FUNCTION ftExportSimple() IN ftExportSimple.C --------
  
  SignificantCommentHandlers *schandlers = DEFAULT_SIG_COMMENT_HANDLERS;
 
  ftt_TreeChanged(ftt, ft_Root(ft));
 
  int continuationCharacter = ' ';	// Instead of '*'
  ftt_ExportToFile(ftt, outf, continuationCharacter, schandlers);
  
  //---------------------------------------------------------------------
 
  CloseNormalizedOutFile();

  return 0;
}

///////////////////////////////////////////////////////////////////////
