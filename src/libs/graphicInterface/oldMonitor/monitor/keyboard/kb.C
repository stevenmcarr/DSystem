/* $Id: kb.C,v 1.1 1997/06/25 14:49:10 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/* 								*/
/*	kb.c							*/
/* 								*/
/*	Keyboard input function file.				*/
/*	Last edited: August 10, 1990 by DGB			*/
/* 								*/
/****************************************************************/

#include <string.h>

#include <include/bstring.h>

#include <libs/graphicInterface/oldMonitor/monitor/keyboard/kb.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/keymap.h>
#include <libs/support/strings/rn_string.h>


	/* KEYBOARD INFORMATION */

char			*kb_names[] = {		/* the name of each keyboard		*/
					"default",
					"IBM",
					"Sun3",
					"Sparc",
					(char *) 0
			};
short			kb_keyboard_id;		/* the id of our keyboard		*/
Boolean			kb_swap_bs_del;		/* swap backspace and delete		*/
char			*KB_bogus_name = 	/* the name of the unknown character	*/
				"<Undefined>";
static	Keymap		*sym_to_kbchar;		/* the keymapping from symbolic to stngs*/

struct	block		{			/* CONTIGUOUS BLOCK OF CHARACTERS	*/
	KbChar		first;			/* first KbChar in the block		*/
	short		num;			/* number of KbChars in the block	*/
	char		**names;		/* the symbolic names of those chars	*/
			};
#define	NUM_BLOCKS	5			/* number of contiguous blocks		*/
typedef	struct	kbd	{			/* KEYBOARD DEFINITION			*/
	struct	block	block[NUM_BLOCKS];	/* the character blocks			*/
			} Keyboard;


	/* DEFINE SYMBOLIC KEYBOARD */

static	char		*sym_ascii_names[] = {	/* names of the ascii characters	*/
/* \00x */	"<NUL>",	"^A",		"^B",		"^C",		"^D",		"^E",		"^F",		"^G",
/* \01x */	"<BS>",		"<TAB>",	"<LF>",		"^K",		"^L",		"<RETURN>",	"^N",		"^O",
/* \02x */	"^P",		"^Q",		"^R",		"^S",		"^T",		"^U",		"^V",		"^W",
/* \03x */	"^X",		"^Y",		"^Z",		"<ESC>",	"^\\",		"^]",		"^^",		"^_",
/* \04x */	"<SP>",		"!",		"\\\"",		"#",		"$",		"%",		"&",		"'",
/* \05x */	"(",		")",		"*",		"+",		",",		"-",		".",		"/",
/* \06x */	"0",		"1",		"2",		"3",		"4",		"5",		"6",		"7",
/* \07x */	"8",		"9",		":",		";",		"\\<",		"=",		">",		"?",
/* \10x */	"@",		"A",		"B",		"C",		"D",		"E",		"F",		"G",
/* \11x */	"H",		"I",		"J",		"K",		"L",		"M",		"N",		"O",
/* \12x */	"P",		"Q",		"R",		"S",		"T",		"U",		"V",		"W",
/* \13x */	"X",		"Y",		"Z",		"[",		"\\\\",		"]",		"\\^",		"_",
/* \14x */	"`",		"a",		"b",		"c",		"d",		"e",		"f",		"g",
/* \15x */	"h",		"i",		"j",		"k",		"l",		"m",		"n",		"o",
/* \16x */	"p",		"q",		"r",		"s",		"t",		"u",		"v",		"w",
/* \17x */	"x",		"y",		"z",		"{",		"|",		"}",		"~",		"<DEL>",
/* \20x */	"\\200",	"\\201",	"\\202",	"\\203",	"\\204",	"\\205",	"\\206",	"\\207",
/* \21x */	"\\210",	"\\211",	"\\212",	"\\213",	"\\214",	"\\215",	"\\216",	"\\217",
/* \22x */	"\\220",	"\\221",	"\\222",	"\\223",	"\\224",	"\\225",	"\\226",	"\\227",
/* \23x */	"\\230",	"\\231",	"\\232",	"\\233",	"\\234",	"\\235",	"\\236",	"\\237",
/* \24x */	"\\240",	"\\241",	"\\242",	"\\243",	"\\244",	"\\245",	"\\246",	"\\247",
/* \25x */	"\\250",	"\\251",	"\\252",	"\\253",	"\\254",	"\\255",	"\\256",	"\\257",
/* \26x */	"\\260",	"\\261",	"\\262",	"\\263",	"\\264",	"\\265",	"\\266",	"\\267",
/* \27x */	"\\270",	"\\271",	"\\272",	"\\273",	"\\274",	"\\275",	"\\276",	"\\277",
/* \30x */	"\\300",	"\\301",	"\\302",	"\\303",	"\\304",	"\\305",	"\\306",	"\\307",
/* \31x */	"\\310",	"\\311",	"\\312",	"\\313",	"\\314",	"\\315",	"\\316",	"\\317",
/* \32x */	"\\320",	"\\321",	"\\322",	"\\323",	"\\324",	"\\325",	"\\326",	"\\327",
/* \33x */	"\\330",	"\\331",	"\\332",	"\\333",	"\\334",	"\\335",	"\\336",	"\\337",
/* \34x */	"\\340",	"\\341",	"\\342",	"\\343",	"\\344",	"\\345",	"\\346",	"\\347",
/* \35x */	"\\350",	"\\351",	"\\352",	"\\353",	"\\354",	"\\355",	"\\356",	"\\357",
/* \36x */	"\\360",	"\\361",	"\\362",	"\\363",	"\\364",	"\\365",	"\\366",	"\\367",
/* \37x */	"\\370",	"\\371",	"\\372",	"\\373",	"\\374",	"\\375",	"\\376",	"\\377",
			};
