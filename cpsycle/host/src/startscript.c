/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "startscript.h"
/* host */
#include "mainframe.h"
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

static const int CHILDVIEWPORT = 1;
static const int FRAMEVIEWPORT = 2;
static const int TOOLBARVIEWPORT = 3;

static const int MDI = 3;
static const int SDI = 4;

static const int CREATELAZY = 5;
static const int CREATEATSTART = 6;

/* Link */
/* implementation */
void link_init(Link* self)
{
	assert(self);

	self->label_ = NULL;
	self->path_ = NULL;
	self->viewport_ = CHILDVIEWPORT;
	self->userinterface_ = MDI;
}

void link_init_all(Link* self, const char* path,
	const char* label, int viewport, int userinterface)
{
	assert(self);

	self->path_ = psy_strdup(path);
	self->label_ = psy_strdup(label);
	self->viewport_ = viewport;
	self->userinterface_ = userinterface;
}

void link_dispose(Link* self)
{
	assert(self);

	free(self->label_);
	self->label_ = NULL;
	free(self->path_);
	self->path_ = NULL;
}

Link* link_alloc(void)
{
	return (Link*)malloc(sizeof(Link));
}

Link* link_alloc_init(void)
{
	Link* rv;

	rv = link_alloc();
	if (rv) {
		link_init(rv);
	}
	return rv;
}

Link* link_clone(const Link* self)
{
	Link* rv;

	rv = link_alloc_init();
	if (rv) {		
		link_copy(rv, self);
	}
	return rv;
}

void link_copy(Link* self, const Link* other)
{
	assert(self);

	psy_strreset(&self->label_, other->label_);
	psy_strreset(&self->path_, other->path_);
	self->viewport_ = other->viewport_;
	self->userinterface_ = other->userinterface_;
}

const char* link_path(const Link* self)
{
	assert(self);

	return self->path_;
}

/* Links */

void links_init(Links* self)
{
	assert(self);

	psy_table_init(&self->container);
}

void links_dispose(Links* self)
{
	assert(self);

	psy_table_dispose_all(&self->container, (psy_fp_disposefunc)link_dispose);
}

void links_add(Links* self, const Link* link)
{
	assert(self);

	psy_table_insert(&self->container, psy_table_size(&self->container),
		link_clone(link));
}

const Link* links_at(const Links* self, uintptr_t index)
{
	assert(self);

	return (const Link*)psy_table_at_const(&self->container, index);
}


/* StartScript */

/* prototypes */
int startscript_add_menu(lua_State*);
int startscript_replace_menu(lua_State*);
int startscript_add_extension(lua_State*);
int startscript_terminal_output(lua_State*);
int startscript_cursor_test(lua_State*);
int startscript_alert(lua_State*);
int startscript_confirm(lua_State*);
/* implementation */
void startscript_init(StartScript* self, MainFrame* mainframe)
{
	assert(self);
	
	psyclescript_init(&self->script, NULL);
	self->mainframe = mainframe;
}

void startscript_dispose(StartScript* self)
{
	assert(self);

	psyclescript_dispose(&self->script);
}

void startscript_prepare(StartScript* self)
{
  static const luaL_Reg methods[] = {    
    {"addmenu", startscript_add_menu},
    {"replacemenu", startscript_replace_menu},
    {"addextension", startscript_add_extension},
    {"output", startscript_terminal_output},
    {"cursortest", startscript_cursor_test},
    {"alert", startscript_alert},
    {"confirm", startscript_confirm},	
    {NULL, NULL}
  };
  psyclescript_preparestate(&self->script, methods, self);  
  lua_getglobal(self->script.L, "psycle"); 
  lua_pushinteger(self->script.L, CHILDVIEWPORT);
  lua_setfield(self->script.L, -2, "CHILDVIEWPORT");
  lua_pushinteger(self->script.L, FRAMEVIEWPORT);
  lua_setfield(self->script.L, -2, "FRAMEVIEWPORT");
  lua_pushinteger(self->script.L, TOOLBARVIEWPORT);
  lua_setfield(self->script.L, -2, "TOOLBARVIEWPORT");
  lua_pushinteger(self->script.L, MDI);
  lua_setfield(self->script.L, -2, "MDI");
  lua_pushinteger(self->script.L, SDI);
  lua_setfield(self->script.L, -2, "SDI");
  lua_pushinteger(self->script.L, CREATELAZY);
  lua_setfield(self->script.L, -2, "CREATELAZY");
  lua_pushinteger(self->script.L, CREATEATSTART);
  lua_setfield(self->script.L, -2, "CREATEATSTART");
  lua_pop(self->script.L, 1);
}

void startscript_run(StartScript* self)
{
	char path[4096];
	int err;

	psy_snprintf(path, 4096, "%s/%s", 
		psy_configuration_value_str(
			psycleconfig_directories(&self->mainframe->workspace_.cfg_),
			"plugins.luascripts", PSYCLE_LUASCRIPTS_DEFAULT_DIR),
		"start.lua");
	if ((err = psyclescript_load(&self->script, path))) {
		return;	
	}
	startscript_prepare(self);
	if ((err = psyclescript_run(&self->script))) {
		return;	
	}	
}

int startscript_add_menu(lua_State* L)
{  
	StartScript* self;
	Link link;
	const char* name;

	self = (StartScript*)psyclescript_host(L);
	name = luaL_checkstring(L, 1);
	/* parse link table */
	lua_getfield(L, 2, "plugin");
	lua_getfield(L, 2, "label");    
	lua_getfield(L, 2, "viewport");
	lua_getfield(L, 2, "userinterface");  
		
	link_init_all(&link,
		luaL_checkstring(L, -4),
		luaL_checkstring(L, -3),
		(int)luaL_checkinteger(L, -2),
		(int)luaL_checkinteger(L, -1));
	mainviews_add_link(&self->mainframe->main_views_, &link);	
	link_dispose(&link);
	return 0;
}

int startscript_replace_menu(lua_State* L)
{
  return 0;
}

int startscript_add_extension(lua_State* L)
{
  return 0;
}

int startscript_terminal_output(lua_State* L)
{
	return 0;
}

int startscript_cursor_test(lua_State* L)
{
	return 0;
}

int startscript_alert(lua_State* L)
{
	return 0;
}

int startscript_confirm(lua_State* L)
{
	return 0;
}
