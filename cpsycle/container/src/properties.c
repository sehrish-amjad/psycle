/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2023 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "properties.h"
/* local */
#include "list.h"
#include "hashtbl.h"
#include "qsort.h"
/* std */
#include <stddef.h>
/* platform */
#include "../../detail/portable.h"

typedef struct {
	void* target;
	psy_PropertyCallback callback;
	uintptr_t level;
} PropertiesCallbackContext;

static int properties_enumerate_rec(psy_Property*, PropertiesCallbackContext*);
static int psy_property_onsearchenum(psy_Property*, psy_Property*, uintptr_t level);
static int psy_property_onsearchpropertyenum(psy_Property*, psy_Property*,
	uintptr_t level);

/* psy_PropertyItem */

/* implementation */
void psy_propertyitem_init(psy_PropertyItem* self)
{
	assert(self);

	self->key = NULL;
	self->text = NULL;
	self->shorttext = NULL;	
	self->comment = NULL;
	self->typ = PSY_PROPERTY_TYPE_INTEGER;
	self->value.i = 0;
	self->min = 0;
	self->max = 0;
	self->hint = PSY_PROPERTY_HINT_NONE;
	self->hide = FALSE;
	self->disposechildren = TRUE;
	self->save = TRUE;
	self->allow_append = FALSE;
	self->readonly = FALSE;
	self->translate = TRUE;
	self->id = -1;
	self->icon_dark_id = psy_INDEX_INVALID;
	self->icon_light_id = psy_INDEX_INVALID;
	self->marked = FALSE;
	self->foldable = FALSE;
	self->folded_at_start = FALSE;
	self->set = TRUE;
}

void psy_propertyitem_dispose(psy_PropertyItem* self)
{
	assert(self);	

	free(self->key);
	self->key = NULL;
	free(self->text);
	self->text = NULL;
	free(self->shorttext);
	self->shorttext = NULL;
	free(self->comment);
	self->comment = NULL;
	if (self->typ == PSY_PROPERTY_TYPE_STRING ||
		self->typ == PSY_PROPERTY_TYPE_FONT) {
		free(self->value.s);
		self->value.s = NULL;
	}
}

void psy_propertyitem_copy(psy_PropertyItem* self, const psy_PropertyItem*
	source)
{
	assert(self);

	if (self == source) {
		return;
	}	
	psy_strreset(&self->key, source->key);
	psy_strreset(&self->text, source->text);
	psy_strreset(&self->shorttext, source->shorttext);
	psy_strreset(&self->comment, source->comment);
	if (source->typ == PSY_PROPERTY_TYPE_STRING ||
			source->typ == PSY_PROPERTY_TYPE_FONT) {
		psy_strreset(&self->value.s, source->value.s);
	} else {
		self->value = source->value;
	}
	self->min = source->min;
	self->max = source->max;
	self->typ = source->typ;
	self->hint = source->hint;
	self->hide = source->hide;
	self->disposechildren = source->disposechildren;
	self->save = source->save;
	self->id = source->id;
	self->icon_dark_id = source->icon_dark_id;
	self->icon_light_id = source->icon_light_id;
	self->allow_append = source->allow_append;
	self->readonly = source->readonly;
	self->save = source->save;
	self->marked = source->marked;
	self->foldable = source->foldable;
	self->folded_at_start = source->folded_at_start;
	self->set = source->set;
}


/* psy_Property */

/* prototypes */
static psy_Property* psy_property_create_string(const char* key,
	const char* value);
static psy_Property* psy_property_create_font(const char* key,
	const char* value);
static psy_Property* psy_property_create_int(const char* key, intptr_t value,
	intptr_t min, intptr_t max);
static psy_Property* psy_property_create_bool(const char* key, bool value);
static psy_Property* psy_property_create_choice(const char* key, intptr_t value);
static intptr_t psy_property_comp_key(psy_Property* p, psy_Property* q);

static const char* searchkey;
static psy_PropertyType searchtyp;
static psy_Property* keyfound;
static psy_Property* searchproperty;

/* implementation */
void psy_property_init(psy_Property* self)
{
	assert(self);

	psy_property_init_type(self, "root", PSY_PROPERTY_TYPE_ROOT);
}

void psy_property_init_key(psy_Property* self, const char* key)
{
	assert(self);

	psy_property_init_type(self,
		(key)
			? key
			: "root",
		PSY_PROPERTY_TYPE_ROOT);
}

void psy_property_init_type(psy_Property* self, const char* key,
	psy_PropertyType typ)
{
	assert(self);

	psy_signal_init(&self->changed);
	psy_signal_init(&self->rebuild);
	psy_signal_init(&self->scrollto);
	psy_signal_init(&self->before_destroyed);	
	psy_propertyitem_init(&self->item);	
	psy_strreset(&self->item.key, key);	
	self->item.typ = typ;
	self->item.hint = PSY_PROPERTY_HINT_EDIT;
	self->children = NULL;
	self->dispose = NULL;
	self->parent = NULL;
}

void psy_property_dispose(psy_Property* self)
{
	assert(self);

	psy_signal_emit(&self->before_destroyed, self, 0);
	if (self->item.disposechildren) {
		psy_list_deallocate(&self->children, (psy_fp_disposefunc)
			psy_property_dispose);
	}
	psy_propertyitem_dispose(&self->item);
	psy_signal_dispose(&self->before_destroyed);
	psy_signal_dispose(&self->rebuild);
	psy_signal_dispose(&self->changed);
	psy_signal_dispose(&self->scrollto);
}

