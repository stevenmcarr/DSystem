/* $Id: filer_dia.C,v 1.2 2001/09/17 00:42:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                               filer_dia.c                            */
/*                                                                      */
/************************************************************************/

#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <libs/graphicInterface/oldMonitor/include/dialogs/filer.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/confirm.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>

#include <libs/graphicInterface/oldMonitor/include/items/regex.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/file/ScanDir.h>
#include <libs/support/file/UnixFile.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>

#include <libs/graphicInterface/oldMonitor/dialogs/filer/table_item.h>


/*
Regular expression describing a UNIX file name starting with a slash,
containing no adjacent slashes '/', and with no occurrences of
"/../" or "/./".
*/

#define CLEAN_FILE_NAME \
"^/(([^\\./]|\\.[^\\./]|\\.\\.[^/])[^/]*/)*(([^\\./]|\\.[^\\./]|\\.\\.[^/])[^/]*)?$"
/*
A clean file name starts with a slash, then has zero or more filename
components each followed by a slash, and then has an optional trailing
filename component.  This is the RE for a filename component:
    ([^\\./]|\\.[^\\./]|\\.\\.[^/])[^/]*
It appears twice in the RE for CLEAN_FILE_NAME.
*/
typedef FUNCTION_POINTER(int,ScanDirSelectFunc,(const struct dirent *));
typedef FUNCTION_POINTER(Boolean,OkayerFunc,(Generic,char*));

#define DIR_START	1
#define DIR_END		1
#define BASE_START	3
#define BASE_END	3

 /* item id's */
#define SELECT_BUT	DIALOG_DEFAULT_ID
#define CD_UP_BUT       1
#define CD_DOWN_BUT     2
#define INFO_BUT	3

#define MAKE_DIR_BUT    20

#define ITEM_TABLE      31
#define FROM_PATH       32
#define TO_PATH         33
#define UP_BUT		34
#define DOWN_BUT	35

struct filedia
{/* file dialog structure */
    Boolean	fromactive;
    Dialog	*dialog;	/* the actual dialogue				*/
    DiaDesc	*fromItem;	/* regex item managing "from" field */
    DiaDesc	*toItem;	/* regex item managing "to" field */
    char	*toptr;		/* the front end path name - for rename, copy	*/
    char	*fromptr;	/* the 'current path'				*/

    char	*cd;		/* "dirname" of current filename		*/
    char	*cfile;		/* "basename" of current filename		*/
    char	*lastdir;
    struct dirent
		**namelist;	/* contents of the current dir			*/
    short	numfiles;
    char	**entries;	/* files in the current directory		*/
    short	selection;	/* the index of the 'selected' file		*/
    Boolean	redraw_table;

 /* select file support */
    Generic	handle;
    Boolean	ok;
    OkayerFunc	okayer;
    Boolean	made_selection;	
};

STATIC(void,     move_to_active,(Dialog *di, aFileDia *pd, char *s));
STATIC(void,     goto_file,(aFileDia *pd));
STATIC(Boolean,  file_read_directory,(aFileDia *pd));
STATIC(void,     file_stat,(char *filename, Boolean *exists, Boolean *dir));
STATIC(void,     file_split_name,(aFileDia *pd, char *filename, char **ncd, 
                                  char **nfile));
STATIC(void,     file_help,(char *filename));
static short     file_dialog_font;
STATIC(Boolean,  file_dialog_handler,(Dialog *di, aFileDia *pd, Generic item));
STATIC(Boolean,  file_select_dialog_handler,(Dialog *di, aFileDia *pd, Generic item));
STATIC(void,     unscandir, (struct dirent **namelist, Generic num_entries));
STATIC(int,  no_dot_or_dot_dot, (const struct dirent *ent));

static void move_to_active(Dialog *di, aFileDia *pd, char *s)
{
	if (pd->fromactive)
	{
	    smove(&pd->fromptr, s);
	    dialog_item_modified(di,FROM_PATH);
	}
	else
	{
	    smove(&pd->toptr, s);
	    dialog_item_modified(di,TO_PATH);
	}
}

