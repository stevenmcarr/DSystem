/* $Id: keymap.C,v 1.1 1997/06/25 14:49:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <include/bstring.h>

#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/lists/list.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/keymap.h>
#include <libs/support/strings/rn_string.h>

/* HISTORY
	 1985 ?		Ben Chase	Original version
	   Jun 86	Ben Chase	Push-back added for new monitor
	 6 Feb 87 	Ben Chase	queuing version created
	19 Feb 87       Don Baker       Pulled out "kb.c" file
	21 Apr 87	Ben Chase	Plugged mem leak in keymap_destroy()
	21 Apr 87	Ben Chase	Fixed bug in keymap_bind_range()
*/

typedef enum MapNodeTypeEnum{
	keymap_interior,
	keymap_leaf,
	keymap_leaves
} Mapnodetype;

typedef struct KeyNode {
	UtilNode ord;			/* member of list in ascii ("a..z") order */
	UtilNode mru;			/* member of list in most-recently-used order */
	Mapnodetype type;
	UtilList map_ord;		/* if nodetype == keymap_interior, list of chars in ascii order */
	UtilList map_mru;		/* if nodetype == keymap_interior, list of chars in mru order */
	Generic info;			/* information private to the owner of this keymap */
	KbChar firstchar;		/* if nodetype == keymap_leaves, then firstchar != lastchar */
	KbChar lastchar;
} Mapnode;

struct keymap {
	UtilNode owners;		/* List of keymap descriptors that share root_keymap	*/
	Mapnode *current_keymap;	/* current keymap to use when mapping a char		*/
	Mapnode *root_keymap;		/* beginning keymap to use when starting a key sequence */
	Generic unbound_info;		/* binding to use in the absence of explicit binding	*/
	KbChar *report_str;		/* string for reporting in keymap_report_all		*/
	KbChar *mapped_str;		/* string seen on input so far, an improper prefix of	*/
					/* some mapped sequence					*/
	unsigned short mindex;		/* next position in mapped_str to place a character	*/
	unsigned short mapdepth;	/* current depth of keymap				*/
	KbChar *queue;			/* queue of input waiting to be mapped			*/
	unsigned int queue_front;	/* index of first KbChar in queue			*/
	unsigned int queue_length;	/* number of KbChars in queue				*/
	Boolean done;			/* true iff mapping was just completed.  Implies info	*/
					/* is meaningful					*/
	Boolean push;			/* Should we automatically push back characters when a	*/
					/* sequence is not bound?				*/
	Boolean prune;			/* Should we do pruning when adding bindings which are	*/
					/* prefixes of existing bindings?			*/
	Boolean reset;			/* Reset keymap on next map request.  			*/
};

#define GLOBAL	/* An exported entry point */
#define COREDUMP()	abort()
#define ASSERT(bool)	{if ( !(bool) ) COREDUMP();}

STATIC(void, delete_key, (Mapnode *key)); 

/* return codes */
#define NULL_KEYNODE (Mapnode *)0

/*
 * keynode_of_node - given a node (a list member), return the Mapnode associated with it.
 */
static Mapnode *keynode_of_node(UtilNode *node)
{
	return (Mapnode *)UTIL_NODE_ATOM(node);
}

#define KEYNODE_OF_NODE(n)	((Mapnode *)UTIL_NODE_ATOM(n))

static Mapnode *keymap_head_ord(Mapnode *mn)
{
	UtilNode *node = UTIL_HEAD(&(mn->map_ord));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}

#ifdef unused 
static Mapnode *keymap_head_mru(Mapnode *mn)
{
	UtilNode *node = UTIL_HEAD(&(mn->map_mru));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}
#endif

static Mapnode *keymap_tail_ord(Mapnode *mn)
{
	UtilNode *node = UTIL_TAIL(&(mn->map_ord));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}

#ifdef unused 
static Mapnode *keymap_tail_mru(Mapnode *mn)
{
	UtilNode *node = UTIL_TAIL(&(mn->map_mru));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}
#endif

static Mapnode *keymap_next_ord(Mapnode *mn)
{
	UtilNode *node = UTIL_NEXT(&(mn->ord));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}

static Mapnode *keymap_prev_ord(Mapnode *mn)
{
	UtilNode *node = UTIL_PREV(&(mn->ord));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}

#ifdef unused 
static Mapnode *keymap_next_mru(Mapnode *mn)
{
	UtilNode *node = UTIL_NEXT(&(mn->mru));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}
#endif

#ifdef unused 
static Mapnode *keymap_prev_mru(Mapnode *mn)
{
	UtilNode *node = UTIL_PREV(&(mn->mru));
	return (node == NULLNODE) ?
		NULL_KEYNODE :
		KEYNODE_OF_NODE(node);
}
#endif


/*
 * keynode_of_list - given a list, return the Mapnode that owns it.
 */
static Mapnode *keynode_of_list(UtilList *list)
{
	return (Mapnode *)UTIL_LIST_ATOM(list);
}


/*
 * mru_list_of_keynode - given a Mapnode, return the mru list of which it is a member.
 *	If "key" is the root of the keymap, NULLLIST is returned, since the root Mapnode
 *	is a member of no list.
 */
static UtilList *mru_list_of_keynode(Mapnode *key)
{
	return util_list( &(key->mru) );
}


/*
 * keymap_deepen_map - increases the maximum length to "len" of bindings
 *	that may placed in "map".  This routine is called automatically
 *	when a long binding is encountered.
 */