psy_Property* psy_property_allocinit_key(const char* key)
{
	psy_Property* rv;

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		psy_property_init_key(rv, key);
	}
	return rv;
}

void psy_property_deallocate(psy_Property* self)
{
	if (self) {		
		psy_property_dispose(self);
		free(self);
	}
}

psy_Property* psy_property_clone(const psy_Property* self)
{
	psy_Property* rv;
	
	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		rv->parent = NULL;
		rv->children = NULL;
		rv->dispose = NULL;
		psy_signal_init(&rv->before_destroyed);
		psy_signal_init(&rv->changed);
		psy_signal_init(&rv->rebuild);
		psy_signal_init(&rv->scrollto);
		psy_propertyitem_init(&rv->item);
		psy_propertyitem_copy(&rv->item, &self->item);
		if (self->children) {
			psy_List* p;

			for (p = self->children; p != NULL; psy_list_next(&p)) {
				psy_Property* q;
				psy_Property* src;

				src = (psy_Property*)p->entry;
				q = psy_property_clone(src);
				psy_list_append(&rv->children, q);
				q->parent = rv;
			}
		}
	}
	return rv;
}

psy_Property* psy_property_sync(psy_Property* self, const psy_Property* source)
{
	assert(self);

	if (self != source) {
		psy_List* i;

		for (i = psy_property_begin((psy_Property*)source); i != NULL;
			psy_list_next(&i)) {
			psy_Property* q;
			const psy_Property* p;

			p = (const psy_Property*)psy_list_entry(i);
			q = psy_property_at(self, psy_property_key(p),
					PSY_PROPERTY_TYPE_NONE);			
			if (q && !psy_property_readonly(q)) {
				if (psy_property_type(p) == PSY_PROPERTY_TYPE_STRING) {
					psy_property_set_str(self, psy_property_key(p),
						psy_property_item_str(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_INTEGER) {
					psy_property_set_int(self, psy_property_key(p),
						psy_property_item_int(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_CHOICE) {
						psy_property_set_choice(self, psy_property_key(p),
							psy_property_item_int(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_BOOL) {
					psy_property_set_bool(self, psy_property_key(p),
						psy_property_item_bool(p));
				} else if (psy_property_type(p) == PSY_PROPERTY_TYPE_FONT) {
					psy_property_set_font(self, psy_property_key(p),
						psy_property_item_str(p));
				}
				if (!psy_property_empty(q) && !psy_property_empty(p)) {
					psy_property_sync(q, p);
				}
			}
		}
	}
	return self;
}

uintptr_t psy_property_size(const psy_Property* self)
{
	assert(self);

	return psy_list_size(self->children);
}

bool psy_property_empty(const psy_Property* self)
{
	assert(self);

	return self->children == NULL;
}

psy_List* psy_property_begin(psy_Property* self)
{
	assert(self);

	return self->children;
}

const psy_List* psy_property_begin_const(const psy_Property* self)
{
	assert(self);

	return self->children;
}

psy_Property* psy_property_first(psy_Property* self)
{
	assert(self);

	if (!psy_property_empty(self)) {
		return (psy_Property*)psy_list_entry(self->children);
	}
	return NULL;
}

const psy_Property* psy_property_first_const(const psy_Property* self)
{
	return psy_property_first((psy_Property*)self);
}

psy_Property* psy_property_parent(psy_Property* self)
{
	assert(self);

	return self->parent;
}

const psy_Property* psy_property_parent_const(const psy_Property* self)
{
	assert(self);

	return self->parent;
}

psy_Property* psy_property_parent_level(psy_Property* self, uintptr_t level)
{
	psy_Property* rv;
	psy_Property* p;
	psy_List* q;
	psy_List* r;

	assert(self);

	rv = NULL;
	q = NULL;
	p = self;
	while (psy_property_parent(p)) {
		p = psy_property_parent(p);
		psy_list_append(&q, p);
	}
	if (level < psy_list_size(q)) {
		r = psy_list_at(q, psy_list_size(q) - level - 1);
		if (r) {
			rv = (psy_Property*)r->entry;
		}
	}
	psy_list_free(q);
	return rv;
}

psy_Property* psy_property_remove(psy_Property* self, psy_Property* property)
{
	psy_List* q = 0;
	psy_List* p;

	assert(self);

	p = self->children;
	while (p != NULL) {
		if (p->entry == property) {
			psy_property_deallocate(p->entry);
			q = psy_list_remove(&self->children, p);
			break;
		}
		p = p->next;
	}
	if (q) {
		return q->entry;
	}
	return NULL;
}

psy_Property* psy_property_moveup(psy_Property* self)
{
	if (self->parent) {
		psy_Property* parent;
		psy_List* node;

		parent = self->parent;
		node = psy_list_find_entry(parent->children, self);
		if (node && node->prev) {
			psy_List* prev;

			prev = node->prev->prev;
			psy_list_remove(&parent->children, node);
			psy_list_insert(&parent->children, prev, self);
		}
	}
	return self;
}

psy_Property* psy_property_movedown(psy_Property* self)
{
	if (self->parent) {
		psy_Property* parent;
		psy_List* node;

		parent = self->parent;
		node = psy_list_find_entry(parent->children, self);
		if (node && node->next) {
			psy_List* next;

			next = node->next;
			psy_list_remove(&parent->children, node);
			psy_list_insert(&parent->children, next, self);
		}
	}
	return self;
}

psy_Property* psy_property_prev(psy_Property* self)
{
	psy_Property* parent;

	assert(self);

	parent = self->parent;
	if (parent && parent->children) {
		psy_List* node;

		node = psy_list_find_entry(parent->children, self);
		if (node) {			
			node = node->prev;			
			if (node) {
				return (psy_Property*)psy_list_entry(node);
			}
		}
	}
	return NULL;
}

psy_Property* psy_property_next(psy_Property* self)
{
	psy_Property* parent;

	assert(self);

	parent = self->parent;
	if (parent && parent->children) {
		psy_List* node;

		node = psy_list_find_entry(parent->children, self);
		if (node) {
			node = node->next;
			if (node) {
				return (psy_Property*)psy_list_entry(node);
			}
		}
	}
	return NULL;
}

void psy_property_clear(psy_Property* self)
{
	psy_List* p;

	assert(self);

	for (p = self->children; p != NULL; psy_list_next(&p)) {
		psy_property_deallocate((psy_Property*)psy_list_entry(p));
	}
	psy_list_free(self->children);
	self->children = NULL;
}

bool psy_property_in_section(const psy_Property* self, psy_Property* section)
{
	assert(self);

	if (section) {
		keyfound = 0;
		searchproperty = (psy_Property*)self;
		psy_property_enumerate(section, section,
			(psy_PropertyCallback)psy_property_onsearchpropertyenum);
		return keyfound != FALSE;
	}
	return FALSE;
}

void psy_property_enumerate(psy_Property* self, void* target,
	psy_PropertyCallback callback)
{
	PropertiesCallbackContext context;

	assert(self);

	context.target = target;
	context.callback = callback;
	context.level = 0;
	properties_enumerate_rec(self, &context);
}

int properties_enumerate_rec(psy_Property* self,
	PropertiesCallbackContext* context)
{
	psy_List* p;

	assert(self);

	p = self->children;
	while (p != NULL) {
		int walkoption = context->callback(context->target, p->entry,
			context->level);
		if (walkoption == 0) {
			return 0;
		} else if (walkoption == 1) {
			if (((psy_Property*)(p->entry))->children) {
				++context->level;
				if (!properties_enumerate_rec(p->entry, context)) {
					if (context->level > 0) {
						--context->level;
					}
					return 0;
				}
				if (context->level > 0) {
					--context->level;
				}
			}
		}
		p = p->next;
	}
	return 1;
}

psy_Property* psy_property_find(psy_Property* self, const char* key,
	psy_PropertyType typ)
{
	assert(self);

	searchtyp = typ;
	searchkey = key;
	keyfound = 0;
	if (psy_strlen(key) > 0) {
		psy_property_enumerate(self, self, (psy_PropertyCallback)
			psy_property_onsearchenum);
	}
	return keyfound;
}

const psy_Property* psy_property_find_const(const psy_Property* self,
	const char* key, psy_PropertyType typ)
{
	assert(self);

	return psy_property_find((psy_Property*)self, key, typ);
}

int psy_property_onsearchenum(psy_Property* self, psy_Property* property, uintptr_t level)
{
	assert(self);

	if (property->item.key &&
		(searchtyp == PSY_PROPERTY_TYPE_NONE ||
			property->item.typ == searchtyp) &&
		strcmp(property->item.key, searchkey) == 0) {
		keyfound = property;
		return 0;
	}
	return 1;
}

int psy_property_onsearchpropertyenum(psy_Property* self, psy_Property* property,
	uintptr_t level)
{
	assert(self);

	if (property == searchproperty) {
		keyfound = property;
		return 0;
	}
	return 1;
}

psy_Property* psy_property_findsection(psy_Property* self, const char* key)
{
	psy_Property* prev = 0;

	return psy_property_findsectionex(self, key, &prev);
}

const psy_Property* psy_property_findsection_const(const psy_Property* self , const char* key)
{
	return psy_property_findsection((psy_Property*)self, key);
}

psy_Property* psy_property_findsectionex(psy_Property* self, const char* key,
	psy_Property** prev)
{
	psy_Property* p;
	char* text;
	char seps[] = " .";
	char* token;

	assert(self);

	p = self;
	*prev = p;
	text = psy_strdup(key);
	token = strtok(text, seps);
	while (token != 0) {
		psy_Property* q;
		
		q = psy_property_find(p, token, PSY_PROPERTY_TYPE_SECTION);
		if (!q) {
			q = psy_property_find(p, token, PSY_PROPERTY_TYPE_CHOICE);
			if (!q) {
				p = NULL;
				break;
			}
		}
		p = q;
		*prev = p;
		token = strtok(0, seps);
	}
	free(text);
	return p;
}

char_dyn_t* psy_property_sections(const psy_Property* self)
{
	char_dyn_t* rv;
	const psy_Property* p;
	psy_List* tokens;
	psy_List* q;
	uintptr_t size;

	assert(self);

	p = self;
	tokens = NULL;
	size = 1;
	while (p != NULL) {
		if (psy_property_type(p) == PSY_PROPERTY_TYPE_SECTION) {
			psy_list_insert(&tokens, NULL, (void*)psy_property_key(p));
			size += psy_strlen(psy_property_key(p));
			if (p->parent) {
				++size;
			}
		}
		p = psy_property_parent_const(p);
	}
	rv = (char_dyn_t*)malloc(size);
	*rv = '\0';
	for (q = tokens; q != NULL; psy_list_next(&q)) {
		if (q->prev != NULL) {
			strcat(rv, ".");
		}
		strcat(rv, (char*)(q->entry));
	}
	psy_list_free(tokens);
	return rv;
}

char_dyn_t* psy_property_full_key(const psy_Property* self)
{
	char_dyn_t* rv;
	
	assert(self);

	rv = psy_property_sections(self);
	if (psy_strlen(rv) > 0) {
		rv = psy_strcat_realloc(rv, ".");
		rv = psy_strcat_realloc(rv, psy_property_key(self));
	} else {
		rv = psy_strcat_realloc(rv, psy_property_key(self));
	}
	return rv;
}

void psy_property_sort_keys(psy_Property* self)
{	
	if (self) {
		uintptr_t i;
		uintptr_t num;
		psy_List* p;
		psy_Table propertiesptr;

		num = psy_property_size(self);
		psy_table_init(&propertiesptr);				
		p = psy_property_begin(self);
		for (i = 0; p != NULL && i < num; p = p->next, ++i) {
			psy_table_insert(&propertiesptr, i, (psy_Property*)psy_list_entry(p));
		}
		psy_qsort(&propertiesptr, 
			(psy_fp_set_index_double)psy_table_insert,
			(psy_fp_index_double)psy_table_at,
			0, (int)(num - 1), (psy_fp_comp)psy_property_comp_key);
		p = psy_property_begin(self);
		for (i = 0; p != NULL && i < num; p = p->next, ++i) {
			p->entry = psy_table_at(&propertiesptr, i);
		}		
		psy_table_dispose(&propertiesptr);
	}
}

void psy_property_sort_keys_recursive(psy_Property* self)
{	
	psy_List* p;

	assert(self);

	psy_property_sort_keys(self);	
	for (p = psy_property_begin(self); p != NULL; psy_list_next(&p)) {
		psy_property_sort_keys((psy_Property*)p->entry);				
	}		
}

intptr_t psy_property_comp_key(psy_Property* p, psy_Property* q)
{
	const char* left;
	const char* right;

	left = psy_property_key(p);	
	right = psy_property_key(q);	
	return strcmp(left, right);
}


psy_Property* psy_property_at(psy_Property* self, const char* key,
	psy_PropertyType type)
{
	psy_List* p = NULL;
	char* c;

	assert(self);

	if (!key) {
		return NULL;
	}
	c = strrchr(key, '.');
	if (!c) {
		p = self->children;
	} else {
		char* path;
		ptrdiff_t count;

		count = c - key;
		path = malloc(count + 1);
		if (path) {
			psy_Property* q;

			strncpy(path, key, count);
			path[count] = '\0';
			key = c + 1;
			q = psy_property_findsection(self, path);
			if (q) {
				p = q->children;
			}
			free(path);
		}
	}
	while (p != NULL) {
		psy_Property* property;
		const char* item_key;

		property = (psy_Property*)psy_list_entry(p);
		item_key = psy_property_key(property);
		assert(key);
		if (((type == PSY_PROPERTY_TYPE_NONE) || (psy_property_type(property) == type)) &&
			(strcmp(key, item_key) == 0)) {
			break;
		}
		psy_list_next(&p);
	}
	return (p)
		? (psy_Property*)psy_list_entry(p)
		: NULL;
}

static psy_Property* psy_property_binsearch(psy_Property*,
	const char* str, uintptr_t low, uintptr_t up);

psy_Property* psy_property_at_sorted(psy_Property* self, const char* key,
	psy_PropertyType type)
{
	psy_List* p = NULL;
	char* c;
	psy_Property* found;
	psy_Property* property;

	assert(self);

	if (!key) {
		return NULL;
	}
	c = strrchr(key, '.');
	property = self;
	if (c) {		
		char* path;
		ptrdiff_t count;

		count = c - key;
		path = malloc(count + 1);
		if (path) {
			strncpy(path, key, count);
			path[count] = '\0';
			key = c + 1;
			property = psy_property_findsection(self, path);
			free(path);
		}
	}	
	if (property) {		
		found = psy_property_binsearch(property, key, 0, psy_list_size(property->children) - 1);
		if (found) {
			if (((type == PSY_PROPERTY_TYPE_NONE) || (found->item.typ == type))) {
				return found;
			}
		}
	}
	return NULL;
}

psy_Property* psy_property_binsearch(psy_Property* self, const char* str,
	uintptr_t low, uintptr_t up)
{
	psy_List* p;
	const char* midstr;
	uintptr_t mid;
	int comp;

	if (up < low) {
		return NULL;
	}
	mid = (low + up) / 2;
	p = psy_list_at(self->children, mid);
	if (!p) {
		return NULL;
	}
	midstr = (const char*)psy_property_key((p->entry));
	if (!midstr) {
		return NULL;
	}
	comp = strcmp(str, midstr);
	if (comp == 0) {
		return (psy_Property*)p->entry;
	}
	if (comp < 0) {
		return psy_property_binsearch(self, str, low, mid - 1);
	} else {
		return psy_property_binsearch(self, str, mid + 1, up);
	}	
}

const psy_Property* psy_property_at_const(const psy_Property* self,
	const char* key, psy_PropertyType typ)
{
	return psy_property_at((psy_Property*)self, key, typ);
}

psy_Property* psy_property_at_section(psy_Property* self, const char* key)
{
	return psy_property_at(self, key, PSY_PROPERTY_TYPE_SECTION);
}

const psy_Property* psy_property_at_section_const(const psy_Property* self,
	const char* key)
{
	return psy_property_at((psy_Property*)self, key,
		PSY_PROPERTY_TYPE_SECTION);
}

psy_Property* psy_property_at_index(psy_Property* self, intptr_t index)
{
	assert(self);

	if (index >= 0) {
		psy_List* p;

		p = psy_list_at(self->children, index);
		if (p) {
			return (psy_Property*)psy_list_entry(p);
		}
	}
	return NULL;
}

const psy_Property* psy_property_at_index_const(const psy_Property* self,
	intptr_t index)
{
	return psy_property_at_index((psy_Property*)self, index);
}

uintptr_t psy_property_index(const psy_Property* self)
{
	assert(self);

	if (psy_property_parent_const(self)) {
		return psy_list_entry_index(self->parent->children, self);
	}
	return psy_INDEX_INVALID;
}

psy_Property* psy_property_set_bool(psy_Property* self, const char* key,
	bool value)
{
	psy_Property* rv;

	assert(self);

	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_BOOL);
	if (rv) {		
		psy_property_set_item_bool(rv, value);
	} else {
		rv = psy_property_append_int(self, key, value, 0, 0);
	}
	return rv;
}

bool psy_property_at_bool(const psy_Property* self, const char* key,
	bool defaultvalue)
{
	const psy_Property* property;

	assert(self);

	property = psy_property_at_const(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property && (property->item.typ == PSY_PROPERTY_TYPE_BOOL ||
			property->item.typ == PSY_PROPERTY_TYPE_INTEGER)) {
		return property->item.value.i != FALSE;
	}
	return defaultvalue;
}

psy_Property* psy_property_set_int(psy_Property* self, const char* key,
	intptr_t value)
{
	psy_Property* rv;

	assert(self);

	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_INTEGER);
	if (rv) {
		psy_property_set_item_int(rv, value);
	} else {
		rv = psy_property_append_int(self, key, value, 0, 0);
	}
	return rv;
}

intptr_t psy_property_at_int(const psy_Property* self, const char* key, intptr_t defaultvalue)
{
	const psy_Property* property;

	assert(self);

	property = psy_property_at_const(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property && (property->item.typ == PSY_PROPERTY_TYPE_INTEGER ||
		property->item.typ == PSY_PROPERTY_TYPE_CHOICE)) {
		return property->item.value.i;
	}
	return defaultvalue;
}

uint32_t psy_property_at_colour(const psy_Property* self, const char* key, uint32_t defaultvalue)
{
	const psy_Property* property;

	assert(self);

	property = psy_property_at_const(self, key, PSY_PROPERTY_TYPE_NONE);
	if (property && (property->item.typ == PSY_PROPERTY_TYPE_INTEGER ||
			property->item.typ == PSY_PROPERTY_TYPE_CHOICE)) {
		return (uint32_t)property->item.value.i;
	}
	return defaultvalue;
}

bool psy_property_int_valid(const psy_Property* self, intptr_t value)
{
	assert(self);

	if (psy_property_int_has_range(self)) {
		return (value >= self->item.min) && (value <= self->item.max);
	}
	return TRUE;
}

bool psy_property_int_has_range(const psy_Property* self)
{
	return !(self->item.min == 0 && self->item.max == 0);
}

psy_Property* psy_property_set_double(psy_Property* self, const char* key,
	double value)
{
	psy_Property* rv;

	assert(self);

	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_DOUBLE);
	if (rv) {
		psy_property_set_item_double(rv, value);
	} else {
		rv = psy_property_append_double(self, key, value, 0, 0);
	}
	return rv;
}

double psy_property_at_real(const psy_Property* self, const char* key,
	double defaultvalue)
{
	const psy_Property* property;

	assert(self);

	property = psy_property_at_const(self, key, PSY_PROPERTY_TYPE_DOUBLE);
	if (property && property->item.typ == PSY_PROPERTY_TYPE_DOUBLE) {
		return property->item.value.d;
	}
	return defaultvalue;
}

psy_Property* psy_property_set_str(psy_Property* self, const char* key,
	const char* str)
{
	psy_Property* rv;

	assert(self);

	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_STRING);
	if (rv) {
		psy_property_set_item_str(rv, str);
	} else {
		rv = psy_property_append_str(self, key, str);
	}
	return rv;
}

const char* psy_property_at_str(const psy_Property* self, const char* key,
	const char* defaulttext)
{
	const psy_Property* property;

	assert(self);

	property = psy_property_at_const(self, key, PSY_PROPERTY_TYPE_NONE);
	if ((property && (property->item.typ == PSY_PROPERTY_TYPE_STRING)) ||
		(property && (property->item.typ == PSY_PROPERTY_TYPE_FONT))) {
		return property->item.value.s;
	}
	return defaulttext;
}

psy_Property* psy_property_set_font(psy_Property* self, const char* key,
	const char* font)
{
	psy_Property* rv;

	assert(self);

	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_FONT);
	if (rv) {
		psy_property_set_item_font(rv, font);
	} else {
		rv = psy_property_append_font(self, key, font);
	}
	return rv;
}

psy_Property* psy_property_set_choice(psy_Property* self, const char* key,
	intptr_t value)
{
	psy_Property* rv;

	assert(self);
	
	rv = psy_property_at(self, key, PSY_PROPERTY_TYPE_NONE);
	if (rv) {
		if (!psy_property_readonly(rv)) {
			psy_signal_emit(&self->changed, self, 0);			
			rv->item.typ = PSY_PROPERTY_TYPE_CHOICE;
			psy_property_set_item_int(rv, value);
		}
	} else {
		rv = psy_property_append_int(self, key, value, 0, 0);
		rv->item.typ = PSY_PROPERTY_TYPE_CHOICE;
	}
	return rv;
}

psy_Property* psy_property_at_choice(psy_Property* self)
{
	assert(self);

	return psy_property_at_index(self, psy_property_item_int(self));
}

bool psy_property_is_choice_item(const psy_Property* self)
{
	assert(self);

	return self->parent &&
		(self->parent->item.typ == PSY_PROPERTY_TYPE_CHOICE);
}

intptr_t psy_property_choiceitem_index(const psy_Property* self)
{
	const psy_List* p;
	uintptr_t rv;
	const psy_Property* choice;
	
	assert(self);

	choice = psy_property_parent_const(self);
	rv = 0;
	if (choice) {
		
		p = psy_property_begin_const(choice);
		while (p != NULL) {
			if (psy_list_entry_const(p) == self) {
				break;
			}
			++rv;
			p = p->next;
		}
	}
	return rv;
}

psy_Property* psy_property_append_property(psy_Property* self, psy_Property* property)
{
	assert(self);

	if (property) {
		psy_list_append(&self->children, property);
		if (psy_property_type(property) == PSY_PROPERTY_TYPE_ROOT) {
			property->item.typ = PSY_PROPERTY_TYPE_SECTION;
		}
		property->parent = self;
	}
	return property;
}

psy_Property* psy_property_append_section(psy_Property* self, const char* name)
{
	psy_Property* rv;

	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {

		psy_property_init_type(rv, name, PSY_PROPERTY_TYPE_SECTION);
	}
	return psy_property_append_property(self, rv);
}

psy_Property* psy_property_create_string(const char* key, const char* value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_STRING);
		property->item.value.s = psy_strdup(value);
	}
	return property;
}

