/* $Id: file_read.C,v 1.1 1997/06/24 17:56:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/****************************************************************/
		/*			file_read.c				*/
		/* This file is a module which will evaluate a text expression	*/
		/* as if it were an integer expression in the language 'c'.	*/
		/* Help files are also read.					*/
		/*								*/
		/****************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <libs/support/misc/general.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/cmdProcs/help/help_file.h>

	/* globals */
static	char		*error_message;		/* the error message			*/
static	char		*cptr;			/* the pointer to the current character	*/
static	short		helpcp_num_args;	/* the number of arguments		*/
static	short		*helpcp_arg_list;	/* the list of arguments		*/
static	Generic		helpcp_handle;		/* the owner handle			*/
static	helpcp_heading_callback	helpcp_handle_heading;	/* a new heading			*/
static	helpcp_transfer_callback	helpcp_handle_transfer;	/* a new transfer id			*/
static	helpcp_identification_callback	helpcp_handle_identification;/* a new identification id		*/
static	helpcp_positioning_callback	helpcp_handle_positioning;/* a new positioning id		*/
static	helpcp_text_callback	helpcp_handle_text;	/* a new block of text			*/
static	short		helpcp_heading_level;	/* level of the current tree heading	*/
static	Boolean		helpcp_interactive;	/* true if interpretation is interactive*/

static	char		kb_prefix[] = "KB_";	/* common prefix of keyboard variables	*/


STATIC(int, conditional_op, (Boolean eval));

	/* EXPRESSION EVALUATION */

/* PARENTHESIS, NEGATION, LOGICAL NOT, BITWIZE NOT, LOOKUP OPERATOR, VARIABLES, NUMBERS	*/
static
int
unary_ops(Boolean eval)
{
register int		res;			/* the result of the calculation	*/
char			name[100];		/* variable name storage		*/
char			*ptr;			/* pointer into name			*/

	switch (*cptr++)
	{
		case '(':	/* look for ( exp ) */
			res = conditional_op(eval);
			if (cptr[0] == ')')
				cptr++;
			else
			{/* missing parenthesis */
				error_message = "Missing ')'.";
			}
			break;
		case '-':	/* unary negation */
			res = -unary_ops(eval);
			break;
		case '!':	/* logical negation */
			res = !unary_ops(eval);
			break;
		case '~':	/* bitwise negation */
			res = ~unary_ops(eval);
			break;
		case '#':	/* lookup operator */
			res = unary_ops(eval);
			if ((res >= 0) && (res < helpcp_num_args))
				res = helpcp_arg_list[res];
			else if (eval)
			{/* # value out of range */
				error_message = "Argument to # out of bounds";
			}
			break;
		default:	/* number or identifier */
			cptr--;
			if (isdigit(*cptr))
			{/* read through the digits and calculate the number */
				for (res = 0; isdigit(cptr[0]); cptr++)
					res = res * 10 + (cptr[0] - '0');
			}
			else if (isalpha(*cptr))
			{/* we have the start of a variable name--interpret it */
				ptr = name;
				while (isalnum(*cptr) || *cptr == '_')
				{/* add this character to the variable name */
					*ptr++ = *cptr++;
				}
				*ptr ='\0';
				if (strcmp(name, "interactive") == 0)
				{/* get the keyboard index */
					res = (int) helpcp_interactive;
				}
				else if (strcmp(name, "swap_bs_del") == 0)
				{/* get the keyboard index */
					res = (int) kb_swap_bs_del;
				}
				else if (strcmp(name, "keyboard") == 0)
				{/* get the keyboard index */
					res = kb_keyboard_id;
				}
				else if (strncmp(name, kb_prefix, strlen(kb_prefix)) == 0)
				{/* try to look up keyboard names */
					ptr = name + strlen(kb_prefix);
					for (res = 0; kb_names[res]; res++)
					{/* walk down the keyboard names list */
						if (strcmp(ptr, kb_names[res]) == 0)
						{/* we have matched this keyboard variable */
							break;
						}
					}
					if (!kb_names[res])
					{/* couldn't make sense out of the keyboard variable */
						error_message = "Couldn't recognize keyboard variable.";
					}
				}
				else
				{/* couldn't find the variable */
					error_message = "Couldn't make sense of identifier.";
				}
			}
			else
			{/* we were expecting a number or identifier */
				error_message = "Number or identifier expected.";
			}
			break;
	}
	return (res);
}


