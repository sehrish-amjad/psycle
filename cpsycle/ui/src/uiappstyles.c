/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiappstyles.h"
/* local*/
#include "uimaterial.h"
/* platform */
#include "../../detail/os.h"


/* prototypes */
static void psy_ui_appstyles_initdarktheme(psy_ui_Styles*, bool keepfont);
static void psy_ui_appstyles_initlighttheme(psy_ui_Styles*, bool keepfont);

/* implementation */
void psy_ui_appstyles_inittheme(psy_ui_Styles* self, psy_ui_ThemeMode theme,
	bool keepfont)
{	
	self->theme_mode = theme;
	switch (theme) {	
	case psy_ui_LIGHTTHEME:
		psy_ui_appstyles_initlighttheme(self, keepfont);
		break;
	default:
		psy_ui_appstyles_initdarktheme(self, keepfont);
		break;
	}	
}

/* DarkTheme */
void psy_ui_appstyles_initdarktheme(psy_ui_Styles* self,
	bool keepfont)
{
	psy_ui_Style* style;
	psy_ui_MaterialTheme material;
	psy_ui_Font oldfont;
	
	psy_ui_materialtheme_init(&material, self->theme_mode);
	/* root */
	if (keepfont) {
		style = psy_ui_styles_at(self, psy_ui_STYLE_ROOT);
		if (style) {
			psy_ui_font_init(&oldfont, NULL);
			psy_ui_font_copy(&oldfont, &style->font);
		} else {
			keepfont = FALSE;
		}
	}
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onsurface, material.accent),
		material.surface);		
	if (keepfont) {
		psy_ui_font_init(&style->font, NULL);
		psy_ui_font_copy(&style->font, &oldfont);
	} else {
#if defined(DIVERSALIS__OS__UNIX)
		psy_ui_style_set_font(style, "FreeSans", 12);
#else
		psy_ui_style_set_font(style, "Tahoma", 16);
#endif	
	}	
	if (keepfont) {
		psy_ui_font_dispose(&oldfont);
	}
	psy_ui_styles_set_style(self, psy_ui_STYLE_ROOT, "root", style);
	/* label */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.accent));
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL, "label", style);
	/* label::disabled */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
	psy_ui_colour_weighted(material.onsurface, material.weak));
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL_DISABLED, "label::disabled", style);
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.secondary);	
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL_SELECT, "label::select", style);
	/* edit */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.accent));
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDIT, "edit", style);
	/* edit::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.accent));
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(6));
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDIT_FOCUS, "edit::focus", style);
	/* button */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON, "button", style);
	/* button::hover */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onsurface, material.accent),
		psy_ui_colour_make_overlay(4));	
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_HOVER, "button::hover", style);
	/* button::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.secondary);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_SELECT, "button::select", style);
	/* button::active */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onsurface, material.strong),
		psy_ui_colour_make_overlay(4));
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_ACTIVE, "button::active", style);
	/* button::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.medium));
	psy_ui_border_init_solid_radius(&style->border, material.secondary, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_FOCUS, "button::focus", style);
	/* combobox */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.medium));
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX, "combobox", style);
	/* combobox::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.strong));
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_HOVER, "combobox::hover", style);
	/* combobox::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.strong));
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_SELECT, "combobox::select", style);
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.medium));
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_TEXT, "combobox::text", style);
	/* tabbar */
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, psy_ui_STYLE_TABBAR, "tabbar", style);
	/* tab */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.primary, material.medium));
	psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);	
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB, "tab", style);
	/* tab::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(6));
	psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_7p, 6.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_HOVER, "tab::hover", style);
	/* tab::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary);
	psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_init_all(&style->border, psy_ui_BORDER_NONE,
		psy_ui_BORDER_NONE, psy_ui_BORDER_SOLID, psy_ui_BORDER_NONE);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_weighted(material.secondary, material.weak));
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_SELECT, "tab::select", style);
	/* tab_label */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.primary, material.weak));
	psy_ui_style_set_padding_em(style, 0.0, 1.0, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_LABEL, "tab::label", style);
	/* horizontal scrollbar */	
	/* scrollpane */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(5));
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLPANE, "hscrollpane", style);
	/* scrollthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(5));	
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLTHUMB, "hscrollthumb", style);
	/* scrollthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onsurface,
		psy_ui_colour_make_overlay(20));
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLTHUMB_HOVER, "hscrollthumb::hover", style);
	/* hscrollbutton */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.0, 1.0));
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLBUTTON, "hscrollbutton", style);
	/* hscrollbutton::hover */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.0, 1.0));
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLBUTTON_HOVER, "hscrollbutton::hover", style);
	/* hscrollbutton::active */	
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_ACTIVE));
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.0, 1.0));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_HSCROLLBUTTON_ACTIVE, "hscrollbutton::active", style);
	/* vertical scrollbar */
	/* scrollpane */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(5));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLPANE, "vscrollpane", style);
	/* vscrollthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(5));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLTHUMB, "vscrollthumb", style);
	/* vscrollthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		material.onsurface, psy_ui_colour_make_overlay(20));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLTHUMB_HOVER, "vscrollthumb::hover", style);		
	/* vscrollbutton */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.3, 1.0));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON, "vscrollbutton", style);
	/* vscrollbutton::hover */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.3, 1.0));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON_HOVER, "vscrollbutton::hover", style);
	/* vscrollbutton::active */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_ACTIVE));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_style_set_size(style, psy_ui_size_make_em(2.3, 1.0));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON_ACTIVE, "vscrollbutton::active", style);
	/* sliderpane */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		psy_ui_colour_make_overlay(5));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.08));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERPANE, "sliderpane", style);
	/* sliderthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		material.secondary, material.secondary);
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERTHUMB, "sliderthumb", style);		
	/* sliderthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary, material.onprimary);
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERTHUMB_HOVER, "sliderthumb::hover", style);
	/* splitter */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.05));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER, "splitter", style);
	/* splitter::hover */
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER_HOVER, "splitter::hover", style);
	/* splitter::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_make_overlay(16));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER_SELECT, "splitter::select", style);
	/* psy_ui_STYLE_PROGRESSBAR */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_make(0xA036A3));
	psy_ui_style_set_size(style, psy_ui_size_make_em(20.0, 1.0));	
	psy_ui_styles_set_style(self, psy_ui_STYLE_PROGRESSBAR, "progressbar", style);
	/* switch */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.pale));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH, "switch", style);
	/* switch::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.pale));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH_HOVER, "switch::hover", style);
	/* switch::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.secondary, material.weak));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH_SELECT, "switch::select", style);
	/* listbox */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onprimary, material.medium),
		psy_ui_colour_make_overlay(2));
	psy_ui_styles_set_style(self, psy_ui_STYLE_LISTBOX, "listbox", style);
	/* checkmark */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onprimary, material.medium),
		psy_ui_colour_make_overlay(8));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_8p, 6.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_CHECKMARK, "checkmark", style);
	/* checkmark::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onprimary, material.medium),
		psy_ui_colour_make_overlay(16));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_8p, 6.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_CHECKMARK_HOVER, "checkmark::hover", style);
	/* checkmark::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onprimary, material.medium),
		psy_ui_colour_weighted(material.secondary, material.weak));
	psy_ui_border_init_solid_radius(&style->border,
		material.surface_overlay_8p, 6.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_CHECKMARK_SELECT, "checkmark::select", style);
	/* listitem */
	style = psy_ui_style_allocinit();
	//psy_ui_style_set_colour(style, material.onprimary_medium);
	psy_ui_styles_set_style(self, psy_ui_STYLE_LIST_ITEM,
		"list_item", style);
	/* listitem::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_make(0xFFFFFF),
		psy_ui_colour_make(0x009B7800));
	psy_ui_styles_set_style(self, psy_ui_STYLE_LIST_ITEM_SELECT,
		"list_item::select", style);	
	/* editor */
	style = psy_ui_style_allocinit();