static void keymap_deepen_map(Keymap *map, short depth)
{
	UtilList *owner_list;
	UtilNode *node;
	int nb;
	int olddepth;

	ASSERT(depth >=1);
	owner_list = util_list( &(map->owners) );
	node = util_head(owner_list);
	while (node != NULLNODE)
	{
		map = (Keymap *)util_node_atom(node);

		depth = max((int) depth,(int) map->mapdepth);
		depth = max((int) depth,(int) map->queue_length);
		nb = (depth+1)*sizeof(KbChar);
		olddepth = map->mapdepth;
		map->mapdepth = depth;
		map->report_str = (KbChar*)reget_mem((void*)map->report_str, nb, "deepen keymap" );
		map->mapped_str = (KbChar*)reget_mem((void*)map->mapped_str, nb, "deepen keymap" );
		map->queue      = (KbChar*)reget_mem((void*)map->queue,      nb, "deepen keymap" );

		/*
		 * Must diddle the (circular) queue, in case it is currently wrapped back to 0.
		 * Push the queued stuff that lies before the wrap out to the new end of the queue array.
		 */
		bcopy(map->queue+map->queue_front,
		      map->queue+map->queue_front+(depth-olddepth),
		      (olddepth-map->queue_front)*sizeof(KbChar));
		map->queue_front += (depth-olddepth);
		map->queue_front %= depth;
	
		node = util_next(node);
	}
}


/*
 * keymap_queue_flush - throw away the contents of the queue
 */
GLOBAL void keymap_queue_flush(Keymap *map)
{
	map->queue_front  = 0;
	map->queue_length = 0;
}


/*
 * keymap_queue_empty - returns true iff there are characters in the queue.
 */
GLOBAL Boolean keymap_queue_empty(Keymap *map)
{
	return BOOL((map->queue_length == 0));
}


/*
 * keymap_enqueue_KbChar - appends a kbchar to the queue of characters waiting to be mapped
 */
GLOBAL void keymap_enqueue_KbChar(Keymap *map, KbChar kbc)
{
	int place;

	if (map->queue_length+1 > map->mapdepth)
		keymap_deepen_map(map,(short) map->queue_length+1);
	place = (map->queue_front + map->queue_length++) % map->mapdepth;
	map->queue[place] = kbc;
}


/*
 * keymap_enqueue_KbString - appends a kbstring to the queue of characters waiting to be mapped
 */
GLOBAL void keymap_enqueue_KbString(Keymap *map, KbString kbs)
{
	short place;
	short i;

	for (i = 0; i < kbs.num_kc; i++)
	{
		if (map->queue_length+1 > map->mapdepth)
			keymap_deepen_map(map,(short) map->queue_length+1);
		place = (map->queue_front + map->queue_length++) % map->mapdepth;
		map->queue[place] = kbs.kc_ptr[i];
	}

	if (kbs.ephemeral)
	{/* free the storage associated with this KbString */
		freeKbString(kbs);
	}
}


/*
 * queue_butt_in - place a character at the front of the queue
 */
static void queue_butt_in(Keymap *map, KbChar kbc)
{
	int place;

	map->queue_length++;
	if (map->queue_length > map->mapdepth)
		keymap_deepen_map(map,(short) map->queue_length);
	place = (map->queue_front-1);
	place = (place >= 0) ? place : map->mapdepth-1;
	map->queue[place] = kbc;
	map->queue_front = place;
}


/*
 * queue_leave - service the head of the queue
 */
static KbChar queue_leave(Keymap *map)
{
	KbChar head;

	head = map->queue[map->queue_front++];
	map->queue_front %= map->mapdepth;
	map->queue_length--;

	return head;
}


/*
 * keymap_unmap_and_requeue - Because of a change to the mapping tree
 *	(eg. a binding is occurring), we must unmap all key sequences
 *	partially mapped, placing those key sequences at the front
 *	of the queue of their keymap descriptors.
 */
static void keymap_unmap_and_requeue(Keymap *map)
{
	UtilList *owner_list;
	UtilNode *node;
	Keymap *owner;
	
	owner_list = util_list( &(map->owners) );
	for (node = util_head(owner_list); node != NULLNODE; node = util_next(node) )
	{
		owner = (Keymap *)util_node_atom(node);

		/* Does owner currently have nothing mapped? */
		if (owner->root_keymap == owner->current_keymap)
			continue;

		/* Can owner map no further along the current path? (ie. at leaf) */
		if (owner->done)
			continue;

		/*
		Found a keymap descriptor for a partially mapped sequence.
		Transfer everything in mapped_str to the queue.
		*/
		while (map->mindex != 0)
			queue_butt_in(map,map->mapped_str[--map->mindex]);
		
		owner->current_keymap = owner->root_keymap;
	}
}


/*
 * make_keynode - Create a Mapnode of type keymap_leaf or keymap_leaves.
 */
static Mapnode *make_keynode(KbChar first, KbChar last, Generic info, char *who)
{
	Mapnode *key;

	if ( first > last )
	{/* "first" comes after "last" */
		abort();
		return NULL_KEYNODE;
	}

	key = (Mapnode *)get_mem( sizeof(Mapnode), "make_keynode for %s", who);

	(void) util_node_init( &(key->ord), (Generic) key);
	(void) util_node_init( &(key->mru), (Generic) key);
	(void) util_list_init( &(key->map_ord), (Generic) key);
	(void) util_list_init( &(key->map_mru), (Generic) key);
	key -> firstchar = first;
	key ->  lastchar = last;
	key ->      type = (first == last) ? keymap_leaf : keymap_leaves;
	key ->      info = info;

	return key;
}


/*
 * single_to_keymap - given either a keymap_leaf or keymap_leaves node, trustingly
 *	convert it to a keymap_interior node by creating the next level of bindings,
 *	all initially bound to 'info'.  The information bound to the node
 *	"key" is left undisturbed.
 */
static void single_to_keymap(Mapnode *key, Generic info)
/* info: default binding for whole keymap */
{
	/*
	For all the key sequences which pass through this new map, give them
	the binding 'info' (perhaps a value signifying an unbound key
	sequence).  We do this so that no key sequence is truly unbound.
	*/
	Mapnode *full_range = make_keynode( KB_first, KB_last, info, "single_to_keymap" );

	ASSERT(key->type != keymap_interior);

	util_push( &(full_range->ord), &(key->map_ord) );
	util_push( &(full_range->mru), &(key->map_mru) );
	key->type = keymap_interior;
}


