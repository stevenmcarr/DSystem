/* $Id: read_util.C,v 1.1 1997/06/25 15:10:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/utilities/read_util.c				        */
/*									*/
/*	read_util.c -- functions for reading graph and map file		*/
/*									*/
/*	get_field_d()                					*/
/*									*/
/*									*/
/************************************************************************/


#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/utilities/read_util.h>


/************************************************************************/
/*		F I E L D    A C C E S S    F U N C T I O N S		*/
/*									*/
/* These return a pointer to the start of the next field after separator*/
/* The fields "value" is stored at the address in the second argument.	*/
/* "!" is assumed to be the field separator.	mpal, 910622		*/
/************************************************************************/

/*--------------------------------------------------------------------------
  skip_field() - skip past this field.
  --------------------------------------------------------------------------*/
char*
skip_field(char* start, char** buf)
{
    char	*str;
    /* find start of next field	*/
    for( str = start; (*str != '!') && (*str != '\n') && (*str != '\0'); str++);
    
    return( str + 1 );
}

/*--------------------------------------------------------------------------
  get_field_b() - Store the Boolean value of this field at bool.
  --------------------------------------------------------------------------*/
char* 
get_field_b(char* start, Boolean* bool)
{
    char	*str;
    /* find start of next field	*/
    for( str = start; (*str != '!') && (*str != '\n') && (*str != '\0'); str++);
    
    if( *start == '1' )
	*bool = true;
    else
	*bool = false;
    
    return( str + 1 );
}

/*--------------------------------------------------------------------------
  get_field_d() - Store the decimal value of this field at num.
  --------------------------------------------------------------------------*/
char*
get_field_d(char* start, int* num)
{
    char	*str;

    /* find start of next field	*/
    for( str = start; (*str != '!') && (*str != '\n') && (*str != '\0'); str++);
    
    *str	= '\0';
    sscanf( start, "%d", num );
    return( str + 1 );
}

/*--------------------------------------------------------------------------
  get_field_c() - Store the consistent value in this field at cons_ptr.
  --------------------------------------------------------------------------*/
char*
get_field_c(char* start, ConsistentType* cons_ptr)
{
    char	*str;

    /* find start of next field	*/
    for( str = start; (*str != '!') && (*str != '\n') && (*str != '\0'); str++);
    
    *str	= '\0';

    /* convert PSERVE format into local data type	*/
    if( *start == '\0' || *start == '0' )
      *cons_ptr	= inconsistent;
    else if( *start == '1' )
      *cons_ptr = consistent_SIV;
    else if( *start == '2' )
      *cons_ptr = consistent_MIV;
    else
      *cons_ptr	= inconsistent;

    return( str + 1 );
}

/*--------------------------------------------------------------------------
  get_field_s() - Store a pointer to this field at buf.
  --------------------------------------------------------------------------*/
char* 
get_field_s(char* start, char** buf)
{
    char	*str;
    
    *buf	= start;
    /* find start of next field	*/
    for( str = start; (*str != '!') && (*str != '\n') && (*str != '\0'); str++); 
    
    *str	= '\0';			
    return( str + 1 );			/* pointer to start of next field */
}