/*ARGSUSED*/
static          Boolean
file_dialog_handler(Dialog *di, aFileDia *pd, Generic item)
{
    char	*temp;
    int		ret;
    char	*prompt;
    Boolean	file_exists,
		file_isdir,
		doit;
    Boolean	killed;
    char	*filename;

    killed = false;

    if (pd->fromactive)
	filename = pd->fromptr;
    else
	filename = pd->toptr;

    switch (item)
    {				       /* figure what to do based on the item modified */
    case UP_BUT:
	/* move the contents of TO to FROM */
	smove(&pd->fromptr,pd->toptr);
        smove(&pd->toptr, "");
	dialog_item_modified(di,FROM_PATH);
	dialog_item_modified(di,TO_PATH);
	pd->fromactive = true;
	(void) dialog_item_set_focus(pd->dialog,FROM_PATH);
	break;

    case DOWN_BUT:
	/* move the contents of FROM to TO */
	smove(&pd->toptr,pd->fromptr);
	dialog_item_modified(di,TO_PATH);
	pd->fromactive = false;
	(void) dialog_item_set_focus(pd->dialog,TO_PATH);
	break;

    case FROM_PATH:
	if (!pd->fromactive)	       /* clear out the to field */
	{
	    smove(&pd->toptr, "");
	    dialog_item_modified(di,TO_PATH);
	    pd->fromactive = true;
	}
	break;

    case TO_PATH:
	if (pd->fromactive)	       /* initialize the to field, the first time */
	{
	    smove(&pd->toptr, pd->fromptr);
	    dialog_item_modified(di,TO_PATH);
	    pd->fromactive = false;
	}
	break;

    case CD_UP_BUT:
	move_to_active(di,pd,pd->cd);
	break;

    case CD_DOWN_BUT:
	temp = nssave(2,
		(pd->fromactive ?
			pd->fromptr : 
			pd->toptr),
		"/");
	move_to_active(di,pd,temp);
	sfree(temp);
	break;

    case MAKE_DIR_BUT:
	if (ret = mkdir(filename, 0777))
	       dialog_message(pd->dialog, "Error creating directory.");
	break;

    case ITEM_TABLE:
	if (pd->lastdir[strlen(pd->lastdir) - 1] == '/')
	    temp = nssave(2, pd->lastdir, pd->entries[pd->selection]);
	else
	    temp = nssave(3, pd->lastdir, "/", pd->entries[pd->selection]);

	file_stat(temp, &file_exists, &file_isdir);
	if (!file_exists)
	{	
	    sfree(temp);
	    temp = nssave(3,pd->lastdir,"/", pd->entries[pd->selection]);
        }

	move_to_active(di,pd,temp);
        sfree(temp);
	break;

    case INFO_BUT:
	if (pd->fromactive)
	    file_help(pd->fromptr);

	else
	    file_help(pd->toptr);
	break;

    case DIALOG_CANCEL_ID:
	killed = true;
	break;
    }

    if (killed)
	return DIALOG_QUIT;

    goto_file(pd);
    return DIALOG_NOMINAL;
}

/* select dialog code */

/* Create a file dialogue.                                             */
Boolean
file_select_dialog_run(char *title, char *bname, char **toptr, 
                       OkayerFunc okayer, Generic handle)
{ 
    int            i;
    aFileDia       *pd;		       /* the file dialogue            */
    Boolean        made_selection;     /* the user made a selection    */

    pd = (aFileDia *) get_mem(sizeof(aFileDia), "file dialogue");
    file_dialog_font = fontOpen("screen.7.rnf");
    pd->made_selection = false;
    pd->ok = false;
    pd->handle = handle;
    pd->okayer = okayer;
    pd->toptr = ssave(*toptr);
    pd->fromptr = ssave("");
    pd->cd = ssave("");
    pd->cfile = ssave("");
    pd->lastdir = ssave("");
    pd->entries = 0;
    pd->numfiles = 0;
    pd->redraw_table = false;
	/* moved here by bro */
    pd->fromactive = false;
    pd->selection = -1;
    pd->namelist = 0;
	/* added by bro */
    pd->dialog = (Dialog *)0 ;
    pd->fromItem = (DiaDesc *)0 ;

    pd->dialog = dialog_create(
	title,
	(dialog_handler_callback)file_select_dialog_handler,
	(dialog_helper_callback)0,
	(Generic)pd,
	dialog_desc_group(
	    DIALOG_HORIZ_CENTER,
	    2,
	    dialog_desc_group(
		DIALOG_VERT_LEFT,
		2,
		pd->toItem = item_gregex(
		    TO_PATH,
		    "to:",
		    DEF_FONT_ID,
		    (Generic)&pd->toptr,
		    CLEAN_FILE_NAME,
		    70,
		    false,
		    (item_gregex_toval_func)toText,
		    (item_gregex_fromval_func)fromText
		),
		item_table(
		    ITEM_TABLE,
		    DEF_FONT_ID,
		    &pd->numfiles,
		    &pd->entries,
		    &pd->selection,
		    &pd->redraw_table,
		    makePoint(74, 18)
		)
	    ),
	    dialog_desc_group(
		DIALOG_VERT_LEFT,
		4,
		item_button(
		    CD_UP_BUT,
		    "cd up    ",
		    file_dialog_font,
		    false
		),
		item_button(
		    CD_DOWN_BUT,
		    "cd down  ",
		    file_dialog_font,
		    false
		),
		item_button(
		    INFO_BUT,
		    "info     ",
		    file_dialog_font,
		    false
		),
		item_button(
		    SELECT_BUT,
		    bname,
		    file_dialog_font,
		    true
		)
	    )
	)
    );

    goto_file(pd);

    dialog_modal_run(pd->dialog);

 /* save return values */
    smove(toptr, pd->toptr);
    made_selection = pd->made_selection;

 /* clean up */
    if (pd->entries)
    {   /* if we have created this list, then free it */
	for (i = 0; i < pd->numfiles; i++)
	     sfree(pd->entries[i]);
	free_mem((void*) pd->entries);
    }

    sfree(pd->fromptr);
    sfree(pd->toptr);
    sfree(pd->cd);
    sfree(pd->cfile);
    sfree(pd->lastdir);
    dialog_destroy(pd->dialog);
    fontClose(file_dialog_font);
    free_mem((void*) pd);
    return made_selection;
}