/* MULTIPLY, DIVIDE, or MOD								*/
static
int
binary_multiplication(Boolean eval)
{
register int		res;			/* the result of the calculation	*/
register int		res_denom;		/* the denominator result		*/

	res = unary_ops(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '*') || (cptr[0] == '/') || (cptr[0] == '%'))
		{/* perform a cascaded multiplication type operator */
			switch (*cptr++)
			{
				case '*':
					res = res * unary_ops(eval);
					break;
				case '/':
					res_denom = unary_ops(eval);
					if (!eval)
						res = 0;
					else if (res_denom == 0)
						error_message = "Division by zero.";
					else
						res = res / res_denom;
					break;
				case '%':
					res_denom = unary_ops(eval);
					if (!eval)
						res = 0;
					else if (res_denom == 0)
						error_message = "Division by zero.";
					else
						res = res % res_denom;
					break;
			}
		}
	}
	return (res);
}


/* PLUS or MINUS									*/
static
int
binary_addition(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = binary_multiplication(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '+') || (cptr[0] == '-'))
		{/* perform a cascaded addition type operation */
			switch (*cptr++)
			{
				case '+':
					res = res + binary_multiplication(eval);
					break;
				case '-':
					res = res - binary_multiplication(eval);
					break;
			}
		}
	}
	return (res);
}


/* Shift left or right									*/
static
int
binary_shift(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = binary_addition(eval);
	if (!error_message)
	{/* no error has occurred */
		while ( ((cptr[0] == '<') && (cptr[1] == '<')) || ((cptr[0] == '>') && (cptr[1] == '>')) )
		{/* perform a cascaded shift type operation */
			switch (*cptr++)
			{
				case '<':
					cptr++;
					res = res << binary_addition(eval);
					break;
				case '>':
					cptr++;
					res = res >> binary_addition(eval);
					break;
			}
		}
	}
	return (res);
}


/* Less than (<), less than or equal to (<=), greater than (>), greater than or equal	*/
/* to (>=)										*/
static
int
logical_comparison(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = binary_shift(eval);
	if (!error_message)
	{/* no error has occurred */
		while ( ((cptr[0] == '<') && (cptr[1] != '<')) || ((cptr[0] == '>') && (cptr[1] != '>')) )
		{/* perform a cascaded comparison type operation */
			switch (*cptr++)
			{
				case '<':
					if (cptr[0] == '=')
					{/* <= */
						cptr++;
						res = (res <= binary_shift(eval));
					}
					else
						res = (res < binary_shift(eval));
					break;
				case '>':
					if (cptr[0] == '=')
					{/* >= */
						cptr++;
						res = (res >= binary_shift(eval));
					}
					else
						res = (res > binary_shift(eval));
					break;
			}
		}
	}
	return (res);
}


/* Equals (==) and not equals (!=)							*/
static
int
logical_equality(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = logical_comparison(eval);
	if (!error_message)
	{/* no error has occurred */
		while (((cptr[0] == '!') || (cptr[0] == '=')) && (cptr[1] == '='))
		{/* perform a cascaded equality type operation */
			switch (*cptr++)
			{
				case '=':
					cptr++;
					res = (res == logical_comparison(eval));
					break;
				case '!':
					cptr++;
					res = (res != logical_comparison(eval));
					break;
			}
		}
	}
	return (res);
}


/* Bitwise AND										*/
static
int
bitwise_and(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = logical_equality(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '&') && (cptr[1] != '&'))
		{/* perform a cascaded bitwise and operation */
			cptr++;
			res = (res & logical_equality(eval));
		}
	}
	return (res);
}


/* Bitwise XOR										*/
static
int
bitwise_xor(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = bitwise_and(eval);
	if (!error_message)
	{/* no error has occurred */
		while (cptr[0] == '^')
		{/* perform a cascaded bitwise xor operation */
			cptr++;
			res = (res ^ bitwise_and(eval));
		}
	}
	return (res);
}


/* Bitwise OR										*/
static
int
bitwise_or(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = bitwise_xor(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '|') && (cptr[1] != '|'))
		{/* perform a cascaded bitwise or operation */
			cptr++;
			res = (res | bitwise_xor(eval));
		}
	}
	return (res);
}