/*
 * keymap_to_single - given a Mapnode which is a keymap_interior node, convert it
 *	to a keymap_leaf node by deleting all the Mapnodes in its map, and
 * 	convert the given Mapnode into a keymap_leaf node.  If any of the
 * 	Mapnodes in the map are themselves map nodes, recursively convert
 * 	them, and then delete them.
 */
static void keymap_to_single(Mapnode *keymap, Generic info)
/* info: The info to bind at keymap, after the map is deleted */
{
	UtilList *map = &(keymap->map_ord);
	UtilNode *node;
	Mapnode *key;

	/* For every node in "map". */
	for ( node = util_head(map); node != NULLNODE ; node = util_head(map) )
	{
		key = keynode_of_node(node);

		/* If key is a map node, recursively delete its map. */
		if (key->type == keymap_interior)
			keymap_to_single(key, info);
		delete_key(key);
	}

	keymap->type = (keymap->firstchar == keymap->lastchar) ? keymap_leaf : keymap_leaves;
	keymap->info = info;

	/*
	Could check_coalesce here, but it would be silly when converting large/deep
	keymaps to single nodes.  Better for the caller to do it, if he desires.
	*/	
}


/*
 * delete_key - trustingly delete the Mapnode, someone else is taking care of details,
 *	like when key is a range node.  Only special case is when key is a map node.
 */
static void delete_key(Mapnode *key)
{
	/* If it's a keymap_interior node, just convert it to something simpler! */
	if (key->type == keymap_interior)
		keymap_to_single(key, UNUSED );

	/* Pluck it out of the lists of the enclosing keymap */
	util_pluck( &(key->ord) );
	util_pluck( &(key->mru) );

	free_mem((void*)key );
}


/*
 * split_range - Split a range node [first..last] into possibly as many three new nodes:
 *	[first..nfirst-1], [nfirst,nlast], [nlast+1..last]
 *	The 1st and 3rd nodes have the binding of the original, while the 2nd has
 *	a new binding, and may even be a KEYMAP node.  Note that either the 1st or 3rd
 *	nodes will not be created by this function if the ranges they represent are empty.
 *	The 1st or 3rd will be keymap_leaf nodes rather than keymap_leaves nodes if their
 *	ranges contain only one element.
 */
static Mapnode *split_range(Keymap *map, Mapnode *key, Mapnodetype type, 
                            Generic info, KbChar nfirst, KbChar nlast)
/* key: The node to split */
/* type: Type of new node being created */
/* info: Info of new node being created */
/* nfirst: First character of new node being created */
/* nlaast: Last  character of new node being created */
{
	UtilNode *node_ord = &(key->ord);
	UtilNode *node_mru = &(key->mru);
	KbChar first;				/* First character of node being split */
	KbChar last;				/* Last  character of node being split */
	Generic o_info = key->info;		/* Info of node being split */
	Mapnode *before;
	Mapnode *after;

	first = key->firstchar;
	last  = key->lastchar;

	ASSERT(key->type == keymap_leaves);	/* Can only split a range node... */

	/* first <= nfirst <= nlast <= last */
	ASSERT( first  <= nfirst );
	ASSERT( nfirst <= nlast  );
	ASSERT( nlast  <= last   );

	if ( o_info == info && type != keymap_interior )
	/* All our work is done */
		return key;

	if ( first < nfirst )
	{/* Make new keymap_leaves node from [first..nfirst-1] with old bindings. */
		before = make_keynode( first, nfirst-1, o_info, "split_range1" );
		util_insert_before( &(before->ord), node_ord );
		util_insert_before( &(before->mru), node_mru );
	}

	if ( nlast < last )
	{/* Make new keymap_leaves node from [nlast+1..last] with old bindings. */
		after = make_keynode( nlast+1, last, o_info, "split_range2" );
		util_insert_after( node_ord, &(after->ord) );
		util_insert_after( node_mru, &(after->mru) );
	}

	/* Set up new node that we split from the middle of the old range node. */
	key->firstchar = nfirst;
	key->lastchar = nlast;
	key->info = info;

	if (type == keymap_interior)
		single_to_keymap(key,map->unbound_info);
	else
		type = (nfirst == nlast) ?
			keymap_leaf :
			keymap_leaves ;

	key->type = type;

	return key;
}


/*
 * check_coalesce - Checks to see if a node should be merged with its nearest ordinal neighbors.
 *	Two nodes may be coalesced if they are neither keymap_interior nodes, and their 'info' fields
 *	are the same.  This procedure should be called whenever the need for coalescing may exist.
 *	"key_two" is assumed to not be of type keymap_interior.
 */
static void check_coalesce(Mapnode *key_2)
{
	Mapnode *key_1;
	Mapnode *key_3;

	ASSERT(key_2->type != keymap_interior);

	key_1 = keymap_prev_ord(key_2);
	while(key_1 != NULL_KEYNODE)
	{/* Coalesce backwards, with predecessor of key_2? */
		if (key_1->type == keymap_interior || key_1->info != key_2->info)
			break; /* No more coalescing backwards */

		key_2->firstchar = key_1->firstchar;
		key_2->type = keymap_leaves;
		delete_key( key_1 );
		key_1 = keymap_prev_ord(key_2);
	}

	key_3 = keymap_next_ord(key_2);
	while(key_3 != NULL_KEYNODE)
	{/* Coalesce forwards, with successor of key_2? */
		if (key_3->type == keymap_interior || key_3->info != key_2->info)
			break; /* No more coalescing forwards */

		key_2->lastchar = key_3->lastchar;
		key_2->type = keymap_leaves;
		delete_key( key_3 );
		key_3 = keymap_next_ord(key_2);
	}
}


/*
 * bind_char - Makes sure that the node for "ch" within "keymap" is bound to
 *	"info".  Basically, this function is used for binding the final
 *	character in a binding sequence.  It will split a range node if
 * 	necessary.  If the value of "replace" is true, then the subtree
 *	rooted at "keymap" (if one exists) will be deleted and replaced with
 *	the node corresponding to this new binding.
 */
