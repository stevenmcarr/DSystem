
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    config.C
 *
 * Author
 *    Kevin Cureton - major overhaul
 *
 * Date
 *    Aug 1994
 *
 * Description
 *    Resolve the configuration from the configuration file.  Prints the
 *    copyright message, etc.
 *
 ******************************************************************************/

/**************************** system include files ****************************/

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************** user include files *****************************/

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/config/config.h>

/**************************** variable definitions ****************************/

char D_copyright[] = "";


   /* seconds the notice must show */
float D_copyright_uptime = 0.1;  

   /* Startup command processor index */ 
short D_startup_cp = 0;        

   /* The default font name */ 
char *DEF_FONT_NAME = "screen.12.rnf";  

   /* Underline character (glyph from file) */ 
unsigned char UNDERLINE_CH = 0x80;     

   /* Cursor character (glyph from file) */
unsigned char CURSOR_CH    = 0x81; 

   /* location of first of 10 glyphs used for profiling in ExMon */
unsigned char LITTLE_0     = 0x90;      

   /* location of first of 9 glyphs used for profiling in ExMon */   
unsigned char LITTLE_1e    = 0x9a;     

   /* glyph for a check mark */
char *CHECK_MARKED = "\212";           

   /* absence of a check */
char *CHECK_EMPTY  = " ";             

   /* glyph for a checked box */
char *BOX_MARKED   = "\204\205";      

   /* glyph for an empty box */
char *BOX_EMPTY = "\202\203";          

   /* official architecture name - see arch.h for more info */

#ifdef DSYSTEM_ARCH_SUNOS_4x      
char *D_arch_name = "SunOS-4.x";
#endif

#ifdef DSYSTEM_ARCH_SUNOS_5x      
char *D_arch_name = "SunOS-5.x";
#endif

#ifdef DSYSTEM_ARCH_AIX_3x      
char *D_arch_name = "AIX-3.x";
#endif

#ifdef DSYSTEM_ARCH_OSF1      
char *D_arch_name = "OSF1";
#endif

   /* Zero terminated list of directories */ 
char *D_font_dirs[] =
  {
    "",                                 /* works for full path name & cwd */
    "$(DSYSTEM)/lib/fonts/",            /* standard Rn font directory */
    0                                   /* must be null terminated */
  };

  /* general Rn help file */
char *D_helpfile = "general.H";

  /* Help_cp directory list for help files */ 
char *D_help_dirs[] =
  {
    "",                                 /* works for full path name & cwd */
    "helptexts/",                       /* local helptexts directory */
    "$(DSYSTEM)/lib/helptexts/",        /* standard Rn help texts directory */
    0                                   /* must be null terminated */
  };

  /* standard gtd tables directory */ 
char *D_gtd_dirs[] =
  {
    "",                                 /* works for full path name & cwd */
    "gtd_tables/",                      /* local gtd table directory */
    "$(DSYSTEM)/lib/gtd_tables/",       /* standard Rn gtd tables directory */
    0                                   /* must be null terminated */
  };

/*  This needs to be taken out ---KLC */

  /* D program compiler executable */ 
char *D_pcomp = "$(DSYSTEM)/bin/pcomp";

  /* D compiling executable */
char *D_compile = "$(DSYSTEM)/bin/compile";

  /* D source pretty printer */ 
char *D_ptree = "$(DSYSTEM)/bin/ptree";

  /* D fortran compiler (native) */  
char *D_f77 = "$(DSYSTEM)/bin/rnf77"; 

  /* D dependence graph server */ 
char *D_pfc_server = "PARASCOP@ricevm1.rice.edu"; 

  /* D_pfc_server will send the graph file to this address. */
char *D_return_addr_cmd = 
#ifdef distribution
   "long.host.name";
#else
   "cs.rice.edu";
#endif                                        

/*  End of the takeout  */

  /* D executables directory */ 
char *D_bin = "$(DSYSTEM)/bin/";               

  /* color preference file program name */
char *D_resource_identifier =  "envX";        

  /* pathname of default color file */  
char *D_color_defaults_file = "$(DSYSTEM)/.dsystem_color_prefs";

  /* symbolic analysis is normally on - use "off" to disable (ignored) */
char *D_symbolic_analysis = "on";     

  /* use "off" to disable simplification */
char *D_sym_simplify = "on";                    

  /* use "on" to enable GSA form */
char *D_sym_gated = "off";              

  /* use "on" to enable array SSA form */
char *D_sym_arrays = "off";        

  /* use "on" to enable symbolic IP anal */