static	char		*sym_top_names[] = {	/* names of top keys			*/
		"<F0>",		"<F1>",		"<F2>",		"<F3>",		"<F4>",
		"<F5>",		"<F6>",		"<F7>",		"<F8>",		"<F9>",
		"<F10>",	"<F11>",	"<F12>",	"<F13>",	"<F14>",
		"<F15>",	"<F16>",	"<F17>",	"<F18>",	"<F19>",
		"<F20>",	"<F21>",	"<F22>",	"<F23>",	"<F24>",
			};
static	char		*sym_right_names[] = {	/* names of right keys			*/
		"<R0>",		"<R1>",		"<R2>",		"<R3>",		"<R4>",
		"<R5>",		"<R6>",		"<R7>",		"<R8>",		"<R9>",
		"<R10>",	"<R11>",	"<R12>",	"<R13>",	"<R14>",
		"<R15>",	"<R16>",	"<R17>",	"<R18>",	"<R19>",
		"<R20>",	"<R21>",	"<R22>",	"<R23>",	"<R24>",
			};
static	char		*sym_left_names[] = {	/* names of left keys			*/
		"<L0>",		"<L1>",		"<L2>",		"<L3>",		"<L4>",
		"<L5>",		"<L6>",		"<L7>",		"<L8>",		"<L9>",
		"<L10>",	"<L11>",	"<L12>",	"<L13>",	"<L14>",
		"<L15>",	"<L16>",	"<L17>",	"<L18>",	"<L19>",
		"<L20>",	"<L21>",	"<L22>",	"<L23>",	"<L24>",
			};
static	char		*sym_arrow_names[] = {	/* names of the arrow keys		*/
		"<UP>",		"<DOWN>",	"<LEFT>",	"<RIGHT>"
			};

static	Keyboard	symbolic = {		/* the symbolic keyboard definition	*/
		KB_first_ascii, KB_num_ascii,	sym_ascii_names,
		KB_first_top,	KB_num_top,	sym_top_names,
		KB_first_right,	KB_num_right,	sym_right_names,
		KB_first_left,	KB_num_left,	sym_left_names,
		KB_first_arrow,	KB_num_arrow,	sym_arrow_names		
			};

	/* DEFAULT NAMES */

static	Keyboard	default_names = {	/* the names of the default keys	*/
		KB_first_ascii,	128,		sym_ascii_names,
		KB_top(1),	0,		(char **) 0,
		KB_right(1),	0,		(char **) 0,
		KB_left(1),	0,		(char **) 0,
		KB_first_arrow,	0,		(char **) 0
			};

	/* IBM NAMES */