static void bind_char(Keymap *map, UtilList *keymap, KbChar ch, 
                 Generic info, Boolean replace)
/* keymap: The keymap in which to bind */
/* ch: The KbChar to which to bind */
/* info: The information to bind */
/* replace: Controls subtree pruning */
{
	UtilNode *curnode;
	Mapnode *curkey;

	/*
	 * If we are processing a list of bindings in alphabetic order
	 * then looking from the end of the list could be a good idea.
	 * So, we search backwards through the list.
	 */
	curnode = util_tail(keymap);
	curkey = keynode_of_node(curnode);
	while( ch < curkey->firstchar )
		curkey = keymap_prev_ord(curkey);

	switch (curkey->type)
	{
		case keymap_leaves:
			(void) split_range(map,curkey,keymap_leaf,info,ch,ch);
			break;

		case keymap_leaf:
			if (curkey->info == info)
				return;

			curkey->info = info;

			check_coalesce( curkey );
			break;

		case keymap_interior:
			if (replace)
			{/* Replace the keymap node with a keymap_leaf node */
				keymap_to_single(curkey,info);
				check_coalesce( curkey );
			}
			else
			{/* Give the keymap node a binding.  Currently, there's not much use for this though.	*/
				curkey->info = info;
			}
			break;
	}
}


/*
 * find_or_enter_keymap - Push into the keymap tree, creating nodes as needed.
 *	Given a keymap (interior) node in the keymap tree, select (and possibly
 *	create) the descendent of it which corresponds to ks.  Return the last
 *	(ie deepest) node found or entered.
 */
static Mapnode *find_or_enter_keymap(Keymap *map, UtilList *keylist, KbString ks)
{
	KbChar ch;
	Mapnode *curkey;
	UtilNode *curnode;
	int i;

	for (i=0;i<ks.num_kc;i++)
	{
		ch = ks.kc_ptr[i];
		curnode = util_head(keylist);
		curkey = keynode_of_node(curnode);
		while ( ch < curkey->firstchar || curkey->lastchar < ch )
		{/* ch is not in [curkey->firstchar..curkey->lastchar] */
			curnode = util_next(curnode);
			curkey = keynode_of_node(curnode);
		}

		/* Maintain mru order.  If node isn't the head of the list, put it there. */
		if (curnode != util_head(keylist))
		{
			util_pluck(curnode);
			util_push(curnode,keylist);
		}
	
		switch (curkey->type)
		{
			case keymap_leaves:
				curkey = split_range(map,curkey,keymap_interior,curkey->info,ch,ch);
				break;
	
			case keymap_leaf:
				/* We need to replace a node for a single character with a keymap node. */
				single_to_keymap(curkey,map->unbound_info);
				break;
	
			case keymap_interior:
				/* Too easy.  Our work is already done. */
				break;
	
			default:
				abort();
				return (Mapnode *)0;
		}

		keylist = &(curkey->map_mru);
	}
	/*
	 * Return the last node investigated, which corresponds to the
	 * mapping of the string.
	 */
	return keynode_of_list(keylist);
}


/*
 * keymap_bind_KbChar - Binds a single KbChar to some information, instead of
 *	a KbChar string.
 */
GLOBAL void keymap_bind_KbChar(Keymap *map, Generic info, KbChar ch)
{
	keymap_unmap_and_requeue(map);
	bind_char( map, &(map->root_keymap->map_ord), ch, info, map->prune );
}


/*
 * keymap_bind_KbString - Similar to emacs' bind-to-key routine in function.
 *	Whether or not info is a function is left to the user of these
 *	routines.
 */
GLOBAL void keymap_bind_KbString(Keymap *map, Generic info, KbString ks)
{
	KbChar *str = ks.kc_ptr;
	short len = ks.num_kc;
	UtilList *keylist;		/* The keymap where the binding is placed */
	Mapnode *curkey;

	len = ks.num_kc;
	if ( str == 0 || len == 0 )
	{
		if (ks.ephemeral)
		{/* free the storage associated with this KbString */
			freeKbString(ks);
		}
		return;
	}

	if ( len >= (short)map->mapdepth )
		keymap_deepen_map(map,len);

	keymap_unmap_and_requeue(map);

	/*
	 * Map all but the last character of the string.
	 */
	ks.num_kc--;
	curkey = find_or_enter_keymap( map, &(map->root_keymap->map_mru), ks );
	ks.num_kc++;

	keylist = &(curkey->map_ord);

	/* Complete the binding.  Bind the leaf node corresponding to str[len] to info */
	bind_char( map, keylist, str[len-1], info, map->prune );

	if (ks.ephemeral)
	{/* free the storage associated with this KbString */
		freeKbString(ks);
	}
}


/*
 * keymap_bind_range - At the interior node corresponding to "prefix", create a
 *	range node from "start" to "end", with binding "info", trampling
 *	whatever might have been bound to that range before.  For example, this
 *	function might be used to bind all the digits at the root level to the
 *	function handle_digit like this:
 *		keymap_bind_range(map,
 *			(Generic)handle_digit,
 *			makeKbString("","blah"),
 *			makeKbChar('0','\0' ),
 *			makeKbChar('9','\0' ) );
 *	If all the printable characters had previously been bound to something
 *	else (perhaps handle_alpha), then the previous example would cause
 *	the range node for handle_alpha to be split into two new range nodes,
 *	one for all the printable characters less than '0', and one for all
 *	printable characters greater than '9'.
 */
GLOBAL void keymap_bind_range(Keymap *map, Generic info, KbString prefix, 
                              KbChar start, KbChar end)