/*ARGSUSED*/
static          Boolean
file_select_dialog_handler(Dialog *di, aFileDia *pd, Generic item)
{
    Boolean         killed;
    char           *temp;
    Boolean         to_exists,
		    to_isdir;

    killed = false;
    switch (item)
    {				       /* figure what to do based on the item modified */
    case ITEM_TABLE:
	if (pd->lastdir[strlen(pd->lastdir) - 1] == '/')
	    temp = nssave(2, pd->lastdir, pd->entries[pd->selection]);
	else
	    temp = nssave(3, pd->lastdir, "/", pd->entries[pd->selection]);

	file_stat(temp, &to_exists, &to_isdir);
	if (!to_exists)
	{	
	    sfree(temp);
	    temp = nssave(3, pd->lastdir, "/", pd->entries[pd->selection]);
        }
	smove(&pd->toptr, temp);
	sfree(temp);
	break;
    case TO_PATH:
	if (pd->fromactive)	       /* initialize the to field, the first time */
	{
	    smove(&pd->toptr, pd->fromptr);
	    dialog_item_modified(di,TO_PATH);
	    pd->fromactive = false;
	}
	break;
    case CD_UP_BUT:
	smove(&pd->toptr, pd->cd);
	dialog_item_modified(di,TO_PATH);
	break;
    case CD_DOWN_BUT:
	temp = nssave(2, pd->toptr, "/");
	smove(&pd->toptr, temp);
	dialog_item_modified(di,TO_PATH);
	sfree(temp);
	break;
    case INFO_BUT:
	if (pd->fromactive)
	    file_help(pd->fromptr);
	else
	    file_help(pd->toptr);
	break;
    case SELECT_BUT:
        pd->made_selection = true;
	killed = true;
	break;
    case DIALOG_CANCEL_ID:
	killed = true;
	break;
    }

    if (killed)
	return DIALOG_QUIT;

    goto_file(pd);
    return DIALOG_NOMINAL;
}

/* auxillary code */

/*
 * unscandir - the library neglected to provide us with a clean way to free memory grabbed by scandir. 
 */
static void
unscandir(struct dirent **namelist, Generic num_entries)
{

/*
  THESE ARE free() calls.. do not change these.. this data was not allocated with get_mem 
*/
    while (--num_entries >= 0)
	free((char *) namelist[num_entries]);
    free((char *) namelist);
}

