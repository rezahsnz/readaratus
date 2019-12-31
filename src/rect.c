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

#include "rect.h"
#include <math.h>

Rect *
rect_new ()
{
    Rect *rect = g_malloc(sizeof(Rect));
    return rect;
}

void 
rect_free(Rect *rect)
{
    g_free(rect);
}

Rect *
rect_copy(Rect *rect)
{
    Rect *copy = rect_new();
    copy->x1 = rect->x1;
    copy->y1 = rect->y1;
    copy->x2 = rect->x2;
    copy->y2 = rect->y2;
    return copy;
}

double
rect_width (Rect *rect)
{
    return rect->x2 - rect->x1;
}

double
rect_height (Rect *rect)
{
    return rect->y2 - rect->y1;
}

double
rect_center_x(Rect *rect)
{
    return (rect->x1 + rect->x2) / 2;
}

double
rect_center_y(Rect *rect)
{
    return (rect->y1 + rect->y2) / 2;
}

Rect *
rect_from_poppler_rectangle (PopplerRectangle *p_rect)
{
    Rect *rect = rect_new();
    rect->x1 = MIN(p_rect->x1, p_rect->x2);;
    rect->y1 = MIN(p_rect->y1, p_rect->y2);
    rect->x2 = MAX(p_rect->x1, p_rect->x2);
    rect->y2 = MAX(p_rect->y1, p_rect->y2);
    return rect;
}

Rect
map_physical_rect_to_image(Rect  *phys_rect,
                           double phys_width,
                           double phys_height,
                           double image_width,
                           double image_height, 
                           double dx,
                           double dy)
{
    double Sx = image_width / phys_width;
    double Sy = image_height / phys_height;
    Rect img_rect;
    img_rect.x1 = phys_rect->x1 * Sx + dx;
    img_rect.y1 = phys_rect->y1 * Sy + dy;
    img_rect.x2 = phys_rect->x2 * Sx + dx;
    img_rect.y2 = phys_rect->y2 * Sy + dy;
    return img_rect;
}

Rect
map_image_rect_to_physical(Rect  *img_rect,
                           double phys_width,
                           double phys_height,
                           double image_width,
                           double image_height, 
                           double dx,
                           double dy)
{
    double Sx = image_width / phys_width;
    double Sy = image_height / phys_height;
    Rect phys_rect;
    phys_rect.x1 = (img_rect->x1 - dx) / Sx;
    phys_rect.y1 = (img_rect->y1 - dy) / Sy;
    phys_rect.x2 = (img_rect->x2 - dx) / Sx;
    phys_rect.y2 = (img_rect->y1 - dy) / Sy;;
    return phys_rect;
}

gboolean 
rect_contains_point(Rect *rect,
					double x,
					double y)
{
    double normal_x1 = MIN(rect->x1, rect->x2);;
    double normal_y1 = MIN(rect->y1, rect->y2);
    double normal_x2 = MAX(rect->x1, rect->x2);
    double normal_y2 = MAX(rect->y1, rect->y2);
    return x >= normal_x1 && x <= normal_x2 &&
	       y >= normal_y1 && y <= normal_y2;
}

gboolean 
rect_list_contains_point(GList *rects,
                         double x,
                         double y)
{
    GList *rect_p = rects;
    while(rect_p){
        if(rect_contains_point(rect_p->data,
                               x, y))
        {
            break;
        }
        rect_p = rect_p->next;
    }
    return rect_p != NULL;
}

gboolean
rect_contains_rect(Rect *rect,
                   Rect *x)
{
    return rect_contains_point(rect, x->x1, x->y1) &&
           rect_contains_point(rect, x->x2, x->y2);
}

gboolean rect_topleft_of(Rect *rect,
                         Rect *x)
{
    if(x->y1 < rect->y1){
        return TRUE;
    }
    else if(x->y1 == rect->y1){
        return x->x1 < rect->x1;
    }
    else{
        return FALSE;
    }
}

double euclid_dist(Rect *a,
                   Rect *b)
{
    double a_center_x = a->x1 + (a->x2 - a->x1) / 2,
           a_center_y = a->y1 + (a->y2 - a->y1) / 2;
    double b_center_x = b->x1 + (b->x2 - b->x1) / 2,
           b_center_y = b->y1 + (b->y2 - b->y1) / 2;
    return sqrt((b_center_y - a_center_y) * (b_center_y - a_center_y) +
                (b_center_x - a_center_x) * (b_center_x - a_center_x));
}

void
dump_rect (Rect *rect)
{
    g_print("tl(%f, %f), br(%f, %f), w: %f, h: %f\n",
            rect->x1,
            rect->y1,
            rect->x2,
            rect->y2,
            rect->x2 - rect->x1,
            rect->y2 - rect->y1);
}

gboolean 
rects_have_intersection(Rect *a,
                        Rect *b)
{
    gboolean intersects = rect_contains_point(a,
                                              b->x1, 
                                              b->y1) ||
                          rect_contains_point(a,
                                              b->x1, 
                                              b->y2) ||
                          rect_contains_point(a,
                                              b->x2, 
                                              b->y1) ||
                          rect_contains_point(a,
                                              b->x2, 
                                              b->y2) ||
                          rect_contains_point(b,
                                              a->x1, 
                                              a->y1) ||
                          rect_contains_point(b,
                                              a->x1, 
                                              a->y2) ||
                          rect_contains_point(b,
                                              a->x2, 
                                              a->y1) ||
                          rect_contains_point(b,
                                              a->x2, 
                                              a->y2);
    return intersects;
}

gboolean
rect_intersects_rect_list(Rect  *x,
                          GList *rects)
{
    GList *rect_p = rects;
    while(rect_p){
        if(rects_have_intersection(rect_p->data,
                                   x))
        {
            break;
        }
        rect_p = rect_p->next;
    }
    return rect_p ? TRUE : FALSE;
}

gboolean
rect_lists_intersect(GList *rects_a,
                     GList *rects_b)
{
    GList *rect_a_p = rects_a;
    while(rect_a_p){        
        if(rect_intersects_rect_list(rect_a_p->data,
                                     rects_b)){
            break;
        }
        rect_a_p = rect_a_p->next;
    }
    return rect_a_p ? TRUE : FALSE;
}
