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

#ifndef FIGURE_H
#define FIGURE_H

#include <glib.h>
#include "rect.h"

typedef struct
{
    Rect *physical_layout;
    double distance_to_image;
}Caption;

typedef struct Figure Figure;
struct Figure
{
    char *whole_match;
	char *label;
    gboolean is_label_exclusive;
	char *id;
    gboolean is_id_complex;
    Rect *caption_physical_layout; 
    
    int page_num;
    int image_id; /* index given by poppler to images */
    Rect *image_physical_layout;

    GList *captions;

    Figure *reference;
};

typedef struct ReferencedFigure ReferencedFigure;
struct ReferencedFigure
{
    char *label;
    char *id;
    GList *find_results;
    GList *activated_find_result;
    Figure *reference;
};

void
figure_module_init(void);

void 
figure_module_destroy(void);

Figure *
figure_new (void);

Figure *
figure_copy(const Figure *figure);

void
figure_free (Figure *);

GList * 
extract_figure_captions (const char *);

GList * 
extract_figure_references (const char *);

gboolean
are_figure_labels_equal(const char *l1,
                        const char *l2);

#endif