/* Logical AND										*/
static
int
logical_and(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = bitwise_or(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '&') && (cptr[1] == '&'))
		{/* perform a cascaed logical and operation */
			cptr += 2;
			eval = BOOL(eval && res);
			res = (bitwise_or(eval) && res);	/* put function first to make sure it is called */
		}
	}
	return (res);
}


/* Logical OR										*/
static
int
logical_or(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = logical_and(eval);
	if (!error_message)
	{/* no error has occurred */
		while ((cptr[0] == '|') && (cptr[1] == '|'))
		{/* perform a cascaded logical or operation */
			cptr += 2;
			eval = BOOL(eval && !res);
			res = (logical_and(eval) || res);	/* put function first to make sure it is called */
		}
	}
	return (res);
}


/* Conditional operator									*/
static
int
conditional_op(Boolean eval)
{
register int		res;			/* the result of the calculation	*/

	res = logical_or(eval);
	if (!error_message)
	{/* no error has occurred */
		if (cptr[0] == '?')
		{/* this is a conditional */
			cptr++;
			if (res)
			{/* the first condition counts */
				res = conditional_op(eval);
				if (!error_message)
				{/* no error in the first condition */
					if (cptr[0] != ':')
					{/* we have an error */
						error_message = "Colon expected (:).";
					}
					else
					{/* ignore the second part of the conditional */
						cptr++;
						(void) conditional_op(false);
						error_message = (char *) 0;
					}
				}
			}
			else
			{/* the second condition counts */
				(void) conditional_op(false);
				error_message = (char *) 0;
				if (cptr[0] != ':')
				{/* we have an error */
					error_message = "Colon expected (:).";
				}
				else
				{/* return the second conditional */
					cptr++;
					res = conditional_op(eval);
				}
			}
		}
	}
	return (res);
}


	/* FILE READING */

	/* special textfile characters & format values */
#define	MAX_LINE_SIZE		256		/* maximum number of chars/input line	*/
#define SPACE			' '		/* ascii space character		*/
#define EOLN			'\n'		/* ascii eoln				*/
#define	COMMENT			';'		/* comment separater character		*/
#define	HEADING			'\\'		/* beginning of a subject heading	*/
#define IDENTIFICATION		'!'		/* identification follows		*/
#define POSITIONING		'@'		/* positioning follows			*/
#define TRANSFER		'*'		/* transfer follows			*/
#define CHOICE			'{'		/* beginning of conditional		*/
#define END_CHOICE		'}'		/* end of conditional			*/
#define	FILE_INCLUDE		'~'		/* file inclusion			*/
#define DOUBLE_SPACE(c)		((c == '.') || (c == '?') || (c == ':'))
						/* double space after if this is TRUE	*/

#define	BUFFER_INCREMENT	1000		/* buffer increment value		*/
static	char		*buffer = NULL;		/* the current node's text		*/
static	short		buff_size = 0;		/* the size allocated for the buffer	*/
static	short		buff_len;		/* number of characters in the buffer	*/


/* Add a character to the end of the buffer.  Expand the buffer if necessary		*/
#define	ADD_CHAR(c)										\
/* char			c;			/* the character to be added		*/	\
{												\
	if (buff_len == buff_size)								\
		augment_buffer();							\
	buffer[buff_len++] = c;									\
}


/* Increase the size of the buffer.							*/
static
void
augment_buffer()
{
register char		*new_buffer;		/* the new buffer			*/

	buff_size += BUFFER_INCREMENT;
	new_buffer = (char *) get_mem(buff_size, "Helpcp: (file_read.c) text buffer increment");
	if (buffer)
	{/* there was something in the old buffer */
		(void) strncpy(new_buffer, buffer, buff_len);
		free_mem((void*) buffer);
	}
	buffer = new_buffer;
}


/* Terminate the buffer by deleting trailing separaters & adding an endstring		*/
static
void
terminate_buffer()
{
	ADD_CHAR(SPACE);
	while (buff_len && ( (buffer[buff_len - 1] == SPACE) || (buffer[buff_len - 1] == PARAGRAPH) ))
		buffer[--buff_len] = '\0';
}