static	char		*ibm_ascii_names[] = {	/* names of the ascii characters	*/
/* \00x */	"^@",		"^A",		"^B",		"^C",		"^D",		"^E",		"^F",		"^G",
/* \01x */	"<Backspace>",	"<Tab>",	"^J",		"^K",		"^L",		"<Enter>",	"^N",		"^O",
/* \02x */	"^P",		"^Q",		"^R",		"^S",		"^T",		"^U",		"^V",		"^W",
/* \03x */	"^X",		"^Y",		"^Z",		"<Esc>",	"^\\",		"^]",		"^^",		"^_",
/* \04x */	"<Space>",	"!",		"\"",		"#",		"$",		"%",		"&",		"'",
/* \05x */	"(",		")",		"*",		"+",		",",		"-",		".",		"/",
/* \06x */	"0",		"1",		"2",		"3",		"4",		"5",		"6",		"7",
/* \07x */	"8",		"9",		":",		";",		"<",		"=",		">",		"?",
/* \10x */	"@",		"A",		"B",		"C",		"D",		"E",		"F",		"G",
/* \11x */	"H",		"I",		"J",		"K",		"L",		"M",		"N",		"O",
/* \12x */	"P",		"Q",		"R",		"S",		"T",		"U",		"V",		"W",
/* \13x */	"X",		"Y",		"Z",		"[",		"\\",		"]",		"^",		"_",
/* \14x */	"`",		"a",		"b",		"c",		"d",		"e",		"f",		"g",
/* \15x */	"h",		"i",		"j",		"k",		"l",		"m",		"n",		"o",
/* \16x */	"p",		"q",		"r",		"s",		"t",		"u",		"v",		"w",
/* \17x */	"x",		"y",		"z",		"{",		"|",		"}",		"~",		"<Delete>",
			};
static	char		*ibm_top_names[] = {	/* names of top keys			*/
		"<F1>",		"<F2>",		"<F3>",		"<F4>",		"<F5>",
		"<F6>",		"<F7>",		"<F8>",		"<F9>",		"<F10>",
		"<F11>",	"<F12>"
			};
static	char		*ibm_right_names[] = {	/* names of right keys			*/
		"<Num-/>",	"<Num-*>",	"<Num-->",	"<Num-7>",	"<Num-8>",
		"<Num-9>",	"<Num-4>",	"<Num-5>",	"<Num-6>",	"<Num-1>",
		"<Num-2>",	"<Num-3>",	"<Num-0>",	"<Num-.>",	"<Num-Enter>",
		"<Num-+>"
			};
static	char		*ibm_left_names[] = {	/* names of left keys			*/
		"<Insert>",	"<Home>",	"<PageUp>",	"<End>",	"<PageDown>"
			};
static	char		*ibm_arrow_names[] = {	/* names of the arrow keys		*/
		"<Up>",		"<Down>",	"<Left>",	"<Right>"
			};

static	Keyboard	ibm_names = {		/* the names of the ibm keys		*/
		KB_first_ascii,	128,		ibm_ascii_names,
		KB_top(1),	12,		ibm_top_names,
		KB_right(1),	16,		ibm_right_names,
		KB_left(1),	5,		ibm_left_names,
		KB_first_arrow,	KB_num_arrow,	ibm_arrow_names
			};

	/* SUN3 NAMES */

static	char		*sun3_ascii_names[] = {	/* names of the ascii characters	*/
/* \00x */	"^@",		"^A",		"^B",		"^C",		"^D",		"^E",		"^F",		"^G",
/* \01x */	"<BackSpace>",	"<Tab>",	"<LineFeed>",	"^K",		"^L",		"<Return>",	"^N",		"^O",
/* \02x */	"^P",		"^Q",		"^R",		"^S",		"^T",		"^U",		"^V",		"^W",
/* \03x */	"^X",		"^Y",		"^Z",		"<Esc>",	"^\\",		"^]",		"^^",		"^_",
/* \04x */	"<Space>",	"!",		"\"",		"#",		"$",		"%",		"&",		"'",
/* \05x */	"(",		")",		"*",		"+",		",",		"-",		".",		"/",
/* \06x */	"0",		"1",		"2",		"3",		"4",		"5",		"6",		"7",
/* \07x */	"8",		"9",		":",		";",		"<",		"=",		">",		"?",
/* \10x */	"@",		"A",		"B",		"C",		"D",		"E",		"F",		"G",
/* \11x */	"H",		"I",		"J",		"K",		"L",		"M",		"N",		"O",
/* \12x */	"P",		"Q",		"R",		"S",		"T",		"U",		"V",		"W",
/* \13x */	"X",		"Y",		"Z",		"[",		"\\",		"]",		"^",		"_",
/* \14x */	"`",		"a",		"b",		"c",		"d",		"e",		"f",		"g",
/* \15x */	"h",		"i",		"j",		"k",		"l",		"m",		"n",		"o",
/* \16x */	"p",		"q",		"r",		"s",		"t",		"u",		"v",		"w",
/* \17x */	"x",		"y",		"z",		"{",		"|",		"}",		"~",		"<Delete>",
			};