char *D_sym_ip  = "off";                       

  /* use "3" to turn ip anal up */
char *D_sym_level = "0";                    

static char *environ_var_name  = NULL;
static char *environ_var_value = NULL;
STATIC(char, *GetEnvVarName,(char *str));

/* Set up the list of configuration variables */

/* 
   The structure below is for use with the D configuration file.  The file consists  
   of bindings for important strings.  If there is a binding entry in the file which  
   matches below, the entry will replace the default binding specified above.  The first
   entry (below) specifies an tag which may appear in the binding file.  The second    
   entry specifies a pointer to a string which may become modified as a result of      
   reading the file.  The third entry provides space for saving the strings read from  
   the file.  The file consists of blank lines, comment lines, and binding lines.       
   Leading blanks and tabs are ignored on all lines.  Comment lines begin with a '#'.   
   All other lines are binding lines which consist of a tag and a binding separated by 
   any number of blanks or tabs.  The tag must consist of only alphanumerics or the    
   underscore (_) character and it must appear below.  The binding string will become  
   the value of the corresponding string pointer. 
*/                                   

  /* D configuration file */
static  char *config_file = ".dsystemrc";   

  /* A TAGGED STRING VARIABLE */  
struct  var_tag                               
{
  char *name;                                  /* the name of the variable  */
  char **var;                                  /* the pointer to the variable */
  char storage[512];                           /* room for replacement text */ 
};                                     

  /* Note:  This structure is expected to  */
  /* be the same as that of mon/main.c.    */
  /* Do not change one without changing    */
  /* the other.                            */

  /* the list of D variables */  
static  struct  var_tag config_list[] =       
{
  { "fonts", &D_font_dirs[1], "" },
  { "helptexts", &D_help_dirs[2], "" },
  { "gtd", &D_gtd_dirs[2], "" },
  { "pcomp", &D_pcomp, "" },
  { "compile", &D_compile, "" },
  { "ptree", &D_ptree, "" },
  { "f77", &D_f77, "" },
  { "pfc_server", &D_pfc_server, "" },
  { "return_addr_cmd", &D_return_addr_cmd, "" },
  { "bin", &D_bin, "" },
  { "resource_id", &D_resource_identifier, "" },
  { "colorFile", &D_color_defaults_file, "" },
  { "symbolic_analysis", &D_symbolic_analysis, "" },
  { "sym_simplify", &D_sym_simplify, "" },
  { "sym_gated", &D_sym_gated, "" },
  { "sym_arrays", &D_sym_arrays, "" },
  { "sym_ip", &D_sym_ip, "" },
  { "sym_level", &D_sym_level,"" },                     
};

  /* the number of entries in the list  */
static short num_configs = sizeof(config_list) / sizeof(struct var_tag);        
 
  /* get an environment variable */
EXTERN(char*, getenv,(const char*)); 

  /* expect 200 character lines */
#define LINE_SIZE 200

