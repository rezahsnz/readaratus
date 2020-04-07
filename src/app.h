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

#ifndef APP_H
#define APP_H

#include <gtk/gtk.h>
#include <poppler/glib/poppler.h>
#include "rect.h"
#include "toc.h"

enum AppMode
{
    StartMode, ReadingMode, TOCMode, HelpMode
};

enum ZoomLevel
{
    PageFit, WidthFit, In, Out
};

struct Ui
{
    GtkWidget *main_window;
    GList *icons;
    gboolean is_fullscreen;
    GtkWidget *vellum;
    GdkCursor *default_cursor;
    GdkCursor *text_cursor;
    GdkCursor *pointer_cursor;
    /* app mode */
    enum AppMode app_mode;
    /* start mode widgets */
    gboolean is_import_area_hovered;
    Rect *import_area_rect;
    Rect *book_details_area_rect;
    gboolean is_continue_to_book_button_hovered;
    Rect *continue_to_book_button_rect;
    Rect *app_info_area_rect;
    const char *app_info_text;
    /* reading mode widgets */
    /* panel */
    Rect *panel_rect;
    gboolean is_panel_hovered;
    /* dynamic objects */
    gboolean is_find_result_hovered;
    gboolean is_link_hovered;
    gboolean is_unit_hovered;
    /* page navigation */
    gboolean is_next_prev_page_hovered;
    /* zoom widget */
    gboolean is_zoom_widget_PF_hovered;
    Rect *zoom_widget_PF_rect;
    gboolean is_zoom_widget_WF_hovered;
    Rect *zoom_widget_WF_rect;
    gboolean is_zoom_widget_IN_hovered;
    Rect *zoom_widget_IN_rect;
    gboolean is_zoom_widget_OUT_hovered;
    Rect *zoom_widget_OUT_rect;
    /* page buttons */
    gboolean is_prev_page_button_hovered;
    Rect *prev_page_button_rect;
    gboolean is_next_page_button_hovered;
    Rect *next_page_button_rect;
    /* teleport launcher*/
    gboolean is_teleport_launcher_hovered;
    Rect *teleport_launcher_rect;
    /* find text launcher*/
    gboolean is_find_text_launcher_hovered;
    Rect *find_text_launcher_rect;
    /* TOC launcher*/
    gboolean is_toc_launcher_hovered;
    Rect *toc_launcher_rect; 
    /* widgets */
    GtkWidget *teleport_widget;
    GtkWidget *find_widget;
}ui;

struct TOC
{
    TOCItem *head_item;
    int max_depth;
    GList *flattened_items;
    TOCItem *hovered_item;
    GList *labels;
    GList *navigation_button_rects;
    Rect *hovered_navigation_button;
    GList *where;
    double origin_x;
    double origin_y;
};

struct FindDetails
{
    GList *find_results;
    GList *selected_p;
    int max_results;
    int max_results_page_num;
};

struct GoBack
{
    double progress_x;
    double progress_y;
    int page_num;
};
typedef struct GoBack GoBack;

struct DocumentInfo
{
    char *book_info_label;
    char *book_info_data;
};

struct Document 
{
    char *filename;
    PopplerDocument *doc;
    struct DocumentInfo doc_info;
    GPtrArray *metae;
        
    cairo_surface_t *image;
    double image_origin_x;
    double image_origin_y;
    double preserved_progress_x;
    double preserved_progress_y;  
    
    enum ZoomLevel zoom_level;
    
    GHashTable *page_label_num_hash;
    int num_pages;
    int cur_page_num;

    struct FindDetails find_details;
    struct TOC toc;
    GQueue *go_back_stack;
}d;

void 
init_app(GtkApplication *app);

#endif