/* map: The map in which to bind a range		*/
/* info: The info to bind to the specified range	*/
/* prefix: Common prefix of start and end		*/
/* start: The character that starts the range		*/
/* end: The character that ends the range		*/
{
        Mapnode *startkey;      /* The mapnode that contains start
*/
        Mapnode *curkey;        /* The mapnode that contains end
*/
        Mapnode *endkey;        /* A mapnode between startkey and endkey
*/

	if ( prefix.num_kc >= (short)map->mapdepth )
		keymap_deepen_map( map, prefix.num_kc+1 );

	keymap_unmap_and_requeue(map);

	curkey = find_or_enter_keymap( map, &(map->root_keymap->map_mru), prefix );
	if (prefix.ephemeral)
	{/* free the storage of this KbString */
		freeKbString(prefix);
	}

	/*
	 * Find the node that starts the new range.
	 */
	startkey = keymap_head_ord(curkey);
	while ( start > startkey->lastchar )
		startkey = keymap_next_ord(startkey);

	/*
	 * Rebind the node that starts the new range.
	 */
	switch(startkey->type)
	{
	    case keymap_leaf:
		startkey->info = info;
		break;
	    case keymap_interior:
		keymap_to_single(startkey,info);
		break;
	    case keymap_leaves:
		startkey = split_range( map,
			startkey,
			keymap_leaves,
			info,
			start,
			(KbChar)min((int) end, (int) startkey->lastchar) );

		break;
	}

	/*
	 * Find the node that ends the new range.
	 */
	endkey = keymap_tail_ord(curkey);
	while ( end < endkey->firstchar )
		endkey = keymap_prev_ord(endkey);

	/*
	 * Rebind the node that ends the new range.
	 */
	switch(endkey->type)
	{
	    case keymap_leaf:
		endkey->info = info;
		break;
	    case keymap_interior:
		keymap_to_single(endkey,info);
		break;
	    case keymap_leaves:
		endkey = split_range( map,
			endkey,
			keymap_leaves,
			info,
			(KbChar)max((int) start,(int) endkey->firstchar),
			end );
		break;
	}
	if (startkey == endkey)
		return;
	for (curkey = keymap_next_ord(startkey); curkey!= endkey; curkey=keymap_next_ord(curkey))
		keymap_to_single(curkey, info);
	check_coalesce(startkey);
}


/*
 * map_kbchar - Interprets a sequence of calls as a possible key binding, and when one is
 *	completed, makes the binding of that sequence available via key_get_info.  This
 *	routine's primary use is for actual keyboard bindings, since the entire string
 *	is not available at once when reading from the keyboard.
 *
 *	If 'ch' completes a bound key sequence, it returns 0.  It also sets
 *	'binding' so that 'key_get_info(binding)' is meaningful, until the next
 *	call to a mapping, a binding, or keymap_reset.
 *
 *	If 'ch' does not complete a binding, it changes the concept of the
 *	current keymap stored in 'binding' (ie. if 'ch' was imbedded in a binding,
 *	and we have just seen the previous characters in that binding), and returns -1.
 *
 *	Used exclusively by keymap_mapping_complete().
 */
static Boolean map_kbchar(Keymap *map, KbChar ch)
{
	UtilList *keymap;
	UtilNode *node;
	Mapnode *key;

	/* Get the map in which we want to look up 'ch'. */
	keymap = &(map->current_keymap->map_mru);
	node = util_head(keymap);
	key  = keynode_of_node(node);

	/* "while ch not in [key->firstchar..key->lastchar]" (pardon my high-level lingo) */
	while( ch < key->firstchar || key->lastchar < ch )
	{
		node = util_next(node);
		key  = keynode_of_node(node);
	}

	/*
 	At this point, 'key' is the Mapnode we want, and 'node' = &(key->ord).
	*/

	/* Maintain mru order.  If node isn't the head of the list, put it there. */
	if (node != util_head(keymap))
	{
		util_pluck(node);
		util_push(node,keymap);
	}

	/* Record for posterity the character that has caused this step in the mapping. */
	map->mapped_str[map->mindex++] = ch;

	map->current_keymap = key;
	if (key->type == keymap_interior)
	{/* step one level deeper into the mapping tree */
		return false;
	}

	/* We've reached a leaf node of the keymap tree, the end of a binding. */

	if (map->push && key->info == map->unbound_info)
	{/* The binding we have reached is unbound, and the caller wants us to fix this via push-back. */
		/*
		Step upward through the mapping tree until we find a meaningful binding hidden
		at an internal (keymap_interior) node, pushing the unused suffix of the binding
		sequence onto the front of the internal queue.
		Each iteration sets "keymap" to the list of Mapnodes that contains "key".  If we reach
		a key that has no enclosing list, we have reached the root of the keymap.
		*/
		while(	(keymap = mru_list_of_keynode(key)) != NULLLIST )
		{
			/* Set "key" to the Mapnode that owns "keymap".  Lists should always have an owner Mapnode. */
			if ((key = keynode_of_list(keymap)) == NULL_KEYNODE)
				abort();

			queue_butt_in(map,map->mapped_str[--map->mindex]);

			/* We've found a prefix of the whole sequence which is bound to something meaningful. */
			if (key->info != map->unbound_info)
				break;
		}

		/*
		Select the binding corresponding to the prefix left in mapped_str.
		Later, the requeued KbChars will be remapped when  keymap_mapping_complete() is called.

		Note that key->info can still equal map->unbound_info, if no meaningful bindings were discovered on the
		way back to the root of the mapping tree.  In that case, the first character of the sequence is mapped,
		and its binding, the unbound binding, is the one selected.  In this way, we make some progress, rather
		than requeueing back the entire string.
		*/
		map->current_keymap = key;
		if (map->current_keymap == map->root_keymap)
		{
			/*
			We were unable to map any of the sequence seen, and will
			be unable to map it the next time we see it.  So, we throw
			it away, and tell the caller that we're not done mapping.
			*/
			keymap_queue_flush(map);
			return false;
		}
	}

	map->mapped_str[map->mindex] = 0;
	map->done = true;
	return true;
}


/*
 * keymap_mapping_complete - returns true if a mapping has been completed.
 */
