/* $Id: WriteFortD.C,v 1.7 1997/03/11 14:29:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//----------------------------------------------------------------
// author : Seema Hiranandani
// contents : Writes the FortranDProblem out to the database
// date  : Since March 1992
//----------------------------------------------------------------
#include <libs/support/misc/general.h>
#include <libs/frontEnd/fortTree/InitInfo.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/fortD/localInfo/CommBlock.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/ipAnalysis/ipInfo/callsite.h>


//----------------------------------------------------------------
// FortranDProblem writes itself out to the database
//----------------------------------------------------------------
void FortranDProblem::write(Generic LocInfo)
{
 FortranDHashTableEntry *f;
 NameEntry *n_entry;
 ModuleIPinfoListEntry *m_entry;
   
  ((LInfo*)LocInfo)->dbport->Write(FortranDInfoString, 
                                  strlen(FortranDInfoString));
  for(m_entry = ((LInfo*)LocInfo)->m->First();  m_entry != 0;
      m_entry = ((LInfo*)LocInfo)->m->Next()) 
 {

//-------------------------------------------------
// get the fortran d info structure from IPinfoTree  
  ((LInfo*)LocInfo)->fd = (FortranDInfo*)(m_entry->info->tree->fd);

//-------------------------------------------------------------------
// if the fortran d info structure is null, it must be an entry point

   if (((LInfo*)LocInfo)->fd == 0)
   {
   // cout<<form("Entry points not handled by the fortran D compiler \n");
   // cout<<form ("Entry point %s appeared in the program \n", 
   //             m_entry->info->name);
   }
   else 
   {
//--------------------------------------------
// write the n$procs if it has been specified

  ((LInfo*)LocInfo)->dbport->Write(((LInfo*)LocInfo)->fd->def_numprocs); 

  if (((LInfo*)LocInfo)->fd->def_numprocs)
  {
  ((LInfo*)LocInfo)->dbport->Write(((LInfo*)LocInfo)->fd->numprocs);
  }
//-------------------------------------------------
// write the decomposition information

  ((LInfo*)LocInfo)->dbport->Write
                      (((LInfo*)LocInfo)->fd->namedlist->Count());
  for(n_entry = ((LInfo*)LocInfo)->fd->namedlist->first_entry();  n_entry != 0;
      n_entry = ((LInfo*)LocInfo)->fd->namedlist->next_entry()) {

   f = ((LInfo*)LocInfo)->fd->GetEntry(n_entry->name());
   ((LInfo*)LocInfo)->dbport->Write(f->name(), NAME_LENGTH);
   ((LInfo*)LocInfo)->dbport->Write(f->fform);
   ((LInfo*)LocInfo)->dbport->Write(f->getdim());
   f->d->write(*((LInfo*)LocInfo)->dbport, DECOMPTYPE, f->getdim());
   }
//-------------------------------------------------
// write the array alignment information

  ((LInfo*)LocInfo)->dbport->Write
                            (((LInfo*)LocInfo)->fd->namealist->Count());
  for(n_entry = ((LInfo*)LocInfo)->fd->namealist->first_entry();  n_entry != 0;
      n_entry = ((LInfo*)LocInfo)->fd->namealist->next_entry()) {
   f = ((LInfo*)LocInfo)->fd->GetEntry(n_entry->name());
   ((LInfo*)LocInfo)->dbport->Write(f->name(), NAME_LENGTH);
   ((LInfo*)LocInfo)->dbport->Write(f->fform);
   ((LInfo*)LocInfo)->dbport->Write(f->type);
   ((LInfo*)LocInfo)->dbport->Write(f->getdim());
   f->d->write(*((LInfo*)LocInfo)->dbport, ARRAYTYPE, f->getdim());
   }    

//-------------------------------------------------
// write the overlap information for the procedure

  ((FortranDInfo *) (m_entry->info->tree->fd))->
                     WriteOverlapInfo(*((LInfo*)LocInfo)->dbport);

//---------------------------------------------------
// write the common block declaration information

 ((FortranDInfo *) (m_entry->info->tree->fd))->common_blks_decl->Write(
                                *((LInfo*)LocInfo)->dbport);


//-------------------------------------------------
// write the callsite information

    ((FortranDInfo*)
    (m_entry->info->tree->fd))->WriteCallInfo(m_entry->info->tree,
                                          *((LInfo*)LocInfo)->dbport);
  }
 }
}








