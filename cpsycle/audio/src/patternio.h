/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_PATTERNIO_H
#define psy_audio_PATTERNIO_H

#include <dsptypes.h>
#include "../../detail/stdint.h"


#ifdef __cplusplus
extern "C" {
#endif

struct psy_audio_Pattern;
struct psy_Path;

void psy_audio_patternio_load(struct psy_audio_Pattern*, const struct psy_Path*, double bpl);
void psy_audio_patternio_save(struct psy_audio_Pattern*, const struct psy_Path*, double bpl, uintptr_t songtracks);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_WAVSONGIO_H */