#if defined(DIVERSALIS__OS__UNIX)
	psy_ui_style_set_font(style, "FreeMono", 12);
#else
	psy_ui_style_set_font(style, "Consolas", 16);
#endif	
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDITOR, "", style);
}

/* LightTheme */
void psy_ui_appstyles_initlighttheme(psy_ui_Styles* self,
	bool keepfont)
{
	psy_ui_Style* style;
	psy_ui_MaterialTheme material;	
	psy_ui_Font oldfont;
	
	self->theme_mode = psy_ui_LIGHTTHEME;
	psy_ui_materialtheme_init(&material, self->theme_mode);	
	/* root */
	if (keepfont) {
		style = psy_ui_styles_at(self, psy_ui_STYLE_ROOT);
		if (style) {
			psy_ui_font_init(&oldfont, NULL);
			psy_ui_font_copy(&oldfont, &style->font);
		} else {
			keepfont = FALSE;
		}
	}
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onsurface, material.surface);
	if (keepfont) {
		psy_ui_font_init(&style->font, NULL);
		psy_ui_font_copy(&style->font, &oldfont);		
	} else {
#if defined(DIVERSALIS__OS__UNIX)
		psy_ui_style_set_font(style, "FreeSans", 12);
#else
		psy_ui_style_set_font(style, "Tahoma", 16);
#endif		
	}
	if (keepfont) {
		psy_ui_font_dispose(&oldfont);
	}
	psy_ui_styles_set_style(self, psy_ui_STYLE_ROOT, "", style);
	/* label */
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL, "", style);
	/* label::disabled */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.weak));
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL_DISABLED, "", style);
	/* label::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.secondary);	
	psy_ui_styles_set_style(self, psy_ui_STYLE_LABEL_SELECT, "", style);
	/* edit */
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDIT, "", style);
	/* edit::focus */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(6));
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDIT_FOCUS, "", style);
	/* button */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON, "", style);
	/* button::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(4));
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_HOVER, "", style);
	/* button::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_blue());
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_SELECT, "", style);
	/* button::active */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		psy_ui_colour_weighted(material.onsurface, material.strong),
		psy_ui_colour_make_overlay(4));
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_ACTIVE, "", style);
	/* button::focus */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onsurface, material.medium));
	psy_ui_border_init_solid_radius(&style->border, material.secondary, 6.0);
	psy_ui_style_set_padding_em(style, 0.25, 1.0, 0.25, 1.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_BUTTON_FOCUS, "", style);
	/* combobox */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.0, 0.5, 0.0, 0.5);
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX, "combobox", style);
	/* combobox::hover */
	style = psy_ui_style_allocinit();	
	psy_ui_style_set_padding_em(style, 0.0, 0.5, 0.0, 0.5);
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_HOVER,
		"combobox::hover", style);
	/* combobox::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.onprimary, material.strong));	
	psy_ui_style_set_padding_em(style, 0.0, 0.5, 0.0, 0.5);
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_SELECT,
		"combobox::select", style);
	/* combobox_text */
	style = psy_ui_style_allocinit();	
	psy_ui_styles_set_style(self, psy_ui_STYLE_COMBOBOX_TEXT,
		"combobox_text", style);
	/* tabbar */
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, psy_ui_STYLE_TABBAR, "", style);
	/* tab */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.primary, material.medium));
	psy_ui_style_set_padding_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB, "", style);
	/* tab::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary);
	psy_ui_style_set_background_colour(style, psy_ui_colour_make_overlay(4));
	psy_ui_style_set_padding_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_HOVER, "", style);
	/* tab::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.onprimary);
	psy_ui_style_set_padding_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_border_init_bottom(&style->border, psy_ui_BORDER_SOLID, psy_ui_colour_blue());
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_SELECT, "", style);
	/* tab_label */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_weighted(material.primary, material.weak));
	psy_ui_style_set_padding_em(style, 0.0, 1.9, 0.0, 1.0);
	psy_ui_style_set_margin_em(style, 0.0, 0.3, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_TAB_LABEL, "", style);
	/* scrollpane */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		material.onsurface, psy_ui_colour_make_overlay(5));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLPANE, "", style);
	/* scrollthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		material.onsurface, psy_ui_colour_make_overlay(10));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLTHUMB, "", style);
	/* scrollthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style,
		material.onsurface, psy_ui_colour_make_overlay(20));
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLTHUMB_HOVER, "", style);
	/* scrollbutton */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON, "", style);
	/* scrollbutton::hover */
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_HOVER));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON_HOVER, "", style);
	/* scrollbutton::active */
	style = psy_ui_style_allocinit();
	style = psy_ui_style_clone(psy_ui_styles_at(self, psy_ui_STYLE_BUTTON_ACTIVE));
	psy_ui_style_set_padding_em(style, 0.0, 0.0, 0.0, 0.0);
	psy_ui_styles_set_style(self, psy_ui_STYLE_VSCROLLBUTTON_ACTIVE, "", style);
	/* sliderpane */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_background_colour(style,
		psy_ui_colour_make_overlay(5));
	psy_ui_border_init_style(&style->border, psy_ui_BORDER_SOLID);
	psy_ui_border_setcolour(&style->border,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.08));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERPANE, "", style);
	/* sliderthumb */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_blue(), psy_ui_colour_blue());
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERTHUMB, "", style);
	/* sliderthumb::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_white(), psy_ui_colour_white());
	psy_ui_styles_set_style(self, psy_ui_STYLE_SLIDERTHUMB_HOVER, "", style);
	/* splitter */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style,
		psy_ui_colour_overlayed(&material.surface, &material.overlay, 0.05));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER, "", style);
	/* splitter::hover */
	style = psy_ui_style_allocinit();
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER_HOVER, "", style);
	/* splitter::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.secondary,
		psy_ui_colour_make_overlay(16));
	psy_ui_styles_set_style(self, psy_ui_STYLE_SPLITTER_SELECT, "", style);
	/* psy_ui_STYLE_PROGRESSBAR */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, material.secondary);
	psy_ui_styles_set_style(self, psy_ui_STYLE_PROGRESSBAR, "", style);
	/* switch */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_black());
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH, "", style);
	/* switch::hover */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_black());
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH_HOVER, "", style);
	/* switch::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colour(style, psy_ui_colour_blue());
	psy_ui_styles_set_style(self, psy_ui_STYLE_SWITCH_SELECT, "", style);
	/* listbox */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_gray(), psy_ui_colour_white());
	psy_ui_styles_set_style(self, psy_ui_STYLE_LISTBOX, "", style);
	/* checkmark */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_gray(), psy_ui_colour_white());
	psy_ui_styles_set_style(self, psy_ui_STYLE_CHECKMARK, "", style);
	/* listitem */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, material.onprimary_medium,
		material.surface_overlay_5p);
	psy_ui_styles_set_style(self, psy_ui_STYLE_LIST_ITEM,
		"list_item", style);
	/* listitem::select */
	style = psy_ui_style_allocinit();
	psy_ui_style_set_colours(style, psy_ui_colour_white(),
		psy_ui_colour_blue());
	psy_ui_styles_set_style(self, psy_ui_STYLE_LIST_ITEM_SELECT,
		"list_item::select", style);
	/* editor */
	style = psy_ui_style_allocinit();
#if defined(DIVERSALIS__OS__UNIX)
	psy_ui_style_set_font(style, "FreeMono", 12);
#else
	psy_ui_style_set_font(style, "Consolas", 16);
#endif
	psy_ui_styles_set_style(self, psy_ui_STYLE_EDITOR, "", style);
}