/* Read and interpret the configuration file. */
void
resolveConfiguration()
{
  char filename[50];           /* temporary file name */
  char line[LINE_SIZE];        /* the input line from config file */
  char* d_home;                /* d home string pointer */
  char* start;                 /* the starting char of interest */
  char* end;                   /* the ending char of interest */
  char* store;                 /* pointer to the rest of storage */
  FILE* fp;                    /* the config file pointer */
  short i;                     /* the current binding entry */
  short found;                 /* true if a match is found */ 
  short line_number;           /* the current line number of file */

     /* check for DSYSTEM environment variable */
  if (!(d_home = getenv("DSYSTEM")) || *d_home != '/')
  {
     fprintf(stderr, "DSYSTEM environment variable not set correctly.\n");
     exit (1);
  }

  /* open configuration file, check in home directory & $DSYSTEM/src */
  fp = NULL;

     /* check in the home directory */
  if (!fp)
  {
     sprintf(filename, "%s/%s", getenv("HOME"), config_file);
     fp = fopen(filename, "r");
  }

     /* try the global directory */
  if (!fp)
  {
    sprintf(filename, "%s/src/%s", d_home, config_file);
    fp = fopen(filename, "r");
  }

     /* interpret the file */
  if (fp)
  {
        /* interpret a line from the input file */
     for (line_number = 1; fgets(line, LINE_SIZE - 1, fp); line_number++)
     {
           /* set start to be the first valid character */
        for (start = line; isspace(*start); start++) { }

           /* this line is not a comment or blank */
        if ((*start != '#') && (*start != '\0'))
        {
               /* set end to be the terminator of the tag */
           for (end = start; isalnum(*end) || (*end == '_'); end++) { }

           found = 0;

              /* search for a match in the configuration file */
           for (i = 0; (i < num_configs) && !found; i++)
           {
                 /* we have a match */
              if ((strlen(config_list[i].name) == end - start) && 
                  (strncmp(config_list[i].name, start, end - start) == 0))
              {
                 found = 1;

                    /* set start to be the first character of the value */
                 for (start = end; isspace(*start); start++) { }
                 store = config_list[i].storage;

                    /* transfer a character to the storge string */
                 for (end = start; (*end != '\0') && (*end != '\n'); end++)
                 {
                       /* expand the home string here */
                    if (*end == '$')
                    {
                       end = GetEnvVarName(end);
                       strcpy(store, environ_var_value);
                       store += strlen(environ_var_value);
                    }
                    else
                    {
                       *store++ = *end;
                    }
                 }
                 *config_list[i].var = config_list[i].storage;
              }
           }

              /* could not find the line */
           if (!found)
           {
              fprintf(stderr, "Could not interpret configuration file (%s) line %d in config.c.\n", config_file, line_number);
              fclose(fp);
              exit (1);
           }
        }
     }

     fclose(fp);
  }

     /* Expand the ~ to d_home for the unmodified variables in the list */
  for (i = 0; i < num_configs; i++)
  {
        /* this one needs to be expanded */
     if (*config_list[i].var != config_list[i].storage && strchr(*config_list[i].var, '$'))
     {
        store = config_list[i].storage;

           /* walk down the string */
        for (end = *config_list[i].var; *end; end++)
        {
              /* expand the home string here */
           if (*end == '$')
           {
              end = GetEnvVarName(end);
              strcpy(store, environ_var_value);
              store += strlen(environ_var_value);
           }
           else
           {
              *store++ = *end;
           }
        }
        *config_list[i].var = config_list[i].storage;
     }
  }
}

/* 
 * Interpret an <argc, argv> pair. Find and extract the "standard"
 * arguments that are defined for all ParaScope executables. As
 * each standard argument is consumed, it is removed from argv. The
 * number of nonstandard arguments is returned.
 */
int filterStandardArgs(int argc, char **argv)
{
  char* cptr;         /* the current modifier character */
  short arg;          /* the current argument being interpreted */
  short next_arg;     /* the slot in argc into which a copy of the next */
                      /* non-standard argument should be placed         */     
  int len;

     /* walk down the argument list */
  for (arg = 1, next_arg = 1; arg < argc; arg++)
  {
     cptr = argv[arg];
     len = strlen(cptr);
      
        /* skip any argument that doesn't start with '-' */
     if (len == 2 && cptr[0] == '-' && cptr[1] == 'm')
     {
        turn_on_mem_debug();    /* turn on memory debugging output */
     }
     else if (len == 2 && cptr[0] == '-' && cptr[1] == 'z')
     {
        set_zap_mem();          /* turn on the zapping of allocated & freed memory */
     }
     else if (len == 2 && cptr[0] == '-' && 
              (cptr[1] == '0' || cptr[1] == '1' || cptr[1] == '2'))
     {
#if 0
        malloc_debug(*cptr++ - '0');
#endif 
     }
     else
     {
        argv[next_arg++] = argv[arg];
     }
  }
  
  return next_arg;
}

/*
 * This function is invoked with the address of a string starting with an
 * environment variable reference $(XXX), and returns the address of the 
 * the closing paren, and fills in the string environ_var_name declared above.
 */
static 
char* GetEnvVarName(char* str)
{
  char *orig_str = str;
  char *end_str;

  assert(*str == '$');
  str++;
  if (*str != '(')
  {
    fprintf(stderr, "GetEnvVarName: error parsing environment variable at start of\n:");
    fprintf(stderr, "                %s\n", orig_str);
    fprintf(stderr, "                No opening paren found.\n");
    exit(-1);
  }
  str++;
  end_str = strrchr(str, ')');
  if (end_str == NULL)
  {
    fprintf(stderr, "GetEnvVarName: error parsing environment variable at start of\n:");
    fprintf(stderr, "                %s\n", orig_str);
    fprintf(stderr, "                No closing paren found.\n");
    exit(-1);
  }
  *end_str = '\0';

     /* grab the name of the environment variable */
  if (environ_var_name != NULL)
  {
    sfree(environ_var_name);
    sfree(environ_var_value);
  }
  environ_var_name = ssave(str);
  environ_var_value = ssave(getenv(environ_var_name));

     /* replace closing paren */
  *end_str = ')';

     /* closing paren */
  return end_str;
}