static	char		*sun3_top_names[] = {	/* names of top keys			*/
		"<F1>",		"<F2>",		"<F3>",		"<F4>",		"<F5>",
		"<F6>",		"<F7>",		"<F8>",		"<F9>",		"<Break>"
			};
static	char		*sun3_right_names[] = {	/* names of right keys			*/
		"<R1>",		"<R2>",		"<R3>",		"<R4>",		"<R5>",
		"<R6>",		"<R7>",		"",		"<R9>",		"",
		"<R11>",	"",		"<R13>",	"",		"<R15>"
			};
static	char		*sun3_left_names[] = {	/* names of left keys			*/
		"<L1>",		"<L2>",		"<L3>",		"<L4>",		"<L5>",
		"<L6>",		"<L7>",		"<L8>",		"<L9>",		"<L10>"
			};
static	char		*sun3_arrow_names[] = {	/* names of the arrow keys		*/
		"<Up-Arrow>",	"<Down-Arrow>",	"<Left-Arrow>",	"<Right-Arrow>"
			};

static	Keyboard	sun3_names = {		/* the names of the sun3 keys		*/
		KB_first_ascii,	128,		sun3_ascii_names,
		KB_top(1),	10,		sun3_top_names,
		KB_right(1),	15,		sun3_right_names,
		KB_left(1),	10,		sun3_left_names,
		KB_first_arrow,	KB_num_arrow,	sun3_arrow_names
			};


	/* SPARC NAMES */

static	char		*sparc_ascii_names[] = {/* names of the ascii characters	*/
/* \00x */	"^@",		"^A",		"^B",		"^C",		"^D",		"^E",		"^F",		"^G",
/* \01x */	"<BackSpace>",	"<Tab>",	"<LineFeed>",	"^K",		"^L",		"<Return>",	"^N",		"^O",
/* \02x */	"^P",		"^Q",		"^R",		"^S",		"^T",		"^U",		"^V",		"^W",
/* \03x */	"^X",		"^Y",		"^Z",		"<Esc>",	"^\\",		"^]",		"^^",		"^_",
/* \04x */	"<Space>",	"!",		"\"",		"#",		"$",		"%",		"&",		"'",
/* \05x */	"(",		")",		"*",		"+",		",",		"-",		".",		"/",
/* \06x */	"0",		"1",		"2",		"3",		"4",		"5",		"6",		"7",
/* \07x */	"8",		"9",		":",		";",		"<",		"=",		">",		"?",
/* \10x */	"@",		"A",		"B",		"C",		"D",		"E",		"F",		"G",
/* \11x */	"H",		"I",		"J",		"K",		"L",		"M",		"N",		"O",
/* \12x */	"P",		"Q",		"R",		"S",		"T",		"U",		"V",		"W",
/* \13x */	"X",		"Y",		"Z",		"[",		"\\",		"]",		"^",		"_",
/* \14x */	"`",		"a",		"b",		"c",		"d",		"e",		"f",		"g",
/* \15x */	"h",		"i",		"j",		"k",		"l",		"m",		"n",		"o",
/* \16x */	"p",		"q",		"r",		"s",		"t",		"u",		"v",		"w",
/* \17x */	"x",		"y",		"z",		"{",		"|",		"}",		"~",		"<Delete>",
			};
static	char		*sparc_top_names[] = {	/* names of top keys			*/
		"<F1>",		"<F2>",		"<F3>",		"<F4>",		"<F5>",
		"<F6>",		"<F7>",		"<F8>",		"<F9>",		"<F10>",
		"<F11>",	"<F12>"
			};
static	char		*sparc_right_names[] = {/* names of right keys			*/
		"<Pause>",	"<PrSc>",	"<ScrollLock>",	"<num-=>",	"<num-/>",
		"<num-*>",	"<num-7>",	"",		"<num-9>",	"",
		"<num-5>",	"",		"<num-1>",	"",		"<num-3>",
		"<num-->",	"<num-+>",	"<num-0>",	"<num-.>",	"<Enter>"
			};
