/* $Id: static.C,v 1.4 2000/02/07 23:56:44 mjbedy Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <unistd.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <include/frontEnd/astnode.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/classes.h>
#include <libs/f2i/mnemonics.h>

#include <stdio.h>

/* forward declarations */

extern int  proc_type; 	/* shared with procedures.c */

static void AddConstantsToTheFileOfStaticData(SymDescriptor,fst_index_t,Generic);
static void byte_fill(int, int, int*, char *);



/* generate static data area including initializations from DATA statements */
void aiGenerateStaticArea()
{
  register int st;

  FILE	*fd;

  char command[128];
  char *TempIn = ".aiDataIn";
  char *TempOut = ".aiDataOut";
  int offset;
  int size;

  /* need to generate external symbol entries for this routine		*/
  st = aiStaticLabel();
  if (proc_type != GEN_BLOCK_DATA)
  {
    generate(0, NAME,
	     (int) fst_my_GetFieldByIndex(ft_SymTable, st, SYMTAB_NAME), 
	     0, 0, NOCOMMENT);
    generate(0, NAME, (Generic) proc_name, 0, 0, NOCOMMENT);
  }

  fd = fopen(TempIn, "w");

  if (fd == NULL)
  {
    (void) sprintf(error_buffer, "Cannot open temporary file '%s'", TempIn);
    ERROR("aiGenerateStaticArea", error_buffer, FATAL);
  }

  if (aiDebug > 0)
     (void) fprintf(stdout, "\tGenerateStaticArea: using filenames '%s' and '%s'.\n", 
	     TempIn, TempOut);


  generate(0, NOP, 0, 0, 0, "Initialized data");

  if (aiSparc == 0 && proc_type != GEN_BLOCK_DATA)
  {

    /* static area holds pointer to code in loc[0]	  RT specific? 	*/

    (void) fprintf(fd, "%4d %10d %4d %s\n", -1, 0,  TYPE_LABEL,	proc_name);
    if (aiRt)
      {
       (void) fprintf(fd, "%4d %10d %4d %d\n", -1, 4,  TYPE_INTEGER, 0);
       (void) fprintf(fd, "%4d %10d %4d %d\n", -1, 8,  TYPE_INTEGER, 0);
       (void) fprintf(fd, "%4d %10d %4d %d\n", -1, 12, TYPE_INTEGER, 0);
      }

    ProcessData(fd);

    /*  Add constants to the file of static data  */
    fst_ForAll (ft_SymTable, AddConstantsToTheFileOfStaticData, (Generic) fd);

  }
  else /* Block Data Subprogram */
    ProcessData(fd); 			/* no constant pool needed */

  /*  close the temporary file  */
  (void) fclose(fd);

  (void) sprintf(command, "sort -n -k 1 -k 2 %s >%s", TempIn, TempOut);
  (void) system(command);

  fd = fopen(TempOut, "r");

  if (fd == NULL)
  {
    (void) sprintf(error_buffer, "Cannot open temporary file '%s'", TempOut);
    ERROR("aiGenerateStaticArea", error_buffer, FATAL);
  }

  OutputData(fd);

  (void) fclose(fd);

  if (aiDebug < 1)
  {
    (void) unlink(TempIn);
    (void) unlink(TempOut);
  }

  if (aiNextStack > aiStackSize)
  {
    (void) sprintf(error_buffer, 
	"Actual stack size (%d) is greater than PrePass predicition (%d)",
	aiNextStack, aiStackSize);
    ERROR("aiGenerateStaticArea", error_buffer, SERIOUS);

    (void) sprintf(error_buffer, "NextStack %d, ExpSpace %d, StackSize %d",
	aiNextStack, aiExpressionStackSpace, aiStackSize);
    ERROR("aiGenerateStaticArea", error_buffer, SERIOUS);
  }
  if (aiGenerate == 0)
    {
     (void) sprintf(error_buffer,
     "The routine required %d static bytes and %d stack bytes.",
     aiNextStatic, aiNextStack);
     generate(0, NOP, 0, 0, 0, error_buffer);
    }
} /* aiGenerateStaticArea */




