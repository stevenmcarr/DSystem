/* $Id: text.i,v 1.2 1997/03/11 14:34:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 							*/
		/* 		          text.h			*/
		/*	     Local text screen module include.		*/
		/* 							*/
		/********************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/sms/text_sm.h>

struct	text_pane_info	{			/* TEXT PANE INFORMATION STRUCTURE	*/
	Point		pane_size;		/* current pixel size of pane		*/
	Point		map_origin;		/* pixel coord of upper left char	*/
	Point		map_size;		/* size of the pane in characters	*/
        short		font;			/* font id of the current font		*/
        Point		glyph_size;		/* size a character in the current font	*/
	Boolean		move_horiz;		/* true if horizontal move ability	*/
	Boolean		move_vert;		/* true if vertical move ability	*/
	short		batch_touch;		/* true if we are in batch touch mode	*/
	RectList	batch_rl;		/* the batch touch RectList		*/
	Generic		region_id;		/* the client id 			*/
	Point		region_dead_return;	/* what to return if region is dead	*/
	Point		(*region_mover)();	/* interactive region mover		*/
			};

#define	PANE_SIZE(p)		((struct text_pane_info *) p->pane_information)->pane_size
#define MAP_ORIGIN(p)		((struct text_pane_info *) p->pane_information)->map_origin
#define MAP_SIZE(p)		((struct text_pane_info *) p->pane_information)->map_size
#define FONT(p)			((struct text_pane_info *) p->pane_information)->font
#define GLYPH_SIZE(p)		((struct text_pane_info *) p->pane_information)->glyph_size
#define MOVE_HORIZ(p)		((struct text_pane_info *) p->pane_information)->move_horiz
#define MOVE_VERT(p)		((struct text_pane_info *) p->pane_information)->move_vert
#define BATCH_TOUCH(p)		((struct text_pane_info *) p->pane_information)->batch_touch
#define BATCH_RL(p)		((struct text_pane_info *) p->pane_information)->batch_rl
#define REGION_ID(p)		((struct text_pane_info *) p->pane_information)->region_id
#define	REGION_DEAD_RETURN(p)	((struct text_pane_info *) p->pane_information)->region_dead_return
#define REGION_MOVER(p)		((struct text_pane_info *) p->pane_information)->region_mover