static	char		*sparc_left_names[] = {	/* names of left keys			*/
		"<Stop>",	"<Again>",	"<Props>",	"<Undo>",	"<Front>",
		"<Copy>",	"<Open>",	"<Paste>",	"<Find>",	"<Cut>",
		"<Help>"
			};
static	char		*sparc_arrow_names[] = {/* names of the arrow keys		*/
		"<Up-Arrow>",	"<Down-Arrow>",	"<Left-Arrow>",	"<Right-Arrow>"
			};

static	Keyboard	sparc_names = {		/* the names of the sparc keys		*/
		KB_first_ascii,	128,		sparc_ascii_names,
		KB_top(1),	12,		sparc_top_names,
		KB_right(1),	20,		sparc_right_names,
		KB_left(1),	11,		sparc_left_names,
		KB_first_arrow,	KB_num_arrow,	sparc_arrow_names
			};


static	Keyboard	*names[] = {		/* Keyboard names			*/
		&default_names,			/* default names			*/
		&ibm_names,			/* IBM names				*/
		&sun3_names,			/* sun3 names				*/
		&sparc_names,			/* Sparc names				*/
			};



	/* BINDINGS */

static	char		*just_ascii_bindings[] = {/* bindings of the ascii characters	*/
/* \00x */	"\000",		"\001",		"\002",		"\003",		"\004",		"\005",		"\006",		"\007",
/* \01x */	"\010",		"\011",		"\012",		"\013",		"\014",		"\015",		"\016",		"\017",
/* \02x */	"\020",		"\021",		"\022",		"\023",		"\024",		"\025",		"\026",		"\027",
/* \03x */	"\030",		"\031",		"\032",		"\033",		"\034",		"\035",		"\036",		"\037",
/* \04x */	"\040",		"\041",		"\042",		"\043",		"\044",		"\045",		"\046",		"\047",
/* \05x */	"\050",		"\051",		"\052",		"\053",		"\054",		"\055",		"\056",		"\057",
/* \06x */	"\060",		"\061",		"\062",		"\063",		"\064",		"\065",		"\066",		"\067",
/* \07x */	"\070",		"\071",		"\072",		"\073",		"\074",		"\075",		"\076",		"\077",
/* \10x */	"\100",		"\101",		"\102",		"\103",		"\104",		"\105",		"\106",		"\107",
/* \11x */	"\110",		"\111",		"\112",		"\113",		"\114",		"\115",		"\116",		"\117",
/* \12x */	"\120",		"\121",		"\122",		"\123",		"\124",		"\125",		"\126",		"\127",
/* \13x */	"\130",		"\131",		"\132",		"\133",		"\134",		"\135",		"\136",		"\137",
/* \14x */	"\140",		"\141",		"\142",		"\143",		"\144",		"\145",		"\146",		"\147",
/* \15x */	"\150",		"\151",		"\152",		"\153",		"\154",		"\155",		"\156",		"\157",
/* \16x */	"\160",		"\161",		"\162",		"\163",		"\164",		"\165",		"\166",		"\167",
/* \17x */	"\170",		"\171",		"\172",		"\173",		"\174",		"\175",		"\176",		"\177",
			};

	/* DEFAULT BINDINGS */

static	Keyboard	default_bindings = {	/* the default bindings			*/
		KB_first_ascii,	128,		just_ascii_bindings,
		KB_top(1),	0,		(char **) 0,
		KB_right(1),	0,		(char **) 0,
		KB_left(1),	0,		(char **) 0,
		KB_first_arrow,	0,		(char **) 0
			};

	/* IBM BINDINGS */

static	char		*ibm_top_bindings[] = {	/* bindings of top keys			*/
		"[001q",	"[002q",	"[003q",	"[004q",	"[005q",
		"[006q",	"[007q",	"[008q",	"[009q",	"[010q",
		"[011q",	"[012q"
			};
static	char		*ibm_right_bindings[] = {/* bindings of right keys		*/
		"[179q",	"[187q",	"[198q",	"[172q",	"[182q",
		"[190q",	"[174q",	"[184q",	"[192q",	"[176q",
		"[186q",	"[194q",	"[178q",	"[196q",	"[100q",
		"[200q"
			};
