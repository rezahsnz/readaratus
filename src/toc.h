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
    Section = 11,
    Chapter = 12,
    Part = 13
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

TOCItem 
toc_find_dest(PopplerDocument *doc,
	       	    PopplerDest     *dest);

enum TOCType
toc_get_item_type(const char *label);

GList *
toc_extract_items(const char *text);

const TOCItem *
toc_search_by_id(const TOCItem *head_item,
                 const char *label,
                 const char *id);

const TOCItem *
toc_search_by_title(const TOCItem *toc_item,
                    const char    *title);

void
toc_create_from_poppler_index(PopplerDocument *doc,
                              TOCItem        **head_item,
                              int             *max_toc_depth);

void
toc_create_from_contents_pages(GPtrArray *page_meta_list,
                               TOCItem   **head_item,
                               int       *max_toc_depth);

void
toc_destroy(TOCItem *head_item);

void
toc_flatten_items(TOCItem *toc_item,
                  GList  **flattened_item_list);

void
toc_where_am_i(int      page_num,
               TOCItem *toc_item,
               GList  **where);

void
toc_get_labels(const TOCItem *toc_item,
               GList        **labels);

#endif