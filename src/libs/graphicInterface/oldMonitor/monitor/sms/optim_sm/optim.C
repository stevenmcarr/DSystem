/* $Id: optim.C,v 1.1 1997/06/25 14:58:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
        /****************************/
        /*                          */
        /*       optim.c            */
        /*  Optimal update module   */
        /*                          */
        /****************************/

#include <include/bstring.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/optim_sm/optim_private.h>


    /* declare the screen module */
STATIC(void,        optim_start,(void));
STATIC(void,        optim_create,(Pane *p));
STATIC(void,        optim_resize,(register Pane *p));
STATIC(void,        optim_destroy,(Pane *p));
STATIC(void,        optim_input,(Pane *p, Rectangle r));

aScreenModule scr_mod_optim = {
    "optimal update (optim)",
    optim_start,
    standardFinish,
    optim_create,
    optim_resize,
    standardNoSubWindowPropagate,
    optim_destroy,
    optim_input,
    standardTileNoWindow,
    standardDestroyWindow
};


static	short		text_sm;	/* the index of the text screen module	*/
Boolean optim_debugging = false;	/* set this flag to true for additional runtime checking */

/* Start the optimal update screen module.					*/
static void
optim_start()
{
	text_sm = sm_text_get_index();
}


/* Set the default status for this pane: non-inverted and no buttons returned.      */
static void
optim_create(Pane *p)
{
    p->pane_information = (Generic)get_mem(sizeof(struct optim), "optim_sm/optim.c: optim_create");
    bzero( (char *)INFO(p), sizeof(struct optim) );

    TP(p) = newSlavePane(p, text_sm, p->position, p->size, p->border_width);
    p->border_width = 0;
    sm_optim_set_move_status(p, false, false);
    CLEAR(p) = makeTextChar(' ', STYLE_NORMAL );
    OLD_SIZE(p) = Origin;
    RESIZING(p) = 0;
}


void sm_optim_resizing(Pane *p, Boolean am_resizing)
{
	if (am_resizing)
		RESIZING(p)++;
	else
		RESIZING(p)--;
}

TextChar *sm_optim_alloc_line(Pane *p)
{
	register TextChar *line;
	register short width = sm_optim_width(p);

	line = (TextChar *)get_mem(width * sizeof(TextChar),"sm_optim_alloc_line");
	optim_clear_line( line, width, CLEAR(p) );
	return line;
}


/*
 * Resize an optim pane.
 */
static void
optim_resize(register Pane *p)
{
    register short i;
    static char *who = "optim_resize";
    Point size;

    /* Redraw the saved map for the area of the text pane we know about */
    resizePane(TP(p), p->position, p->size);

    /* Perform violent size change only if the text pane has actually changed character size.*/
    if ( equalPoint( sm_text_size(TP(p)), OLD_SIZE(p) ) )
    {/* update pane but do not touch. */
        sm_optim_resizing(p,true);
    	sm_optim_refresh(p);
	sm_optim_resizing(p,false);
	return;
    }

    /* true size change... */

    optim_free(p);

    size = sm_optim_size( p );		/* size of the pane in chars */

    OLD_SIZE(p)		= size;
    HT_SIZE(p)		= size.y*2;     /* could do better than this... */
    HASH_TABLE(p)	= (bucket *) get_mem( sizeof(bucket)*HT_SIZE(p), who );
    LINE(p)		= (osm_line *) get_mem( sizeof(osm_line)*size.y, who );

    /* Initial all the linked lists hanging off the hash buckets. */
    for (i=0;i<HT_SIZE(p);i++)
    {
	(void) util_list_init( &(HASH_TABLE(p)[i].actual),  (Generic)INFO(p) );
	(void) util_list_init( &(HASH_TABLE(p)[i].desired), (Generic)INFO(p) );
    }

    bzero( (char *)LINE(p), sizeof(osm_line)*size.y );

    for (i=0;i<size.y;i++)
    {
        register osm_line  *line = &(LINE(p)[i]);

        line->old = sm_optim_alloc_line(p);
        line->New = sm_optim_alloc_line(p);

        line->lineno = i;
        line->last_mod = 0;
        line->first_mod = size.x - 1;
        line->info = OLD;

        (void) util_node_init( &(line->nlist), (Generic)line );
        (void) util_node_init( &(line->olist), (Generic)line );
        (void) util_node_init( &(line->Class), (Generic)line );
#ifdef notyet
        hash_enter_actual( p, line );
#endif
    }
}