static	char		*ibm_left_bindings[] = {/* bindings of left keys		*/
		"[139q",	"[H",		"[150q",	"[146q",	"[154q"
			};
static	char		*ibm_arrow_bindings[] = {/* bindings of the arrow keys		*/
		"[A",		"[B",		"[D",		"[C"
			};

static	Keyboard	ibm_bindings = {	/* the bindings of the ibm keys		*/
		KB_first_ascii,	128,		just_ascii_bindings,
		KB_top(1),	12,		ibm_top_bindings,
		KB_right(1),	16,		ibm_right_bindings,
		KB_left(1),	5,		ibm_left_bindings,
		KB_first_arrow,	KB_num_arrow,	ibm_arrow_bindings
			};

	/* SUN3 BINDINGS */

static	char		*sun3_top_bindings[] = {/* bindings of top keys			*/
		"[224z",	"[225z",	"[226z",	"[227z",	"[228z",
		"[229z",	"[230z",	"[231z",	"[232z",	"[233z"
			};
static	char		*sun3_right_bindings[] = {/* bindings of right keys		*/
		"[208z",	"[209z",	"[210z",	"[211z",	"[212z",
		"[213z",	"[214z",	"",		"[216z",	"",
		"[218z",	"",		"[220z",	"",		"[222z"
			};
static	char		*sun3_left_bindings[] = {/* bindings of left keys		*/
		"[192z",	"[193z",	"[194z",	"[195z",	"[196z",
		"[197z",	"[198z",	"[199z",	"[200z",	"[201z"
			};
static	char		*sun3_arrow_bindings[] = {/* bindings of the arrow keys		*/
		"[A",		"[B",		"[D",		"[C"
			};

static	Keyboard	sun3_bindings = {	/* the bindings of the sun3 keys	*/
		KB_first_ascii,	128,		just_ascii_bindings,
		KB_top(1),	10,		sun3_top_bindings,
		KB_right(1),	15,		sun3_right_bindings,
		KB_left(1),	10,		sun3_left_bindings,
		KB_first_arrow,	KB_num_arrow,	sun3_arrow_bindings
			};

	/* SPARC BINDINGS */

static	char		*sparc_top_bindings[] = {/* bindings of top keys		*/
		"[224z",	"[225z",	"[226z",	"[227z",	"[228z",
		"[229z",	"[230z",	"[231z",	"[232z",	"[233z",
		"[234z",	"[235z"
			};
static	char		*sparc_right_bindings[] = {/* bindings of right keys		*/
		"[208z",	"[209z",	"[210z",	"[211z",	"[212z",
		"[213z",	"[214z",	"",		"[216z",	"",
		"[218z",	"",		"[220z",	"",		"[222z",
		"[254z",	"[253z",	"[247z",	"[249z",	"[250z"
			};
static	char		*sparc_left_bindings[] = {/* bindings of left keys		*/
		"[192z",	"[193z",	"[194z",	"[195z",	"[196z",
		"[197z",	"[198z",	"[199z",	"[200z",	"[201z",
		"[207z"
			};
static	char		*sparc_arrow_bindings[] = {/* bindings of the arrow keys	*/
		"[A",		"[B",		"[D",		"[C"
			};

static	Keyboard	sparc_bindings = {	/* the bindings of the sparc keys	*/
		KB_first_ascii,	128,		just_ascii_bindings,
		KB_top(1),	12,		sparc_top_bindings,
		KB_right(1),	20,		sparc_right_bindings,
		KB_left(1),	11,		sparc_left_bindings,
		KB_first_arrow,	KB_num_arrow,	sparc_arrow_bindings
			};

static	Keyboard	*bindings[] = {		/* Keyboard bindings			*/
		&default_bindings,		/* default bindings			*/
		&ibm_bindings,			/* IBM bindings				*/
		&sun3_bindings,			/* sun3 bindings			*/
		&sparc_bindings,		/* Sparc bindings			*/
			};


STATIC(char*, lookup_name, (Keyboard *kbp, KbChar kbc, char *result));

