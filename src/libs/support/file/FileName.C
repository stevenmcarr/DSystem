/* $Id: FileName.C,v 1.1 1997/06/25 15:14:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <string.h>

#include <libs/support/file/FileName.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

/*
 * file_shortname - return a shorthand name for the filename passed to it.
 * This function basically assumes that filename does not end in a slash. 
 */

char* file_shortname (char* filename)
{
    char           *atslash = (char*)strrchr (filename, '/');

    if (atslash != (char *) 0)
	/* Filename contains no slashes, already in short form */
	return filename;

    if (atslash == filename && strlen (filename) <= (size_t)1)
    {				/* filename == "" || filename == "/" */
	return atslash;
    }
    else
    {				/* filename == "/blah" || "....foo/bar" || "...foobar/" */
	return atslash + 1;
    }
}

/*
 * Split a string at occurrences of the character "sep".
 * Save the pieces and return them in (char *)0 terminated
 * array gotten via get_mem.  This array, and the ssaved
 * string in it, should be free with freeComponents().
 */
char** getComponents (char* s, char sep)
{
    char **compV;
    char *ss;
    char *seploc;
    int numSeps;
    int numComps;
    int i;
    char temp[2];

    if (s == (char *)0 || strlen(s) == 0)
    {
	compV = (char **)get_mem(2*sizeof(char *),"getComponents");
	compV[0] = ssave("");
	compV[1] = (char *)0;
	return compV;
    }

    temp[0] = sep;
    temp[1] = '\0';

    numSeps = char_count(s,temp);
    numComps = numSeps
		+ 1;	/* One more component than separator. */

    compV = (char **)get_mem(sizeof(char *)*(numComps+1),"getComponents");

    ss = ssave(s);
    compV[0] = seploc = ss;
    for (i=0; i<numSeps; i++,compV[i] = seploc )
    {
	seploc = (char*)strchr(seploc,sep);
	*seploc = '\0';
	seploc++;
    }

    for (i=0;i<numComps;i++)
	compV[i] = ssave(compV[i]);

    compV[numComps] = (char *)0;

    sfree(ss);
    return compV;
}

/*
 * Turns a file name into an argv style array.  The elements of the array are
 * pointers to components of the file name.  Eg:
 *	/usr/include/stdio.h
 * becomes
 *	fileVec[0] = "usr"
 *	fileVec[1] = "include"
 *	fileVec[2] = "stdio.h"
 *	fileVec[3] = (char *)0
 * and *countp, analogous to argc, is set to the number of components.
 * If the file name ends in a slash, the last component is the empty string.
 * The returned array should be freed with freeComponents().
 */
char** getFilenameComponents(char* f, int* countp)
{
    char **compVec = getComponents(f,'/');
    char **compVecSave = compVec;
    char **fileVec;
    int numComps;
    int i;

    if (f[0] == '/')
    {
	/* Skip first component of compVec */
	sfree(compVec[0]);
	compVec++;
    }

    for (i=0; compVec[i] != (char *)0; i++ );
    numComps = i;
    fileVec = (char **)get_mem(sizeof(char *)*numComps+1,"getFilenameComponents");

    for (i=0; i<=numComps; i++)
	fileVec[i] = compVec[i];
    
    free_mem((void*)compVecSave);
    *countp = numComps; 
    return fileVec;
}

/*
 * Frees an argv style vector gotten via getFilenameComponents()
 */
void freeComponents(char** c)
{
    int i;

    for (i=0;c[i] != (char *)0;i++)
	sfree(c[i]);
    free_mem((void*)c);
}