psy_Property* psy_property_append_str(psy_Property* self, const char* key,
	const char* value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_string(key, value));
}

psy_Property* psy_property_create_font(const char* key, const char* value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_FONT);
		property->item.value.s = psy_strdup(value);
	}
	return property;
}

psy_Property* psy_property_append_font(psy_Property* self, const char* key,
	const char* value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_font(key, value));
}

psy_Property* psy_property_append_userdata(psy_Property* self, const char* key,
	void* value, void (*dispose)(psy_PropertyItem*))
{
	psy_Property* property;

	assert(self);

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_list_append(&self->children, property);
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_USERDATA);
		property->dispose = dispose;
		property->item.value.ud = value;
	}
	return property;
}

psy_Property* psy_property_create_int(const char* key, intptr_t value,
	intptr_t minval, intptr_t maxval)
{
	psy_Property* property;

	assert(minval <= maxval);
	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_INTEGER);
		property->item.value.i = value;
		property->item.min = minval;
		property->item.max = maxval;
	}
	return property;
}

psy_Property* psy_property_append_action(psy_Property* self, const char* key)
{
	psy_Property* property;

	assert(self);

	property = psy_property_create_int(key, 0, 0, 0);
	if (property) {
		property->item.typ = PSY_PROPERTY_TYPE_ACTION;		
		psy_property_append_property(self, property);
	}
	return property;
}