/* Look up a name in a keyboard structure.						*/
static char*
lookup_name(Keyboard *kbp, KbChar kbc, char *result)
/* kbp:  the keyboard pointer			*/
/* kbc: the key to look up			*/
/* *result: what to return if unfound		*/
{
short			bn;			/* the current block being bound	*/
struct	block		*bp;			/* the pointer to the current block	*/
short			temp;			/* temporary guess of char in block	*/

	for (bn = 0; bn < NUM_BLOCKS; bn++)
	{/* try to find the block that the character is in */
		bp = &kbp->block[bn];
		temp = kbc - bp->first;
		if ((0 <= temp) && (temp < bp->num))
		{/* the character is in this block */
			result = bp->names[temp];
			break;
		}
	}
	return result;
}

/* Initialize the symbolic keyboard conversion.						*/
void
startKb(void)
{
short			bn;			/* the current block being bound	*/
struct	block		*bp;			/* the pointer to the current block	*/
short			i;			/* the index of the current symbolic nm.*/

	sym_to_kbchar = keymap_create(UNUSED);
	for (bn = 0; bn < NUM_BLOCKS; bn++)
	{/* bind all of the symbolic strings in a block */
		bp = &symbolic.block[bn];
		for (i = 0; i < bp->num; i++)
		{/* bind the ith symbolic string in this block */
			keymap_bind_KbString(
				sym_to_kbchar,
				(Generic) (bp->first + i),
				makeKbString(bp->names[i], "startKb() new binding")
			);
		}
	}
}


/* Finalize the symbolic keyboard conversion.						*/
void
finishKb(void)
{
	keymap_destroy(sym_to_kbchar);
}


/* Create a KbChar from its constituents.						*/
KbChar
makeKbChar(unsigned char base, unsigned char top)
/* base: the low order byte			*/
/* top: the high order byte			*/
{
	return (toKbChar(((unsigned short) top) << 8 + base));
}


/* Create a non-ephemeral KbString of a given length.					*/
KbString
getKbString(short len, char *s)
/* len: the number of KbChars to be used	*/
/* s: identification string		*/
{
KbString		kbs;			/* the returned KbString		*/

	kbs.kc_ptr    = (KbChar *) get_mem(len * sizeof(KbChar), "%s (getKbString)", s);
	kbs.ephemeral = false;
	kbs.num_kc    = len;
	return kbs;
}

/* Create an ephemeral copy of a KbString */
KbString copyKbString(KbString ks)
{
	KbString ns;

	ns.kc_ptr = (KbChar *)get_mem(ks.num_kc*sizeof(KbChar),"copyKbString");
	ns.num_kc = ks.num_kc;
	ns.ephemeral = true;
	bcopy(ks.kc_ptr, ns.kc_ptr, ks.num_kc*sizeof(KbChar));
	return ns;
}

/*
 * regetKbString -- realloc an existing KbString to a new length.
 *	The new empheral KbString is returned.
 */

KbString regetKbString(KbString *ks, int nl)
/* ks: Old KbString to reallocate at a different size */
/* nl: New length */
{
	if (ks->ephemeral)
	{/* Really do a realloc */
		ks->kc_ptr = (KbChar *)reget_mem((void*)ks->kc_ptr,
					nl*sizeof(KbChar),
					"regetKbString");
	}
	else
	{/* Make a KbString from scratch, initializing it from ks */
		KbChar *ns;	/* New string */

		ns = (KbChar *)get_mem( nl*sizeof(KbChar), "regetKbString" );
		bcopy(ks->kc_ptr, ns, ks->num_kc*sizeof(KbChar));
		ks->kc_ptr = ns;
	}

	ks->ephemeral = true;
	ks->num_kc = nl;
	return *ks;
}

/* Create an ephemeral KbString from a null terminated string.				*/
KbString
makeKbString(char *str, char *s)
/* str: a null terminated string      	*/
/* s: identification string		*/
{
KbString		kbs;			/* the returned KbString		*/
short			i;			/* the current character index		*/

	kbs = getKbString(strlen(str), s);
	kbs.ephemeral = true;
	for (i = 0; str[i]; i++)
	{/* transfer each character */
		kbs.kc_ptr[i] = toKbChar(str[i]);
	}
	return kbs;
}


/* Free the resources held by a KbString.						*/
void
freeKbString(KbString kbs)
{
	if (kbs.ephemeral)
		free_mem((void*)kbs.kc_ptr);
}


/* Convert a KbString to a null-terminated char *.					*/
void
convertKbString(KbString kbs, char *buf)
/* kbs: the KbString to convert		*/
/* buf: where to write the converted string	*/
{
short			i;			/* the index of the current KbChar	*/

	for (i = 0; i < kbs.num_kc; i++)
	{/* transfer the ith character */
		*buf++ = (char) kbs.kc_ptr[i];
	}
	*buf = '\0';

	freeKbString(kbs);
}