/* Process the line by processing each character.					*/
static
void
process_line(char *current)
{
char			prev;			/* the previous character in the buffer	*/
char			*New;			/* the new human readable binding	*/
KbString		kbs;			/* the current KbString binding		*/
char			*temp;			/* temporary pointer to a char		*/

	while (*current)
	{/* process a character from the line */
		if (isspace(*current))
		{/* is a space. */
			prev = buffer[buff_len - 1];
			if (buff_len && DOUBLE_SPACE(prev))
			{/* put two spaces here only */
				ADD_CHAR(SPACE);
				ADD_CHAR(SPACE);
			}
			else if (buff_len && (prev != SPACE) && (prev != PARAGRAPH))
			{/* put one space after a non separater */
				ADD_CHAR(SPACE);
			}
		}
		else if (*current == QUOTE_CHAR)
		{/* quote the next character */
			if (current[1])
			{/* move the quoted character */
				if (!isprint(current[1]))
					ADD_CHAR(QUOTE_CHAR);
				ADD_CHAR(*++current);
			}
			else
				error_message = "Cannot place quote character (^Q) at the end of a line.";
		}
		else if (*current == PARAGRAPH)
		{/* eat all previous separaters and put a flush in */
			if (buff_len)
			{/* add a new flush */
				terminate_buffer();
				ADD_CHAR(PARAGRAPH);
			}
		}
		else if (*current == BEGIN_KEYBOARD)
		{/* beginning of a key binding */
			if (temp = strchr(current + 1, END_KEYBOARD))
			{/* we have a conversion to make */
				*temp = '\0';
				kbs = symbolicToKbString(current + 1);
				if (kbs.num_kc != UNUSED)
				{/* a good binding */
					New = actualFromKbString(kbs);
					current = temp;
					ADD_CHAR(BEGIN_KEYBOARD);
					for (temp = New; *temp; temp++)
						ADD_CHAR(*temp);
					ADD_CHAR(END_KEYBOARD);
					free_mem((void*) New);
				}
				else
				{/* a bogus binding */
					error_message = "Bogus keybinding specification.";
				}
			}
			else
			{/* unmatched keyboard bracket */
				error_message = "Missing ^E for ^S in keybinding specification.";
			}
		}
		else if (isprint(*current))
		{/* normal character */
			ADD_CHAR(*current);
		}
		else
		{/* funny character */
			error_message = "Unrecognized character.";
		}
		current++;
	}
	/* add a space to the end of the string  */
		prev = buffer[buff_len - 1];
		if (buff_len && DOUBLE_SPACE(prev))
		{/* put two spaces here only */
			ADD_CHAR(SPACE);
			ADD_CHAR(SPACE);
		}
		else if (buff_len && (prev != SPACE) && (prev != PARAGRAPH))
		{/* put one space after a non separater */
			ADD_CHAR(SPACE);
		}
}


/* Evaluate an input expression at line.						*/
static
int
evaluate_line(char line[])
{
register char		*src;			/* pointer to current position in line	*/
register char		*dest;			/* pointer to current position in line	*/
register int		res;			/* the result of the calculation	*/

	cptr = line;
	/* truncate at start of a comment */
		if (dest = strchr(line, COMMENT))
			*dest = '\0';

	/* eat spaces */
		dest = line;
		for (src = line; *src; src++)
			if (!isspace(*src))
				*dest++ = *src;
		*dest = '\0';

	res = conditional_op(true);
	if (*cptr && !error_message)
	{/* there were more characters to interpret */
		error_message = "Extra characters could not be interpreted.";
	}
	return (res);
}


