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
#ifndef TOC_SYNTHESIS
#define TOC_SYNTHESIS

#include <glib.h>
#include <poppler/glib/poppler.h>
#include "toc.h"

int
string_index_in_list(GList      *list,
                     const char *needle,
                     gboolean    is_case_sensitive);

void
toc_create_from_contents_pages(PopplerDocument *document,
                               GPtrArray       *page_meta_list,
                               GHashTable      *page_label_num_hash,
                               TOCItem        **head_item);

#endif