static void
goto_file(aFileDia *pd)
{
    int             i;

    char           *filename;
    char	   *lcd,*lfile;

    if (pd->fromactive)
	filename = pd->fromptr;
    else
	filename = pd->toptr;

    if (filename[0] != '/')
    {	/* an illegal pathname, disable the dialog */
	dialog_item_ability(pd->dialog, CD_UP_BUT, DIALOG_DISABLE);
	dialog_item_ability(pd->dialog, CD_DOWN_BUT, DIALOG_DISABLE);
	dialog_item_ability(pd->dialog, INFO_BUT, DIALOG_DISABLE);
	dialog_item_ability(pd->dialog, SELECT_BUT, DIALOG_DISABLE);
	dialog_item_ability(pd->dialog, MAKE_DIR_BUT, DIALOG_DISABLE);
	dialog_message(pd->dialog, "Illegal path name.");
	return;
    }

    lcd   = ssave("");
    lfile = ssave("");
    file_split_name(pd, filename, &lcd, &lfile);

    pd->redraw_table = BOOL(strcmp(pd->cd,lcd) == 0);
    smove(&pd->cd,   lcd);
    smove(&pd->cfile,lfile);

    if ( file_read_directory(pd)) {
    	pd->selection = -1;

    	for (i = 0; i < pd->numfiles; i++)
    	{
	    if (strcmp(pd->entries[i], pd->cfile) == 0)
	    {			       /* found the item we want selected */
	    	pd->selection = i;
	    	break;
	    }
    	}
    }

    file_update_all(pd,filename);
}

/*ARGSUSED*/
static void
file_split_name(aFileDia *pd, char *filename, char **ncd, char **nfile)
{
    char *atslash;

    smove( ncd, filename );

    atslash = strrchr( *ncd, '/' );
    smove( nfile, atslash+1 );

    if (atslash == *ncd)
	*(atslash+1) = '\0';
    else
        *atslash = '\0';
}


static int
no_dot_or_dot_dot(const struct dirent *ent)
{
    int l;

    l = (int)strlen(ent->d_name);
    if (ent->d_name[l - 2] == '.' && ent->d_name[l - 1] == '.' && ent->d_name[l] == '\0')
	return 0;
    if (ent->d_name[l - 1] == '.' && ent->d_name[l] == '\0')
	return 0;
    return 1;
}

/* read in the directory pd->cd */
static Boolean
file_read_directory(aFileDia *pd)
{
    int             i;
    struct dirent **savenamelist;      /* contents of the current dir   */
    int             savenumfiles;

    savenumfiles = pd->numfiles;
    savenamelist = pd->namelist;

    pd->numfiles = scandir(pd->cd, &pd->namelist, (ScanDirSelectFunc) no_dot_or_dot_dot,
			   alphasort);

    if (pd->numfiles < 0)	       /* Not a directory */
    {
	pd->numfiles = savenumfiles;
	pd->namelist = savenamelist;
	return false;
    }
    else
    {
        smove(&pd->lastdir, pd->cd);

 /* free up the previous namelist if necessary */
	if (savenamelist != 0)
	    unscandir(savenamelist, savenumfiles);

	if (pd->entries)
	{			       /* if we have created this list, then free it */
	    for (i = 0; i < savenumfiles; i++)
		sfree(pd->entries[i]);
	    free_mem((void*) pd->entries);
	}

 /* copy the files into "entries" */
	pd->entries = (char **) get_mem(sizeof(char *) * (pd->numfiles + 1),
	    "table entries");

	for (i = 0; i < pd->numfiles; i++)
	    pd->entries[i] = ssave(pd->namelist[i]->d_name);

	return true;
    }
}

/* is filename the name of a directory */
static void
file_stat(char *filename, Boolean *exists, Boolean *dir)
{
    struct stat     statbuf;

    *exists = BOOL(stat(filename, &statbuf) != -1);
    *dir = false;
    if ((statbuf.st_mode & S_IFMT) == S_IFDIR)
	*dir = true;
    return;
}

static char     file_help_text[] = "\
Name:      %s \n\
Owner:     %s \n\
Group:     %s \n\
Protection:%s \n\
Size:      %d \n\
Blocksize: %ld \n\
Accessed:  %s \n\
Modified:  %s \n\
Changed:   %s \n\
Links:     %d \n\
Inode:     %d \n\
Type:      %s \n\
Set:       %s \n\
";