/*ARGSUSED*/
/* add constants to the file of static data */
static void AddConstantsToTheFileOfStaticData(SymDescriptor SymTab,
					      fst_index_t i,Generic fd)
// SymDescriptor SymTab;
// fst_index_t i;
// Generic fd;
{
  if ((fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_STORAGE_CLASS) & SC_CONSTANT) && 
      (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_INTEGER)	    &&
      (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_LOGICAL)	    &&
      (strcmp((char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME), "2nd reg")
	!=0))
    {
      if (fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE) != TYPE_CHARACTER)
	(void) fprintf((FILE *) fd, "%4d %10d %4d %s\n", -1,
		       fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset), 
		       fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE),
		       Prefix((char *)
				fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME)));
      else
	PadAndPut(i, (FILE *) fd);
    }
} /* AddConstantsToTheFileOfStaticData */




/* generate iloc DATA statements */
void OutputData( FILE *fd )
  // FILE * fd;
{
  int   area, new_area, new_name;
  char	*name;			/* name of current data area	*/
  int	length;			/* length of current data area	*/
  int	new_type;
  int	offset, new_offset;
  char	text[1024];
  char  label[64];
  char  temp_name[64];

  int	dups, flag;

  dups = 0;

  /* set it up */
  area    = -2;
  length  = 0;
  offset  = 0;
  new_name= 1;

  flag = fscanf(fd, "%d %10d %4d %[^\n]s", 
		&new_area, &new_offset, &new_type, text);

  while (flag != EOF)
  {
    if (area != new_area)
    {
      if (offset < length)		/* pad it out to full length */
	 byte_fill(offset, length, &new_name, label);

      offset = 0;			/* and reset the offset	*/

      area = new_area;
      if (area == -1)	/* static data area */
      {
	name	= (char *)
		fst_my_GetFieldByIndex(ft_SymTable, aiStaticLabel(), SYMTAB_NAME);
	length	= aiNextStatic;
      }
      else if (0 < area &&
		(fst_GetFieldByIndex(ft_SymTable, area, SYMTAB_OBJECT_CLASS) &
			    OC_IS_COMMON_NAME)) /* a common block */
      {
	(void) strcpy(temp_name, (char *) (fst_GetFieldByIndex(ft_SymTable, area, SYMTAB_NAME)+1));
	temp_name[strlen(temp_name)-1]='\0';
	(void) sprintf(label, "_%s_", temp_name);
	name	= label;
	length	= fst_GetFieldByIndex(ft_SymTable, area, SYMTAB_SIZE);
	generate(0, NAME, (Generic) name, 0, 0, NOCOMMENT);
	new_name= 1;
      }
      else
      {
	(void) sprintf(error_buffer, 
	       "Data area index (%d) is out of bounds for common_table", area);
	ERROR("OutputData", "AI logic error - OutputData()", SERIOUS);
	ERROR("OutputData", error_buffer, SERIOUS);
	ERROR("OutputData", "Generates incorrect initialization", FATAL);
      }
    }

    if (offset < new_offset)	/* fill a hole, if one exists	*/
    {
      byte_fill(offset, new_offset, &new_name, label);
      offset = new_offset;
    }
    else if (offset > new_offset)
    {
      (void) sprintf(error_buffer, "Alignment error in data area '%s'", name);
      ERROR("OutputData", "AI logic error - OutputData()", SERIOUS);
      ERROR("OutputData", error_buffer, SERIOUS);
      ERROR("OutputData", "Generates incorrect initialization", FATAL);
    }
	
      if (new_name == 0)	   /* OUTPUT the data item	 	*/
	 generate(0, bDATA, (Generic) text, 1, DataFlagFromIType(new_type), NOCOMMENT);
      else
      {
	generate_string(name, bDATA, (Generic) text, 1, DataFlagFromIType(new_type), 
		 	NOCOMMENT);
	new_name = 0;
      }

    if (new_type != TYPE_CHARACTER)		/* Now, adjust the offset */
       new_offset += SizeOfType(new_type);	/* and set up for the     */
    else					/* next iteration	  */
       new_offset += QuotedLength(text);

    offset = new_offset;

    flag = fscanf(fd, "%d %10d %4d %[^\n]s", 
		  &new_area, &new_offset, &new_type, text);

    // If it's the end of the file, there seems to be a problem here...
    // MJB
    while (area == new_area && offset > new_offset && flag != EOF) /* a check for duplicates */
    {
      dups ++;
      flag = fscanf(fd, "%d %10d %4d %[^\n]s", 
		    &new_area, &new_offset, &new_type, text);
    }
  }

  if (new_offset < length)
     byte_fill(new_offset, length, &new_name, label);

  if (dups > 0)
  {
    (void) sprintf(error_buffer,
	"Data statements initialize a single location multiple times (%d occurences)",
	dups);
    ERROR("OutputData", error_buffer, WARNING);
  }
} /* OutputData */