GLOBAL Boolean keymap_mapping_complete(Keymap *map)
{
	/*
	 * If we completed a keymap the last time we were called,
	 * and inquiries have been made about that mapping,
	 * we need to reset the map. */
	if (map->reset)
		keymap_reset(map);
	else
	{
		if (map->done)
		{/* Keymap completed, but nobody asked about it.  Give it to them again. */
			return true;
		}
	}

	while (! keymap_queue_empty(map) )
	{
		if( map_kbchar( map, queue_leave(map)))
			return true;	/* completed a mapping */
	}

	return false;	/* queue empty before mapping completed */
}


/*
 * Find the first valid completion of the mapped sequence, starting at node
 */
static UtilNode *find_completion(Keymap *map, UtilNode *node)
{
	Mapnode *key;

	if (node == NULLNODE)
		return NULLNODE;

	for(; node != NULLNODE; node = util_next(node))
	{
		key = keynode_of_node(node);
		if (key->info != map->unbound_info ||
		    key->type == keymap_interior)
			return node;
	}
	return NULLNODE;
}


/*
 * Try to complete the partial mapping.
 */
GLOBAL Boolean keymap_mapping_extend_auto(Keymap *map)
{
	UtilList *keymap;
	UtilNode *node;
	Mapnode *key;

	if (map->reset)
		keymap_reset(map);

	if (!keymap_queue_empty(map))
		if (keymap_mapping_complete(map))
			return true;

	/* Get the map in which we want to look up 'ch'. */
	keymap = &(map->current_keymap->map_mru);
	node = find_completion(map,util_head(keymap));
	if (node == NULLNODE)
		return false;	/* No completions */

	key = keynode_of_node(node);
	if(key->type == keymap_leaves)
		return false;	/* Not unique */

	if (find_completion(map,util_next(node)) != NULLNODE)
		return false;	/* Not unique */

	/*
	 * Found something...
	 */

	if (node != util_head(keymap))
	{/* Maintain mru order.  If node isn't the head of the list, put it there. */
		util_pluck(node);
		util_push(node,keymap);
	}

	/* Record for posterity the character that has caused this step in the mapping. */
	map->mapped_str[map->mindex++] = key->firstchar;
	map->current_keymap = key;

	if (key->type == keymap_leaf)
	{/* Found a unique completion! */
		map->mapped_str[map->mindex] = 0;
		map->done = true;
		return true;
	}

	/*
	 * key->type == keymap_interior, so we try to extend some more,
	 * and then report success, regardless of the success of the
	 * attempt at additional extension.
	 */
	(void) keymap_mapping_extend_auto(map);

	return true;
}


/*
 * Walk back up a level in the keymap.  Useful for interactively deleting something that
 * has already been mapped.  Returns true iff the retract is possible.
 */
GLOBAL Boolean keymap_retract(Keymap *map)
{
	if (map->reset)
		map->reset = false;
	if (map->done)
		map->done = false;

	if (map->mindex == 0)
		return false;

	map->mindex--;
	map->current_keymap = keynode_of_list(mru_list_of_keynode(map->current_keymap));
	return true;
}


GLOBAL void keymap_set_push(Keymap *map, Boolean push)
{
	map->push = push;
	keymap_queue_flush(map);
}


GLOBAL void keymap_set_prune(Keymap *map, Boolean prune)
{
	map->prune = prune;
}


GLOBAL Keymap_report keymap_report_all(Keymap *map)
{
	Keymap_report rep;
	int i;

	if (map->done)
		map->reset = true;

	rep.done = map->done;
	rep.info = map->current_keymap->info;
	rep.seq  = getKbString((short) map->mindex,"keymap_report_all");
	rep.seq.ephemeral = true;
	for (i=0;i<rep.seq.num_kc;i++)
		rep.seq.kc_ptr[i] = map->mapped_str[i];

	return rep;
}

/*
 * Look up the binding for "kbs" in "map", and return a Keymap_report for it.
 */
GLOBAL Keymap_report keymap_get_binding(Keymap *map, KbString kbs)
{
	Keymap_report km_rep;
	Keymap *md = keymap_share(map);

	keymap_enqueue_KbString(md,kbs);
	(void) keymap_mapping_complete(md);
	km_rep = keymap_report_all(md);
	keymap_destroy(md);
	return km_rep;
}

GLOBAL short keymap_seq_len(Keymap *map)
{
	if (map->done)
		map->reset = true;
	return map->mindex;
}


GLOBAL KbString keymap_seq_KbString(Keymap *map)
{
	KbString ks;
	int i;

	if (map->done)
		map->reset = true;
	ks = getKbString((short) map->mindex, "keymap_seq_KbString()");
	ks.ephemeral = true;
	for (i=0;i<(int)map->mindex;i++)
		ks.kc_ptr[i] = map->mapped_str[i];
	return ks;
}

/*
 * keymap_info - returns the info that was bound to the key sequence
 *	just completed by successive calls to keymap_mapchar.
 *	If a key sequence was not just completed, (eg. we are in the middle
 *	of a sequence, or just performed a binding), then the information
 *	associated with an unbound sequence is returned.
 */
GLOBAL Generic keymap_info(Keymap *map)
{
	if (map->done)
		map->reset = true;
	return map->current_keymap->info;
}


/*
 * Allocate and initialize the keymap descriptor.
 * The descriptor returned by this call should be basically usuable
 * for things like keymap_queue_* immediately after the call.
 * Note that the caller may want to set the prune and push flags,
 * add bindings, and who knows what else.
 */
