/* $Id: ProcFortDInfo.C,v 1.1 1997/03/11 14:34:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ProcFortDInfo.C
//
// Author: Gil Hansen						April 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <libs/support/strings/rn_string.h>

#include <libs/ipAnalysis/ipInfo/ProcFortDInfo.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/fortD/misc/FortD.h>


//***************************************************************************
// class ProcFortDInfo interface operations
//***************************************************************************

ProcFortDInfo::ProcFortDInfo(const char *_name) : ProcLocalInfo(_name)

{
  tree = new IPinfoTree;
}

ProcFortDInfo::~ProcFortDInfo()
{
#if 0
  // since this structure is pointed to by the FORTD_LOCAL_INFO annotations
  // and deleted from there, we cannot delete it here. hack hack ...
  // THIS ALL NEEDS TO BE REWRITTEN -- JMC 6/94
  delete tree;
#endif
}

int ProcFortDInfo::ReadUpCall(FormattedFile *ffile)
{ 
   /* NOTE: this is assumed to be the object to read in and be of type
            (LInfo *) */
  /* TBD
     Installed calls ReadFortranDInformation(context, name).
     How does one read the FormattedFile into their info?
   */
   return 1;
}

int ProcFortDInfo::WriteUpCall(FormattedFile *ffile)
{ 
#if 0
   FortranDHashTableEntry *f;
   NameEntry *n_entry;
   FortranDInfo *fd;

   /* don't write out FortranDInfoString since the name of the formatted
      file clearly identifies the information */

   //-------------------------------------------------
   // get the fortran d info structure from IPinfoTree
   fd = (FortranDInfo *)(this->tree->tree->fd);

   //-------------------------------------------------------------------
   // if the fortran d info structure is null, it must be an entry point

   if (fd == 0) {
      // cout<<form("Entry points not handled by the Fortran D compiler \n");
      // cout<<form ("Entry point %s appeared in the program \n", this->name);
   } else {
      //--------------------------------------------
      // write the n$procs if it has been specified

      ffile->Write(fd->def_numprocs);

      if (fd->def_numprocs) {
         ffile->Write(fd->numprocs);
       }

      //-------------------------------------------------
      // write the decomposition information

      ffile->Write(fd->namedlist->Count());
      for (n_entry = fd->namedlist->first_entry();  n_entry != 0;
           n_entry = fd->namedlist->next_entry()) {
          f = fd->GetEntry(n_entry->name());
          ffile->Write(f->name(), NAME_LENGTH);
          ffile->Write(f->fform);
          ffile->Write(f->getdim());
          f->d->write(*ffile, DECOMPTYPE, f->getdim());
       }

      //-------------------------------------------------
      // write the array alignment information

      ffile->Write(fd->namealist->Count());
      for (n_entry = fd->namealist->first_entry();  n_entry != 0;
           n_entry = fd->namealist->next_entry()) {
          f = fd->GetEntry(n_entry->name());
          ffile->Write(f->name(), NAME_LENGTH);
          ffile->Write(f->fform);
          ffile->Write(f->type);
          ffile->Write(f->getdim());
          f->d->write(*ffile, ARRAYTYPE, f->getdim());
       }

       //-------------------------------------------------
       // write the overlap information for the procedure
       fd->WriteOverlapInfo(*ffile);

       //---------------------------------------------------
       // write the common block declaration information
       fd->common_blks_decl->Write(*ffile);

       //-------------------------------------------------
       // write the callsite information
       fd->WriteCallInfo(this->tree->tree, *ffile);
   }
#endif
   return 1; // we do not really write it out yet; graceful failure
}

