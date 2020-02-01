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

#ifndef PAGE_META_H
#define PAGE_META_H

#include <gmodule.h>
#include <cairo.h>
#include <poppler/glib/poppler.h>
#include "rect.h"
#include "figure.h"

typedef struct Link Link;
struct Link
{
	Rect *physical_layout;
	char *tip;
    gboolean is_hovered;
    int target_page_num;
    double target_progress_x;
    double target_progress_y;
};

typedef struct PageMeta PageMeta;
struct PageMeta
{
	PopplerPage *page;
	char *text;
    int page_num;
    char *page_label;
    char *reading_progress_text;
	double page_width;
	double page_height;
    double aspect_ratio;
    unsigned int num_layouts;
	GPtrArray *physical_text_layouts;
	double mean_line_height;
	GList *links;
	GList *converted_units;

    GHashTable *figures;
	GList *referenced_figures;
    ReferencedFigure *active_referenced_figure;

    GList *find_results;
};

#endif