static Keymap *keymap_desc_alloc(UtilList *owners_list, Mapnode *root, 
                                 int depth, Generic unbound_info)
{
	Keymap *new_map;
	static char *who = "keymap_desc_alloc";
	int nb = (depth+1)*sizeof(KbChar *);

	/* Get the memory for this descriptor */
	new_map = (Keymap *)get_mem(sizeof(Keymap),who);
	new_map->mapped_str = (KbChar *)get_mem(nb,who);
	new_map->report_str = (KbChar *)get_mem(nb,who);
	new_map->queue      = (KbChar *)get_mem(nb,who);

	/* Initialize indices to sensible starting values */
	new_map->mindex = 0;
	new_map->queue_front = 0;
	new_map->queue_length = 0;
	new_map->mapdepth = depth;
	new_map->done = false;
	new_map->reset = false;

	/* Set various defaults */
	new_map->push = false;
	new_map->prune = false;
	new_map->unbound_info = unbound_info;

	/* Attach the mapping tree to this descriptor */
	new_map->root_keymap = root;
	new_map->current_keymap = root;

	/*
	Place the descriptor that we are creating on the list of owners
	of the keymap tree.  (ie. increment the reference count for the
	mapping tree by 1.)
	*/
	(void) util_node_init( &(new_map->owners), (Generic)new_map );
	util_push( &(new_map->owners), owners_list );

	return new_map;
}


#define INITIAL_MAPDEPTH 10		/* Just an initial allocation, not a hard limit... */

/*
 * keymap_create - get a keymap descriptor.
 */
GLOBAL Keymap *keymap_create(Generic unbound_info)
{
	Mapnode *root;
	UtilList *owners_list;

	/*
	Create the root level list of character bindings.  Initially, this
	list contains one node, binding every character to unbound_info
	*/
	root = make_keynode( KB_first, KB_last, unbound_info, "keymap_create" );
	single_to_keymap( root, unbound_info );

	/*
	 * Make a list of keymap descriptors for sharing the keymap we
	 * are creating.
	 */
	owners_list = util_list_alloc(0,"keymap_create");

	return keymap_desc_alloc( owners_list,
				  root,
				  INITIAL_MAPDEPTH,
				  unbound_info );
}


/*
 * keymap_share - Create a new keymap descriptor, but not a new mapping tree.
 *	Instead, share the mapping tree of an existing keymap descriptor.
 *	Keymaps which share a mapping tree behave just like privately owned
 *	ones, except that placing new bindings in a shared mapping tree does
 *	of course cause that new binding to be effective for every keymap
 *	descriptor that shares the changed tree.
 *	When no longer needed, a descriptor obtained via this call should be
 *	destroyed via keymap_destroy(), just like those obtained via
 *	keymap_create().
 */
GLOBAL Keymap *keymap_share(Keymap *existing_map)
{
	Keymap *new_map;

	new_map = keymap_desc_alloc( util_list( &(existing_map->owners) ),
			existing_map->root_keymap,
			(int) existing_map->mapdepth,
			existing_map->unbound_info);

	new_map->push  = existing_map->push;
	new_map->prune = existing_map->prune;

	return new_map;
}


/*
 * keymap_create_fancy - get a fancier keymap descriptor than provided
 *	by keymap_create.
 */
GLOBAL Keymap *keymap_create_fancy(Generic unbound_info, Generic default_info)
{
	Keymap *root = keymap_create(unbound_info);

	/*
	Here we bind all printable characters to default_info.  All the
	control characters, including DEL, are left bound to unbound_info.
	We suppose that this is the way many people will want it.
	*/
	keymap_bind_range(root, default_info, makeKbString("", "keymap_create_fancy()"),
		KB_first_print,
		KB_last_print );

	return root;
}


/*
 * keymap_reset flushes all information about previously read characters
 */
GLOBAL void keymap_reset(Keymap *root)
{
	int i;

	root->current_keymap = root->root_keymap;

	for( i = 0; i < (int)root->mindex; root->mapped_str[i++] = 0 );

	root->mindex = 0;
	root->done = false;
	root->reset = false;
}


/*
 * Call this when you are done with a keymap.
 */
GLOBAL void keymap_destroy(Keymap *root)
{
	UtilList *owners_list;

	if (root == NULL_KEYMAP)
		return;

	owners_list = util_list( &(root->owners) );
	util_pluck( &(root->owners) );
	if ( util_list_empty(owners_list) )
	{/* Reference count on the actual keymap tree has gone to 0 */
		keymap_to_single(root->root_keymap, UNUSED);
		free_mem((void*)(root->root_keymap) );
		util_list_free( owners_list );
	}

	/* Free all memory associated with the descriptor */
	free_mem((void*)(root->mapped_str) );
	free_mem((void*)(root->report_str) );
	free_mem((void*)(root->queue) );
	free_mem((void*)root);
}


/*ARGSUSED*/
GLOBAL int keymap_walk_noop(KeymapWalk kw)
{
	return 0;
}

GLOBAL KeymapWalk keymap_walk_create(Keymap *map)
{
	KeymapWalk kw;

	kw = (KeymapWalk)get_mem(sizeof(struct KeymapWalkStruct), "keymap_walk_create");
	kw->low			= getKbString((short) map->mapdepth,"keymap_walk_create");
	kw->high		= getKbString((short) map->mapdepth,"keymap_walk_create");
	kw->handle		= UNUSED;
	kw->walk_control	= TW_PREORDER;
	kw->interior_handler	= keymap_walk_noop;
	kw->leaf_handler	= keymap_walk_noop;
	kw->leaves_handler	= keymap_walk_noop;
	kw->priv		= keymap_share(map);

	return kw;
}

static int keymap_walk_recurse(KeymapWalk kw)
{
	Keymap 	*md;
	Mapnode *mapnode;
	Mapnode *savemap;
	int ret = 0;

	md = kw->priv;
	savemap = md->current_keymap;
	kw->binding = savemap->info;
	kw->low.kc_ptr[kw->depth]  = savemap->firstchar;
	kw->high.kc_ptr[kw->depth] = savemap->lastchar;

	switch (savemap->type)
	{
	    case keymap_interior:
		if (kw->walk_control & TW_BEFORE)
			ret = (kw->interior_handler)(kw);
		
		if (kw->walk_control & TW_DESCEND)
		{
			kw->depth++;
			kw->low.num_kc++;
			kw->high.num_kc++;

			mapnode = keymap_head_ord(savemap);
			while (	(mapnode != NULL_KEYNODE) &&
				(ret == 0) &&
				(!(kw->walk_control & TW_QUIT_SIBS)) )
			{
				md->current_keymap = mapnode;
				ret = keymap_walk_recurse(kw);
				mapnode = keymap_next_ord(mapnode);
			}

			kw->high.num_kc--;
			kw->low.num_kc--;
			kw->depth--;
		}

		if ( (kw->walk_control & TW_AFTER) && ret == 0)
			ret = (kw->interior_handler)(kw);
		break;
	    case keymap_leaf:
		if (kw->walk_control & TW_LEAF)
			ret = (kw->leaf_handler)(kw);
		break;
	    case keymap_leaves:
		if (kw->walk_control & TW_LEAF)
			ret = (kw->leaves_handler)(kw);
		break;
	}
	return ret;
}