psy_Property* psy_property_append_int(psy_Property* self, const char* key,
	intptr_t value, intptr_t min, intptr_t max)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_int(key, value, min, max));
}

psy_Property* psy_property_create_bool(const char* key, bool value)
{
	psy_Property* property;

	property = psy_property_create_int(key, value != FALSE, 0, 1);
	if (property) {
		property->item.typ = PSY_PROPERTY_TYPE_BOOL;		
	}
	return property;
}

psy_Property* psy_property_append_bool(psy_Property* self,
	const char* key, bool value)
{
	psy_Property* rv;

	assert(self);

	rv = (psy_Property*)malloc(sizeof(psy_Property));
	if (rv) {
		psy_property_init_type(rv, key, PSY_PROPERTY_TYPE_BOOL);
		rv->item.value.i = (value != FALSE);		
	}
	return psy_property_append_property(self, rv);
}

psy_Property* psy_property_append_double(psy_Property* self, const char* key,
	double value, double min, double max)
{
	psy_Property* property;

	assert(self);

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_DOUBLE);
		property->item.value.d = value;
	}
	return psy_property_append_property(self, property);
}

psy_Property* psy_property_create_choice(const char* key, intptr_t value)
{
	psy_Property* property;

	property = (psy_Property*)malloc(sizeof(psy_Property));
	if (property) {
		psy_property_init_type(property, key, PSY_PROPERTY_TYPE_CHOICE);
		property->item.value.i = value;
		property->item.hint = PSY_PROPERTY_HINT_LIST;
	}
	return property;
}