/* Convert a char* (symbolic representation) to a the corresponding KbString.		*/
KbString
symbolicToKbString(char *sym)
/* sym: the symbolic string to be converted	*/
{
KbString		kbs;			/* the KbString to be returned		*/

	/* get a KbString which is big enough (conservative estimate) */
		kbs = getKbString(strlen(sym), "symbolicToKbString()");
		kbs.ephemeral = true;
		kbs.num_kc    = 0;

	if (*sym)
	{/* try to run the string through the symbolic mapping */
		keymap_enqueue_KbString(sym_to_kbchar, makeKbString(sym, "symbolicToKbString()"));
		while (keymap_mapping_complete(sym_to_kbchar))
		{/* we have mapped a new KbChar */
			kbs.kc_ptr[kbs.num_kc++] = toKbChar(keymap_info(sym_to_kbchar));
		}
		if (NOT(keymap_queue_empty(sym_to_kbchar)))
		{/* there is a problem with this string--clean up */
			freeKbString(kbs);
			kbs.ephemeral = false;
			kbs.num_kc    = UNUSED;
			kbs.kc_ptr    = (KbChar *) 0;
			keymap_reset(sym_to_kbchar);
			keymap_queue_flush(sym_to_kbchar);
		}
	}
	return (kbs);
}


/* Convert a KbChar to the corresponding null-terminated symbolic (get_mem()ed) string	*/
char *
symbolicFromKbChar(KbChar kbc)
/* kbc: the KbChar to be converted		*/
{
	return lookup_name(&symbolic, kbc, KB_bogus_name);
}


/* Convert a KbString to the corresponding null-terminated symbolic (get_mem()ed) string*/
char *
symbolicFromKbString(KbString kbs)
/* kbs: the KbString to be converted		*/
{
short			i;			/* the index of the current KbChar	*/
short			len = 0;		/* the length of the sequence		*/
char			*result;		/* the resulting string			*/

	/* figure the length of the needed string */
		for (i = 0; i < kbs.num_kc; i++)
		{/* figure the length of this KbChar  */
			len += strlen(symbolicFromKbChar(kbs.kc_ptr[i]));
		}

	/* get enough room for the entire string */
		result = (char *) get_mem(len + 1, "symbolicFromKbString()");
		result[0] = '\0';


	/* stuff the result in the needed string */
		for (i = 0; i < kbs.num_kc; i++)
		{/* append the current KbChar */
			(void) strcat(result, symbolicFromKbChar(kbs.kc_ptr[i]));
		}

	freeKbString(kbs);

	return (result);
}


/* Return the (allocated) actual name for a KbChar.					*/
char *
actualFromKbChar(KbChar kbc)
/* kbc: the KbChar to be converted		*/
{
	return lookup_name(names[kb_keyboard_id], kbc, KB_bogus_name);
}


/* Return the (allocated) actual name for a KbString.					*/
char *
actualFromKbString(KbString kbs)
/* kbs: the KbString to be converted		*/
{
short			i;			/* the index of the current KbChar	*/
short			len = 0;		/* the length of the sequence		*/
char			*result;		/* the resulting string			*/

	/* figure the length of the needed string */
		for (i = 0; i < kbs.num_kc; i++)
		{/* figure the length of this KbChar  */
			len += strlen(actualFromKbChar(kbs.kc_ptr[i]));
		}
		len += kbs.num_kc - 1;	/* figure intermediate separaters */

	/* get enough room for the entire string */
		result = (char *) get_mem(len + 1, "actualFromKbString()");
		result[0] = '\0';


	/* stuff the result in the needed string */
		for (i = 0; i < kbs.num_kc; i++)
		{/* append the current KbChar */
			if (i)
			{/* add the intermediate separater */
				(void) strcat(result, "-");
			}
			(void) strcat(result, actualFromKbChar(kbs.kc_ptr[i]));
		}

	freeKbString(kbs);

	return (result);
}


/* Return the keyboard-generated string for a KbChar.					*/
char *
inputFromKbChar(KbChar kbc)
/* kbc: the KbChar to be converted		*/
{
	return lookup_name(bindings[kb_keyboard_id], kbc, "");
}