GLOBAL int keymap_walk_begin(KeymapWalk kw)
{
	Keymap 	*md;
	Mapnode *mapnode;
	Mapnode *savemap;
	int ret = 0;

	md = kw->priv;
	keymap_reset(md);
	savemap = md->current_keymap;

	kw->depth = 0;
	kw->low.num_kc = 1;
	kw->high.num_kc = 1;

	if (kw->walk_control & TW_DESCEND)
	{
		mapnode = keymap_head_ord(savemap);
		while ( (mapnode != NULL_KEYNODE) &&
			(ret == 0) &&
			(!(kw->walk_control & TW_QUIT_SIBS)) )
		{
			md->current_keymap = mapnode;
			ret = keymap_walk_recurse(kw);
			mapnode = keymap_next_ord(mapnode);
		}
	}
	return ret;
}

GLOBAL void keymap_walk_destroy(KeymapWalk kw)
{
	keymap_destroy(kw->priv);
	freeKbString(kw->low);
	freeKbString(kw->high);
	free_mem((void*)kw);
}

/************* Routines for Debugging ****************/

typedef FUNCTION_POINTER (char*, BindingName_functPtr, (Generic));

typedef struct Keymap_dump_struct {
	BindingName_functPtr getbindingname;
	Boolean do_unbound;
} Keymap_dump_params;

static int keymap_describe_interior(KeymapWalk wd)
{
	char *lname;
	char *bname;
	Keymap_dump_params *handle;

	handle = (Keymap_dump_params*)(wd->handle);
	if (NOT(handle->do_unbound))
		if (wd->binding == wd->priv->unbound_info)
			return 0;

	lname = symbolicFromKbString(wd->low);
	bname = (handle->getbindingname)(wd->binding);

	(void) printf("%-15s%s\t%s\n",
		"Interior:",
		lname,
		bname);
	sfree(lname);
	return 0;
}

static int keymap_describe_leaf(KeymapWalk wd)
{
	char *lname;
	char *bname;
	Keymap_dump_params *handle;

	handle = (Keymap_dump_params*)(wd->handle);
	if (NOT(handle->do_unbound))
		if (wd->binding == wd->priv->unbound_info)
			return 0;

	lname = symbolicFromKbString(wd->low);

	bname = (handle->getbindingname)(wd->binding);

	(void) printf("%-15s%s\t%s\n",
		"Single:",
		lname,
		bname);
	sfree(lname);
	return 0;
}


static int keymap_describe_leaves(KeymapWalk wd)
{
	char *bname;
	char *lname;
	char *hname;
	Keymap_dump_params *handle;

	handle = (Keymap_dump_params*)(wd->handle);
	if (NOT(handle->do_unbound))
		if (wd->binding == wd->priv->unbound_info)
			return 0;

	lname = symbolicFromKbString(wd->low);
	hname = symbolicFromKbString(wd->high);

	bname = (handle->getbindingname)(wd->binding);

	(void) printf("%-15s%s..%s\tR\t%s\n",
		"Range:",
		lname,
		hname,
		bname);
	sfree(lname);
	sfree(hname);
	return 0;
}

void keymap_dump_bindings(Keymap *map, BindingName_functPtr getbindingname, 
                          Boolean do_unbound, Boolean do_internal)
/* map: Keymap containing bindings we want described */
/* getbindingname: Function should map (Generic binding) to (char *bindingname) */
/* do_unbound: if true, dump bindings for unbound nodes also */
/* do_internal: if true, dump bindings for interior nodes also */
{
	KeymapWalk wd;
	Keymap_dump_params params;

	wd = keymap_walk_create(map);

	params.do_unbound	= do_unbound;
	params.getbindingname	= getbindingname;
	wd->handle		= (Generic)(&params);
	wd->walk_control	= do_internal ? TW_PREORDER : TW_LEAVES;
	wd->interior_handler	= keymap_describe_interior;
	wd->leaf_handler	= keymap_describe_leaf;
	wd->leaves_handler	= keymap_describe_leaves;
	(void) keymap_walk_begin(wd);
	keymap_walk_destroy(wd);
}

KbString keymap_get_queue(Keymap *map)
{
	KbString ks;
	int ki;
	int qi;

	ks.ephemeral = true;
	ks.num_kc = map->queue_length;
	ks.kc_ptr = (KbChar *)get_mem(ks.num_kc * sizeof(KbChar), "keymap_get_queue");

	for (ki = 0, qi = map->queue_front;
		ki < map->queue_length;
		ki++, qi = (qi+1)%(int)map->mapdepth )
	{

		ks.kc_ptr[ki] = map->queue[qi];
	}

	return ks;
}

char *keymap_queue_debug(Keymap *map)
{
	char buf[512];
	char *q1,*q2;
	char *s;
	int savelen;

	(void) sprintf(buf,
		"Allocated: %5d   Length: %5d   Front: %5d",
		map->mapdepth,
		map->queue_length,
		map->queue_front);

	q1 = symbolicFromKbString(keymap_get_queue(map));

	savelen = map->queue_length;
	map->queue_length = map->mapdepth;
	q2 = symbolicFromKbString(keymap_get_queue(map));
	map->queue_length = savelen;

	s = nssave(6,buf,"\nQueue:",q1,"\nAlloc'ed:",q2,"\n");

	sfree(q1);
	sfree(q2);

	return s;
}