psy_Property* psy_property_append_choice(psy_Property* self, const char* key,
	intptr_t value)
{
	assert(self);

	return psy_property_append_property(self,
		psy_property_create_choice(key, value));
}

char* pathend(const char* path, char* section, char* key)
{
	char* p;

	p = strrchr(path, '.');
	return p;
}

void psy_property_change_key(psy_Property* self, const char* key)
{
	assert(self);

	psy_strreset(&self->item.key, key);
}

const char* psy_property_key(const psy_Property* self)
{
	assert(self);

	return self->item.key;
}

psy_PropertyType psy_property_type(const psy_Property* self)
{
	assert(self);

	return self->item.typ;
}

psy_Property* psy_property_setreadonly(psy_Property* self, bool on)
{
	assert(self);

	self->item.readonly = on;
	return self;
}

bool psy_property_readonly(const psy_Property* self)
{
	assert(self);

	return self->item.readonly;
}

psy_Property* psy_property_set_hint(psy_Property* self, psy_PropertyHint hint)
{
	assert(self);

	self->item.hint = hint;
	return self;
}

psy_PropertyHint psy_property_hint(const psy_Property* self)
{
	assert(self);

	return self->item.hint;
}

psy_Property* psy_property_prevent_save(psy_Property* self)
{
	assert(self);

	self->item.save = FALSE;
	return self;
}