void sm_optim_free_line(TextChar *line)
{
	free_mem( (void*)line );
}

/*
 * optim_free - free all size-dependent structure associated with the optim screen module
 *	The INFO(p), a struct optim attached to the pane_information field of the pane,
 *	is *not* freed.  That structure is allocated by optim_create, and freed by
 *	optim_destroy.
 */
void optim_free(Pane *p)
{
    osm_line *lines = LINE(p);
    short nlines = OLD_SIZE(p).y;
    short i;

    if (lines == NULL_LINE)
        return;

    for (i=0;i<nlines;i++)
    {
        free_mem( (void*)(lines[i].old) );
        free_mem( (void*)(lines[i].New) );
    }

    free_mem( (void*)HASH_TABLE(p) );
    free_mem( (void*)lines );
}

/* Destroy the pane and all structures below it.                    */
static
void
optim_destroy(Pane *p)
{
	destroyPane(TP(p));		/* Destroy the text slave pane */

	optim_free(p);			/* Free (size dependent) structures below INFO(p) */

	free_mem( (void*)INFO(p) );	/* And free INFO(p) */
}


/* Get the index of this screen module.  Install it if necessary.           */
short
sm_optim_get_index()
{
	return (getScreenModuleIndex(&scr_mod_optim));
}

Point
sm_optim_pane_size(Point size, short font_id)
{
	return sm_text_pane_size(size,font_id);
}

void
sm_optim_set_move_status(Pane *p, Boolean horiz, Boolean vert)
{
	sm_text_set_move_status(TP(p), horiz, vert);
}

void
sm_optim_change_font(Pane *p, short font_id)
{
	sm_text_change_font(TP(p),font_id);
	optim_resize(p);
}

/* Handle input to the window manager screen module.                    */
/*ARGSUSED*/
static void
optim_input(Pane *p, Rectangle r)
{
	handlePane(TP(p));
}


/************ debugging aids ******************/


#ifdef DEBUG
static struct optim *osm_pi(Pane *p)
{
	return INFO(p);
}

static void optim_dump(Pane *p)
{
    Point size;
    short i,j;

    size = sm_optim_size( p );

    (void)printf("%d rows by %d columns\n",size.y,size.x);

    for (j=0;j<size.x;j++)
        (void)printf(" %1d",j/10);
    (void)printf("\n");

    for (j=0;j<size.x;j++)
        (void)printf(" %1d",j%10);
    (void)printf("\n");

    for (i=0;i<size.y; i++)
    {
        osm_line *line = &(LINE(p)[i]);
        char type;

        for (j=0;j<size.x;j++)
        {
            TextChar *pos = &(line->old[j]);

            (void)printf("%1c", (pos->style == STYLE_NORMAL) ? ' ' : (char )(pos->style+'0') );
            (void)printf("%1c", pos->ch);
        }
        for (j=0;j<size.x;j++)
        {
            TextChar *pos = &(line->New[j]);

            (void)printf("%1c", (pos->style == STYLE_NORMAL) ? ' ' : (char )(pos->style+'0') );
            (void)printf("%1c", pos->ch);
        }
        switch(line->info)
        {
            case TRASHED:   type='T';   break;
            case NEEDED:    type='N';   break;
            case UNNEEDED:  type='U';   break;
            case OLD:       type='O';   break;
            case NEW:       type='F';   break;  /* for lines that have been fixed */
            default:        type='?';
        }
        (void)printf("  %c",type);
        (void)printf(" %d",line->ohash);
        (void)printf(" %d",line->nhash);
        (void)printf("\n");
    }
}
#endif /* DEBUG */
