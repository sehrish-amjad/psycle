/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(psy_TRANSLATOR_H)
#define psy_TRANSLATOR_H

/* container */
#include <properties.h>
#include <signal.h>
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*!
** @struct psy_Translator
** @brief Defines a dictionary for localization with a default set in english.
**
** @details
** It can test "ini" files for language definitions and can load a dictionary from an
** "ini" file.
**
** psy_Translator <>----- psy_Property        ; dictionary
**    |
**    - - <<use>>- -> psy_propertiesio      ; dictionary load
**
** Structure of properties/ini-file:
** root.lang: sets the language id used to identify the language (de, en, es)
** sections:
** [file] [undo] [play] [main] [help] [machinebar] [edit] [lpb] [render] [gear]
** [settingsview] [instrumentview] [machineview] [newmachine] [samplesview]
** [cmds]
** word definitions: see translator.c definekeys
*/

typedef struct psy_Translator {
	/* signals */
	psy_Signal signal_language_changed;
	/*! @internal */
	psy_Dictionary dictionary_;
	char* test_id_;
} psy_Translator;

void psy_translator_init(psy_Translator*);
void psy_translator_dispose(psy_Translator*);
/* sets a default dictionary */
void psy_translator_set_default(psy_Translator*, const psy_Dictionary*);
/* reset to default (english) */
void psy_translator_reset(psy_Translator*);
/*
** loads a.ini file containing a dictionary
** first it resets to default (english) and overwrites available properties
** with the new definitions
*/
bool psy_translator_load(psy_Translator* self, const char* path);
/* tests root.lang property. (workspace uses this to scan the language list) */
bool psy_translator_test(const psy_Translator*, const char* path, char* id);
/*
** translate a key
**  word definitions: see translator.c definekeys
**  example  translate("file.load")
**    returns: Load (for 'en')
**  translate("help.load")
**    returns: load (not found in help, return key without section prefix)
*/
const char* psy_translator_translate(psy_Translator*, const char* key);
const char* psy_translator_lang_id(const psy_Translator*);

#ifdef __cplusplus
}
#endif

#endif /* psy_TRANSLATOR_H */