psy_Property* psy_property_enablesave(psy_Property* self)
{
	assert(self);

	self->item.save = TRUE;
	return self;
}

psy_Property* psy_property_enableappend(psy_Property* self)
{
	assert(self);

	self->item.allow_append = TRUE;
	return self;
}

psy_Property* psy_property_show(psy_Property* self)
{
	assert(self);

	self->item.hide = FALSE;
	return self;
}

psy_Property* psy_property_hide(psy_Property* self)
{
	assert(self);

	self->item.hide = TRUE;
	return self;
}

bool psy_property_hidden(const psy_Property* self)
{
	assert(self);

	return (self->item.hide != FALSE);
}

psy_Property* psy_property_enable_foldable(psy_Property* self,
	bool folded_at_start)
{
	assert(self);

	self->item.foldable = TRUE;
	self->item.folded_at_start = TRUE;
	return self;
}

bool psy_property_foldable(const psy_Property* self)
{
	assert(self);

	return (self->item.foldable != FALSE);
}

bool psy_property_folded_at_start(const psy_Property* self)
{
	assert(self);

	return (self->item.folded_at_start != FALSE);
}

psy_Property* psy_property_preventtranslate(psy_Property* self)
{
	assert(self);

	self->item.translate = FALSE;
	return self;
}

