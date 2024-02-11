/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "inputhandler.h"
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"


/* InputSlot */

/* implementation */
void inputslot_init(InputSlot* self, InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input input)
{
	assert(self);
	
	self->type = type;
	self->cmdtype = cmdtype;
	self->section = psy_strdup(section);
	self->id = id;
	self->context = context;
	self->input = input;	
}

void inputslot_dispose(InputSlot* self)
{
	free(self->section);
	self->section = NULL;
}

InputSlot* inputslot_alloc(void)
{
	return (InputSlot*)malloc(sizeof(InputSlot));
}

InputSlot* inputslot_allocinit(InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input  input)
{
	InputSlot* rv;

	rv = inputslot_alloc();
	if (rv) {
		inputslot_init(rv, type, cmdtype, section, id, context, input);
	}
	return rv;
}


/* InputHandler */

/* prototypes */
static bool inputhandler_sendmessage(InputHandler*, int msg, void* param1);

/* implementation */
void inputhandler_init(InputHandler* self)
{
	assert(self);

	self->cmd_.id = -1;
	self->input_ = psy_eventdriverinput_make(0, 0, 0);
	self->slots_ = NULL;
	self->hostcontext_ = NULL;
	self->hostcallback_ = NULL;
	self->sender_ = NULL;
}

void inputhandler_dispose(InputHandler* self)
{
	assert(self);
	
	psy_list_deallocate(&self->slots_, (psy_fp_disposefunc)inputslot_dispose);
}

void inputhandler_connect(InputHandler* self, InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section, uintptr_t id,
	void* context, fp_inputhandler_input input)
{
	psy_list_append(&self->slots_,
		inputslot_allocinit(type, cmdtype, section, id, context, input));
}

void inputhandler_connect_host(InputHandler* self, void* context,
	fp_inputhandler_hostcallback callback)
{
	self->hostcontext_ = context;
	self->hostcallback_ = callback;
}

void inputhandler_event_driver_input(InputHandler* self, psy_EventDriver* sender)
{
	psy_List* p;

	for (p = self->slots_; p != NULL; p = p->next) {
		InputSlot* slot;
		bool emit;

		slot = (InputSlot*)p->entry;
		emit = TRUE;
		if (emit && slot->type == INPUTHANDLER_FOCUS) {
			emit = inputhandler_sendmessage(self,
				INPUTHANDLER_HASFOCUS, slot->context);
		}		
		if (emit) {
			self->cmd_ = psy_eventdriver_getcmd(sender, slot->section);
			self->input_ = psy_eventdriver_input(sender);
			self->sender_ = sender;
			if (self->cmd_.type == slot->cmdtype && slot->input(slot->context,
					self)) {
				break;
			}
		}
	}
}

void inputhandler_send(InputHandler* self, const char* section, psy_EventDriverCmd cmd)
{
	psy_List* p;

	self->cmd_ = cmd;
	for (p = self->slots_; p != NULL; p = p->next) {
		InputSlot* slot;
		bool emit;

		slot = (InputSlot*)p->entry;
		emit = TRUE;		
		if (emit && slot->type == INPUTHANDLER_FOCUS) {
			emit = inputhandler_sendmessage(self,
				INPUTHANDLER_HASFOCUS, slot->context);
		}				
		if (emit) {
			if (section && slot->section && strcmp(section, slot->section) != 0) {
				continue;
			}
			if (!section && slot->section) {
				continue;
			}
			self->sender_ = NULL;
			if (self->cmd_.type == slot->cmdtype && slot->input(slot->context,
					self)) {
				break;
			}
		}
	}
}

bool inputhandler_sendmessage(InputHandler* self, int msg, void* param1)
{
	if (self->hostcallback_) {
		return self->hostcallback_(self->hostcontext_, msg, param1);
	}
	return FALSE;
}

psy_EventDriverCmd inputhandler_cmd(const InputHandler* self)
{
	return self->cmd_;
}

psy_EventDriverInput inputhandler_input(const InputHandler* self)
{
	return self->input_;
}

psy_EventDriver* inputhandler_sender(const InputHandler* self)
{
	return self->sender_;
}