static void
file_help(char *filename)
{
    struct stat     sb;
    struct passwd  *pwent;
    struct group   *grent;
    char           *f_type;
    char           *f_set;
    char            protbuf[10];

#define TIME_BUF    26
#define NL_INDEX TIME_BUF - 2
    char            atime_buf[TIME_BUF];
    char            mtime_buf[TIME_BUF];
    char            ctime_buf[TIME_BUF];
    int             i;

    if (stat(filename, &sb) < 0)
    {
	message("Can't stat %s", filename);
	return;
    }

    switch (sb.st_mode & S_IFMT)
    {
    case S_IFDIR:
	f_type = "directory";
	break;
    case S_IFCHR:
	f_type = "character special";
	break;
    case S_IFBLK:
	f_type = "block special";
	break;
    case S_IFREG:
	f_type = "regular";
	break;
    case S_IFLNK:
	f_type = "symbolic link";
	break;
    case S_IFSOCK:
	f_type = "socket";
	break;
    default:
	f_type = "unknown";
	break;
    }

    switch (sb.st_mode & 0007000)
    {
    case S_ISUID:
	f_set = "uid";
	break;
    case S_ISGID:
	f_set = "gid";
	break;
    case S_ISUID | S_ISGID:
	f_set = "uid and gid";
	break;
    case 0:
	f_set = "none";
	break;

    case S_ISVTX:
    default:
	f_set = "sticky?";
	break;
    }

    sb.st_mode = sb.st_mode & 0000777;
    for (i = 0; i < 9; sb.st_mode = sb.st_mode << 3, i += 3)
    {
	if (sb.st_mode & S_IREAD)
	    protbuf[i] = 'r';
	else
	    protbuf[i] = '-';

	if (sb.st_mode & S_IWRITE)
	    protbuf[i + 1] = 'w';
	else
	    protbuf[i + 1] = '-';

	if (sb.st_mode & S_IEXEC)
	    protbuf[i + 2] = 'x';
	else
	    protbuf[i + 2] = '-';
    }
    protbuf[9] = '\0';

 /*
  * Convert times values to local time strings.  asctime uses the same buffer each time, so we have to strcpy to
  * separate buffers. We place nulls on top of \n's that asctime kindly provided for us. 
  */

    (void) strcpy(atime_buf, asctime(localtime(&(sb.st_atime))));
    atime_buf[NL_INDEX] = '\0';
    (void) strcpy(mtime_buf, asctime(localtime(&(sb.st_mtime))));
    mtime_buf[NL_INDEX] = '\0';
    (void) strcpy(ctime_buf, asctime(localtime(&(sb.st_ctime))));
    ctime_buf[NL_INDEX] = '\0';

    pwent = getpwuid(sb.st_uid);
    grent = getgrgid(sb.st_gid);

    message(file_help_text,
	filename,
	pwent->pw_name,		       /* char * */
	grent->gr_name,		       /* char * */
	protbuf,		       /* char[10] */
	sb.st_size,		       /* off_t */
	sb.st_blocks,		       /* long */
	atime_buf,		       /* time_t */
	mtime_buf,
	ctime_buf,
	sb.st_nlink,		       /* short */
	sb.st_ino,		       /* ino_t */
	f_type,
	f_set
	);
}

void
file_update_all(aFileDia *pd, char *filename)
{
    Dialog *d = pd->dialog;
    Boolean         fC,	/* from Correct */
		    fE, /* from Exists */
		    fD, /* from is a Directory */
		    tC, /* to Correct */
		    tE, /* to Exists */
		    tD; /* to is a Directory */
    Boolean endsWithSlash;

    endsWithSlash = (strlen(filename) == 0) ?
			false :
			((filename[strlen(filename)-1] == '/') ?
				true :
				false );

    fC = fE = fD = tC = tE = tD = false;

    dialog_item_modified(d, FROM_PATH);	/* moved here by bro : otherwise, */
    dialog_item_modified(d, TO_PATH);	/* the regex_status is wrong	  */

    switch(item_regex_status(pd->toItem))
    {
	case itemRegexMatched:
	case itemRegexStillMatched:
	    tC = true;
	    file_stat(pd->toptr,   &tE,   &tD);
	    break;
    }

    dialog_item_ability(d,  CD_UP_BUT, DIALOG_ENABLE);
    dialog_item_modified(d, ITEM_TABLE);

#define enable(p)	((p) ? DIALOG_ENABLE : DIALOG_DISABLE)
#define disable(p)	((p) ? DIALOG_DISABLE : DIALOG_ENABLE)

    pd->ok = (pd->okayer) (pd->handle, filename);
    dialog_item_ability(d, CD_DOWN_BUT,  enable(tD && !endsWithSlash));
    dialog_item_ability(d, SELECT_BUT,   enable(pd->ok));
    dialog_item_ability(d, INFO_BUT,     enable(tE));
}

/* 
 * Handle for the save dialog, which determines if it is possible to save
 * filename. This code has been lifted from ted_sm by Alan Carle. Thanks go to 
 * Ben Chase and Kathryn McKinley, the originators of ted_cp and ted_sm.
 */

Boolean file_select_ok_to_write(Generic handle, char *filename)
{
  return file_ok_to_write(filename);
}