/* Read the help_file and add it to the tree at the specified level.			*/
static
Boolean
read_file(char *err_buf, char *file_name, short level)
{
		/* directories to look for help text files */
extern	char		*D_help_dirs[];	/* directories to searh in (0 termin.)	*/
char			**trial_dir;		/* the remaining directory list		*/
char			file[100];		/* trial file path name			*/
FILE			*fd = NULL;		/* the current file descriptor		*/
		/* line processing variables */
short			line_count;		/* the number of file lines processed	*/
char			i_line[MAX_LINE_SIZE];	/* the current input line		*/
char			*current;		/* ptr to current char			*/
char			*start;			/* ptr to first valid char in i_line	*/
		/* file inclusion processing variables */
char			*included_file;		/* the name of the included file	*/
char			included_message[200];	/* message string to pass to recursions	*/
		/* conditional processing variables */
short			conditional_count;	/* the # of { lines minus # of } lines	*/
short			eating;			/* depth of condition levels skipping	*/
		/* other variables */
int			val;			/* the value of a text expression	*/
Boolean			seen_positioning = false;/* flag for positioning error		*/
short			i;			/* dummy index number			*/

	for (trial_dir = D_help_dirs; *trial_dir && !fd; trial_dir++)
	{/* try opening the ith directory */
		(void) strcpy(file, *trial_dir);
		(void) strcat(file, file_name);
		fd = fopen(file, "r");
	}
	if (fd == NULL)
	{/* could not open file -- quit */
		(void) sprintf(err_buf, "Help file '%s' could not be opened.", file_name);
		return (false);
	}

	line_count = 0;		/* the number of lines read in			*/
	conditional_count = 0;	/* open conditionals balance close conditionals */
	eating = 0;		/* so far, we are not skipping any text		*/

	error_message = (char *) 0;
	while (!error_message && fgets(i_line, MAX_LINE_SIZE, fd))
	{/* process a line from the text file */
		line_count++;
		/* delete the trailing EOLN and end the string */
			i = strlen(i_line);
			if (i && (i_line[i - 1] == EOLN))
				i_line[--i] = '\0';
		/* set start to the first valid character of the line */
			for (start = i_line; isspace(*start); start++)
				;

		switch (*start)
		{/* interpret the line that begins at start */
			case (int) NULL:

			case COMMENT:		/* comment or blank line--ignore it */
				break;

			case HEADING:		/* new subject heading */
				if (!eating)
				{/* process a new header */
					terminate_buffer();
					if (helpcp_handle_text && buff_len)
						helpcp_handle_text(helpcp_handle, buffer, buff_len);
					buff_len = 0;

					/* figure out the level in the outline of the new node */
						current = start + 1;
						while(*current && !isspace(*current))
							current++;
						*(current++) = '\0';
						val = evaluate_line(start + 1) + level;
						if (error_message)
						{/* bad evaluation */
							break;
						}
					if (val > helpcp_heading_level + 1 || val < 0 || (val < 1 && helpcp_heading_level != UNUSED))
					{/* the supposed parent entry is above level 0 or there is    */
					/* a gap between this new's level and the first possible parent*/
						error_message = "Inconsistent heading number.";
						break;
					}
					/* eat the blank space between the level and the header */
						while(isspace(*current))
							current++;
					/* figure the node's text */
						process_line(current);
						terminate_buffer();
						if (helpcp_handle_heading)
							helpcp_handle_heading(helpcp_handle, buffer, buff_len, val);
						buff_len = 0;
					helpcp_heading_level = val;
					seen_positioning = false;
				}
				break;

			case POSITIONING:	/* a node positioning mark */
				if (helpcp_heading_level == UNUSED)
				{/* bad place for positioning information */
					error_message = "Positioning information seen before first heading.";
				}
				else if (!eating)
				{/* this is a correct positioning--set the current node & come up in text mode */
					if (seen_positioning)
					{/* duplicate positioning */
						error_message = "Positioning information duplicated in this heading.";
					}
					else
					{/* this is the first positioning */
						val = evaluate_line(start + 1);
						if (!error_message && helpcp_handle_positioning)
							helpcp_handle_positioning(helpcp_handle, val);
						seen_positioning = true;
					}
				}
				break;

			case IDENTIFICATION:	/* a node identification */
				if (helpcp_heading_level == UNUSED)
				{/* bad place for identification information */
					error_message = "Identification information seen before first heading.";
				}
				else if (!eating)
				{/* this is a valid identification */
					val = evaluate_line(start + 1);
					if (!error_message && helpcp_handle_identification)
						helpcp_handle_identification(helpcp_handle, val);
				}
				break;

			case TRANSFER:		/* a transfer */
				if (helpcp_heading_level == UNUSED)
				{/* bad place for transfer information */
					error_message = "Transfer information seen before first heading.";
				}
				else if (!eating)
				{/* this is valid--augment the transfer information */
					val = evaluate_line(start + 1);
					if (!error_message && helpcp_handle_transfer)
						helpcp_handle_transfer(helpcp_handle, val);
				}
				break;

			case CHOICE:		/* a conditional set follows */
				if (eating || !( evaluate_line(start + 1) ))
				{/* another sub-condition level to be eaten */
					eating++;
				}
				conditional_count++;
				break;

			case END_CHOICE:	/* the end of a conditional choice */
				if (eating)
				{/* one less sub-condition level to be eaten */
					eating--;
				}
				conditional_count--;
				if (conditional_count < 0)
				{/* there are more close than open conditionals at this point */
					error_message = "Too many close conditionals (}).";
				}
				break;

			case FILE_INCLUDE:	/* include another file */
				if (!eating)
				{/* include a file here */
					/* clear out any buffered text */
						terminate_buffer();
						if (helpcp_handle_text && buff_len)
							helpcp_handle_text(helpcp_handle, buffer, buff_len);
						buff_len = 0;
					current = start + 1;
					while (*current && isspace(*current))
						current++;
					included_file = current;
					if (*current == '\0')
					{/* no file name */
						error_message = "No file name specified for file inclusion.";
					}
					else
					{/* we have a file name */
						while (*current && !isspace(*current))
							current++;
						if (*current == '\0')
						{/* nothing after the file name */
							error_message = "No separation between file name and depth expression.";
						}
						else
						{/* finish interpretation of the line */
							*current = '\0';	/* end included_file */
							val = evaluate_line(current + 1);
							if (!error_message)
							{/* read in the included file */
								if (!read_file(included_message, included_file, val + level))
								{/* error in the included file */
									error_message = included_message;
								}
							}
						}
						
					}
				}
				break;

			default:		/* text--read right through it */
				if (!eating)
				{/* add this text to the current buffer */
					if (helpcp_heading_level == UNUSED)
					{/* bad place for text information */
						error_message = "Text seen before first heading.";
					}
					else
					{/* process the line */
						process_line(start);
					}
				}
				break;
		}
	}
	(void) fclose (fd);

	if (!error_message && (helpcp_heading_level == UNUSED))
	{/* the file had no nodes */
		error_message = "No headings in the file.";
	}
	else if (!error_message && conditional_count)
	{/* the file has unmatched conditionals */
		error_message = "Too many open conditionals ({) detected.";
	}
	else if (!error_message)
	{/* set up current and last & dump text */
		terminate_buffer();
		if (helpcp_handle_text && buff_len)
			helpcp_handle_text(helpcp_handle, buffer, buff_len);
		buff_len = 0;
	}
	else
	{/* clear out the buffer anyway */
		buff_len = 0;
	}

	if (error_message)
	{/* print out a reasonable error message */
		(void) sprintf(err_buf, "Error in line %d of help file '%s':\n%s", line_count, file_name, error_message);
		return false;
	}
	else
	{/* we have a successful run */
		return true;
	}
}


