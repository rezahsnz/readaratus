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

#ifndef FIND_H
#define FIND_H

#include "rect.h"
#include "page_meta.h"

typedef struct FindResult FindResult;
struct FindResult
{
    int page_num;
    char *match;  
    GList *physical_layouts;
    FindResult *page_prefix;
    FindResult *page_postfix;
    double certainty;
    char *tip;
};

FindResult *
find_result_new();

void
find_result_free(FindResult *fr);

int
compare_rects(const void *a,
              const void *b);

int
compare_poppler_rects(const void *a,
                      const void *b);

int
compare_find_results(const void *a,
                     const void *b);
GList *
find_text(PopplerDocument *document,
          const GPtrArray *metae,
          const char *find_term,
          int         start_page,
          int         pages_length,
          gboolean    is_dualpage,
          gboolean    is_whole_words);


#endif