bool psy_property_translation_prevented(const psy_Property* self)
{
	assert(self);

	return !self->item.translate;
}

bool psy_property_hasid(const psy_Property* self, int id)
{
	assert(self);

	return (self->item.id == id);
}

psy_Property* psy_property_connect(psy_Property* self, void* context, void* fp)
{
	assert(self);

	psy_signal_connect(&self->changed, context, fp);
	return self;
}

void psy_property_disconnect(psy_Property* self, void* context)
{
	assert(self);
		
	psy_signal_disconnect_context(&self->changed, context);
	psy_signal_disconnect_context(&self->rebuild, context);
	psy_signal_disconnect_context(&self->before_destroyed, context);
}

psy_Property* psy_property_connect_children(psy_Property* self, int recursive,
	void* context, void* fp)
{
	psy_List* p;

	assert(self);

	for (p = self->children; p != NULL; p = p->next) {
		psy_Property* curr;

		curr = (psy_Property*)p->entry;
		psy_property_connect(curr, context, fp);		
		if (recursive &&
			psy_property_type(curr) != PSY_PROPERTY_TYPE_ACTION) {
			psy_property_connect_children(curr, recursive, context, fp);
		}
	}
	return self;
}

void psy_property_notify(psy_Property* self)
{
	assert(self);
	
	psy_signal_emit(&self->changed, self, 0);
}

void psy_property_notify_all(psy_Property* self)
{
	psy_List* p;

	assert(self);

	if (psy_property_type(self) != PSY_PROPERTY_TYPE_ACTION) {
		psy_signal_emit(&self->changed, self, 0);
	}
	for (p = self->children; p != NULL; p = p->next) {
		psy_Property* curr;

		curr = (psy_Property*)p->entry;
		psy_property_notify_all(curr);		
	}
}

void psy_property_rebuild(psy_Property* self)
{
	assert(self);

	psy_signal_emit(&self->rebuild, self, 0);
}

void psy_property_scrollto(psy_Property* self)
{
	assert(self);

	psy_signal_emit(&self->scrollto, self, 0);
}

psy_Property* psy_property_set_item_bool(psy_Property* self, bool value)
{
	assert(self);

	if (!self->item.readonly && self->item.value.i != value) {
		self->item.value.i = value != FALSE;
		psy_signal_emit(&self->changed, self, 0);
	}
	return self;
}

bool psy_property_item_bool(const psy_Property* self)
{
	assert(self);

	return self->item.value.i != FALSE;
}

psy_Property* psy_property_set_item_int(psy_Property* self, intptr_t value)
{
	assert(self);

	if (!self->item.readonly && psy_property_int_valid(self, value)) {
		intptr_t oldvalue;

		oldvalue = self->item.value.i;
		self->item.value.i = value;
		if (oldvalue != value) {
			psy_signal_emit(&self->changed, self, 0);
		}
		if (oldvalue != value && psy_property_type(self) ==
				PSY_PROPERTY_TYPE_CHOICE) {
			psy_Property* item;
			
			item = psy_property_at_index(self, value);
			if (item) {
				psy_signal_emit(&item->changed, item, 0);
			}
			item = psy_property_at_index(self, oldvalue);
			if (item) {
				psy_signal_emit(&item->changed, item, 0);
			}			
		}
	}	
	return self;
}

bool psy_property_is_hex(const psy_Property* self)
{
	assert(self);

	return psy_property_is_int(self) &&
		(psy_property_hint(self) == PSY_PROPERTY_HINT_EDITHEX ||
		 psy_property_hint(self) == PSY_PROPERTY_HINT_EDITCOLOR);
}

bool psy_property_is_bool(const psy_Property* self)
{
	assert(self);

	return psy_property_type(self) == PSY_PROPERTY_TYPE_BOOL;
}

bool psy_property_is_int(const psy_Property* self)
{
	assert(self);

	return psy_property_type(self) == PSY_PROPERTY_TYPE_INTEGER;
}