/* Read through the file calling client supplied subroutines as needed.			*/
char *
helpcp_file_read(char *file_name, short num_args, short *arg_list, 
                 Boolean interactive, Generic handle, 
		 helpcp_heading_callback handle_heading, 
                 helpcp_transfer_callback handle_transfer, 
		 helpcp_identification_callback handle_identification, 
                 helpcp_positioning_callback handle_positioning, 
		 helpcp_text_callback handle_text)
{
static	char		error_buffer[200];	/* storage for error result string	*/
Boolean			success;		/* true if the file was read properly	*/

	/* set up to run the evaluator */
		helpcp_num_args               = num_args;
		helpcp_arg_list               = arg_list;
		helpcp_handle                 = handle;
		helpcp_handle_heading         = handle_heading;
		helpcp_handle_transfer        = handle_transfer;
		helpcp_handle_identification  = handle_identification;
		helpcp_handle_positioning     = handle_positioning;
		helpcp_handle_text            = handle_text;
		helpcp_heading_level          = UNUSED;
		helpcp_interactive	      = interactive;

	/* doit */
		success = read_file(error_buffer, file_name, 0);

	if (buffer)
	{/* delete the text buffer */
		free_mem((void*) buffer);
		buffer = NULL;
		buff_size = 0;
		buff_len = 0;
	}

	return (success) ? (char *) 0 : error_buffer;
}



