/* $Id: putenv2.C,v 1.1 1997/06/25 15:16:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*  putenv2  --  put value into environment
 *
 *  Usage:  i = putenv2 (name,value)
 *	int i;
 *	char *name, *value;
 *
 *  Putenv associates "value" with the environment parameter "name".
 *  If "value" is 0, then "name" will be deleted from the environment.
 *  Putenv returns 0 normally, -1 on error (not enough core for malloc).
 *
 *  Putenv may need to add a new name into the environment, or to
 *  associate a value longer than the current value with a particular
 *  name.  So, to make life simpler, putenv2() copies your entire
 *  environment into the heap (i.e. malloc()) from the stack
 *  (i.e. where it resides when your process is initiated) the first
 *  time you call it.
 *
 *  HISTORY
 * 20-Nov-79  Steven Shafer (sas) at Carnegie-Mellon University
 *	Created for VAX.  Too bad Bell Labs didn't provide this.  It's
 *	unfortunate that you have to copy the whole environment onto the
 *	heap, but the bookkeeping-and-not-so-much-copying approach turns
 *	out to be much hairier.  So, I decided to do the simple thing,
 *	copying the entire environment onto the heap the first time you
 *	call putenv2(), then doing realloc() uniformly later on.
 *	Note that "putenv2(name,getenv(name))" is a no-op; that's the reason
 *	for the use of a 0 pointer to tell putenv2() to delete an entry.
 *
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libs/support/misc/general.h>

#define EXTRASIZE 5		/* increment to add to env. size */

static int envsize = -1;	/* current size of environment */
extern char **environ;		/* the global which is your env. */
int putenv2 (char *name, char *value);

STATIC(int, findenv,(char *name));      /* look for a name in the env. */
STATIC(int, newenv,(void));		/* copy env. from stack to heap */
STATIC(int, moreenv,(void));       	/* incr. size of env. */

int putenv2 (char *name, char *value)
{
	register int i,j;
	register char *p;

	if (envsize < 0) {	/* first time putenv2 called */
		if (newenv() < 0)	/* copy env. to heap */
			return (-1);
	}

	i = findenv (name);	/* look for name in environment */

	if (value) {		/* put value into environment */
		if (i < 0) {	/* name must be added */
			for (i=0; environ[i]; i++) ;
			if (i >= (envsize - 1)) {	/* need new slot */
				if (moreenv() < 0)  return (-1);
			}
			p = (char*)malloc ((unsigned)(strlen(name) + strlen(value) + 2));
			if (p == 0)	/* not enough core */
				return (-1);
			environ[i+1] = 0;	/* new end of env. */
		}
		else {		/* name already in env. */
			p = (char*)realloc (environ[i],
				(unsigned) (strlen(name) + strlen(value) + 2));
			if (p == 0)  return (-1);
		}
		(void) sprintf (p,"%s=%s",name,value);	/* copy into env. */
		environ[i] = p;
	}
	else {			/* delete name from environment */
		if (i >= 0) {	/* name is currently in env. */
			free (environ[i]);
			for (j=i; environ[j]; j++) ;
			environ[i] = environ[j-1];
			environ[j-1] = 0;
		}
	}

	return (0);
}

static int findenv(char *name)
{
	register char *namechar, *envchar;
	register int i,found;

	found = 0;
	for (i=0; environ[i] && !found; i++) {
		envchar = environ[i];
		namechar = name;
		while (*namechar && (*namechar == *envchar)) {
			namechar++;
			envchar++;
		}
		found = (*namechar == '\0' && *envchar == '=');
	}
	return (found ? i-1 : -1);
}

static int newenv ()
{
	register char **env, *elem;
	register int i,esize;

	for (i=0; environ[i]; i++) ;
	esize = i + EXTRASIZE + 1;
	env = (char **) malloc ((unsigned) (esize * sizeof (elem)));
	if (env == 0)  return (-1);

	for (i = 0; environ[i]; i++) {
		elem = (char*)malloc ((unsigned)(strlen(environ[i]) + 1));
		if (elem == 0)  return (-1);
		env[i] = elem;
		(void) strcpy (elem,environ[i]);
	}

	env[i] = 0;
	environ = env;
	envsize = esize;
	return (0);
}

static int moreenv()
{
	register int esize;
	register char **env;

	esize = envsize + EXTRASIZE;
	env = (char **) realloc ((char *)environ,(unsigned)(esize * sizeof(*env)));
	if (env == 0)  return (-1);
	environ = env;
	envsize = esize;
	return (0);
}
