/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(CMDSGENERAL_H)
#define CMDSGENERAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*
** Defines a set of command ids to define keyboard shortcuts.
** Note shortcut ids are defined in cmdsnotes.h
** See also cmdproperties.h for the keyboard mapping
*/

/* Start index for note input (corresponds to note C-0) */
#define CS_KEY_START 0
/* Start index of inmediate commands (play, stop, change view,...) */
#define CS_IMM_START 256
/* Start index for edit-related commands (select, cut..) */
#define CS_EDT_START 512		

enum {
	/* immediate commands */
	CMD_IMM_EDITTOGGLE = CS_IMM_START,

	CMD_IMM_OCTAVEUP,
	CMD_IMM_OCTAVEDN,

	CMD_IMM_MACHINEDEC,
	CMD_IMM_MACHINEINC,

	CMD_IMM_INSTRDEC,
	CMD_IMM_INSTRINC,

	CMD_IMM_PLAYROWTRACK,
	CMD_IMM_PLAYROWPATTERN,
	CMD_IMM_PLAYSTART,
	CMD_IMM_PLAYSONG,
	CMD_IMM_PLAYFROMPOS,
	CMD_IMM_PLAYBLOCK,
	CMD_IMM_PLAYSTOP,

	CMD_IMM_INFOPATTERN,
	CMD_IMM_INFOMACHINE,
	CMD_IMM_INFOTRACKNAMES,

	CMD_IMM_EDITMACHINE,
	CMD_IMM_EDITPATTERN,
	CMD_IMM_EDITINSTR,
	CMD_IMM_EDITSAMPLE,
	CMD_IMM_EDITWAVE,
	CMD_IMM_ADDMACHINE,
	CMD_IMM_TERMINAL,
	CMD_IMM_STYLES,

	CMD_IMM_PATTERNINC,
	CMD_IMM_PATTERNDEC,
	
	CMD_IMM_SONGPOSINC,
	CMD_IMM_SONGPOSDEC,

	CMD_IMM_COLUMNPREV,		/* < TAB */
	CMD_IMM_COLUMNNEXT,		/* < S-TAB */

	CMD_IMM_NAVUP,
	CMD_IMM_NAVDN,
	CMD_IMM_NAVLEFT,
	CMD_IMM_NAVRIGHT,
	CMD_IMM_NAVPAGEUP,	/* < PGUP */
	CMD_IMM_NAVPAGEDN,	/* < PGDN */
	CMD_IMM_NAVTOP,		/* < HOME */
	CMD_IMM_NAVBOTTOM,	/* < END */

	CMD_IMM_SELECTMACHINE,	/* < ENTER */
	CMD_IMM_UNDO,
	CMD_IMM_REDO,
	CMD_IMM_FOLLOWSONG,
	CMD_IMM_MAXPATTERN,
	CMD_IMM_HELP,
	CMD_IMM_HELPSHORTCUT,
	CMD_IMM_SETTINGS,
	CMD_IMM_ENABLEAUDIO,
	CMD_IMM_LOADSONG,
	CMD_IMM_SAVESONG,
	CMD_IMM_SAVESONGAS,
	CMD_IMM_LOAD_SAMPLE,
	CMD_IMM_TAB1,
	CMD_IMM_TAB2,
	CMD_IMM_TAB3,
	CMD_IMM_TAB4,
	CMD_IMM_TAB5,
	CMD_IMM_TAB6,
	CMD_IMM_TAB7,
	CMD_IMM_TAB8,
	CMD_IMM_TAB9,

	CMD_IMM_FULLSCREEN,
	CMD_IMM_EDTMODE,

	CMD_EDT_TRANSPOSECHANNELINC = CS_EDT_START,	
	CMD_EDT_TRANSPOSECHANNELDEC,
	CMD_EDT_TRANSPOSECHANNELINC12,
	CMD_EDT_TRANSPOSECHANNELDEC12,
	CMD_EDT_TRANSPOSEBLOCKINC,
	CMD_EDT_TRANSPOSEBLOCKDEC,
	CMD_EDT_TRANSPOSEBLOCKINC12,
	CMD_EDT_TRANSPOSEBLOCKDEC12,

	CMD_EDT_PATTERNCUT,
	CMD_EDT_PATTERNCOPY,
	CMD_EDT_PATTERNPASTE,

	CMD_EDT_ROWINSERT,
	CMD_EDT_ROWDELETE,
	CMD_EDT_ROWCLEAR,
	
	CMD_EDT_BLOCKSTART,
	CMD_EDT_BLOCKEND,
	CMD_EDT_BLOCKUNMARK,
	CMD_EDT_BLOCKDOUBLE,
	CMD_EDT_BLOCKHALVE,
	CMD_EDT_BLOCKCUT,
	CMD_EDT_BLOCKCOPY,
	CMD_EDT_BLOCKPASTE,
	CMD_EDT_BLOCKMIX,
	CMD_EDT_BLOCKINTERPOLATE,
	CMD_EDT_BLOCKSETMACHINE,
	CMD_EDT_BLOCKSETINSTR,
	
	CMD_EDT_SELECTALL,
	CMD_EDT_SELECTCOL,

	CMD_EDT_EDITQUANTIZEDEC,
	CMD_EDT_EDITQUANTIZEINC,

	/*
	** new ones have to go at bottom of each section or else bad registry
	** reads
	*/
	CMD_EDT_PATTERNMIXPASTE,
	CMD_EDT_PATTERNTRACKMUTE,
	CMD_EDT_KEYSTOPANY,	/* < NOTE STOP */
	CMD_EDT_PATTERNDELETE,
	CMD_EDT_BLOCKDELETE,
	CMD_EDT_PATTERNTRACKSOLO,
	CMD_EDT_PATTERNTRACKRECORD,
	CMD_EDT_SELECTBAR	
};

#ifdef __cplusplus
}
#endif

#endif /* CMDSGENERAL_H */
