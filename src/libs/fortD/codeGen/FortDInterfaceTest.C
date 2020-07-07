/* $Id: FortDInterfaceTest.C,v 1.4 1999/06/11 20:39:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



#include <iostream>

#include <libs/fortD/codeGen/private_dc.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/fortD/codeGen/FortDInterface.i>
#include <libs/support/strings/OrderedSetOfStrings.h>
#include <libs/support/strings/StringBuffer.h>

#undef is_open

#include <libs/frontEnd/ast/ast_include_all.h>

using namespace std;
//--------------------------------------------------------------------
// Returns the string that contains the non local section accessed
//--------------------------------------------------------------------
void FortDMesg::MesgInfoStr()
{
 FD_String *oss;
 StringBuffer *s;
 s = new StringBuffer(100);
 oss = new FD_String();
 switch(CommType()) 
  {
   case FD_COMM_SHIFT: // include pipelining and standard shift
    cout << "\n---------SHIFT COMMUNICATION HAS OCCURED--------\n";
   break;

   case FD_COMM_BCAST:
    cout << "\n--------BROADCAST COMMUNICATION HAS OCCURED-------\n";
   break;

   case FD_COMM_REDUCE:    // done
    cout << "----------REDUCTION HAS OCCURED----------\n";
   break;

   case FD_COMM_SEND_RECV:
   // send receive communication has occured
    cout << "\n-------SINGLE SEND RECV COMMUNICATION------ \n";
   break;

   default:
   cout << "Communication Type not implemented" << endl;
   return;
   break;
  }
   oss = RecvSectionString();
   oss->Print();
   oss = SendSectionString();
   oss->Print();
   oss = SendProcessorRangeString();
   oss->Print();
   oss = RecvProcessorRangeString();
   oss->Print();
}

//-------------------------------------------------------------
// Produce Output as Follows: This is an example
// LOOP INDEPENDENT MESSAGE resulting in a SHIFT communication
// NON LOCAL ELEMENTS   : a[0:1,1:100], ......, ......
// RECEIVING PROCESSORS : 0:10
// SENDING PROCESSORS   : 1:11
//--------------------------------------------------------------
void FortDLoop::AllMesgsStr()
{
  FortDMesg* m;
  FortDRsd* f_rsd;
  FortDRsdSet* rsd_set;
  FortDAstSet* ast_set, *ast_set2;

  AST_INDEX f_ast;
  StringBuffer *buf;
  int i, j;
  FortDMesgSet* ms = Mesgs();

//-------------------------------------------
// iterate over all the messages for the loop

/*  tree_print(repr->myLoop); */
  switch(LoopType())
  {
   case FD_LOOP_REPLICATED:
   cout << " LOOP_REPLICATED \n";
   break;

   case FD_LOOP_PARALLEL:
   cout << " LOOP_PARALLEL \n";
   break;

   case FD_LOOP_PIPELINED:
   cout << " LOOP_PIPELINED \n";
   break;

   case FD_LOOP_SEQUENTIAL:
   cout << " LOOP_SEQUENTIAL \n";
   break;

   case FD_LOOP_ONEPROC:
   cout << " LOOP_ONEPROC \n";
   break;
  
   }

  while(m = (*ms)()) 
 {
   m->MesgTypeStr()->Print();
   m->CommTypeStr()->Print();
  //-------------------------------------------
  // iterate over all the rsds for the message

   ast_set2 = this->ArrayRefs();

   printf(" Printing Array References for the loop \n");

   while (f_ast = (*ast_set2)())
    {
      tree_print(f_ast);      
    }

   rsd_set = m->NonLocalRsds();
   if (rsd_set != NULL)
   {
    while (f_rsd = (*rsd_set)())
     {
     ast_set = f_rsd->Refs();
     while (f_ast = (*ast_set)())
      {
      tree_print(f_ast);      
      }
     }
   }
    //-----------------------------------------------------------
    // is Size() > 1 then message aggregation has occured
    // NOTE: at this time, we do not know if coalescing has occured
    // since the rsd at a reference is discarded if it has been 
    // merged with another rsd. Hence communication at the reference
    // level is not clear

     //---------- extract non local section -------------//
     //---------- extract sending processor(s) ----------//
     //---------- extract receiving processor(s) --------//
      m->MesgInfoStr();
      
  }
}

//---------------------------------------------------------------
// Test the reference coloring
//---------------------------------------------------------------
void FortDRef::Test()
{
  FD_String *s;
  s = GetColorStr();
  s->Print();
}

//----------------------------------------------------------------
// Test the statement coloring
//---------------------------------------------------------------
void FortDStmt::Test()
{
 FD_String *s;
 s = GetColorStr();
 s->Print();
}