bool psy_property_is_choice(const psy_Property* self)
{
	assert(self);

	return psy_property_type(self) == PSY_PROPERTY_TYPE_CHOICE;
}

bool psy_property_is_double(const psy_Property* self)
{
	assert(self);

	return psy_property_type(self) == PSY_PROPERTY_TYPE_DOUBLE;
}

bool psy_property_is_string(const psy_Property* self)
{
	return psy_property_type(self) == PSY_PROPERTY_TYPE_STRING;
}

bool psy_property_is_font(const psy_Property* self)
{
	return psy_property_type(self) == PSY_PROPERTY_TYPE_FONT;
}

bool psy_property_is_action(const psy_Property* self)
{
	return psy_property_type(self) == PSY_PROPERTY_TYPE_ACTION;
}

bool psy_property_is_section(const psy_Property* self)
{
	assert(self);

	return psy_property_type(self) == PSY_PROPERTY_TYPE_SECTION;
}

bool psy_property_is_set(const psy_Property* self)
{
	assert(self);
	
	return self->item.set;
}

intptr_t psy_property_item_int(const psy_Property* self)
{
	assert(self);

	return self->item.value.i;
}

uint32_t psy_property_item_colour(const psy_Property* self)
{
	assert(self);

	return (uint32_t)self->item.value.i;
}

psy_Property* psy_property_set_item_double(psy_Property* self, double value)
{
	assert(self);

	if (!self->item.readonly && self->item.value.d != value) {
		self->item.value.d = value;
		psy_signal_emit(&self->changed, self, 0);
	}
	return self;
}

double psy_property_item_double(const psy_Property* self)
{
	assert(self);

	return self->item.value.d;
}

psy_Property* psy_property_set_item_str(psy_Property* self, const char* str)
{
	assert(self);

	if (!self->item.readonly) {
		if ((self->item.typ == PSY_PROPERTY_TYPE_STRING ||
				self->item.typ == PSY_PROPERTY_TYPE_FONT)) {
			if (self->item.value.s == NULL || str == NULL || strcmp(str, self->item.value.s) != 0) {
				psy_strreset(&self->item.value.s, str);
				psy_signal_emit(&self->changed, self, 0);
			}
		} else {
			self->item.typ = PSY_PROPERTY_TYPE_STRING;
			self->item.value.s = psy_strdup(str);
		}
	}
	return self;
}

const char* psy_property_item_str(const psy_Property* self)
{
	assert(self);

	return (self->item.value.s) ? self->item.value.s : "";
}

psy_Property* psy_property_set_item_font(psy_Property* self, const char* value)
{
	assert(self);

	if (!self->item.readonly) {		
		psy_strreset(&self->item.value.s, value);
		psy_signal_emit(&self->changed, self, 0);		
	}
	return self;
}

const char* psy_property_item_font(const psy_Property* self)
{
	assert(self);

	return (self->item.value.s) ? self->item.value.s : "";
}

psy_Property* psy_property_item_choice_parent(psy_Property* self)
{
	assert(self);

	return (psy_property_is_choice_item(self))
		? psy_property_parent(self)
		: NULL;
}

uintptr_t psy_property_item_min(const psy_Property* self)
{
	assert(self);

	return self->item.min;
}

uintptr_t psy_property_item_max(const psy_Property* self)
{
	assert(self);

	return self->item.max;
}

bool psy_property_has_key(const psy_Property* self, const char* key)
{
	assert(self);

	return strcmp(self->item.key, key) == 0;
}

bool psy_property_has_type(const psy_Property* self, psy_PropertyType type)
{
	assert(self);

	return self->item.typ == type;
}

psy_Property* psy_property_set_id(psy_Property* self, intptr_t id)
{
	assert(self);

	self->item.id = id;
	return self;
}

intptr_t psy_property_id(const psy_Property* self)
{
	assert(self);

	return self->item.id;
}

psy_Property* psy_property_set_text(psy_Property* self, const char* text)
{
	assert(self);

	psy_strreset(&self->item.text, text);	
	return self;
}

const char* psy_property_text(const psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.text) != 0)
		? self->item.text
		: (psy_strlen(self->item.key) != 0)
			? self->item.key
			: "";
}

psy_Property* psy_property_set_short_text(psy_Property* self, const char* text)
{
	assert(self);

	psy_strreset(&self->item.shorttext, text);	
	return self;
}

const char* psy_property_short_text(const psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.shorttext) != 0)
		? self->item.shorttext
		: psy_strlen(self->item.text) != 0
			? self->item.text
			: psy_strlen(self->item.key) != 0
				? self->item.key
				: "";
}

psy_Property* psy_property_set_comment(psy_Property* self, const char* text)
{
	assert(self);

	psy_strreset(&self->item.comment, text);
	return self;
}

const char* psy_property_comment(const psy_Property* self)
{
	assert(self);

	return (psy_strlen(self->item.comment) != 0)
		? self->item.comment
		: "";
}

psy_Property* psy_property_set_icon(psy_Property* self,
	uintptr_t icon_light_id, uintptr_t icon_dark_id)
{
	assert(self);

	self->item.icon_light_id = icon_light_id;
	self->item.icon_dark_id = icon_dark_id;
	return self;
}

void psy_property_trace(const psy_Property* self)
{
	printf("property %s\n", psy_property_key(self));
	if (self->children) {
		const psy_List* p;

		for (p = self->children; p != NULL; psy_list_next_const(&p)) {
			psy_Property* q;

			q = (psy_Property*)p->entry;
			printf("- %s\n", psy_property_key(q));
		}
	}
}
