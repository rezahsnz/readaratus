/*
 * Copyright © 2020 Reza Hasanzadeh
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3.0 of the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TOC_H
#define TOC_H

#include "rect.h"
#include <poppler/glib/poppler.h>

enum TOCType
{
    None = 0,
    Subsection = 10,
    Section = 20,
    Chapter = 30,
    Part = 40
};
typedef struct TOCItem TOCItem;
struct TOCItem
{
    char *title;
    char *label;
    char *id;
	int depth;
	int page_num;
	int length;
    Rect *rect;
	TOCItem *parent;
    TOCItem *next;
    TOCItem *previous;
	GList *children;
	
    double offset_x;
	double offset_y;
};


void
toc_module_init(void);

void
toc_module_destroy(void);

TOCItem *
toc_item_new(void);

void 
toc_item_free(TOCItem *toc_item);

void
toc_dump(TOCItem *toc_item);

void 
toc_calc_length(TOCItem *toc_item);

TOCItem 
toc_find_dest(PopplerDocument *doc,
	       	    PopplerDest     *dest);

char *
toc_infer_child_label(const char *label_parent);

int
string_index_in_list(GList      *list,
                     const char *needle,
                     gboolean    is_case_sensitive);

void
toc_fix_depth(TOCItem *toc_item);

void
toc_fix_sibling_links(TOCItem *toc_item);

void toc_fix_labels(TOCItem    *toc_item,
                    const char *label_parent,
                    gboolean   *has_labels);

enum TOCType
toc_get_item_type(const char *label);

const TOCItem *
toc_search_by_id(const TOCItem *head_item,
                 const char *label,
                 const char *id);

const TOCItem *
toc_search_by_title(const TOCItem *toc_item,
                    const char    *title);

void
toc_fix_labels_blindly(TOCItem    *toc_item,
                       const char *label_parent);

void
toc_create_from_poppler_index(PopplerDocument *doc,
                              TOCItem        **head_item);

int
translate_page_label(GHashTable *page_label_num_hash,
                     const char *label);

void
toc_destroy(TOCItem *head_item);

void
toc_flatten(TOCItem *toc_item,
            GList  **flattened_item_list);

void
toc_where_am_i(int      page_num,
               TOCItem *toc_item,
               GList  **where);

#endif