/* return flag indicating the type of DATA object to be printed */
int DataFlagFromIType( int type )
  // int type;
{
  int result;

  switch(type) 
  {
	case TYPE_CHARACTER:		result = DATA_CHARACTER;	break;
	case TYPE_LOGICAL:		result = DATA_LOGICAL;		break;
	case TYPE_INTEGER:		result = DATA_INTEGER;		break;
	case TYPE_LABEL:		result = DATA_STRING_LABEL;	break;
	case TYPE_REAL:			result = DATA_FLOAT;		break;
	case TYPE_COMPLEX:		result = DATA_COMPLEX;		break;
	case TYPE_DOUBLE_PRECISION: 	result = DATA_DOUBLE;		break;
	case TYPE_DOUBLE_COMPLEX:	result = DATA_DOUBLE_COMPLEX;	break;	
	case TYPE_UNKNOWN:	
	default:
		result = DONT_KNOW;
		(void) sprintf(error_buffer, 
		    "Invalid data type encountered in iloc DATA statement '%d'",
		    type);
		ERROR("DataFlagFromIType", error_buffer, FATAL);
		break;
  }
  return result;
} /* DataFlagFromIType */




/* ???? */
void PadAndPut(int i, FILE *fd)
  //int i;
  //FILE *fd;
{
  char buffer[2048];
  char *p, *q;
  int len = 0;

  p = (char *) fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_NAME);
  if (*p == '\'')
  {

    p++;
    q = buffer;
    while (*p != '\'' && *p != '\0')
    {
      *q++ = *p++;
      len ++;
    }
    *q = '\0';

  }

  (void) fprintf(fd, "%4d %10d %4d '%s", -1, 
		 fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_offset),
		 fst_my_GetFieldByIndex(ft_SymTable, i, SYMTAB_TYPE), buffer);

  len = Align(len) - len;
  while (len > 0)
  {
    (void) fputc(' ', fd);
    len--;
  }
  (void) fputc('\'', fd); 
  (void) fputc('\n', fd);
} /* PadAndPut */




/* since a byte ('b') can be used in the data area of the assembly code, */
/* an integer fill cannot be used because of alignment concerns,  thus   */
/* uninitialized byte fills will be used.				 */
static void byte_fill(int here, int there, int *New, char *label)
//   int here, there, *new;
//   char *label;
{
  if (*New == 0)
       generate(0, BYTES, there-here, 0, 0, "Uninitialized byte fill");
  else
    {
       generate_string(label, BYTES, there-here, 0, 0, "Uninitialized byte fill");
       *New = 0;
    }
} /* zero_fill */
