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
void
match_found_rects(GList  *list_p,
                  Rect   *prev_rect,
                  double  mean_line_height,
                  GList **matched_rects);

void
find_rects_of_text(PageMeta *meta,
                   GRegex   *regex,
                   gboolean  is_dualpage,
                   gboolean  is_whole_words,
                   GList   **find_results);

GList *
find_text(const GPtrArray *metae,
          const char *find_term,
          int         start_page,
          int         pages_length,
          gboolean    is_dualpage,
          gboolean    is_whole_words);


#endif