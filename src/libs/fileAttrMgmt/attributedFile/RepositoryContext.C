/* $Id: RepositoryContext.C,v 1.1 1997/03/11 14:27:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//*************************************************************************
// RepositoryContext
//
//   an abstraction that supports access to timestamped attributes of
//   files in the context of other files. (for example, information can 
//   be stored about a source file in the context of a composition.)
//
// Author: John Mellor-Crummey                                 June 1993
//
// Copyright 1993, Rice University
//
//*************************************************************************

#include <stdlib.h>
// #include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include <libs/support/file/ScanDir.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/support/strings/rn_string.h>
// #include <include/rn_varargs.h>
#include <libs/support/file/File.h>
#include <libs/support/file/FileUtilities.h>

#include <libs/fileAttrMgmt/attributedFile/RepositoryContext.i>


// ---------- forward declarations -----------

static void ExtendCache(char *name);
static int  PositiveNumeric(struct dirent *ent);
static int  NumericSort(struct dirent **d1, struct dirent **d2);
static char *FilePath(struct RepositoryContextS *context, char *name);


//------------------------------------------------------
// RepositoryContext internal representation
//------------------------------------------------------
struct RepositoryContextS {
  RepositoryContext *parent;

  char *absoluteLocation;
  char *absoluteLocationDir;
  char *absoluteLocationBase;

  char *cacheLocation;
  
  // ***** compatability support *****
  int createdParent;
};


RepositoryContext::RepositoryContext()
{
  hidden = 0;
}


RepositoryContext::~RepositoryContext()
{
  if (hidden) Close();
}


int RepositoryContext::Close()
{
  if (hidden) {
    sfree(hidden->absoluteLocation);
    sfree(hidden->absoluteLocationDir);
    sfree(hidden->absoluteLocationBase);
    sfree(hidden->cacheLocation);
    
    // ***** compatability support *****
    if (hidden->createdParent) delete hidden->parent; 
    
    delete hidden;
    hidden = 0;

    return 0;
  }
  return -1;
}

int RepositoryContext::OpenInternal(char *location, RepositoryContext *parent)
{
  //------------------------------------------------------
  // make sure corresponding reference text exists
  //------------------------------------------------------
  if (FileExists(location) != 0) {
    fprintf(stderr, "Could not allocate RepositoryContext for '%s'\n", 
	    location);
    fprintf(stderr,"since file appears to be non-existent or inaccessible.\n");
    return -1;
  } 
  
  hidden = new RepositoryContextS;

  hidden->parent = parent;

  // ***** compatability support *****
  hidden->createdParent = 0;

#ifdef DEBUG
  //-------------------------------------------------------------------
  // verify that location is a canonical absolute path name -- JMC 7/93
  //-------------------------------------------------------------------
  char canonicalAbsolutePath[MAXPATHLEN];
  if (realpath(location, canonicalAbsolutePath) == 0) {
    perror("RepositoryContext::Open can't compute absolute path");
    return -1;
  } else {
    assert(strcmp(location, canonicalAbsolutePath) == 0);
  }
#endif

  hidden->absoluteLocation = ssave(location);
  hidden->absoluteLocationDir = ssave(FileDirName(location));
  hidden->absoluteLocationBase = ssave(FileBaseName(location));

  return 0;
}

int RepositoryContext::Open(char *location, RepositoryContext *parent)
{ 
  return (((OpenInternal(location, parent) == 0) && !InitAttrCache()) ? 
	  0 : -1);
}


int RepositoryContext::InitAttrCache()
{
  RepositoryContext *parent = hidden->parent;

  if (!parent) {
    char pwd[MAXPATHLEN+1];
    if (!getcwd(pwd, MAXPATHLEN)) {
      errorMsgHandler.HandleMsg("RepositoryContext::InitAttrCache unable to get current working directory, using /tmp instead\n");
      strcpy(pwd,"/tmp");
    }

    char *whereCache = getenv("DSYSTEM_CACHE");

    //------------------------------------------------------------------
    // create, or locate, the cache directory for a top level context
    //------------------------------------------------------------------
    if (!whereCache) {
      hidden->cacheLocation = nssave(7, pwd, "/", 
				     REP_CACHE_DIR_NAME, 
                                     hidden->absoluteLocationDir, "/",
				     hidden->absoluteLocationBase, "/");
    } else {
      if (whereCache[0] == '/') {
        hidden->cacheLocation = nssave(6, whereCache, "/",
                                       hidden->absoluteLocationDir, "/",
                                       hidden->absoluteLocationBase, "/");
      } else {
        hidden->cacheLocation = nssave(8, pwd, "/",
                                       whereCache, "/",
                                       hidden->absoluteLocationDir, "/",
                                       hidden->absoluteLocationBase, "/");
      }
    }
    ExtendCache(hidden->cacheLocation);

  } else {
    //------------------------------------------------------------------
    // create, or locate, the cache directory for a nested context
    //------------------------------------------------------------------
    char *cachePrefix = nssave(6, parent->hidden->cacheLocation, "/",
                          hidden->absoluteLocationDir, "/",
			  hidden->absoluteLocationBase, "/"); 
    ExtendCache(cachePrefix);
    hidden->cacheLocation = nssave(1, cachePrefix);

#if 0    
    //************************************************************************
    // this section of code is OBSOLETE
    // it implements an old style cache that does not use full path names
    //************************************************************************

    struct dirent **namelist;
    int         i, num_files;
    int         high_water_mark;
    char       *linkname;
    char        new_dir[20];

    num_files = scandir(cachePrefix, &namelist, PositiveNumeric, NumericSort);
    
    // get the inode number and divide number of an existing reference text 
    struct stat buf;
    int code = stat(location, &buf);
    assert(code == 0);

    // do any of these directories contain the appropriate link to m_loc?
    for (i = 0; i < num_files; i++) {
      struct stat link_buf;
      linkname = nssave(3, cachePrefix, namelist[i]->d_name, "/uplink");
      // if desired link already exists, exit loop 
      if ((stat(linkname, &link_buf) == 0) && 
	  (link_buf.st_ino == buf.st_ino && link_buf.st_dev == buf.st_dev))
	break;
      sfree(linkname);
    }
    
    //---------------------------------------------------------------------------------
    // pre-condition: 
    //   i < num_files  implies the desired link exists and named by namelist[i]->d_name 
    //   i == num_files implies the desired link must be created
    //---------------------------------------------------------------------------------
    if (i < num_files) {
      hidden->cacheLocation = nssave(3, cachePrefix, namelist[i]->d_name, "/");
    } else {
      // link not found; must be created
      
      if (num_files > 0)
	// remember, file names are sorted
	high_water_mark = atoi(namelist[num_files - 1]->d_name) + 1;
      else
	high_water_mark = 1;
      
      // create the directory
      sprintf(new_dir, "%d", high_water_mark);
      hidden->cacheLocation = nssave(3, cachePrefix, new_dir, "/");
      ExtendCache(hidden->cacheLocation);
      
      // add the link
      linkname = nssave(3, cachePrefix, new_dir, "/uplink");      
      symlink(hidden->absoluteLocation, linkname);
    }
#endif
  }

  return 0;
}

File *RepositoryContext::GetFile(char *fileName)
{
  assert(hidden != 0); // RepositoryContext is Open
  
  char *pathName = FilePath(hidden, fileName);
  File *fp = new File;
  int code =  fp->Open(pathName, "r");
  sfree(pathName);
  if (code) {
    delete fp;
    return 0; // failure
  } else return fp;
}

File *RepositoryContext::CreateFile(char *fileName)
{
  assert(hidden != 0); // RepositoryContext is Open
  
  char *pathName = FilePath(hidden, fileName);
  File *fp = new File;
  int code =  fp->Open(pathName, "w");
  sfree(pathName);
  if (code) {
    delete fp;
    return 0; // failure
  } else return fp;
}


int RepositoryContext::DestroyFile(char *fileName)
{
  assert(hidden != 0); // RepositoryContext is Open
  int returnCode = 0;  // success
  
  char *pathName = FilePath(hidden, fileName);

  int code = unlink(pathName);
  if (code && errno != ENOENT) returnCode = -1;

  sfree(pathName);

  return returnCode;
}


char *RepositoryContext::GetCacheLocation()
{
  return hidden->cacheLocation;
}

//***************************************************************************
// private functions
//***************************************************************************


static char *FilePath(struct RepositoryContextS *context, char *fileName)
{
  assert(context != 0);
  return nssave(2, context->cacheLocation, fileName);
}



#define CACHE_UMASK 0755
static int ExtendCacheInternal(char *name)
{
    int retval ;
    char *sp ;
    if ( sp = strrchr( name, '/' ) ) {
	if ( mkdir( name, CACHE_UMASK ) ) {
	    if ( errno == ENOENT ) {
   		*sp = '\0' ;
		if ( ( retval = ExtendCacheInternal( name ) ) == 0 ) {
	    	    *sp = '/' ;
	    	    return( retval ) ;
		}
		*sp = '/' ;
		if ( mkdir( name, CACHE_UMASK ) ) {
	    	    if ( errno != EEXIST ) {
		    	return( 0 ) ;
		    }
	    	}
	    }
	}
    }
    else {
        if ( mkdir( name, CACHE_UMASK ) ) {
	    if ( errno != EEXIST ) {
	    	return( 0 ) ;
	    }
	}
    }
    return( 1 ) ;
}

static void ExtendCache(char *name)
{
  char *sp;
  int   retval;

  sp = name + strlen(name) - 1;
  assert(*sp == '/');

  *sp = '\0';
  retval = ExtendCacheInternal(name);
  *sp = '/';

  if (!retval)
  {
    fprintf(stderr, "ExtendCache fails on %s\n", name);
    exit(-1);
  }
}

static int PositiveNumeric(struct dirent  *ent)
{
  return (atoi(ent->d_name) != 0);
}

static int NumericSort(struct dirent **d1, struct dirent **d2)
{
  return (atoi((*d1)->d_name) - atoi((*d2)->d_name));
}
