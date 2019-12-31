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

#ifndef RECT_H
#define RECT_H

#include <gmodule.h>
#include <poppler/glib/poppler.h>

typedef struct 
{
    double x1;
    double y1;
    double x2;
    double y2;
} Rect;

Rect *
rect_new ();

void 
rect_free (Rect *rect);

Rect *
rect_copy(Rect *rect);

double
rect_width (Rect *rect);

double
rect_height (Rect *rect);

double
rect_center_x(Rect *rect);

double
rect_center_y(Rect *rect);

Rect *
rect_from_poppler_rectangle (PopplerRectangle *p_rect);

Rect
map_physical_rect_to_image(Rect  *phys_rect,
                           double phys_width,
                           double phys_height,
                           double image_width,
                           double image_height, 
                           double dx,
                           double dy);

Rect
map_image_rect_to_physical(Rect  *img_rect,
                           double phys_width,
                           double phys_height,
                           double image_width,
                           double image_height, 
                           double dx,
                           double dy);

gboolean
rect_contains_point(Rect *,
					double x,
					double y);

gboolean
rect_contains_rect(Rect *rect,
                   Rect *x);

gboolean
rect_contains_rect(Rect *,
                   Rect *);

gboolean rect_topleft_of(Rect *,
                         Rect *);

double
euclid_dist(Rect *,
            Rect *);

void
dump_rect (Rect *);

gboolean
rects_have_intersection(Rect *a,
                        Rect *b);
gboolean
rect_intersects_rect_list(Rect  *x,
                          GList *rects);

gboolean
rect_lists_intersect(GList *rects_a,
                     GList *rects_b);

#endif