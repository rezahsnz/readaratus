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

#include "app.h"
#include <math.h>
#include "teleport_widget.h"
#include "find_widget.h"
#include "page_meta.h"
#include <pango/pangocairo.h>

/* gainsboro: #DCDCDC, (220, 220, 220) */
static const double gainsboro_r = 0.8627;
/* silver: #C0C0C0, (192, 192, 192) */
static const double silver_r = 0.7529;
/* gray: #A8A8A8, (128, 128, 128) */
static const double gray_r = 0.5;
/* dark gray: #A9A9A9, (169, 169, 169) */
static const double dark_gray_r = 0.6627;
/* dim gray: #696969, (105, 105, 105) */
static const double dim_gray_r = 0.4118;
/* cadet gray: #91A3B0, (145, 163, 176) */
static const double cadet_gray_r = 0.5686,
                    cadet_gray_g = 0.6392,
                    cadet_gray_b = 0.6902;                       
/* gotham green: #00573F, (0, 87, 63) */
static const double gotham_green_r = 0.0,
                    gotham_green_g = 0.3412,
                    gotham_green_b = 0.2471;
/* giants_orange: #FE5A1D, (254, 90, 29) */
static const double giants_orange_r = 0.9960,
                    giants_orange_g = 0.3529,
                    giants_orange_b = 0.1137;
/* darkblue: #FE5A1D, (254, 90, 29) */
static const double blue_r = 0,
                    blue_g = 0,
                    blue_b = 1;

static const double dashed_style[2] = {8, 5};
static const int num_dashes = 2;
static const double toc_navigation_panel_height = 72;
static GRegex *navigation_regex = NULL;

static void 
pose_page_widgets(void)
{
    int widget_width = gtk_widget_get_allocated_width(ui.vellum);
    int widget_height = gtk_widget_get_allocated_height(ui.vellum);
    int padding = 8;
    int button_size = 47;
    /* start mode widgets */
    ui.import_area_rect->x1 = 0;
    ui.import_area_rect->y1 = 0;
    ui.import_area_rect->x2 = widget_width;
    ui.import_area_rect->y2 = widget_height * 0.25;
    ui.book_details_area_rect->x1 = 0;
    ui.book_details_area_rect->y1 = 1 + ui.import_area_rect->y2;
    ui.book_details_area_rect->x2 = ui.book_details_area_rect->x1 + widget_width / 2;
    ui.book_details_area_rect->y2 = ui.book_details_area_rect->y1 + widget_height * 0.75;
    ui.app_info_area_rect->x1 = 1 + ui.book_details_area_rect->x2;
    ui.app_info_area_rect->y1 = 1 + ui.import_area_rect->y2;
    ui.app_info_area_rect->x2 = ui.app_info_area_rect->x1 + widget_width / 2;
    ui.app_info_area_rect->y2 = ui.app_info_area_rect->y1 + widget_height * 0.75;
    /* reading mode widgets */
    /* page fit */
    ui.zoom_widget_PF_rect->x1 = widget_width - button_size - padding;
    ui.zoom_widget_PF_rect->y1 = padding;
    ui.zoom_widget_PF_rect->x2 = ui.zoom_widget_PF_rect->x1 + button_size;
    ui.zoom_widget_PF_rect->y2 = ui.zoom_widget_PF_rect->y1 + button_size;
    /* width fit */
    ui.zoom_widget_WF_rect->x1 = widget_width - button_size - padding;
    ui.zoom_widget_WF_rect->y1 = button_size + 2 * padding;
    ui.zoom_widget_WF_rect->x2 = ui.zoom_widget_WF_rect->x1 + button_size;
    ui.zoom_widget_WF_rect->y2 = ui.zoom_widget_WF_rect->y1 + button_size;
    /* zoom in */
    ui.zoom_widget_IN_rect->x1 = widget_width - button_size - padding;
    ui.zoom_widget_IN_rect->y1 = 2 * button_size + 3 * padding;
    ui.zoom_widget_IN_rect->x2 = ui.zoom_widget_IN_rect->x1 + button_size;
    ui.zoom_widget_IN_rect->y2 = ui.zoom_widget_IN_rect->y1 + button_size;
    /* zoom out */
    ui.zoom_widget_OUT_rect->x1 = widget_width - button_size - padding;
    ui.zoom_widget_OUT_rect->y1 = 3 * button_size + 4 * padding;
    ui.zoom_widget_OUT_rect->x2 = ui.zoom_widget_OUT_rect->x1 + button_size;
    ui.zoom_widget_OUT_rect->y2 = ui.zoom_widget_OUT_rect->y1 + button_size;
    /* prev page */
    ui.prev_page_button_rect->x1 = padding;
    ui.prev_page_button_rect->y1 = widget_height / 2 - button_size / 2;
    ui.prev_page_button_rect->x2 = ui.prev_page_button_rect->x1 + button_size;
    ui.prev_page_button_rect->y2 = ui.prev_page_button_rect->y1 + button_size;
    /* next page */
    ui.next_page_button_rect->x1 = widget_width - button_size - padding;
    ui.next_page_button_rect->y1 = widget_height / 2 - button_size / 2;
    ui.next_page_button_rect->x2 = ui.next_page_button_rect->x1 + button_size;
    ui.next_page_button_rect->y2 = ui.next_page_button_rect->y1 + button_size;
    /* teleport launcher */
    ui.teleport_launcher_rect->x1 = padding;
    ui.teleport_launcher_rect->y1 = widget_height - button_size - padding;
    ui.teleport_launcher_rect->x2 = ui.teleport_launcher_rect->x1 + button_size;
    ui.teleport_launcher_rect->y2 = ui.teleport_launcher_rect->y1 + button_size;
    /* find text launcher */
    ui.find_text_launcher_rect->x1 = ui.teleport_launcher_rect->x2 + padding;
    ui.find_text_launcher_rect->y1 = widget_height - button_size - padding;
    ui.find_text_launcher_rect->x2 = ui.find_text_launcher_rect->x1 + button_size;
    ui.find_text_launcher_rect->y2 = ui.find_text_launcher_rect->y1 + button_size;
    /* TOC launcher */
    ui.toc_launcher_rect->x1 = ui.find_text_launcher_rect->x2 + padding;
    ui.toc_launcher_rect->y1 = widget_height - button_size - padding;
    ui.toc_launcher_rect->x2 = ui.toc_launcher_rect->x1 + button_size;
    ui.toc_launcher_rect->y2 = ui.toc_launcher_rect->y1 + button_size;    
}

static cairo_surface_t *
render_page (PageMeta *meta,
             double width,
             double height)
{    
    cairo_surface_t *image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                        width,
                                                        height);
    cairo_t *cr = cairo_create(image);
    cairo_scale(cr, 
                width / meta->page_width,
                height / meta->page_height);    
    cairo_set_source_rgb(cr,
                         1, 1, 1);
    cairo_paint(cr);     
    poppler_page_render(meta->page,
                        cr);
    cairo_destroy(cr);
    return image;
}

static void
scale_page (enum ZoomLevel zl,
            gboolean       in_out_disabled,            
            double         progress_x,
            double         progress_y)
{
    static const double MIN_PAGE_WIDTH = 24;
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       d.cur_page_num);
    double widget_width = gtk_widget_get_allocated_width(ui.vellum);
    double widget_height = gtk_widget_get_allocated_height(ui.vellum);
    double image_width, image_height;
    switch(zl){
    case PageFit:
        image_height = widget_height;
        image_width = image_height / meta->aspect_ratio;
        break;
    case WidthFit:
        image_width = widget_width;
        image_height = image_width * meta->aspect_ratio;
        break;
    case In:
        image_width = cairo_image_surface_get_width(d.image);
        if(!in_out_disabled){
            image_width *= 1.1;
        }
        image_height = image_width * meta->aspect_ratio;
        break;
    case Out:
        image_width = cairo_image_surface_get_width(d.image);
        if(!in_out_disabled){
            if(image_width / 1.1 > MIN_PAGE_WIDTH){
                image_width /= 1.1;
            }
        }
        image_height = image_width * meta->aspect_ratio;
        break;
    }
    if(image_width <= widget_width){
        progress_x = 0;
    }
    if(image_height <= widget_height){
        progress_y = 0;
    }
    if(image_width < widget_width){
        d.image_origin_x = widget_width / 2.0 - image_width / 2.0;    
    }
    else{
        double max_scrollable_width = image_width - widget_width;
        d.image_origin_x = -MIN(progress_x * image_width, fabs(max_scrollable_width));
    }
    if(image_height <= widget_height){
        d.image_origin_y = widget_height / 2.0 - image_height / 2.0;
    }
    else{
        double max_scrollable_height = image_height - widget_height;        
        d.image_origin_y = -MIN(fabs(progress_y * image_height), fabs(max_scrollable_height));
    }
    cairo_surface_destroy(d.image);
    d.image = render_page(meta,
                          image_width,
                          image_height);    
    d.zoom_level = zl;
    gtk_widget_queue_draw(ui.vellum);
}

static void
zoom_page_fit(void)
{
    scale_page(PageFit,
               TRUE,
               0, 0);    
}

static void
zoom_width_fit(void)
{
    scale_page(WidthFit,
               TRUE,
               0, d.preserved_progress_y);
}

static void
zoom_in(void)
{
    scale_page(In,
               FALSE,
               d.preserved_progress_x, d.preserved_progress_y);
}
static void
zoom_out(void)
{
    scale_page(Out,
               FALSE,
               d.preserved_progress_x, d.preserved_progress_y);    
}

static void 
goto_page (int page_num,
           double progress_x,
           double progress_y)
{
    if((page_num == d.cur_page_num) ||
       (page_num < 0 || page_num >= d.num_pages))
    {
        return;
    }
    if(d.toc.max_depth > 0){
        g_list_free(d.toc.where);
        d.toc.where = NULL;
        toc_where_am_i(page_num,
                       d.toc.head_item,
                       &d.toc.where);
        d.toc.where = g_list_prepend(d.toc.where,
                                     d.toc.head_item);
    }
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       page_num);
    meta->active_referenced_figure = NULL;
    d.cur_page_num = page_num;
    scale_page(d.zoom_level, 
               TRUE,
               progress_x, progress_y);
}

static void
next_page(void)
{
    goto_page(d.cur_page_num + 1,
              0, 0);
}

static void
previous_page(void)
{
    goto_page(d.cur_page_num - 1,
              0, 0);
}

static void
toggle_fullscreen(void)
{
    if(ui.is_fullscreen){
        gtk_window_unfullscreen(GTK_WINDOW(ui.main_window));
    }
    else{
        gtk_window_fullscreen(GTK_WINDOW(ui.main_window));
    }
}

static gboolean
window_state_callback(GtkWidget           *widget,
                      GdkEventWindowState *event,
                      gpointer             user_data)
{
    ui.is_fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;    
    return TRUE;
}

static gboolean
configure_callback (GtkWidget *widget,
                    GdkEventConfigure *event,
                    gpointer data)
{   
    pose_page_widgets();
    if(ui.app_mode == ReadingMode){
        double progress_x = fabs(d.image_origin_x) / cairo_image_surface_get_width(d.image);
        double progress_y = fabs(d.image_origin_y) / cairo_image_surface_get_height(d.image);
        scale_page(d.zoom_level,
                   TRUE,
                   progress_x, progress_y);
    }
    return TRUE;
}

static void
scroll_with_pixels(double dx,
                   double dy)
{
    int widget_width = gtk_widget_get_allocated_width(ui.vellum);
    int widget_height = gtk_widget_get_allocated_height(ui.vellum);
    if(ui.app_mode == ReadingMode){
        double image_width = cairo_image_surface_get_width(d.image);
        double image_height = cairo_image_surface_get_height(d.image);
        double hidden_portion_width = image_width - widget_width;
        double hidden_portion_height = image_height - widget_height;
        switch(d.zoom_level){
            case PageFit:
                dx = dy = 0;
                break;
            case WidthFit:
                /* vertical only */
                dx = 0;
                if(dy + d.image_origin_y > 0){
                    dy = 0;
                }
                if(abs(dy + d.image_origin_y) > hidden_portion_height){
                    dy = 0;
                }
                break;
            default:
                /* vertical */
                if(dy + d.image_origin_y > 0){
                    dy = 0;
                }
                if(abs(dy + d.image_origin_y) > hidden_portion_height){
                    dy = 0;
                }
                /* horizontal */
                if(dx + d.image_origin_x > 0){
                    dx = 0;
                }
                if(abs(dx + d.image_origin_x) > hidden_portion_width){
                    dx = 0;
                }
                break;
        }
        if(dx != 0.0 || dy != 0.0){
            d.image_origin_x += dx;
            d.image_origin_y += dy;
            d.preserved_progress_x = (d.image_origin_x) / cairo_image_surface_get_width(d.image);
            d.preserved_progress_y = (d.image_origin_y) / cairo_image_surface_get_height(d.image);
            gtk_widget_queue_draw(ui.vellum);
        }
    }
    else if(ui.app_mode == TOCMode){
        if(d.toc.origin_x - dx >= 0){
            d.toc.origin_x -= dx;
        }
        if(d.toc.origin_y - dy >= 0){
            d.toc.origin_y -= dy;
        }
        gtk_widget_queue_draw(ui.vellum);
    }
    else{        
    }
}

static int compare_rects(const void *a,
                         const void *b)
{
    int comp;
    const Rect *rect_a = a;
    const Rect *rect_b = b;
    if((rect_a->y1 < rect_b->y1) ||
       (rect_a->y1 == rect_b->y1 &&
        rect_a->x1 < rect_b->x1))
    {
        comp = -1;
    }
    else{
        comp = 1;
    }
    /* desc y, asc x*/
    return -comp;
}

static void
match_found_rects(GList  *list_p,
                  Rect   *prev_rect,
                  double  mean_line_height,
                  GList **matched_rects)
{    
    if(!list_p){
        return;
    }
    GList *candidate_rects = NULL;
    double prev_center_y = rect_center_y(prev_rect);
    GList *rects = list_p->data;
    GList *rect_p = rects;
    while(rect_p){
        Rect *rect = rect_p->data;
        double center_y = rect_center_y(rect);
        if((center_y > prev_center_y) &&
           ((center_y - prev_center_y) <= (mean_line_height * 1.9)))
        {
            candidate_rects = g_list_append(candidate_rects,
                                            rect);
        }
        rect_p = rect_p->next;
    }
    if(candidate_rects){
        candidate_rects = g_list_sort(candidate_rects,
                                      compare_rects);
        Rect *rect = g_list_first(candidate_rects)->data;
        *matched_rects = g_list_append(*matched_rects,
                                       rect);
        match_found_rects(list_p->next,
                          rect,
                          mean_line_height,
                          matched_rects);
    }
    g_list_free(candidate_rects);
}

static int compare_poppler_rects(const void *a,
                                 const void *b)
{
    int comp;
    const PopplerRectangle *rect_a = a;
    const PopplerRectangle *rect_b = b;
    if((rect_a->y1 < rect_b->y1) ||
       (rect_a->y1 == rect_b->y1 &&
        rect_a->x1 > rect_b->x1))
    {
        comp = -1;
    }
    else{
        comp = 1;
    }
    /* desc y, asc x */
    return -comp;
}


static int
compare_find_results(const void *a,
                     const void *b)
{
    const FindResult *fr_a = a;
    const FindResult *fr_b = b;
    int comp = fr_a->page_num - fr_b->page_num;
    if(comp == 0){
        const Rect *rect_a = fr_a->physical_layouts->data;
        const Rect *rect_b = fr_b->physical_layouts->data;
        /* asc y, asc  x */
        if((rect_a->y1 < rect_b->y1) ||
           (rect_a->y1 == rect_b->y1 &&
            rect_a->x1 < rect_b->x1))
        {
            comp = -1;
        }
        else{
            comp = 1;
        }
    }
    return comp;    
}

static void
find_rects_of_text(PageMeta *meta,
                   GRegex   *regex,
                   gboolean  is_dualpage,
                   gboolean  is_whole_words,
                   GList   **find_results)
{
    int poppler_find_options = 0;
    poppler_find_options |= is_whole_words ? POPPLER_FIND_WHOLE_WORDS_ONLY : 0;
    if(poppler_find_options == 0){
        poppler_find_options = POPPLER_FIND_DEFAULT;
    }

    GMatchInfo *match_info = NULL;
    g_regex_match(regex,
                  meta->text,
                  0,
                  &match_info);
    const char *pattern = g_regex_get_pattern(regex);
    GList *already_matched_rects = NULL;
    gboolean is_flattend = FALSE;
    while(g_match_info_matches(match_info)){
        char *match = g_match_info_fetch(match_info,
                                         0);
        char **tokens = g_regex_split_simple("\\R",
                                             match,
                                             0,
                                             0);
        int num_tokens = g_strv_length(tokens);
        /* a: flatten match */
        if(!is_flattend){
            char *flattened_match = g_strjoinv(" ",
                                               tokens);
            GList *pop_rects = poppler_page_find_text_with_options(meta->page,
                                                                   flattened_match,
                                                                   (PopplerFindFlags)poppler_find_options);
            GList *rect_p = pop_rects;
            while(rect_p){
                PopplerRectangle *pr = rect_p->data;
                pr->y1 = meta->page_height - pr->y1;
                pr->y2 = meta->page_height - pr->y2;
                Rect *rect = rect_from_poppler_rectangle(pr);
                poppler_rectangle_free(pr);
                double rect_cy = rect_center_y(rect);
                if(is_dualpage){
                    gboolean is_valid = TRUE;
                    /* prefix: rect must be rightmost */
                    if(pattern[0] != '^'){
                        for(int i = 0; i < meta->num_layouts; i++){
                            Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                                i);
                            double text_cy = rect_center_y(text_rect);
                            if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                               text_rect->x1 >= rect->x2)
                            {
                                is_valid = FALSE;
                                break;
                            }
                            if(text_cy - rect_cy > 2 * meta->mean_line_height){
                                break;
                            }
                        }                        
                    }
                    /* postfix: rect must be leftmost */
                    else{
                        for(int i = 0; i < meta->num_layouts; i++){
                            Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                                i);
                            double text_cy = rect_center_y(text_rect);
                            if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                               text_rect->x1 < rect->x1)
                            {
                                is_valid = FALSE;
                                break;
                            }
                            if(text_cy - rect_cy > 2 * meta->mean_line_height){
                                break;
                            }
                        } 
                    }
                    if(!is_valid){
                        rect_free(rect);
                        rect_p = rect_p->next;  
                        continue;
                    }
                }
                FindResult *find_result = find_result_new();
                find_result->page_num = meta->page_num;
                find_result->match = g_strdup(flattened_match);
                find_result->physical_layouts = g_list_append(find_result->physical_layouts,
                                                              rect);
                *find_results = g_list_append(*find_results,
                                              find_result);
                already_matched_rects = g_list_append(already_matched_rects,
                                                      rect);
                rect_p = rect_p->next;                    
            }
            g_free(flattened_match);
            g_list_free(pop_rects);
            is_flattend = TRUE;
        }
        if(num_tokens == 1){
            g_free(match);
            g_strfreev(tokens);
            g_match_info_next(match_info,
                              NULL);
            continue;
        }
        /* b: split match by 'new line' and perform line matching */
        GList *list_of_rects = NULL;
        char **token_p = tokens;
        while(*token_p){
            GList *rects = NULL;
            GList *pop_rects = poppler_page_find_text_with_options(meta->page,
                                                                   *token_p,
                                                                   (PopplerFindFlags)poppler_find_options);
            if(!pop_rects){                   
                g_print("poppler failed to find: '%s' part of '%s'\n",
                        *token_p, match); 
            }          
            pop_rects = g_list_sort(pop_rects,
                                    compare_poppler_rects);
            GList *rect_p = pop_rects;                
            while(rect_p){
                PopplerRectangle *pr = rect_p->data;
                pr->y1 = meta->page_height - pr->y1;
                pr->y2 = meta->page_height - pr->y2;
                Rect *rect = rect_from_poppler_rectangle(pr);
                poppler_rectangle_free(pr);
                GList *already_p = already_matched_rects;
                while(already_p){
                    if(rects_have_intersection(rect,
                                               already_p->data)){
                        break;
                    }
                    already_p = already_p->next;
                }
                if(!already_p){
                    rects = g_list_append(rects,
                                          rect);
                }
                else{
                    rect_free(rect);
                }
                rect_p = rect_p->next;
            }
            g_list_free(pop_rects);
            if(rects){         
                /* at most one rect per line: the first line contains the
                   rightmost rect and subsequent lines contain lefmost rects */
                GList *one_per_line_rects = NULL;
                GList *seen_lines = NULL;
                rect_p = rects;
                while(rect_p){
                    if(g_list_find(seen_lines,
                                   rect_p->data))
                    {
                        rect_p = rect_p->next;
                        continue;
                    }
                    Rect *rect_base = rect_p->data;
                    GList *same_line_rects = NULL;
                    same_line_rects = g_list_append(same_line_rects,
                                                    rect_base);
                    double base_center_y = rect_center_y(rect_base);
                    GList *next_p = rect_p->next;
                    while(next_p){
                        if(g_list_find(seen_lines,
                                       next_p->data))
                        {
                            next_p = next_p->next;
                            continue;
                        }
                        Rect *rect_next = next_p->data;
                        double next_center_y = rect_center_y(rect_next);
                        if(fabs(next_center_y - base_center_y) < 0.05 * meta->mean_line_height){
                            same_line_rects = g_list_append(same_line_rects,
                                                            rect_next);
                        }
                        next_p = next_p->next;
                    }
                    if(list_of_rects){
                        /* leftmost rect */                                                                                
                        one_per_line_rects = g_list_append(one_per_line_rects,
                                                           g_list_first(same_line_rects)->data);
                    }
                    else{
                        /* rightmost rect*/
                        gboolean is_postfix = is_dualpage && (pattern[0] != '^');
                        one_per_line_rects = g_list_append(one_per_line_rects,
                                                           is_postfix ? g_list_first(same_line_rects)->data
                                                                      : g_list_last(same_line_rects)->data);
                    }
                    seen_lines = g_list_concat(seen_lines,
                                               same_line_rects);
                    rect_p = rect_p->next;
                }
                list_of_rects = g_list_append(list_of_rects,
                                              one_per_line_rects);
                g_list_free(seen_lines);
                g_list_free(rects);
            }
            token_p++;
        }
        if(g_list_length(list_of_rects) == num_tokens){
            /* remove rects from list until the max size of any list is <= the smallest list,
               only retain vertically close rects */
            GList *cur_list_p = list_of_rects;
            while(cur_list_p){
                if(cur_list_p->next){
                    GList *cur_rects = cur_list_p->data;  
                    int cur_list_size = g_list_length(cur_rects);                                                                      
                    GList *next_rects = cur_list_p->next->data;
                    int next_list_size = g_list_length(next_rects);
                    if(cur_list_size == next_list_size){
                        cur_list_p = cur_list_p->next;
                        continue;
                    }
                    GList *edited_large_rects = NULL;
                    GList *small_rects = cur_list_size < next_list_size ? cur_rects : next_rects;
                    GList *large_rects =  cur_list_size > next_list_size ? cur_rects : next_rects;
                    double dist_sign = small_rects == cur_rects ? 1 : -1;
                    GList *small_p = small_rects;
                    while(small_p){
                        Rect *rect_small = small_p->data;
                        double small_center_y = rect_center_y(rect_small);
                        double min_dist = meta->page_height;
                        Rect *closest_rect = NULL;
                        GList *large_p = large_rects;
                        while(large_p){
                            Rect *rect_large = large_p->data;
                            if(!g_list_find(edited_large_rects,
                                            rect_large))
                            {
                                double large_center_y = rect_center_y(rect_large);
                                double dist = dist_sign * (small_center_y - large_center_y);
                                if(dist < min_dist && (fabs(dist) > meta->mean_line_height * 0.05)){
                                    min_dist = small_center_y - large_center_y;
                                    closest_rect = rect_large;
                                }
                            }
                            large_p = large_p->next;
                        }
                        edited_large_rects = g_list_append(edited_large_rects,
                                                           closest_rect);
                        small_p = small_p->next;
                    }
                    if(large_rects == cur_rects){
                        cur_list_p->data = edited_large_rects;
                    }
                    else{
                        cur_list_p->next->data = edited_large_rects;
                    }
                    GList *large_p = large_rects;
                    while(large_p){
                        if(!g_list_find(edited_large_rects,
                                        large_p->data))
                        {
                            rect_free(large_p->data);
                        }
                        large_p = large_p->next;
                    }
                    g_list_free(large_rects);
                }
                cur_list_p = cur_list_p->next;
            }
            GList *rect_p = list_of_rects->data;
            while(rect_p){
                Rect *rect = rect_p->data;   
                double rect_cy = rect_center_y(rect);                   
                /* make sure the first rect is right-most except for multipage postfix rects */
                gboolean is_rightmost = TRUE;
                if((num_tokens > 1) && !(is_dualpage && pattern[0] == '^')){
                    for(int i = 0; i < meta->num_layouts; i++){
                        Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                            i);
                        double text_cy = rect_center_y(text_rect);
                        if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                           text_rect->x1 >= rect->x2)
                        {
                            is_rightmost = FALSE;
                            break;
                        }
                        if(text_cy - rect_cy > 2 * meta->mean_line_height){
                            break;
                        }
                    }
                }           
                if(!is_rightmost){
                    rect_p = rect_p->next;
                    continue;
                }      
                GList *matched_rects = NULL;
                matched_rects = g_list_append(matched_rects,
                                              rect);
                match_found_rects(list_of_rects->next,
                                  rect,
                                  meta->mean_line_height,
                                  &matched_rects);
                if(g_list_length(matched_rects) == num_tokens){
                    FindResult *find_result = find_result_new();
                    find_result->page_num = meta->page_num;
                    find_result->match = g_strdup(match);
                    GList *ma_rect_p = matched_rects;        
                    while(ma_rect_p){
                        Rect *fr_rect = ma_rect_p->data;
                        find_result->physical_layouts = g_list_append(find_result->physical_layouts,
                                                                      rect_copy(fr_rect));
                        ma_rect_p = ma_rect_p->next;
                    }       
                    *find_results = g_list_append(*find_results,
                                                  find_result);                        
                }
                g_list_free(matched_rects);                    
                rect_p = rect_p->next;
            }
        }
        GList *list_p = list_of_rects;
        while(list_p){
            GList *rects = list_p->data;
            GList *rect_p = rects;
            while(rect_p){
                rect_free(rect_p->data);
                rect_p = rect_p->next;
            }
            g_list_free(rects);
            list_p = list_p->next;
        }
        g_list_free(list_of_rects);            
        g_strfreev(tokens);
        g_free(match);
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    g_list_free(already_matched_rects);
    if(is_dualpage){
        *find_results = g_list_sort(*find_results,
                                    compare_find_results);
        GList *single_p = (pattern[0] == '^') ? g_list_first(*find_results)
                                              : g_list_last(*find_results);
        GList *rem_list = g_list_remove_link(*find_results,
                                             single_p);
        *find_results = single_p;
        GList *list_p = rem_list;
        while(list_p){
            find_result_free(list_p->data);
            list_p = list_p->next;
        }
        g_list_free(rem_list);
    }
}

static GList *
find_text(const char *find_term,
          int         start_page,
          int         pages_length,
          gboolean    is_dualpage,
          gboolean    is_whole_words)
{ 
    if(!d.metae){
        return NULL;
    }   
    GError *err = NULL;  
    GRegex *nl_regex = g_regex_new(
        "\\R+",
        0,
        0,
        &err);
    if(!nl_regex){
        g_print("nl_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    } 
    char *no_nl_term = g_regex_replace(nl_regex,
                                       find_term,
                                       -1,
                                       0,
                                       " ",
                                       0,
                                       NULL);
    GRegex *trimmer_regex = g_regex_new(
        "^\\s+|\\s+$",
        0,
        0,
        &err);
    if(!trimmer_regex){
        g_print("trimmer_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }
    char *cleaned_term = g_regex_replace(trimmer_regex,
                                         no_nl_term,
                                         -1,
                                         0,
                                         "",
                                         0,
                                         NULL);
    g_free(no_nl_term);
    g_regex_unref(nl_regex);
    g_regex_unref(trimmer_regex);
    gboolean term_has_whitespace = g_regex_match_simple("\\s",
                                                        cleaned_term,
                                                        0,
                                                        0);
    char *temp = cleaned_term;
    cleaned_term = g_regex_escape_string(temp,
                                         -1);
    g_free(temp);
    int term_len = strlen(cleaned_term);
    GString *dashed_term = g_string_sized_new(term_len * 7);
    for(int i = 0; i < strlen(cleaned_term); i++){
        dashed_term = g_string_append_c(dashed_term,
                                        cleaned_term[i]);
        if((cleaned_term[i] != '\\') && (i < strlen(cleaned_term) - 1)){             
            dashed_term = g_string_append(dashed_term,
                                          cleaned_term[i] != '-' ? "(-\\R+)?"
                                                                 : "\\R+");
        }
    }   
    GRegex *whitespace_regex = g_regex_new(
        "\\s+",
        0,
        0,
        &err);
    if(!whitespace_regex){
        g_print("whitespace_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }    
    err = NULL;
    char *pattern = g_regex_replace_literal(whitespace_regex,
                                            dashed_term->str,
                                            -1,
                                            0,
                                            "(\\R|\\s)+",
                                            0,
                                            &err);
    if(err){
        g_print("whitespace_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);   
    }
    g_regex_unref(whitespace_regex);
    g_string_free(dashed_term,
                  TRUE);
    if(is_whole_words){
        temp = pattern;
        pattern = g_strdup_printf("\\b%s\\b",
                                  pattern);
        g_free(temp);
    }
    err = NULL;
    GRegex *multiline_regex = g_regex_new(
        pattern,
        G_REGEX_CASELESS,
        0,
        &err);
    if(!multiline_regex){
        g_print("multiline_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }          
    
    GList *find_results = NULL;
    for(int page_num = start_page; page_num < start_page + pages_length; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);              
        /* 1: tokenize the term with word-wraps(dashes followd by newline)" and try again.
              if 'adios' is requested, we try to find
              {'a-\ndios', 'ad-\niso', 'adi-\nos', 'adio-\ns'}           
           2: each whitespace might be a newline so we suppose whitespaces are newlines and try again.
              if 'adios pegasus camus' is requested, we try to find
              {'adios\npegasus camus', 'adios\npegasus\ncamus', 'adios pegasus\ncamus'}
        */  
        if(strlen(meta->text) >= term_len){
            GList *results = NULL;
            find_rects_of_text(meta,
                               multiline_regex,
                               FALSE,
                               is_whole_words,
                               &results);
            find_results = g_list_concat(find_results,
                                         results);
        }
    } 
    g_regex_unref(multiline_regex);
    g_free(pattern);    
    if(!is_dualpage || !term_has_whitespace){
        g_free(cleaned_term);
        return find_results;
    } 
    GList *multipage_prefix_regex_list = NULL;
    GList *multipage_postfix_regex_list = NULL;
    char **tokens = g_regex_split_simple("\\s+",
                                         cleaned_term,
                                         0,
                                         0);
    int num_tokens = g_strv_length(tokens);
    for(int i = 0; i < num_tokens - 1; i++){
        GString *prefix_pattern = g_string_new("");
        for(int j = 0; j <= i; j++){   
            prefix_pattern = g_string_append(prefix_pattern,
                                             *(tokens + j));
            if(j < i){
                prefix_pattern = g_string_append(prefix_pattern,
                                                 "(\\R|\\s)+");
            }
        }
        if(is_whole_words){
            prefix_pattern = g_string_prepend(prefix_pattern,
                                              "\\b");
        }
        prefix_pattern = g_string_append(prefix_pattern,
                                         "$");
        err = NULL;
        GRegex *prefix_regex = g_regex_new(
            prefix_pattern->str,
            G_REGEX_CASELESS | G_REGEX_MULTILINE,
            0,
            &err);
        if(!prefix_regex){
            g_print("prefix_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                    err->domain, err->code, err->message);
        }    
        multipage_prefix_regex_list = g_list_append(multipage_prefix_regex_list,
                                                    prefix_regex);
        g_string_free(prefix_pattern,
                      FALSE);  

        GString *postfix_pattern = g_string_new("^");
        for(int k = i + 1; k < num_tokens; k++){
            postfix_pattern = g_string_append(postfix_pattern,
                                              *(tokens + k));
            if(k < num_tokens - 1){
                postfix_pattern = g_string_append(postfix_pattern,
                                                  "(\\R|\\s)+");
            }
        }
        if(is_whole_words){
            postfix_pattern = g_string_append(postfix_pattern,
                                               "\\b");
        }
        err = NULL;
        GRegex *postfix_regex = g_regex_new(
            postfix_pattern->str,
            G_REGEX_CASELESS | G_REGEX_MULTILINE,
            0,
            &err);
        if(!postfix_regex){
            g_print("postfix_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                    err->domain, err->code, err->message);
        }
        multipage_postfix_regex_list = g_list_append(multipage_postfix_regex_list,
                                                     postfix_regex);
        g_string_free(postfix_pattern,
                      FALSE);
    }   
    if(g_list_length(multipage_prefix_regex_list) != g_list_length(multipage_postfix_regex_list)){
        g_print("the number of prefix and postfix regexes are not the same.\n");
    }
    /* 3: multipage search. a page might end with some terms and the next page begin with the 
          rest of the terms.
          if 'adios pegasus camus' is requested, we try to find the first set of terms in the
          current page and the rest of the terms in the next page.
    */
    int multipage_regex_num = g_list_length(multipage_prefix_regex_list);
    for(int page_num = start_page; page_num < start_page + pages_length - 1; page_num++){
        PageMeta *meta_prefix = g_ptr_array_index(d.metae,
                                                  page_num);
        PageMeta *meta_postfix = g_ptr_array_index(d.metae,
                                                   page_num + 1);
        for(int i = 0; i < multipage_regex_num; i++){
            GRegex *prefix_regex = g_list_nth_data(multipage_prefix_regex_list,
                                                   i);
            GList *find_results_prefix = NULL;       
            find_rects_of_text(meta_prefix,
                               prefix_regex,
                               TRUE,
                               is_whole_words,
                               &find_results_prefix);            
            if(find_results_prefix){         
                FindResult *find_result_prefix = find_results_prefix->data;
                /* make sure prefix result does not intersect previous find results */
                GList *already_p = find_results;
                while(already_p){
                    FindResult *fr_already = already_p->data;
                    if((fr_already->page_num == meta_prefix->page_num) &&
                       rect_lists_intersect(fr_already->physical_layouts,
                                            find_result_prefix->physical_layouts))
                    {
                        break;
                    }
                    already_p = already_p->next;
                }
                if(already_p){
                    find_result_free(find_result_prefix);
                    g_list_free(find_results_prefix);
                    continue;
                }            

                GRegex *postfix_regex = g_list_nth_data(multipage_postfix_regex_list,
                                                        i);
                GList *find_results_postfix = NULL;
                find_rects_of_text(meta_postfix,
                                   postfix_regex,
                                   TRUE,
                                   is_whole_words,
                                   &find_results_postfix);                
                if(find_results_postfix){
                    FindResult *find_result_postfix = find_results_postfix->data;
                    /* make sure postfix result does not intersect previous find results */
                    GList *already_p = find_results;
                    while(already_p){
                        FindResult *fr_already = already_p->data;
                        if((fr_already->page_num == meta_postfix->page_num) &&
                           rect_lists_intersect(fr_already->physical_layouts,
                                                find_result_postfix->physical_layouts))
                        {
                            break;
                        }
                        already_p = already_p->next;
                    }
                    if(already_p){
                        find_result_free(find_result_prefix);
                        g_list_free(find_results_prefix);
                        find_result_free(find_result_postfix);
                        g_list_free(find_results_postfix);
                        continue;
                    }
                    find_result_prefix->page_postfix = find_result_postfix;
                    find_result_postfix->page_prefix = find_result_prefix;                    
                    find_results = g_list_append(find_results,
                                                 find_result_prefix);
                    find_results = g_list_append(find_results,
                                                 find_result_postfix);
                    g_list_free(find_results_prefix);
                    g_list_free(find_results_postfix);
                    /*g_print("multipage find result: %d: '%s', %d: '%s'\n",
                            page_num, find_result_prefix->match,
                            page_num + 1, find_result_postfix->match);*/
                }
                else{
                    find_result_free(find_result_prefix);                                       
                    g_list_free(find_results_prefix);
                }
            }
        }
    }
    GList *list_p = multipage_prefix_regex_list;
    while(list_p){
        g_regex_unref(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(multipage_prefix_regex_list);
    list_p = multipage_postfix_regex_list;
    while(list_p){
        g_regex_unref(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(multipage_postfix_regex_list);
    g_free(cleaned_term);        
    return find_results;
}

static void
load_text_layouts(PageMeta *meta)
{
    PopplerRectangle *phys_layouts = NULL;
    poppler_page_get_text_layout(meta->page,
                                 &phys_layouts,
                                 &meta->num_layouts);
    meta->physical_text_layouts = g_ptr_array_sized_new(meta->num_layouts);
    meta->mean_line_height = 0.0;
    for(int r = 0; r < meta->num_layouts; r++){
        Rect *phys_layout = rect_from_poppler_rectangle(&phys_layouts[r]);
        meta->mean_line_height += phys_layout->y2 - phys_layout->y1;
        g_ptr_array_add(meta->physical_text_layouts,
                        phys_layout);
    }
    if(meta->num_layouts > 0){
        meta->mean_line_height /= meta->num_layouts;
    }
    g_free(phys_layouts);
}

static void
load_links(PageMeta *meta,
           int       page_num)
{
    meta->links = NULL;
    GList *link_mappings = poppler_page_get_link_mapping(meta->page);
    GList *link_p = link_mappings;
    while(link_p){
        PopplerLinkMapping *link_mapping = link_p->data;
        Link *link = g_malloc(sizeof(Link));
        /* flip and swap y-coordinates */
        link->physical_layout = rect_new();
        link->physical_layout->x1 = link_mapping->area.x1;            
        link->physical_layout->y1 = meta->page_height - link_mapping->area.y2;
        link->physical_layout->x2 = link_mapping->area.x2;
        link->physical_layout->y2 = meta->page_height - link_mapping->area.y1;
        link->is_hovered = FALSE;
        link->tip = NULL;
        link->target_page_num = -1;
        switch(link_mapping->action->type){
            case POPPLER_ACTION_GOTO_DEST:
            {
                PopplerActionGotoDest *goto_dest_action = (PopplerActionGotoDest*)
                    link_mapping->action;
                TOCItem target = toc_find_dest(d.doc,
                                               goto_dest_action->dest);
                link->target_page_num = target.page_num;
                link->target_progress_x = target.offset_x;
                link->target_progress_y = target.offset_y;
                link->tip = g_strdup_printf("Page '%d'",
                                            target.page_num);
                break;
            }
            case POPPLER_ACTION_GOTO_REMOTE:
            {
                PopplerActionGotoRemote *remote_action = (PopplerActionGotoRemote*)
                    link_mapping->action;
                link->tip = g_markup_printf_escaped("Open '%s'",
                                                   remote_action->file_name);
                break;
            }
            case POPPLER_ACTION_URI:
            {
                PopplerActionUri *uri_action = (PopplerActionUri*)
                    link_mapping->action;
                link->tip = g_markup_printf_escaped("URI '%s'",
                                                   uri_action->uri);
                break;
            }
            default:;
        }
        meta->links = g_list_append(meta->links,
                                    link);
        link_p = link_p->next;
    }
    poppler_page_free_link_mapping(link_mappings);
}

static void
load_units(void)
{
    for(int page_num = 0; page_num < d.num_pages; page_num++){ 
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        convert_units(meta->text,
                      &meta->converted_units);
        GList *list_p = meta->converted_units;
        while(list_p){
            ConvertedUnit *cv = list_p->data; 
            GList *find_results = find_text(cv->whole_match,
                                            page_num,
                                            1,
                                            FALSE,
                                            FALSE);
            GList *cur_result_p = find_results;
            while(cur_result_p){
                gboolean intersects = FALSE;
                FindResult *fr_cur = cur_result_p->data;
                GList *rect_cur_p = fr_cur->physical_layouts;
                while(rect_cur_p){
                    Rect *rect_cur = rect_cur_p->data;
                    GList *cv_prev_p = list_p->prev;
                    while(cv_prev_p){
                        ConvertedUnit *cv_prev = cv_prev_p->data;
                        GList *prev_result_p = cv_prev->find_results;
                        while(prev_result_p){
                            FindResult *fr_prev = prev_result_p->data;
                            GList *rect_prev_p = fr_prev->physical_layouts;
                            while(rect_prev_p){
                                Rect *rect_prev = rect_prev_p->data;
                                if(rects_have_intersection(rect_prev,
                                                           rect_cur))
                                {
                                    intersects = TRUE;
                                    break;
                                }
                                rect_prev_p = rect_prev_p->next;
                            }
                            if(intersects){
                                break;
                            }
                            prev_result_p = prev_result_p->next;
                        }
                        if(intersects){
                            break;
                        }
                        cv_prev_p = cv_prev_p->prev;
                    }
                    if(intersects){
                        break;
                    }
                    rect_cur_p = rect_cur_p->next;
                }
                GList *next = cur_result_p->next;
                if(!intersects){                      
                    cv->find_results = g_list_append(cv->find_results,
                                                     fr_cur);
                    find_results = g_list_remove_link(find_results,
                                                      cur_result_p);
                    g_list_free(cur_result_p);
                }
                cur_result_p = next;
            }
            cur_result_p = find_results;
            while(cur_result_p){
                find_result_free(cur_result_p->data);
                cur_result_p = cur_result_p->next;
            }
            g_list_free(find_results);
            GList *next = list_p->next;
            if(!cv->find_results){
                meta->converted_units = g_list_remove_link(meta->converted_units,
                                                           list_p);
                g_list_free(list_p);
                converted_unit_free(cv);
            }            
            list_p = next;
        }
    }
}

static void 
load_toc(void)
{
    toc_create(d.doc,
               &d.toc.head_item,
               &d.toc.max_depth);
    if(d.toc.head_item){
        char *title = poppler_document_get_title(d.doc);
        if(title){
            if(strlen(title) == 0){
                g_free(title);
                title = g_strdup("Head");
            }
        }
        else{
            title = g_strdup("Head");
        }
        d.toc.head_item->title = title;
    }
    d.toc.flattened_items = NULL;    
    toc_flatten_items(d.toc.head_item,
                      &d.toc.flattened_items);
    d.toc.labels = NULL;
    toc_get_labels(d.toc.head_item,
                   &d.toc.labels);
    d.toc.navigation_button_rects = NULL;
    for(int i = 0; i < g_list_length(d.toc.labels) * 2; i++){
        d.toc.navigation_button_rects = g_list_append(d.toc.navigation_button_rects,
                                                      rect_new());       
    }    
    d.toc.origin_x = 0;
    d.toc.origin_y = 0;
    d.toc.where = NULL;
}

static int 
compare_image_mappings(const void *a,
                       const void *b)
{
    int comp;
    const PopplerImageMapping *image_a = a;
    const PopplerImageMapping *image_b = b;
    if((image_a->area.y1 < image_b->area.y1) ||
       (image_a->area.y1 == image_b->area.y1 &&
        image_a->area.x1 > image_b->area.x1))
    {
        comp = -1;
    }
    else{
        comp = 1;
    }
    /* desc y, asc x */
    return -comp;
}

static void
merge_images(GList *image_mappings,
             PageMeta *meta)
{
    /* images that are center(h/v) aligned, might probably be reffered to
       by the same caption. */
    static const double MERGING_AREA_COEFF = 0.9;
    GList *image_p = image_mappings;
    while(image_p){
        PopplerImageMapping *img = image_p->data;
        double img_width = img->area.x2 - img->area.x1,
               img_height = img->area.y2 - img->area.y1,
               img_center_y = img->area.y1 + img_height / 2;
        PopplerRectangle bounding_rect = img->area;
        if(image_p->next){
            GList *image_v = image_p->next;
            PopplerImageMapping *o_img = image_v->data;
            double o_img_width = o_img->area.x2 - o_img->area.x1,
                   o_img_height = o_img->area.y2 - o_img->area.y1,
                   o_img_center_y = o_img->area.y1 + o_img_height / 2;
            bounding_rect.x1 = o_img->area.x1 < bounding_rect.x1 ? o_img->area.x1
                                                                 : bounding_rect.x1;
            bounding_rect.y1 = o_img->area.y1 < bounding_rect.y1 ? o_img->area.y1
                                                                 : bounding_rect.y1;
            bounding_rect.x2 = o_img->area.x2 > bounding_rect.x2 ? o_img->area.x2
                                                                 : bounding_rect.x2;
            bounding_rect.y2 = o_img->area.y2 > bounding_rect.y2 ? o_img->area.y2
                                                                 : bounding_rect.y2;
            double bounding_area = fabs(bounding_rect.x2 - bounding_rect.x1) *
                                   fabs(bounding_rect.y2 - bounding_rect.y1);
            double total_area = fabs(img_width * img_height) +
                                fabs(o_img_width * o_img_height);            
            if(total_area / bounding_area > MERGING_AREA_COEFF){
                Rect space_between_rects;
                space_between_rects.x1 = MIN(img->area.x1, o_img->area.x1);
                space_between_rects.x2 = MAX(img->area.x2, o_img->area.x2);
                space_between_rects.y1 = img_center_y < o_img_center_y ? img->area.y2 : o_img->area.y2;
                space_between_rects.y2 = img_center_y < o_img_center_y ? o_img->area.y1 : img->area.y1;
                gboolean text_exists_between_rects = FALSE;
                for(int li = 0; li < meta->num_layouts; li++){
                    Rect *physical_layout = g_ptr_array_index(meta->physical_text_layouts,
                                                              li);
                    if(rect_contains_point(&space_between_rects,
                                           rect_center_x(physical_layout),
                                           rect_center_y(physical_layout)))
                    {
                        text_exists_between_rects = TRUE;
                        break;
                    }
                }
                if(!text_exists_between_rects){
                    img->area = bounding_rect;
                    image_mappings = g_list_remove_link(image_mappings,
                                                        image_v);
                    poppler_page_free_image_mapping(image_v);
                    merge_images(image_mappings,
                                 meta);
                }
            }            
        }
        image_p = image_p->next;
    }
}

static cairo_surface_t *
stich_merged_images (PageMeta *meta,
                     Figure   *figure)
{    
    /* crop the rectangle that holds the image(s) from the rendered
       page (to the maximum size obtained from the resolution of the surface)  */
    double aspect_ratio = meta->page_height / meta->page_width;
    /* @ smalles image might be better for scaling */    
    cairo_surface_t *surface = poppler_page_get_image(meta->page,
                                                      figure->image_id);
    double surface_width = cairo_image_surface_get_width(surface);
    cairo_surface_destroy(surface);
    double image_physical_width = rect_width(figure->image_physical_layout);
    double page_render_width = surface_width * (meta->page_width / image_physical_width);
    double page_render_height = page_render_width * aspect_ratio;
    cairo_surface_t *rendered_page = render_page(meta,
                                                 page_render_width,
                                                 page_render_height);
    Rect image_layout = map_physical_rect_to_image(figure->image_physical_layout,
                                                   meta->page_width, meta->page_height,
                                                   page_render_width, page_render_height,
                                                   0, 0);    
    double image_width = rect_width(&image_layout);
    double image_height = rect_height(&image_layout);
    cairo_surface_t *image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                        image_width,
                                                        image_height);
    cairo_t *cr = cairo_create(image);
    double source_x = page_render_width * (figure->image_physical_layout->x1 / meta->page_width);
    double source_y = page_render_height * (figure->image_physical_layout->y1 / meta->page_height);
    cairo_set_source_surface(cr,
                             rendered_page,
                             -source_x,
                             -source_y);
    cairo_rectangle(cr,
                    0, 0,
                    image_width, image_height);    
    cairo_fill(cr);
    cairo_surface_destroy(rendered_page);
    cairo_destroy(cr);
    return image;
}

static void
load_figures (int start_page)
{        
    gboolean labels_are_exclusive = FALSE,
             ids_are_complex = FALSE;
    GList *all_figures = NULL;
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        if(page_num < start_page){
            continue;
        }
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        GList *image_mappings = poppler_page_get_image_mapping(meta->page);
        /* tiny image = noise */
        GList *image_mappings_p = image_mappings;
        while(image_mappings_p){
            GList *next = image_mappings_p->next;
            PopplerImageMapping *img = image_mappings_p->data;
            double img_width = img->area.x2 - img->area.x1,
                   img_height = img->area.y2 - img->area.y1;
            if(fabs(img_width) < meta->mean_line_height ||
               fabs(img_height) < meta->mean_line_height)
            {
                image_mappings = g_list_remove_link(image_mappings,
                                                    image_mappings_p);
                poppler_page_free_image_mapping(image_mappings_p);
            }    
            image_mappings_p = next;
        }
        if(!image_mappings){
            continue;
        }
        GList *fig_list = extract_figure_captions(meta->text);
        if(!fig_list){
            poppler_page_free_image_mapping(image_mappings);
            continue;
        }
        // g_print("page %d:\n", page_num);
        // GList *tmp_p = fig_list;
        // while(tmp_p){
        //     Figure *fig = tmp_p->data;
        //     g_print("'%s'\n", fig->whole_match);
        //     tmp_p = tmp_p->next;
        // }
        /* merge images that have non-null inresections */
        if(image_mappings->next){
            GList *image_mappings_p = image_mappings;
            while(image_mappings_p){
                PopplerImageMapping *image_p = image_mappings_p->data;
                GList *image_mappings_v = image_mappings_p->next;
                while(image_mappings_v){
                    GList *next = image_mappings_v->next;
                    PopplerImageMapping *image_v = image_mappings_v->data;
                    Rect *rect_p = rect_from_poppler_rectangle(&image_p->area);
                    Rect *rect_v = rect_from_poppler_rectangle(&image_v->area);
                    if(rects_have_intersection(rect_p,
                                               rect_v))
                    {                
                        image_p->area.x1 = image_p->area.x1 > image_v->area.x1 ? image_v->area.x1 : image_p->area.x1;
                        image_p->area.y1 = image_p->area.y1 > image_v->area.y1 ? image_v->area.y1 : image_p->area.y1;
                        image_p->area.x2 = image_p->area.x2 < image_v->area.x2 ? image_v->area.x2 : image_p->area.x2;
                        image_p->area.y2 = image_p->area.y2 < image_v->area.y2 ? image_v->area.y2 : image_p->area.y2;
                        image_mappings = g_list_remove_link(image_mappings,
                                                            image_mappings_v);
                        poppler_page_free_image_mapping(image_mappings_v);
                    }
                    rect_free(rect_p);
                    rect_free(rect_v);
                    image_mappings_v = next;    
                }
                image_mappings_p = image_mappings_p->next;
            }            
            merge_images(image_mappings,
                         meta);            
            image_mappings = g_list_sort(image_mappings,
                                         compare_image_mappings);
        }
        meta->figures = g_hash_table_new(g_str_hash,
                                         g_str_equal);
        image_mappings_p = image_mappings;      
        while(image_mappings_p){
            PopplerImageMapping *img = image_mappings_p->data;
            double img_width = img->area.x2 - img->area.x1,
                   img_height = img->area.y2 - img->area.y1,
                   img_center_x = img->area.x1 + img_width / 2,
                   img_center_y = img->area.y1 + img_height / 2;
            GList *fig_list_p = fig_list;
            while(fig_list_p){
                Figure *figure = fig_list_p->data;
                /* once we find out that labels are exclusive, figures get filtered out */
                ids_are_complex = ids_are_complex ? TRUE : figure->is_id_complex;
                labels_are_exclusive = labels_are_exclusive ? TRUE : figure->is_label_exclusive;
                if(labels_are_exclusive && !figure->label){                    
                    figure_free(figure);
                    GList *next = fig_list_p->next;
                    fig_list = g_list_remove_link(fig_list,
                                                  fig_list_p);
                    g_list_free(fig_list_p);
                    fig_list_p = next;
                    continue;
                }
                Figure *new_figure = figure_copy(figure);
                new_figure->page_num = page_num;
                new_figure->image_id = img->image_id;
                new_figure->image_physical_layout = rect_from_poppler_rectangle(&img->area); 
                GList *results = poppler_page_find_text(meta->page,
                                                        new_figure->whole_match);
                GList *results_p = results;            
                while(results_p){
                    PopplerRectangle *caption_rect = results_p->data;
                    caption_rect->y1 = meta->page_height - caption_rect->y1;
                    caption_rect->y2 = meta->page_height - caption_rect->y2;
                    double cap_rect_center_x = caption_rect->x1 + (caption_rect->x2 - caption_rect->x1),
                           cap_rect_center_y = caption_rect->y1 + (caption_rect->y2 - caption_rect->y1);
                    Caption *caption = g_malloc(sizeof(Caption));                    
                    caption->physical_layout = rect_from_poppler_rectangle(caption_rect);
                    caption->distance_to_image = sqrt(pow(cap_rect_center_x - img_center_x, 2) + 
                                                      pow(cap_rect_center_y - img_center_y, 2));
                    new_figure->captions = g_list_append(new_figure->captions,
                                                         caption);
                    results_p = results_p->next;
                    poppler_rectangle_free(caption_rect);
                }
                g_list_free(results);
                all_figures = g_list_append(all_figures,
                                            new_figure);
                fig_list_p = fig_list_p->next;
            }                      
            image_mappings_p = image_mappings_p->next;        
        }        
        GList *list_p = fig_list;
        while(list_p){
            figure_free(list_p->data);
            list_p = list_p->next;
        }  
        g_list_free(fig_list);
        poppler_page_free_image_mapping(image_mappings);
    }
    GList *figure_p = NULL;
    if(labels_are_exclusive){  
        figure_p = all_figures;
        while(figure_p){
            GList *next = figure_p->next;
            Figure *figure = figure_p->data;       
            if(!figure->label){
                all_figures = g_list_remove_link(all_figures,
                                                 figure_p);
                g_list_free(figure_p);
                figure_free(figure);
            }
            figure_p = next;            
        }            
    }                     
    GList *figures_per_image = NULL;
    int page_num = -1,
        image_id = -1;
    figure_p = all_figures;
    while(figure_p){
        Figure *figure = figure_p->data;
        GList *list = NULL;        
        if(figure->page_num == page_num &&
           figure->image_id == image_id)
        {
            GList *last = g_list_last(figures_per_image);
            list = last->data;
            list = g_list_append(list,
                                 figure);
            last->data = list;
        }
        else{
            page_num = figure->page_num;
            image_id = figure->image_id;
            list = g_list_append(list,
                                 figure);
            figures_per_image = g_list_append(figures_per_image,
                                              list);
        }             
        figure_p = figure_p->next;
    }
    g_list_free(all_figures);
    GHashTable *processed_figures_hash_table = g_hash_table_new(g_direct_hash,
                                                                g_direct_equal);    
    GHashTable *processed_images_hash_table = g_hash_table_new(g_str_hash,
                                                               g_str_equal);
    GList *figure_list_p = figures_per_image;
    while(figure_list_p){
        double min_dist = DBL_MAX;
        Figure *closest_figure = NULL;
        Caption *closest_caption = NULL;
        figure_p = figure_list_p->data;
        while(figure_p){
            Figure *figure = figure_p->data;        
            PageMeta *meta = g_ptr_array_index(d.metae,
                                                      figure->page_num);
            if(!g_hash_table_contains(meta->figures,
                                      figure->id))
            {
                GList *caption_p = figure->captions;
                while(caption_p){
                    Caption *caption = caption_p->data;            
                    if(caption->distance_to_image < min_dist){
                        min_dist = caption->distance_to_image;
                        closest_figure = figure;
                        closest_caption = caption;
                    }
                    caption_p = caption_p->next;
                }
            }
            figure_p = figure_p->next;
        }
        if(closest_figure){    
            PageMeta *meta = g_ptr_array_index(d.metae,
                                                      closest_figure->page_num);
            closest_figure->caption_physical_layout = closest_caption->physical_layout;
            closest_figure->image = stich_merged_images(meta,
                                                        closest_figure);
            g_hash_table_insert(meta->figures,
                                closest_figure->id,
                                closest_figure);
            g_hash_table_add(processed_images_hash_table,
                             g_strdup_printf("%d-%d",
                                             closest_figure->page_num,
                                             closest_figure->image_id));
            g_hash_table_add(processed_figures_hash_table,
                             closest_figure);
            /*g_print("%d-%d: %s\n", 
                    closest_figure->page_num, closest_figure->image_id, closest_figure->whole_match); */
        }     
        else{
            /*figure_p = figure_list_p->data;
            Figure *figure = figure_p->data;
            g_print("%d-%d: ?\n", 
                    figure->page_num, figure->image_id);*/
        } 
        figure_list_p = figure_list_p->next;
    }
    GList *keys = g_hash_table_get_keys(processed_images_hash_table);
    GList *keys_p = keys;
    while(keys_p){
        g_free((char *)keys_p->data);
        keys_p = keys_p->next;
    }
    g_list_free(keys);
    g_hash_table_unref(processed_images_hash_table);
    
    figure_list_p = figures_per_image;
    while(figure_list_p){
        figure_p = figure_list_p->data;
        while(figure_p){
            Figure *unused_figure = figure_p->data;
            if(!g_hash_table_contains(processed_figures_hash_table,
                                      unused_figure))
            {   
                figure_free(unused_figure);
            }
            figure_p = figure_p->next;
        }
        g_list_free(figure_list_p->data);
        figure_list_p = figure_list_p->next;
    }
    g_list_free(figures_per_image);
    g_hash_table_unref(processed_figures_hash_table);
}

static void
resolve_referenced_figures (int start_page)
{
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        if(page_num < start_page){
            continue;
        }
        GList *ref_figures = extract_figure_references(meta->text);
        GList *list_p = ref_figures;
        while(list_p){
            ReferencedFigure *ref_figure = list_p->data;
            if(meta->figures && g_hash_table_contains(meta->figures,
                                                      ref_figure->id))
            {
                g_free(ref_figure->label);
                g_free(ref_figure->id);
                g_free(ref_figure);
                list_p = list_p->next;
                continue;
            }                    
            for(int ref_page_num = start_page; ref_page_num < d.num_pages; ref_page_num++){
                if(ref_page_num == page_num){
                    continue;
                }
                PageMeta *ref_meta = g_ptr_array_index(d.metae,
                                                       ref_page_num);
                if(!ref_meta->figures){
                    continue;
                }
                Figure *reference = g_hash_table_lookup(ref_meta->figures,
                                                        ref_figure->id);
                if(reference){
                    /*g_print("'%s'(p%d) is located on page %d\n",
                            figure->whole_match, figure->page_num, ref_figure->page_num);*/
                    char *needle = g_strdup_printf("%s %s",
                                                   ref_figure->label,
                                                   ref_figure->id);
                    ref_figure->find_results = find_text(needle,
                                                         page_num,
                                                         1,
                                                         FALSE,
                                                         FALSE);
                    g_free(needle);
                    ref_figure->reference = reference;                    
                    meta->referenced_figures = g_list_append(meta->referenced_figures,
                                                             ref_figure);
                    break;
                }
            }
            if(!ref_figure->reference){
                /*g_print("'%s'(p%d) is not referenced anywhere!\n",
                        figure->whole_match, figure->page_num);*/
                g_free(ref_figure->label);
                g_free(ref_figure->id);
                g_free(ref_figure);
            }            
            list_p = list_p->next;
        }
        g_list_free(ref_figures);
    }
}

static void
load_metae(void)
{
    d.metae = g_ptr_array_sized_new(d.num_pages);
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_malloc(sizeof(PageMeta));
        meta->page_num = page_num;
        meta->page = poppler_document_get_page(d.doc,
                                               page_num);        
        meta->page_label = poppler_page_get_label(meta->page);        
        if(meta->page_label){
            meta->reading_progress_text = g_strdup_printf("<span font='sans 12' foreground='#222222'><i>%s</i> of %d</span>",
                                                          meta->page_label,
                                                          d.num_pages);
        }
        else{
            meta->reading_progress_text = g_strdup_printf("<span font='sans 12' foreground='#222222'><i>%d</i> of %d</span>",
                                                          page_num + 1,
                                                          d.num_pages);
        }        
        poppler_page_get_size(meta->page,
                              &meta->page_width,
                              &meta->page_height);
        meta->aspect_ratio = meta->page_height / meta->page_width;
        meta->text = poppler_page_get_text(meta->page);
        meta->num_layouts = 0;
        meta->physical_text_layouts = NULL;
        load_text_layouts(meta);
        meta->links = NULL;
        load_links(meta,
                   page_num); 
        meta->converted_units = NULL;
        meta->figures = NULL;
        meta->referenced_figures = NULL;
        meta->active_referenced_figure = NULL;
        meta->find_results = NULL;     
        g_ptr_array_add(d.metae,
                        meta);
    }
}

static void
zero_document(void)
{
    d.filename = NULL;
    d.doc = NULL; 
    d.doc_info.book_info_label = NULL;
    d.doc_info.book_info_data = NULL;
    d.metae = NULL;
    d.image = NULL;
    d.image_origin_x = 0.0;
    d.image_origin_y = 0.0;
    d.preserved_progress_x = 0.0;
    d.preserved_progress_y = 0.0;
    d.num_pages = -1;
    d.cur_page_num = -1;
    /*d.zoom_level = PageFit;*/
    d.toc.head_item = NULL;
    d.toc.max_depth = 0;
    d.toc.flattened_items = NULL;
    d.toc.where = NULL;
    d.toc.origin_x = 0;
    d.toc.origin_y = 0;
    d.find_details.selected_p = NULL;
    d.find_details.find_results = NULL;    
}

static void
setup_text_completions(void)
{
    GList *text_list = NULL;
    /* page num/label */
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        char *page_label = meta->page_label ? g_strdup(meta->page_label)
                                            : g_strdup_printf("%d",
                                                              page_num + 1);
        text_list = g_list_append(text_list,
                                  page_label);
    }
    teleport_widget_update_text_completions(text_list,
                                            "Page");
    g_list_free(text_list);
    text_list = NULL;
    /* navigation commands */
    text_list = g_list_append(text_list,
                              g_strdup("next page")); 
    text_list = g_list_append(text_list,
                              g_strdup("previous page"));
    GList *list_p = d.toc.labels;
    while(list_p){
        char *toc_label = list_p->data;
        text_list = g_list_append(text_list,
                                  g_strdup_printf("next %s",
                                                  toc_label));
        text_list = g_list_append(text_list,
                                  g_strdup_printf("previous %s",
                                                  toc_label));
        list_p = list_p->next;
    }
    teleport_widget_update_text_completions(text_list,
                                            "Navigation");
    g_list_free(text_list);
    text_list = NULL;
    /* toc items */
    list_p = d.toc.flattened_items;
    while(list_p){
        TOCItem *toc_item = list_p->data;
        if(toc_item->depth > 1){
            if(toc_item->id){
                text_list = g_list_append(text_list,
                                          g_strdup_printf("%s %s",
                                                          toc_item->label,
                                                          toc_item->id));
            }
            text_list = g_list_append(text_list,
                                      g_strdup(toc_item->title));
        }
        list_p = list_p->next;
    }
    teleport_widget_update_text_completions(text_list,
                                            "TOC");
    g_list_free(text_list);
    text_list = NULL;
    /* figures */
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        if(!meta->figures){
            continue;
        }
        GList *figure_list = g_hash_table_get_values(meta->figures);
        list_p = figure_list;
        while(list_p){
            Figure *figure = list_p->data;
            char *text = g_strdup_printf("%s %s",
                                         figure->label ? figure->label : "Figure",
                                         figure->id);
            text_list = g_list_append(text_list,
                                      text);
            list_p = list_p->next;
        }
        g_list_free(figure_list);
    }
    teleport_widget_update_text_completions(text_list,
                                            "Figure");
    g_list_free(text_list);
}

static void
destroy_document(void)
{
    if(!d.metae){
        return;
    }
    /*save_state();*/
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        /* text layouts */
        for(int li = 0; li < meta->num_layouts; li++){
            rect_free(g_ptr_array_index(meta->physical_text_layouts,
                                        li));
        }
        g_ptr_array_unref(meta->physical_text_layouts);
        /* text */
        g_free(meta->text);
        /* links */
        GList *list_p = meta->links;
        while(list_p){
            Link *link = list_p->data;
            rect_free(link->physical_layout);
            g_free(link->tip);
            g_free(link);
            list_p = list_p->next;
        }
        g_list_free(meta->links);
        /* find resutlts */
        g_list_free(meta->find_results);
        /* units */
        list_p = meta->converted_units;
        while(list_p){
            ConvertedUnit *cv = list_p->data;
            converted_unit_free(cv);
            list_p = list_p->next;
        }
        g_list_free(meta->converted_units);
        /* figures */
        if(meta->figures){
            GHashTableIter iter;
            gpointer key, value;
            g_hash_table_iter_init (&iter, meta->figures);
            while(g_hash_table_iter_next(&iter, &key, &value)){
                Figure *figure = value;
                cairo_surface_destroy(figure->image);
                figure_free(figure);
            }
            g_hash_table_unref(meta->figures);
        }
        /* referenced figures */
        list_p = meta->referenced_figures;
        while(list_p){
            ReferencedFigure *ref_figure = list_p->data;
            g_free(ref_figure->id);
            g_free(ref_figure->label);
            GList *result_p = ref_figure->find_results;
            while(result_p){
                find_result_free(result_p->data);
                result_p = result_p->next;
            }
            g_list_free(ref_figure->find_results);
            g_free(ref_figure);
            list_p = list_p->next;
        }
        g_list_free(meta->referenced_figures);
        g_object_unref(meta->page);        
        g_free(meta->page_label);
        g_free(meta->reading_progress_text);
        g_free(meta);
    }
    g_ptr_array_unref(d.metae);
    cairo_surface_destroy(d.image);
    GList *list_p = d.find_details.find_results;
    while(list_p){
        find_result_free(list_p->data);
        list_p = list_p->next;
    }
    toc_destroy(d.toc.head_item);
    list_p = d.toc.labels;
    while(list_p){
        g_free((char*)list_p->data);
        list_p = list_p->next;
    }
    list_p = d.toc.navigation_button_rects;
    while(list_p){
        rect_free(list_p->data);
        list_p = list_p->next;
    }    
    g_list_free(d.toc.flattened_items);
    g_list_free(d.find_details.find_results);
    if(d.go_back_stack){
        gpointer *p = g_queue_pop_head(d.go_back_stack);
        while(p){
            g_free((GoBack*)p);
            p = g_queue_pop_head(d.go_back_stack);
        }
    }
    g_free(d.filename);    
    g_free(d.doc_info.book_info_label);
    g_free(d.doc_info.book_info_data);
    g_object_unref(d.doc);
    teleport_widget_destroy_text_completions();
    zero_document();
    gtk_widget_queue_draw(ui.vellum);
}

static void 
import_pdf(void)
{
    char *fn;
    GtkWidget *dialog = gtk_file_chooser_dialog_new ("Choose a book(*.pdf)",
                                                     GTK_WINDOW(ui.main_window),
                                                     GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     "_Cancel",
                                                     GTK_RESPONSE_CANCEL,
                                                     "_Open",
                                                     GTK_RESPONSE_ACCEPT,
                                                     NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                        "/home/reza/Desktop");
    GtkFileFilter *file_filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(file_filter,
                                "*.pdf");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog),
                                file_filter);
    int res = gtk_dialog_run (GTK_DIALOG(dialog));
    if(res != GTK_RESPONSE_ACCEPT){ 
        gtk_widget_destroy(dialog);
        return;
    }
    fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    gtk_widget_destroy(dialog);

    if(d.filename
       && g_regex_match_simple(d.filename,
                               fn,
                               0, 0)){
        g_free(fn);
        return;
    }   

    destroy_document();

    GError *err = NULL;
    char *uri = g_filename_to_uri(fn,
                                  NULL,
                                  NULL);
    d.doc = poppler_document_new_from_file(uri,
                                           NULL,
                                           &err);
    g_free(uri);
    if(!d.doc){
        g_print("Failed to load document.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        g_free(d.filename);
        return;
    }
    d.filename = fn;
    d.num_pages = poppler_document_get_n_pages(d.doc);
    char *num_pages_str = g_strdup_printf("%d",
                                      d.num_pages);
    char *title = poppler_document_get_title(d.doc);
    char *author = poppler_document_get_author(d.doc);
    char *subject = poppler_document_get_subject(d.doc);
    char *format = poppler_document_get_pdf_version_string(d.doc); 
    d.doc_info.book_info_label = g_strdup("<span font='sans 10' foreground='blue'>Location\n\nTitle\n\nAuthor\n\nSubject\n\nFormat\n\nPages</span>");
    d.doc_info.book_info_data = g_strdup_printf("<span font='sans 10' foreground='#222222'>%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s</span>",
                                                d.filename,
                                                title ? title : "N/A",
                                                author ? author : "N/A",
                                                subject ? subject : "N/A",
                                                format ? format : "N/A",
                                                num_pages_str);
    g_free(title);
    g_free(author);
    g_free(subject);
    g_free(format);
    g_free(num_pages_str);
    g_print("Importing '%s':\n",
            d.filename);
    load_metae();
    g_print("Loading TOC...\n");
    load_toc();
    g_print("Converting units...\n");
    load_units();
    int main_contents_start_page = 0;
    g_print("Loading figures...\n");
    load_figures(main_contents_start_page);
    resolve_referenced_figures(main_contents_start_page);
    setup_text_completions();
    g_print("Document is ready.\n");    
    ui.app_mode = ReadingMode;
    goto_page(0,
              0, 0);    
}

static void
go_back_save(void)
{
    GoBack *go_back = g_malloc(sizeof(GoBack));
    go_back->page_num = d.cur_page_num;
    go_back->progress_x = fabs(d.image_origin_x) / cairo_image_surface_get_width(d.image);
    go_back->progress_y = fabs(d.image_origin_y) / cairo_image_surface_get_height(d.image);
    g_queue_push_head(d.go_back_stack,
                      go_back);  
}

static void
go_back_restore(void)
{
    if(g_queue_is_empty(d.go_back_stack)){
        g_print("Nothing to go back to.\n");
        return;
    }
    GoBack *go_back = g_queue_pop_head(d.go_back_stack);    
    goto_page(go_back->page_num,
              go_back->progress_x, go_back->progress_y);
    g_free(go_back);
}

static void 
activate_link (Link *link)
{    
    if(link->target_page_num >= 0){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           link->target_page_num);
        go_back_save();
        double progress_x = link->target_progress_x / meta->page_width;
        double progress_y = (meta->page_height - link->target_progress_y) / meta->page_height;
        d.preserved_progress_x = progress_x;
        d.preserved_progress_y = progress_y;
        goto_page(link->target_page_num,
                  progress_x, progress_y);
    }
    else{
        g_print("link: %s\n",
                link->tip);
    }
}

static void
goto_toc_item_page(const TOCItem *toc_item)
{
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       toc_item->page_num < 0 ? 0 : toc_item->page_num);
    go_back_save();
    double progress_x = toc_item->offset_x / meta->page_width;
    double progress_y = (meta->page_height - toc_item->offset_y) / meta->page_height;
    d.preserved_progress_x = progress_x;
    d.preserved_progress_y = progress_y;
    if(toc_item->page_num != d.cur_page_num){
        goto_page(toc_item->page_num,
                  progress_x, progress_y);
    }
    else{
        scale_page(d.zoom_level, 
                   TRUE,
                   progress_x, progress_y);
    }
}

static void
goto_figure_page(const Figure *figure)
{
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       figure->page_num);
    double progress_x = figure->image_physical_layout->x1 / meta->page_width;
    double progress_y = figure->image_physical_layout->y1 / meta->page_height;
    d.preserved_progress_x = progress_x;
    d.preserved_progress_y = progress_y;
    if(figure->page_num != d.cur_page_num){
        goto_page(figure->page_num,
                  progress_x, progress_y);
    }
    else{
        scale_page(d.zoom_level, 
                   TRUE,
                   progress_x, progress_y);
    }
}

static void
previous_subsection(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Subsection){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *subsection_item = item_p->data;        
        if(subsection_item->previous){            
            goto_toc_item_page(subsection_item->previous);
        }
        else{
            g_print("No subsections before.\n");
        }
    }
    g_list_free(where);    
}

static void
next_subsection(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Subsection){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *subsection_item = item_p->data;
        TOCItem *next_subsection_item = subsection_item->next;
        while(next_subsection_item){
            if(next_subsection_item->page_num != subsection_item->page_num){
                break;
            }
            next_subsection_item = next_subsection_item->next;
        }
        if(next_subsection_item){
            goto_toc_item_page(next_subsection_item);
        }
        else{
            g_print("No more subsections ahead.\n");
        }
    }
    g_list_free(where);
}

static void
previous_section(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Section){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *section_item = item_p->data;
        if(section_item->previous){
            goto_toc_item_page(section_item->previous);
        }
        else{
            g_print("No sections before.\n");
        }
    }
    g_list_free(where);    
}

static void
next_section(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Section){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *section_item = item_p->data;
        TOCItem *next_section_item = section_item->next;
        while(next_section_item){
            if(next_section_item->page_num != section_item->page_num){
                break;
            }
            next_section_item = next_section_item->next;
        }       
        if(next_section_item){
            goto_toc_item_page(next_section_item);
        }
        else{
            g_print("No more sections ahead.\n");
        }
    }
    
    g_list_free(where);
}

static void
previous_chapter(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Chapter){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *chapter_item = item_p->data;
        if(chapter_item->previous){
            goto_toc_item_page(chapter_item->previous);
        }
        else{
            g_print("No chapters before.\n");
        }
    }
    g_list_free(where);    
}

static void
next_chapter(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Chapter){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *chapter_item = item_p->data;
        TOCItem *next_chapter_item = chapter_item->next;
        while(next_chapter_item){
            if(next_chapter_item->page_num != chapter_item->page_num){
                break;
            }
            next_chapter_item = next_chapter_item->next;
        }
        if(next_chapter_item){
            goto_toc_item_page(next_chapter_item);
        }
        else{
            g_print("No more chapters ahead.\n");
        }
    }
    g_list_free(where);
}

static void
previous_part(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Part){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *part_item = item_p->data;
        if(part_item->previous){
            goto_toc_item_page(part_item->previous);
        }
        else{
            g_print("No parts before.\n");
        }
    } 
    g_list_free(where);    
}

static void
next_part(void)
{
    if(d.toc.max_depth == 0){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    /* initium and finis have no label */
    TOCItem *main_contents_item = where->data;    
    if(!main_contents_item->label){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Part){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *part_item = item_p->data;
        TOCItem *next_part_item = part_item->next;
        while(next_part_item){
            if(next_part_item->page_num != part_item->page_num){
                break;
            }
            next_part_item = next_part_item->next;
        }
        if(next_part_item){
            goto_toc_item_page(next_part_item);
        }
        else{
            g_print("No more parts ahead.\n");
        }
    }
    g_list_free(where);
}

static void
teleport(const char *term)
{
    char *object_name = g_strdup(term);
    g_strstrip(object_name);
    /* object is page label */
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        if(meta->page_label){
           char *escaped = g_regex_escape_string(object_name,
                                                 -1);
           char *pattern = g_strdup_printf("^%s$",
                                           escaped);
           g_free(escaped);
           if(g_regex_match_simple(pattern,
                                   meta->page_label,
                                   0, 0))
           {
               go_back_save();
               goto_page(page_num,
                         0, 0);
               g_free(pattern);
               g_free(object_name);
               return;
           }
            g_free(pattern);
        }
    }
    /* object is page number(index) */
    if(g_regex_match_simple("^\\d+$",
                            object_name,
                            0, 0))
    {
        int page_num = g_ascii_strtoll(object_name,
                                       NULL,
                                       10);
        if(page_num){
            go_back_save();
            goto_page(page_num,
                      0, 0);
        }
        g_free(object_name);
        return;
    }
    /* object is a navigation request: next/prev page, part, chapter, etc... */
    GMatchInfo *match_info = NULL;
    g_regex_match(navigation_regex,
                  object_name,
                  0,
                  &match_info);
    if(g_match_info_matches(match_info)){
        go_back_save();
        char *command = g_match_info_fetch_named(match_info,
                                                 "command");
        gboolean go_next = g_regex_match_simple("next",
                                                command,
                                                0, 0);
        g_free(command);
        char *label = g_match_info_fetch_named(match_info,
                                               "label");
        enum TOCType toc_type = toc_get_item_type (label);
        g_free(label);
        switch(toc_type){
            case Part:
                if(go_next){
                    next_part();
                }
                else{
                    previous_part();
                }
                break;
            case Chapter:
                if(go_next){
                    next_chapter();
                }
                else{
                    previous_chapter();
                }
                break;
            case Section:
                if(go_next){
                    next_section();
                }
                else{
                    previous_section();
                }
                break;
            case Subsection:
                if(go_next){
                    next_subsection();
                }
                else{
                    previous_subsection();
                }
                break;
            default:
                if(go_next){
                    next_page();
                }
                else{
                    previous_page();
                }
                break;
        }
    }
    g_match_info_free(match_info);
    /* object is toc_item, e.g. chapter 1.2 */
    if(d.toc.max_depth > 0){
        gboolean found = FALSE;
        GList *items = toc_extract_items(object_name);
        if(items){
            TOCItem *toc_item = items->data;
            const TOCItem *target_item = toc_search_by_id(d.toc.head_item,
                                                          toc_item->label, 
                                                          toc_item->id);
            if(target_item){
                goto_toc_item_page(target_item);
                found = TRUE;
            }
            g_free(toc_item->label);
            g_free(toc_item->id);
            g_free(toc_item);
            g_list_free(items);
        }
        else{
            /* object is non-sequencible toc_item: e.g. notes, bibliography, ... */
            const TOCItem *target_item = toc_search_by_title(d.toc.head_item,
                                                             object_name);
            if(target_item){
                goto_toc_item_page(target_item);
                found = TRUE;
            }
        }
        if(found){
            g_free(object_name);
            return;
        }
    }    
    /* object is figure */
    GList *ref_figure_list = extract_figure_references(object_name);
    if(ref_figure_list){
        ReferencedFigure *ref_figure = ref_figure_list->data;
        int page_num;
        for(page_num = 0; page_num < d.num_pages; page_num++){
            PageMeta *meta = g_ptr_array_index(d.metae,
                                               page_num);
            if(!meta || !meta->figures){
                continue;
            }
            Figure *target_figure = g_hash_table_lookup(meta->figures,
                                                        ref_figure->id);
            if(target_figure && are_figure_labels_equal(ref_figure->label,
                                                        target_figure->label))
            {
                go_back_save();
                goto_figure_page(target_figure);
                break;
            }
        }
        GList *list_p = ref_figure_list;
        while(list_p){
            ReferencedFigure *ref_figure_del = list_p->data;
            g_free(ref_figure_del->id);
            g_free(ref_figure_del->label);
            g_free(ref_figure_del);
            list_p = list_p->next;
        }
        g_list_free(ref_figure_list);
        g_free(object_name);
    }
}

static void
on_teleport_request_received(GtkWidget *sender,
                             gpointer   user_data)
{
    if(!d.metae){
        return;
    }
    teleport(user_data);
}

static void
find_next(void)
{    
    if(!d.find_details.find_results){
        return;
    }
    if(d.find_details.selected_p){
        d.find_details.selected_p = d.find_details.selected_p->next;
    }
    if(!d.find_details.selected_p){
        d.find_details.selected_p = g_list_first(d.find_details.find_results);
    }
    FindResult *find_result = d.find_details.selected_p->data;            
    Rect *first_rect = find_result->physical_layouts->data;
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       find_result->page_num); 
    double progress_x = first_rect->x1 / meta->page_width;
    double progress_y = first_rect->y1 / meta->page_height;
    d.preserved_progress_x = progress_x;
    d.preserved_progress_y = progress_y;
    if(find_result->page_num != d.cur_page_num){
        goto_page(find_result->page_num,
                  progress_x, progress_y);
    }
    else{
        scale_page(d.zoom_level, 
                   TRUE,
                   progress_x, progress_y);
    } 
}

static void
find_previous(void)
{    
    if(!d.find_details.find_results){
        return;
    }
    if(d.find_details.selected_p){
        d.find_details.selected_p = d.find_details.selected_p->prev;
    }
    if(!d.find_details.selected_p){
        d.find_details.selected_p = g_list_last(d.find_details.find_results);
    }
    FindResult *find_result = d.find_details.selected_p->data;            
    Rect *first_rect = find_result->physical_layouts->data;
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       find_result->page_num); 
    double progress_x = first_rect->x1 / meta->page_width;
    double progress_y = first_rect->y1 / meta->page_height;
    d.preserved_progress_x = progress_x;
    d.preserved_progress_y = progress_y;    
    if(find_result->page_num != d.cur_page_num){
        goto_page(find_result->page_num,
                  progress_x, progress_y);
    }
    else{
        scale_page(d.zoom_level, 
                   TRUE,
                   progress_x, progress_y);
    }  
}

static void
destroy_find_results(void)
{
    /* clear previous find results */
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        g_list_free(meta->find_results);
        meta->find_results = NULL;  
    }
    GList *result_p = d.find_details.find_results;
    while(result_p){
        find_result_free(result_p->data);
        result_p = result_p->next;
    }
    g_list_free(d.find_details.find_results);
    d.find_details.selected_p = NULL;
    d.find_details.find_results = NULL;
}

static void
on_find_request_received(GtkWidget *sender,
                         gpointer   user_data)
{            
    FindRequestData *find_request = user_data;
    if(d.metae){
        destroy_find_results();
        gtk_widget_queue_draw(ui.vellum);     
        d.find_details.find_results = find_text(find_request->text,
                                                0, d.num_pages,
                                                find_request->is_dualpage_checked,
                                                find_request->is_whole_words_checked);

        d.find_details.find_results = g_list_sort(d.find_details.find_results,
                                                  compare_find_results);
        int num_results = g_list_length(d.find_details.find_results);
        int i = 0;
        GList *result_p = d.find_details.find_results;
        while(result_p){
            FindResult *fr = result_p->data;
            PageMeta *meta = g_ptr_array_index(d.metae,
                                               fr->page_num);
            Rect *first_rect = fr->physical_layouts->data;
            if(fr->page_postfix){
                fr->certainty = first_rect->y1 / meta->page_height;
            }
            else if(fr->page_prefix){
                fr->certainty = 1.0 - (first_rect->y1 / meta->page_height);    
            }
            else{
                fr->certainty = 1.0;
            }
            fr->tip = g_strdup_printf("<span font='sans 10'>Result <i>%d</i> of %d, [certainty: %s]</span>\n"
                                      "  <span font='sans 8'>next(F3), previous(Shift+F3), clear(Ctrl+F3)</span>",
                                      i + 1, num_results,
                                      fr->certainty < 0.75 ? "low" : "high");
            meta->find_results = g_list_append(meta->find_results,
                                               fr);  
            result_p = result_p->next;
            i++;
        }
        find_next();
    }    
    g_free(find_request);
}

static void
get_text_size(cairo_t *cr,
              const char *markup,
              double *text_width,
              double *text_height)
{
    cairo_save(cr);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_markup(layout,
                            markup, -1);
    // PangoFontDescription *font_desc = pango_font_description_from_string(font_str);    
    // pango_layout_set_font_description(layout,
    //                                   font_desc);
    // pango_font_description_free(font_desc);
    int tw, th;
    pango_layout_get_size(layout,
                          &tw, &th);
    g_object_unref(layout);
    cairo_restore(cr);
    *text_width = (double)tw / PANGO_SCALE;
    *text_height = (double)th / PANGO_SCALE;
}

static Rect
draw_text(cairo_t *cr,
          const char *markup,
          PangoAlignment h_alignment,
          PangoAlignment v_alignment,
          Rect *rect)
{
    cairo_save(cr);
    PangoLayout *layout = pango_cairo_create_layout(cr);
    pango_layout_set_markup(layout,
                            markup, -1);
    // PangoFontDescription *font_desc = pango_font_description_from_string(font_str);
    // pango_layout_set_font_description(layout,
    //                                   font_desc);
    // pango_font_description_free(font_desc);
    pango_layout_set_alignment(layout,
                               h_alignment);
    pango_layout_set_width(layout,
                           rect_width(rect) * PANGO_SCALE);
    int text_width, text_height;
    pango_layout_get_size(layout,
                          &text_width, &text_height);
    double start_x = rect->x1;
    double start_y;
    if(v_alignment == PANGO_ALIGN_LEFT){
        start_y = rect->y1;
    }
    else if(v_alignment == PANGO_ALIGN_RIGHT){
        start_y = rect->y2;
    }
    else{
        start_y = rect_center_y(rect) - ((double)text_height / PANGO_SCALE) / 2;
    }

    cairo_move_to(cr,
                  start_x, start_y);
    pango_cairo_show_layout(cr,
                            layout);    
    g_object_unref(layout);
    cairo_restore(cr);
    Rect final_rect;
    final_rect.x1 = start_x;
    final_rect.y1 = start_y;
    final_rect.x2 = start_x + (double)text_width / PANGO_SCALE;
    final_rect.y2 = start_y + (double)text_height / PANGO_SCALE;
    return final_rect;
}

static void
draw_start_mode(cairo_t *cr)
{
    /* import area */       
    cairo_rectangle(cr,
                    ui.import_area_rect->x1, ui.import_area_rect->y1,
                    rect_width(ui.import_area_rect), rect_height(ui.import_area_rect));
    if(ui.is_import_area_hovered){
        cairo_set_source_rgb(cr,
                             cadet_gray_r, cadet_gray_g, cadet_gray_b); 
    }
    else{
        cairo_set_source_rgb(cr,
                             dark_gray_r, dark_gray_r, dark_gray_r);        
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 12' foreground='#222222'><b>I</b>mport a book</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.import_area_rect);
    /* book details area */
    cairo_rectangle(cr,
                    ui.book_details_area_rect->x1, ui.book_details_area_rect->y1,
                    rect_width(ui.book_details_area_rect), rect_height(ui.book_details_area_rect));
    cairo_set_source_rgb(cr,
                         silver_r, silver_r, silver_r);
    cairo_fill(cr);
    if(d.metae){   
        double padding = 8;
        Rect book_details_header_rect = *ui.book_details_area_rect;
        book_details_header_rect.x1 = book_details_header_rect.x2 * 0.2;             
        book_details_header_rect.x2 = book_details_header_rect.x2 * 0.8;
        book_details_header_rect.y1 = book_details_header_rect.y1 + padding;             
        book_details_header_rect.y2 = book_details_header_rect.y1 + padding * 3;
        draw_text(cr,
                  "<span font='sans 12' foreground='#222222'>Book details</span>",
                  PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
                  &book_details_header_rect);
        double text_width, text_height;
        get_text_size(cr,
                      d.doc_info.book_info_label,
                      &text_width, &text_height);
        Rect book_details_label_rect;
        book_details_label_rect.x1 = book_details_header_rect.x2 * 0.05;
        book_details_label_rect.x2 = book_details_label_rect.x1 + text_width;
        book_details_label_rect.y1 = book_details_header_rect.y2 + padding * 2;
        book_details_label_rect.y2 = book_details_label_rect.y1 + text_height;
        book_details_label_rect = draw_text(cr,
                                            d.doc_info.book_info_label,
                                            PANGO_ALIGN_LEFT, PANGO_ALIGN_LEFT,
                                            &book_details_label_rect);             
        get_text_size(cr,
                      d.doc_info.book_info_data,
                      &text_width, &text_height);        
        Rect book_details_data_rect;
        book_details_data_rect.x1 = book_details_label_rect.x2 + padding;
        book_details_data_rect.x2 = book_details_data_rect.x1 + text_width;
        book_details_data_rect.y1 = book_details_label_rect.y1;
        book_details_data_rect.y2 = book_details_data_rect.y1 + text_height;
        draw_text(cr,
                  d.doc_info.book_info_data,
                  PANGO_ALIGN_LEFT, PANGO_ALIGN_LEFT,
                  &book_details_data_rect);
        /* continue to book button */
        ui.continue_to_book_button_rect->x1 = ui.book_details_area_rect->x2 * 0.15;
        ui.continue_to_book_button_rect->x2 = ui.book_details_area_rect->x2 * 0.85;
        ui.continue_to_book_button_rect->y1 = book_details_label_rect.y2 + 3 * padding;
        ui.continue_to_book_button_rect->y2 = book_details_label_rect.y2 + 7 * padding;            
        cairo_rectangle(cr,
                        ui.continue_to_book_button_rect->x1,
                        ui.continue_to_book_button_rect->y1,
                        rect_width(ui.continue_to_book_button_rect),
                        rect_height(ui.continue_to_book_button_rect));   
        if(ui.is_continue_to_book_button_hovered){
            cairo_set_source_rgb(cr,
                                 cadet_gray_r, cadet_gray_g, cadet_gray_b);
        }
        else{
            cairo_set_source_rgb(cr,
                                 gray_r, gray_r, gray_r);
        }
        cairo_fill(cr);
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           d.cur_page_num);
        char *text_continue;
        if(meta->page_label){
            text_continue = g_strdup_printf("<span font='sans 12' foreground='#222222'>On page '%s', continue reading</span>",
                                            meta->page_label);
        }
        else{
            text_continue = g_strdup_printf("<span font='sans 12' foreground='#222222'>On page '%d', continue reading</span>",
                                            d.cur_page_num + 1);
        }  
        
        draw_text(cr,
                  text_continue,
                  PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
                  ui.continue_to_book_button_rect);
        g_free(text_continue);
    }
    else{
        draw_text(cr,
                  "<span font='sans 12' foreground='#222222'>No book is imported yet.</span>",
                  PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
                  ui.book_details_area_rect);
    }
    /* app info area*/
    cairo_rectangle(cr,
                    ui.app_info_area_rect->x1, ui.app_info_area_rect->y1,
                    rect_width(ui.app_info_area_rect), rect_height(ui.app_info_area_rect));
    cairo_set_source_rgb(cr,
                         gray_r, gray_r, gray_r);
    cairo_fill(cr); 
    draw_text(cr,
              ui.app_info_text,
              PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
              ui.app_info_area_rect);
}

static void
draw_reading_mode(cairo_t *cr)
{
    int widget_width = gtk_widget_get_allocated_width(ui.vellum);
    int widget_height = gtk_widget_get_allocated_height(ui.vellum);
    double image_width = cairo_image_surface_get_width(d.image);
    double image_height = cairo_image_surface_get_height(d.image);
    PageMeta *meta = g_ptr_array_index(d.metae,
                                       d.cur_page_num);
    /* page */
    cairo_set_line_width(cr,
                         1.0);
    int centered_origin_y = d.image_origin_y;
    centered_origin_y += image_height < widget_height ? (widget_height - image_height) / 2 : 0;
    cairo_rectangle(cr,
                    d.image_origin_x,
                    d.image_origin_y,
                    image_width,
                    image_height);
    cairo_set_source_surface(cr,
                             d.image,
                             d.image_origin_x,
                             d.image_origin_y);
    cairo_fill(cr);
    /* links */ 
    GList *list_p = meta->links;   
    cairo_set_source_rgb(cr,
                         blue_r, blue_g, blue_b);
    while(list_p){
        Link *link = list_p->data;
        Rect img_rect = map_physical_rect_to_image(link->physical_layout,
                                                   meta->page_width,
                                                   meta->page_height,
                                                   image_width,
                                                   image_height,
                                                   d.image_origin_x,
                                                   d.image_origin_y);
        cairo_rectangle(cr,
                        img_rect.x1, img_rect.y1,
                        rect_width(&img_rect), rect_height(&img_rect));
        if(link->is_hovered){
            cairo_set_dash(cr,
                           dashed_style,
                           num_dashes,
                           0);
        }
        else{
            cairo_set_dash(cr,
                           NULL, 0, 0);
        }
        cairo_stroke(cr);
        list_p = list_p->next;
    }
    cairo_set_dash(cr,
                   NULL, 0, 0);        
    /* converted units */
    list_p = meta->converted_units;
    while(list_p){
        ConvertedUnit *cv = list_p->data;
        GList *result_p = cv->find_results;
        while(result_p){
            FindResult *fr = result_p->data;
            GList *rect_p = fr->physical_layouts;
            while(rect_p){
                Rect rect = map_physical_rect_to_image(rect_p->data,
                                                       meta->page_width,
                                                       meta->page_height,
                                                       image_width,
                                                       image_height,
                                                       d.image_origin_x,
                                                       d.image_origin_y);
                cairo_move_to(cr,
                              rect.x1, rect.y1);
                cairo_line_to(cr,
                              rect.x2, rect.y1);
                if(g_list_last(fr->physical_layouts) == rect_p){
                    cairo_line_to(cr,
                                  rect.x2, rect.y2);
                }
                else{
                    cairo_move_to(cr,
                                  rect.x2, rect.y2);
                }
                cairo_line_to(cr,
                              rect.x1, rect.y2);
                if(g_list_first(fr->physical_layouts) == rect_p){
                    cairo_line_to(cr,
                                  rect.x1, rect.y1);
                }
                rect_p = rect_p->next;
            }
            result_p = result_p->next;
        }
        list_p = list_p->next;
    }
    cairo_set_source_rgb(cr,
                         gotham_green_r, gotham_green_g, gotham_green_b);
    cairo_stroke(cr);
    /* figures */
    // if(meta->figures){
    //     GHashTableIter iter;
    //     gpointer key, value;
    //     g_hash_table_iter_init (&iter, meta->figures);
    //     while(g_hash_table_iter_next(&iter, &key, &value)){        
    //         Figure *fig = value;
    //         Rect image_rect = map_physical_rect_to_image(fig->image_physical_layout,
    //                                                 meta->page_width,
    //                                                 meta->page_height,
    //                                                 image_width,
    //                                                 image_height,
    //                                                 d.image_origin_x,
    //                                                 d.image_origin_y); 
    //         cairo_rectangle(cr,
    //                         image_rect.x1,
    //                         image_rect.y1,
    //                         image_rect.x2 - image_rect.x1,
    //                         image_rect.y2 - image_rect.y1);      
    //         Rect caption_rect = map_physical_rect_to_image(fig->caption_physical_layout,
    //                                                   meta->page_width,
    //                                                   meta->page_height,
    //                                                   image_width,
    //                                                   image_height,
    //                                                   d.image_origin_x,
    //                                                   d.image_origin_y);
    //         cairo_rectangle(cr,
    //                         caption_rect.x1,
    //                         caption_rect.y1,
    //                         caption_rect.x2 - caption_rect.x1,
    //                         caption_rect.y2 - caption_rect.y1);
    //         cairo_set_source_rgb(cr, 1, 0, 0);
    //         cairo_stroke(cr);
    //     }   
    // }
    /* referenced figures */
    list_p = meta->referenced_figures;
    while(list_p){
        ReferencedFigure *ref_figure = list_p->data;
        GList *result_p = ref_figure->find_results;
        while(result_p){
            FindResult *find_result = result_p->data;
            GList *rect_p = find_result->physical_layouts;
            while(rect_p){
                Rect rect = map_physical_rect_to_image(rect_p->data,
                                                       meta->page_width,
                                                       meta->page_height,
                                                       image_width,
                                                       image_height,
                                                       d.image_origin_x,
                                                       d.image_origin_y);
                cairo_move_to(cr,
                              rect.x1, rect.y1);
                cairo_line_to(cr,
                              rect.x2, rect.y1);
                if(g_list_last(find_result->physical_layouts) == rect_p){
                    cairo_line_to(cr,
                                  rect.x2, rect.y2);
                }
                else{
                    cairo_move_to(cr,
                                  rect.x2, rect.y2);
                }
                cairo_line_to(cr,
                              rect.x1, rect.y2);
                if(g_list_first(find_result->physical_layouts) == rect_p){
                    cairo_line_to(cr,
                                  rect.x1, rect.y1);
                }
                rect_p = rect_p->next;
            }
            result_p = result_p->next;
        }
        list_p = list_p->next;
    }
    if(meta->referenced_figures){
        cairo_set_source_rgb(cr,
                             1, 0, 1);
        cairo_stroke(cr);
    }
    /* find results */
    GList *result_p = meta->find_results;
    while(result_p){
        FindResult *find_result =  result_p->data;
        GList *rect_p = find_result->physical_layouts;
        while(rect_p){
            Rect rect = map_physical_rect_to_image(rect_p->data,
                                                   meta->page_width,
                                                   meta->page_height,
                                                   image_width,
                                                   image_height,
                                                   d.image_origin_x,
                                                   d.image_origin_y);
            cairo_move_to(cr,
                          rect.x1, rect.y1);
            cairo_line_to(cr,
                          rect.x2, rect.y1);
            if(g_list_last(find_result->physical_layouts) == rect_p &&
               !find_result->page_postfix)
            {
                cairo_line_to(cr,
                              rect.x2, rect.y2);
            }
            else{
                cairo_move_to(cr,
                              rect.x2, rect.y2);
            }
            cairo_line_to(cr,
                          rect.x1, rect.y2);
            if(g_list_first(find_result->physical_layouts) == rect_p &&
               !find_result->page_prefix)
            {
                cairo_line_to(cr,
                              rect.x1, rect.y1);
            }
            rect_p = rect_p->next;
        }
        if(d.find_details.selected_p && d.find_details.selected_p->data == find_result){
           cairo_set_dash(cr,
                          dashed_style,
                          num_dashes,
                          0);             
        }
        else{
            cairo_set_dash(cr,
                           NULL, 0, 0);
        }
        cairo_set_source_rgb(cr,
                             giants_orange_r, giants_orange_g, giants_orange_b);    
        cairo_stroke(cr); 
        result_p = result_p->next;
    }
    cairo_set_dash(cr,
                   NULL, 0, 0);
    /* page widgets */
    /* prev button */
    if(d.cur_page_num > 0){
        cairo_rectangle(cr,
                        ui.prev_page_button_rect->x1, ui.prev_page_button_rect->y1,
                        rect_width(ui.prev_page_button_rect), rect_height(ui.prev_page_button_rect));
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, ui.is_prev_page_button_hovered ? 0.9 : 0.2); /* light gray */
        cairo_fill(cr);
        draw_text(cr,
                  "<span font='sans 16' foreground='#666666'><b>&#60;</b></span>",
                  PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
                  ui.prev_page_button_rect);    
    }
    /* next button */
    if(d.cur_page_num < d.num_pages - 1){
        cairo_rectangle(cr,
                        ui.next_page_button_rect->x1, ui.next_page_button_rect->y1,
                        rect_width(ui.next_page_button_rect), rect_height(ui.next_page_button_rect));
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, ui.is_next_page_button_hovered ? 0.9 : 0.2); /* light gray */
        cairo_fill(cr);
        draw_text(cr,
                  "<span font='sans 16' foreground='#666666'><b>&#62;</b></span>",
                  PANGO_ALIGN_CENTER, PANGO_ALIGN_CENTER,
                  ui.next_page_button_rect);        
    }
    /* reading progress */
    int padding = 10;
    double text_width, text_height;
    get_text_size(cr,
                  meta->reading_progress_text,
                  &text_width, &text_height);
    Rect reading_progress_rect;
    reading_progress_rect.x1 = widget_width - text_width - 3 * padding;
    reading_progress_rect.y1 = widget_height - text_height - 3 * padding;
    reading_progress_rect.x2 = reading_progress_rect.x1 + text_width + 2 * padding;
    reading_progress_rect.y2 = reading_progress_rect.y1 + text_height + 2 * padding;
    cairo_rectangle(cr,
                    reading_progress_rect.x1,
                    reading_progress_rect.y1,
                    rect_width(&reading_progress_rect),
                    rect_height(&reading_progress_rect));
    cairo_set_source_rgba(cr,
                          dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    cairo_fill(cr);
    cairo_set_source_rgb(cr,
                         0.1333, 0.1333, 0.1333);
    double visual_progress_height = 8;    
    cairo_rectangle(cr,
                    reading_progress_rect.x1 + 1,
                    reading_progress_rect.y2 - visual_progress_height,
                    rect_width(&reading_progress_rect) - 2,
                    visual_progress_height);
    cairo_stroke(cr);
    cairo_rectangle(cr,
                    reading_progress_rect.x1 + 2,
                    reading_progress_rect.y2 - visual_progress_height + 1,
                    ((double)(d.cur_page_num + 1) / d.num_pages) * (rect_width(&reading_progress_rect) - 4),
                    visual_progress_height - 2);
    cairo_fill(cr);
    draw_text(cr,
              meta->reading_progress_text,
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &reading_progress_rect);
    /* zoom widget */
    /* page fit */
    cairo_rectangle(cr,
                    ui.zoom_widget_PF_rect->x1,
                    ui.zoom_widget_PF_rect->y1,
                    rect_width(ui.zoom_widget_PF_rect),
                    rect_height(ui.zoom_widget_PF_rect));
    if(ui.is_zoom_widget_PF_hovered || d.zoom_level == PageFit){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);                
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'>Page\n<b>F</b>it</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.zoom_widget_PF_rect);
    /* width fit */
    cairo_rectangle(cr,
                    ui.zoom_widget_WF_rect->x1,
                    ui.zoom_widget_WF_rect->y1,
                    rect_width(ui.zoom_widget_WF_rect),
                    rect_height(ui.zoom_widget_WF_rect));
    if(ui.is_zoom_widget_WF_hovered  || d.zoom_level == WidthFit){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);                
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'>Fit to\n<b>w</b>idth</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.zoom_widget_WF_rect);
    /* zoom in */
    cairo_rectangle(cr,
                    ui.zoom_widget_IN_rect->x1,
                    ui.zoom_widget_IN_rect->y1,
                    rect_width(ui.zoom_widget_IN_rect),
                    rect_height(ui.zoom_widget_IN_rect));
    if(ui.is_zoom_widget_IN_hovered){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 12' foreground='#222222'>+</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.zoom_widget_IN_rect);
    /* zoom out */
    cairo_rectangle(cr,
                    ui.zoom_widget_OUT_rect->x1,
                    ui.zoom_widget_OUT_rect->y1,
                    rect_width(ui.zoom_widget_OUT_rect),
                    rect_height(ui.zoom_widget_OUT_rect));
    if(ui.is_zoom_widget_OUT_hovered){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'>-</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.zoom_widget_OUT_rect);
    /* launchers */
    /* Teleport */
    cairo_rectangle(cr,
                    ui.teleport_launcher_rect->x1,
                    ui.teleport_launcher_rect->y1,
                    rect_width(ui.teleport_launcher_rect),
                    rect_height(ui.teleport_launcher_rect));
    if(ui.is_teleport_launcher_hovered){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'><b>T</b>ele\nport</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.teleport_launcher_rect);
    /* find */
    cairo_rectangle(cr,
                    ui.find_text_launcher_rect->x1,
                    ui.find_text_launcher_rect->y1,
                    rect_width(ui.find_text_launcher_rect),
                    rect_height(ui.find_text_launcher_rect));
    if(ui.is_find_text_launcher_hovered){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'><u><b>F</b></u>ind</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.find_text_launcher_rect);
    /* toc */
    cairo_rectangle(cr,
                    ui.toc_launcher_rect->x1,
                    ui.toc_launcher_rect->y1,
                    rect_width(ui.toc_launcher_rect),
                    rect_height(ui.toc_launcher_rect));
    if(ui.is_toc_launcher_hovered){
        cairo_set_source_rgba(cr,
                              cadet_gray_r, cadet_gray_g, cadet_gray_b, 0.8);
    }
    else{
        cairo_set_source_rgba(cr,
                              dark_gray_r, dark_gray_r, dark_gray_r, 0.8);
    }
    cairo_fill(cr);
    draw_text(cr,
              "<span font='sans 10' foreground='#222222'><u><b>T</b></u>OC</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              ui.toc_launcher_rect);
    /* active referenced figure is overlaid on the page */
    if(meta->active_referenced_figure){        
        cairo_surface_t *ref_surface = meta->active_referenced_figure->reference->image;
        double ref_image_width = cairo_image_surface_get_width(ref_surface);
        double ref_image_height = cairo_image_surface_get_height(ref_surface);
        double ref_ar = ref_image_height / ref_image_width;
        gboolean scaled = ref_image_width > widget_width || ref_image_height > widget_height;
        if(scaled){
            double scaled_width, scaled_height;
            double sx = widget_width < ref_image_width ? widget_width / ref_image_width : 1;
            double sy = widget_height < ref_image_height ? widget_height / ref_image_height : 1;
            if(sy < 1){
                scaled_height = ref_image_height * sy;
                scaled_width =  scaled_height / ref_ar;
            }
            else{
                scaled_width = ref_image_width * sx;
                scaled_height = scaled_width * ref_ar;
            }
            cairo_surface_t *surface_scaled = 
                cairo_image_surface_create(CAIRO_FORMAT_ARGB32,                                             
                                           scaled_width,
                                           scaled_height);
            cairo_t *cr_scaled = cairo_create(surface_scaled);
            cairo_scale(cr_scaled,
                        scaled_width / ref_image_width,
                        scaled_height / ref_image_height);
            cairo_set_source_surface(cr_scaled,
                                     ref_surface,
                                     0, 0);
            cairo_paint(cr_scaled);   
            cairo_destroy(cr_scaled);
            ref_surface = surface_scaled;
            ref_image_width = cairo_image_surface_get_width(ref_surface);
            ref_image_height = cairo_image_surface_get_height(ref_surface);
        }
        Rect caption_rect;
        FindResult *find_result =  meta->active_referenced_figure->activated_find_result->data;
        GList *rect_p = find_result->physical_layouts;
        while(rect_p){
            Rect rect = map_physical_rect_to_image(rect_p->data,
                                                   meta->page_width,
                                                   meta->page_height,
                                                   image_width,
                                                   image_height,
                                                   d.image_origin_x,
                                                   d.image_origin_y);
            if(rect_p == g_list_first(find_result->physical_layouts)){
                caption_rect = rect;
            }
            cairo_move_to(cr,
                          rect.x1, rect.y1);
            cairo_line_to(cr,
                          rect.x2, rect.y1);
            if(g_list_last(find_result->physical_layouts) == rect_p){
                cairo_line_to(cr,
                              rect.x2, rect.y2);
            }
            else{
                cairo_move_to(cr,
                              rect.x2, rect.y2);
            }
            cairo_line_to(cr,
                          rect.x1, rect.y2);
            if(g_list_first(find_result->physical_layouts) == rect_p){
                cairo_line_to(cr,
                              rect.x1, rect.y1);
            }
            rect_p = rect_p->next;
        }
        double caption_cx = rect_center_x(&caption_rect);
        int padding = 1;
        double frame_x1 = caption_cx - ref_image_width / 2 - padding;
        double frame_y1 = caption_rect.y1 - ref_image_height - 2 * padding;
        if(frame_y1 >= 0){
            if(frame_x1 < 0){
                frame_x1 = 0; 
            }
        }
        else{
            frame_y1 = 0;            
            frame_x1 = caption_rect.x1 - ref_image_width - 2 * padding;
            if(frame_x1 < 0){
                frame_x1 = caption_rect.x2;
            }
        }                
        cairo_rectangle(cr,
                        frame_x1 + padding, frame_y1 + padding,
                        ref_image_width + padding, ref_image_height + padding);  
        cairo_set_source_rgb(cr,
                             0.5, 0.5, 0.5);
        cairo_stroke(cr);
        cairo_rectangle(cr,
                        frame_x1 + padding, frame_y1 + padding,
                        ref_image_width, ref_image_height); 
        cairo_set_source_surface(cr,
                                 ref_surface,
                                 frame_x1 + padding, frame_y1 + padding);
        cairo_fill(cr);  
        if(scaled){
            cairo_surface_destroy(ref_surface);
        }      
    }
}

static void
draw_toc_items(TOCItem *item,
               double   last_row,
               cairo_t *cr)
{
    if(!item){
        return;
    }
    char *markup = g_strdup_printf("<span font='sans 10' foreground='#222222'><b>%s</b></span>\n"
                                   "<span font='sans 8' foreground='#333333'><i>%d Page%s</i></span>",
                                   item->title,
                                   item->length,
                                   item->length > 1 ? "s" : "");
    double text_width, text_height;
    get_text_size(cr,
                  markup,
                  &text_width, &text_height); 
    double cell_padding = 8,
           text_padding = 4;
    if(!item->rect){
        item->rect = rect_new();
    }
    item->rect->x1 = cell_padding + (item->parent ? item->parent->rect->x2 : -d.toc.origin_x);
    item->rect->y1 = cell_padding + last_row;
    item->rect->x2 = item->rect->x1 + text_width + 2 * text_padding;
    item->rect->y2 = item->rect->y1 + text_height + 2 * text_padding;   
    draw_text(cr,
             markup,
             PANGO_ALIGN_CENTER,
             PANGO_ALIGN_CENTER,
             item->rect);
    g_free(markup);
    cairo_rectangle(cr,
                   item->rect->x1, item->rect->y1,
                   rect_width(item->rect), rect_height(item->rect));
    if(g_list_find(d.toc.where,
                   item))
    {
        /* highlight me + parents */
        cairo_set_source_rgb(cr,
                             giants_orange_r, giants_orange_g, giants_orange_b);
    }
    else{
        cairo_set_source_rgb(cr,
                             dim_gray_r, dim_gray_r, dim_gray_r);
    }
    if(item == d.toc.hovered_item){
        cairo_set_dash(cr,
                       dashed_style,
                       num_dashes,
                       0);
    }
    cairo_stroke(cr);
    cairo_set_dash(cr,
                   NULL, 0, 0);
    /* draw an H line from me to my parent's center */
    if(item->parent){
        cairo_move_to(cr,
                      item->rect->x1, rect_center_y(item->rect));
        cairo_line_to(cr,
                      rect_center_x(item->parent->rect), rect_center_y(item->rect));
    }
    cairo_stroke(cr);
    last_row = item->rect->y2;
    GList *child_p = item->children;
    while(child_p){ 
        TOCItem *child_item = child_p->data;       
        draw_toc_items(child_item,
                       last_row,
                       cr);
        if(child_item->children){            
            GList *last_child_p = g_list_last(child_item->children);
            while(last_child_p){
                TOCItem *last_child_item = last_child_p->data;
                last_row = last_child_item->rect->y2;
                last_child_p = g_list_last(last_child_item->children);
            }
        }        
        else{
            last_row = child_item->rect->y2;
        }
        child_p = child_p->next;
    }
    /* draw a V line from me to my children highlighting where_am_i items */
    if(item->children){
        GList *child_p = item->children;
        while(child_p){
            if(g_list_find(d.toc.where,
                           child_p->data))
            {
                break;
            }
            child_p = child_p->next;
        }
        if(child_p){
            TOCItem *where_item = child_p->data;
            cairo_move_to(cr,
                          rect_center_x(item->rect), item->rect->y2);
            cairo_line_to(cr,
                          rect_center_x(item->rect), rect_center_y(where_item->rect));
            cairo_set_source_rgb(cr,
                                 giants_orange_r, giants_orange_g, giants_orange_b);
            cairo_stroke(cr);
            cairo_move_to(cr,
                          rect_center_x(item->rect), rect_center_y(where_item->rect));
        }
        else{
            cairo_move_to(cr,
                          rect_center_x(item->rect), item->rect->y2);            
        }
        TOCItem *last_child_item = g_list_last(item->children)->data;
        cairo_line_to(cr,
                      rect_center_x(item->rect), rect_center_y(last_child_item->rect));
        cairo_set_source_rgb(cr,
                             dim_gray_r, dim_gray_r, dim_gray_r);
        cairo_stroke(cr);
    }
}

static void
draw_toc_mode(cairo_t *cr)
{
    int widget_width = gtk_widget_get_allocated_width(ui.vellum);
    int widget_height = gtk_widget_get_allocated_height(ui.vellum);
    if(d.toc.max_depth == 0){
        Rect wr;
        wr.x1 = wr.y1 = 0;
        wr.x2 = widget_width;
        wr.y2 = widget_height;
        draw_text(cr,
                  "<span font='sans 18' foreground='#222222'>TOC is unavailable,</span>\n"
                  "<span font='sans 10' foreground='#222222'>to continue reading click anywhere or press(R, r).</span>",
                  PANGO_ALIGN_CENTER,
                  PANGO_ALIGN_CENTER,
                  &wr);
        return;
    }
    draw_toc_items(d.toc.head_item,
                   -d.toc.origin_y,
                   cr);
    if(d.toc.hovered_item){
        /* show a thumbnail of toc item's page*/
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           d.toc.hovered_item->page_num < 0 ? 0 : d.toc.hovered_item->page_num);        
        double thumbnail_width = widget_width / 4.236; 
        double thumbnail_height = thumbnail_width * (meta->page_height / meta->page_width);
        cairo_surface_t *page_thumbnail = render_page(meta,
                                                      thumbnail_width, thumbnail_height);
        double progress_y_start = (meta->page_height - d.toc.hovered_item->offset_y) / meta->page_height;
        if(progress_y_start >= 0.999){
            progress_y_start = 0;
        }
        cairo_t *cr_progress = cairo_create(page_thumbnail);
        cairo_rectangle(cr_progress,
                        0, 0,
                        thumbnail_width,
                        thumbnail_height * progress_y_start);
        cairo_set_source_rgba(cr_progress,
                              dim_gray_r, dim_gray_r, dim_gray_r, 0.8);
        cairo_fill(cr_progress);
        cairo_destroy(cr_progress);
        double thumbnail_x = rect_center_x(d.toc.hovered_item->rect) - thumbnail_width / 2;
        thumbnail_x = thumbnail_x < 0 ? d.toc.hovered_item->rect->x1 : thumbnail_x;
        double padding_y = 4;
        double thumbnail_y = (d.toc.hovered_item->rect->y1 - padding_y > thumbnail_height)
                             ? d.toc.hovered_item->rect->y1 - padding_y - thumbnail_height
                             : d.toc.hovered_item->rect->y2 + padding_y; 
        cairo_rectangle(cr,
                        thumbnail_x, thumbnail_y,
                        thumbnail_width, thumbnail_height);
        cairo_set_source_surface(cr,
                                 page_thumbnail,
                                 thumbnail_x, thumbnail_y);
        cairo_fill(cr);
        cairo_surface_destroy(page_thumbnail);        
    }
    /* navigation buttons: next chapter, ... */
    cairo_rectangle(cr,
                    0, widget_height - toc_navigation_panel_height,
                    widget_width, toc_navigation_panel_height);
    cairo_set_source_rgb(cr,
                         gray_r, gray_r, gray_r);
    cairo_fill(cr);
    double button_padding = 8;
    double button_width, button_height;
    /* get largest possible size */
    get_text_size(cr,
                  "previous\n<i><b>subsection</b></i>",
                  &button_width, &button_height);
    button_width += button_padding / 2;
    button_height += button_padding / 2; 
    double button_y1 = (toc_navigation_panel_height - button_height) / 2  + (widget_height - toc_navigation_panel_height);  
    int num_labels = g_list_length(d.toc.labels);
    double last_col = button_padding;
    int label_index = 0;
    GList *list_p = d.toc.labels;
    while(list_p){
        Rect *prev_button = g_list_nth_data(d.toc.navigation_button_rects,
                                            label_index * 2);        
        prev_button->x1 = last_col + button_padding;
        prev_button->y1 = button_y1;
        prev_button->x2 = prev_button->x1 + button_width;
        prev_button->y2 = prev_button->y1 + button_height;
        last_col = prev_button->x2;
        cairo_rectangle(cr,
                        prev_button->x1, prev_button->y1,
                        button_width, button_height);
        if(d.toc.hovered_navigation_button == prev_button){            
            cairo_set_source_rgb(cr,
                                 cadet_gray_r, cadet_gray_g, cadet_gray_b);
        }
        else{
            cairo_set_source_rgb(cr,
                                 silver_r, silver_r, silver_r);
        }
        cairo_fill(cr);
        char *prev_markup = g_strdup_printf("<span font='sans 10' foreground='#222222'>Previous\n<i><b>%s</b></i></span>",
                                            (char*)list_p->data);
        draw_text(cr,
                  prev_markup,
                  PANGO_ALIGN_CENTER,
                  PANGO_ALIGN_CENTER,
                  prev_button);
        g_free(prev_markup);
        Rect *next_button = g_list_nth_data(d.toc.navigation_button_rects,
                                            label_index * 2 + 1);
        next_button->x1 = prev_button->x1 + num_labels * (button_width + button_padding) + 32;
        next_button->y1 = button_y1;
        next_button->x2 = next_button->x1 + button_width;
        next_button->y2 = next_button->y1 + button_height;
        cairo_rectangle(cr,
                        next_button->x1, next_button->y1,
                        button_width, button_height);        
        if(d.toc.hovered_navigation_button == next_button){            
            cairo_set_source_rgb(cr,
                                 cadet_gray_r, cadet_gray_g, cadet_gray_b);
        }
        else{
            cairo_set_source_rgb(cr,
                                 silver_r, silver_r, silver_r);
        }
        cairo_fill(cr);
        char *next_markup = g_strdup_printf("<span font='sans 10' foreground='#222222'>Next\n<i><b>%s</b></i></span>",
                                            (char*)list_p->data);
        draw_text(cr,
                  next_markup,
                  PANGO_ALIGN_CENTER,
                  PANGO_ALIGN_CENTER,
                  next_button);
        g_free(next_markup);
        label_index++;
        list_p = list_p->next;
    }
    if(d.toc.hovered_navigation_button){
    }
}

static void
draw_help_mode(cairo_t *cr)
{
    int widget_width = gtk_widget_get_allocated_width(ui.vellum);
    int widget_height = gtk_widget_get_allocated_height(ui.vellum);
    int padding = 16;
    Rect rect;
    rect.x1 = 0;
    rect.y1 = 0;
    rect.x2 = widget_width;
    rect.y2 =  padding * 2;
    draw_text(cr,
              "<span font='sans 14' foreground='#222'>Help</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &rect);
    char *markup = 
        "<span font='sans 10' foreground='#444'>Modes</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Start mode</i>:</span><span font='sans 10' foreground='blue'> S</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Reading mode</i>:</span><span font='sans 10' foreground='blue'> R</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>TOC mode</i>:</span><span font='sans 10' foreground='blue'> Ctrl + T</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Help mode(this page)</i>:</span><span font='sans 10' foreground='blue'> H</span>\n"
        "\n<span font='sans 10' foreground='#444'>Import</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Import a book</i>:</span><span font='sans 10' foreground='blue'> I</span>\n"
        "\n<span font='sans 10' foreground='#444'>Page navigation</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>First page</i>:</span><span font='sans 10' foreground='blue'> Home</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Last page</i>:</span><span font='sans 10' foreground='blue'> End</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Next page</i>:</span><span font='sans 10' foreground='blue'> N</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Previous page</i>:</span><span font='sans 10' foreground='blue'> P</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Go back</i>:</span><span font='sans 10' foreground='blue'> Backspace</span>\n"
        "\n<span font='sans 10' foreground='#444'>Zoom</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Page fit</i>:</span><span font='sans 10' foreground='blue'> F</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Fit to width</i>:</span><span font='sans 10' foreground='blue'> W</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Zoom in/out</i>:</span><span font='sans 10' foreground='blue'> +/-</span>\n"
        "\n<span font='sans 10' foreground='#444'>Find</span>\n"        
            "\t<span font='sans 10' foreground='#222'><i>Next result</i>:</span><span font='sans 10' foreground='blue'> F3</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Previous result</i>:</span><span font='sans 10' foreground='blue'> Shift + F3</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>clear results</i>:</span><span font='sans 10' foreground='blue'> Ctrl + F3</span>\n"
        "\n<span font='sans 10' foreground='#444'>Tools</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Find dialog</i>:</span><span font='sans 10' foreground='blue'> Ctrl + F</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Teleport dialog</i>:</span><span font='sans 10' foreground='blue'> T</span>\n"
            "\t<span font='sans 10' foreground='#222'><i>Hide any dialog</i>:</span><span font='sans 10' foreground='blue'> Escape</span>\n";                
    rect.x1 = padding;
    rect.y1 = padding;
    rect.x2 = widget_width / 2;
    rect.y2 = gtk_widget_get_allocated_height(ui.vellum);
    rect = draw_text(cr,
                     markup,
                     PANGO_ALIGN_LEFT,
                     PANGO_ALIGN_CENTER,
                     &rect);    
    double box_width = 140;
    double box_height = 32;
    double x1_box = widget_width / 2 + padding;
    double y1_box = rect.y1 + 2 * padding;
    rect.x1 = x1_box;
    rect.y1 = y1_box;
    rect.x2 = widget_width - 2 * padding;
    rect.y2 = rect.y1 + 2 * padding;
    rect = draw_text(cr,
                     "<span font='sans 10' foreground='#222'>When reading you might encounter colorful boxes, hover on to activate them.</span>",
                     PANGO_ALIGN_LEFT,
                     PANGO_ALIGN_LEFT,
                     &rect); 
    y1_box = rect.y1 + 3 * padding;
    cairo_rectangle(cr,
                    x1_box, y1_box,
                    box_width, box_height);
    cairo_set_source_rgb(cr,
                         giants_orange_r, giants_orange_g, giants_orange_b);
    cairo_stroke(cr);
    rect.x1 = x1_box;
    rect.y1 = y1_box;
    rect.x2 = rect.x1 + box_width;
    rect.y2 = rect.y1 + box_height;
    draw_text(cr,
              "<span font='sans 10' foreground='#222'>Find result</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &rect); 
    x1_box += box_width + padding;
    cairo_rectangle(cr,
                    x1_box, y1_box,
                    box_width, box_height);
    cairo_set_source_rgb(cr,
                         gotham_green_r, gotham_green_g, gotham_green_b);
    cairo_stroke(cr);
    rect.x1 = x1_box;
    rect.y1 = y1_box;
    rect.x2 = rect.x1 + box_width;
    rect.y2 = rect.y1 + box_height;
    draw_text(cr,
              "<span font='sans 10' foreground='#222'>Converted unit</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &rect); 
    x1_box = widget_width / 2 + padding;
    y1_box += box_height * 2;
    cairo_rectangle(cr,
                    x1_box, y1_box,
                    box_width, box_height);
    cairo_set_source_rgb(cr,        
                         1, 0, 1);
    cairo_stroke(cr);
    rect.x1 = x1_box;
    rect.y1 = y1_box;
    rect.x2 = rect.x1 + box_width;
    rect.y2 = rect.y1 + box_height;
    draw_text(cr,
              "<span font='sans 10' foreground='#222'>Referenced figure</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &rect); 

    x1_box += box_width + padding;
    cairo_rectangle(cr,
                    x1_box, y1_box,
                    box_width, box_height);
    cairo_set_source_rgb(cr,
                         0, 0, 1);
    cairo_stroke(cr);
    rect.x1 = x1_box;
    rect.y1 = y1_box;
    rect.x2 = rect.x1 + box_width;
    rect.y2 = rect.y1 + box_height;
    draw_text(cr,
              "<span font='sans 10' foreground='#222'>Link</span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &rect);
    rect.x1 = widget_width / 2 + padding; 
    rect.y1 = rect.y1 + 2 * box_height;
    rect.x2 = widget_width - 2 * padding;
    rect.y2 = widget_height - 2 * padding;
    draw_text(cr,
              "<span font='sans 10' foreground='#222'>"
              "* Use <span foreground='blue'>arrow keys</span> and <span foreground='blue'>touch strokes</span> to move within page or screens.\n"
              "* Press <span foreground='blue'>Escape</span> to to get back to reading mode</span>",
              PANGO_ALIGN_LEFT,
              PANGO_ALIGN_CENTER,
              &rect);
}
static gboolean
draw_callback (GtkWidget *widget,
               cairo_t   *cr,
               gpointer   data)
{            
    cairo_set_source_rgb(cr, 
                         gainsboro_r, gainsboro_r, gainsboro_r);
    cairo_paint(cr);
    switch(ui.app_mode){
        case StartMode:
            draw_start_mode(cr);
            break;
        case ReadingMode:
            draw_reading_mode(cr);
            break;
        case TOCMode:
            draw_toc_mode(cr);
            break;
        case HelpMode:
            draw_help_mode(cr);
            break;
    }  
    return FALSE;
}

static void
switch_app_mode(enum AppMode new_mode)
{
    if(new_mode == ui.app_mode){
        return;
    }
    switch(new_mode){
    case StartMode:
        find_widget_hide();
        teleport_widget_hide();  
        ui.is_continue_to_book_button_hovered = FALSE;
        ui.is_import_area_hovered = FALSE;              
        ui.app_mode = StartMode;
        break;
    case ReadingMode:
        if(d.metae){
            ui.app_mode = ReadingMode;
            double progress_x = fabs(d.image_origin_x) / cairo_image_surface_get_width(d.image);
            double progress_y = fabs(d.image_origin_y) / cairo_image_surface_get_height(d.image);
            scale_page(d.zoom_level,
                       TRUE,
                       progress_x, progress_y);
        }
        break;
    case TOCMode:
        find_widget_hide();
        d.toc.hovered_item = NULL;
        d.toc.hovered_navigation_button = NULL;
        ui.app_mode = TOCMode;
        break;
    case HelpMode:
        ui.app_mode = HelpMode;
        break;
    }
    gdk_window_set_cursor(gtk_widget_get_window(ui.vellum),
                          ui.default_cursor);
    gtk_widget_queue_draw(ui.vellum);
}

static gboolean
key_press_callback(GtkWidget   *widget,
                   GdkEventKey *event,
                   gpointer     data)
{
    gboolean handled = TRUE;
    switch(event->keyval){
        case GDK_KEY_S:
        case GDK_KEY_s:
            switch_app_mode(StartMode);            
            break;
        case GDK_KEY_R:
        case GDK_KEY_r:
            switch_app_mode(ReadingMode);
            break;      
        case GDK_KEY_H:      
        case GDK_KEY_h:
            switch_app_mode(HelpMode);
            break;
        case GDK_KEY_F11:
            toggle_fullscreen();
            break;
        case GDK_KEY_Home:
            if(ui.app_mode == ReadingMode){
                goto_page(0,
                          0, 0);
            }
            else if(ui.app_mode == TOCMode){
                d.toc.origin_x = 0;
                d.toc.origin_y = 0;
                gtk_widget_queue_draw(ui.vellum);                
            }
            break;
        case GDK_KEY_End:
            if(ui.app_mode == ReadingMode){
                goto_page(d.num_pages - 1,
                          0, 0);
            }
            break;
        case GDK_KEY_i:
        case GDK_KEY_I:
            import_pdf();
            break;
        case GDK_KEY_n:
        case GDK_KEY_N:
        case GDK_KEY_Page_Down:
            if(ui.app_mode == ReadingMode){
                next_page();
            }
            break;
        case GDK_KEY_p:
        case GDK_KEY_P:
        case GDK_KEY_Page_Up:
            if(ui.app_mode == ReadingMode){  
                previous_page();
            }
            break;
        case GDK_KEY_w:
        case GDK_KEY_W:
            if(ui.app_mode == ReadingMode){
                zoom_width_fit();
            }
            // else if(ui.app_mode == TOCMode){
            //     if(d.toc.max_depth > 0){
            //         TOCItem *toc_item = g_list_last(d.toc.where)->data;
            //         d.toc.origin_y = toc_item->rect->y1;
            //         gtk_widget_queue_draw(ui.vellum);
            //     }
            // }
            break;
        case GDK_KEY_f:
        case GDK_KEY_F:
            if(ui.app_mode == ReadingMode){
                if(event->state & GDK_CONTROL_MASK){
                    find_widget_show();
                }
                else{
                    zoom_page_fit();
                }
            }
            break;
        case GDK_KEY_plus:
            if(ui.app_mode == ReadingMode){
                zoom_in();
            }
            break;
        case GDK_KEY_minus:
            if(ui.app_mode == ReadingMode){
                zoom_out();
            }
            break;
        case GDK_KEY_F3:
            if(ui.app_mode == ReadingMode){
                if(event->state & GDK_CONTROL_MASK){
                    if(d.find_details.find_results){
                        destroy_find_results();
                        gtk_widget_queue_draw(ui.vellum);
                    }
                }                
                else if(event->state & GDK_SHIFT_MASK){
                    find_previous();
                }
                else{
                    find_next();
                }
            }
            break;
        case 65364:
            // arrow down
            if(ui.app_mode == ReadingMode || ui.app_mode == TOCMode){
                scroll_with_pixels(0, -10);                
            }
            break;
        case 65362:
            // arrow up
            if(ui.app_mode == ReadingMode || ui.app_mode == TOCMode){
                scroll_with_pixels(0, 10);                
            }
            break;
        case 65361:
            // arrow left
            if(ui.app_mode == ReadingMode || ui.app_mode == TOCMode){
                scroll_with_pixels(10, 0);                
            }
            break;
        case 65363:
            // arrow right
            if(ui.app_mode == ReadingMode || ui.app_mode == TOCMode){
                scroll_with_pixels(-10, 0);                
            }
            break;        
        case GDK_KEY_t:
        case GDK_KEY_T:
            if(ui.app_mode == ReadingMode){            
                if(event->state & GDK_CONTROL_MASK){
                    switch_app_mode(TOCMode);
                }
                else{
                    teleport_widget_show();                
                }
            }
            else if(ui.app_mode == TOCMode){
                if(!(event->state & GDK_CONTROL_MASK)){
                    teleport_widget_show();                
                }
            }            
            break;
        case GDK_KEY_BackSpace:
            if(ui.app_mode == ReadingMode){
                go_back_restore();
            }
            break;
        case GDK_KEY_Escape:
            teleport_widget_hide();
            find_widget_hide();
            /*if(ui.is_fullscreen){
               gtk_window_unfullscreen(GTK_WINDOW(ui.main_window));
            }*/            
            switch_app_mode(ReadingMode);
            break;
        default:
            handled = FALSE;
    }
    return handled;
}

static gboolean
scroll_event_callback(GtkWidget       *widget,
                      GdkEventScroll  *event,
                      gpointer         data)
{
    if(ui.app_mode == StartMode){
        return TRUE;
    }
    int dx, dy;
    switch(event->direction){
        case GDK_SCROLL_UP:
            dy = 10;
            dx = 0;
            break;
        case GDK_SCROLL_DOWN:
            dy = -10;
            dx = 0;
            break;
        case GDK_SCROLL_LEFT:
            dy = 0;
            dx = 10;
            break;
        case GDK_SCROLL_RIGHT:
            dy = 0;
            dx = -10;
            break;
        default:
            dx = dy = 0;
    }
    scroll_with_pixels(dx, dy);
    return TRUE;
}

static gboolean
button_press_callback (GtkWidget      *widget,
                       GdkEventButton *event,
                       gpointer        data)
{  
    // only handle left click
    if(event->type != GDK_BUTTON_PRESS){
        return TRUE;
    }  
    if(ui.app_mode == StartMode){
        if(rect_contains_point(ui.import_area_rect,
                               event->x, event->y))
        { 
            import_pdf();
        }    
        else if(rect_contains_point(ui.continue_to_book_button_rect,
                                    event->x, event->y))   
        {
            switch_app_mode(ReadingMode);
        } 
        else{
        }
    }
    else if(ui.app_mode == ReadingMode){
        double image_width = cairo_image_surface_get_width(d.image);   
        double image_height = cairo_image_surface_get_height(d.image);   
        PageMeta *meta = g_ptr_array_index(d.metae,
                                       d.cur_page_num);
        /* link */
        GList *link_p = meta->links;
        while(link_p){
            Link *link = link_p->data;
            Rect img_rect = map_physical_rect_to_image(link->physical_layout,
                                                       meta->page_width,
                                                       meta->page_height,
                                                       image_width,
                                                       image_height,
                                                       d.image_origin_x,
                                                       d.image_origin_y);
            if(rect_contains_point(&img_rect,
                                   event->x, event->y))
            {
                activate_link(link);
                break;
            }
            link_p = link_p->next;
        }
        /* navigation widget */
        if(rect_contains_point(ui.prev_page_button_rect,
                               event->x, event->y))
        {
            previous_page();
        }
        if(rect_contains_point(ui.next_page_button_rect,
                               event->x, event->y))
        {
            next_page();
        }
        /* zoom widget */
        if(rect_contains_point(ui.zoom_widget_PF_rect,
                               event->x, event->y))
        {
            zoom_page_fit();
        }
        else if(rect_contains_point(ui.zoom_widget_WF_rect,
                                    event->x, event->y))
        {
            zoom_width_fit();
        }
        else if(rect_contains_point(ui.zoom_widget_IN_rect,
                                    event->x, event->y))
        {
            zoom_in();
        }
        else if(rect_contains_point(ui.zoom_widget_OUT_rect,
                                    event->x, event->y))
        {
            zoom_out();
        }
        else if(rect_contains_point(ui.teleport_launcher_rect,
                                    event->x, event->y))
        {
            teleport_widget_show();
        }
        else if(rect_contains_point(ui.find_text_launcher_rect,
                                    event->x, event->y))
        {
            find_widget_show();
        }
        else if(rect_contains_point(ui.toc_launcher_rect,
                                    event->x, event->y))
        {
            switch_app_mode(TOCMode);
        }
        else{            
        }
    }
    else if(ui.app_mode == TOCMode){
        if(d.toc.max_depth == 0){
            switch_app_mode(ReadingMode);            
        }
        else{
            if(d.toc.hovered_item){
                goto_toc_item_page(d.toc.hovered_item);
                switch_app_mode(ReadingMode);
            }
            if(d.toc.hovered_navigation_button){
                int button_index = g_list_index(d.toc.navigation_button_rects,
                                                d.toc.hovered_navigation_button);
                const char *label = g_list_nth_data(d.toc.labels,
                                                    button_index / 2);
                int saved_cur_page_num = d.cur_page_num;
                enum TOCType toc_type = toc_get_item_type(label);
                switch(toc_type){
                    case Part:
                        if(button_index % 2 == 0){
                            previous_part();
                        }
                        else{
                            next_part();
                        }
                        break;
                    case Chapter:
                        if(button_index % 2 == 0){
                            previous_chapter();
                        }
                        else{
                            next_chapter();
                        }
                        break;
                    case Section:
                        if(button_index % 2 == 0){
                            previous_section();
                        }
                        else{
                            next_section();
                        }
                        break;
                    case Subsection:
                        if(button_index % 2 == 0){
                            previous_subsection();
                        }
                        else{
                            next_subsection();
                        }
                        break;
                    default:;
                }
                if(saved_cur_page_num != d.cur_page_num){
                    // switch_app_mode(ReadingMode);
                }
            }
        }
    }
    else{        
    }
    return TRUE;                     
}

static gboolean
motion_event_callback (GtkWidget      *widget,
                       GdkEventMotion *event,
                       gpointer        data)
{
    gboolean is_cursor_set = FALSE;
    if(ui.app_mode == StartMode){
        ui.is_import_area_hovered = rect_contains_point(ui.import_area_rect,
                                                        event->x, event->y);
        ui.is_continue_to_book_button_hovered =  rect_contains_point(ui.continue_to_book_button_rect,
                                                                     event->x, event->y);
        is_cursor_set = ui.is_import_area_hovered || ui.is_continue_to_book_button_hovered;
    }
    else if(ui.app_mode == ReadingMode){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           d.cur_page_num);
        double image_width = cairo_image_surface_get_width(d.image);
        double image_height = cairo_image_surface_get_height(d.image);
        /* link hover */
        gboolean is_link_hovered = FALSE;
        GList *link_p = meta->links;
        while(link_p){
            Link *link = link_p->data;
            Rect img_rect = map_physical_rect_to_image(link->physical_layout,
                                                       meta->page_width,
                                                       meta->page_height,
                                                       image_width,
                                                       image_height,
                                                       d.image_origin_x,
                                                       d.image_origin_y);
            link->is_hovered = rect_contains_point(&img_rect,
                                                   event->x, event->y);
            if(!is_link_hovered){
                is_link_hovered = link->is_hovered;
            }
            link_p = link_p->next;
        }
        /* show referenced figure */
        meta->active_referenced_figure = NULL;
        GList *list_p = meta->referenced_figures;
        while(list_p && !meta->active_referenced_figure){
            ReferencedFigure *ref_figure = list_p->data;
            GList *result_p = ref_figure->find_results;
            while(result_p && !meta->active_referenced_figure){
                FindResult *find_result = result_p->data;
                GList *rect_p = find_result->physical_layouts;
                while(rect_p){
                    Rect img_layout = map_physical_rect_to_image(rect_p->data,
                                                                 meta->page_width,
                                                                 meta->page_height,
                                                                 image_width,
                                                                 image_height,
                                                                 d.image_origin_x,
                                                                 d.image_origin_y);
                    if(rect_contains_point(&img_layout,
                                           event->x, event->y))
                    {                    
                        break;
                    }
                    rect_p = rect_p->next;
                }
                if(rect_p){
                    ref_figure->activated_find_result = result_p;
                    meta->active_referenced_figure = ref_figure;
                }
                else{
                    ref_figure->activated_find_result = NULL;
                }
                result_p = result_p->next;
            }
            list_p = list_p->next;            
        }
        /* navigation widget hover */
        ui.is_prev_page_button_hovered = d.cur_page_num > 0 &&
                                         rect_contains_point(ui.prev_page_button_rect,
                                                             event->x, event->y);
        ui.is_next_page_button_hovered = d.cur_page_num < d.num_pages - 1 &&
                                         rect_contains_point(ui.next_page_button_rect,
                                                             event->x, event->y);
        /* zoom widget */
        ui.is_zoom_widget_PF_hovered = rect_contains_point(ui.zoom_widget_PF_rect,
                                                           event->x, event->y);    
        ui.is_zoom_widget_WF_hovered = rect_contains_point(ui.zoom_widget_WF_rect,
                                                           event->x, event->y);
        ui.is_zoom_widget_IN_hovered = rect_contains_point(ui.zoom_widget_IN_rect,
                                                           event->x, event->y);
        ui.is_zoom_widget_OUT_hovered = rect_contains_point(ui.zoom_widget_OUT_rect,
                                                            event->x, event->y);
        /* launchers */
        ui.is_teleport_launcher_hovered = rect_contains_point(ui.teleport_launcher_rect,
                                                              event->x, event->y);
        ui.is_find_text_launcher_hovered = rect_contains_point(ui.find_text_launcher_rect,
                                                               event->x, event->y);
        ui.is_toc_launcher_hovered = rect_contains_point(ui.toc_launcher_rect,
                                                         event->x, event->y);
        is_cursor_set = is_link_hovered ||
                        ui.is_prev_page_button_hovered || ui.is_next_page_button_hovered ||
                        ui.is_zoom_widget_PF_hovered || ui.is_zoom_widget_WF_hovered || 
                        ui.is_zoom_widget_IN_hovered || ui.is_zoom_widget_OUT_hovered ||
                        ui.is_teleport_launcher_hovered || ui.is_find_text_launcher_hovered ||
                        ui.is_toc_launcher_hovered;
    }
    else if(ui.app_mode == TOCMode){
        d.toc.hovered_item = NULL;
        d.toc.hovered_navigation_button = NULL;
        double widget_height = gtk_widget_get_allocated_height(ui.vellum);
        if(event->y < widget_height - toc_navigation_panel_height){
            GList *item_p = d.toc.flattened_items;
            while(item_p){
                TOCItem *toc_item = item_p->data;
                if(rect_contains_point(toc_item->rect,
                                       event->x, event->y))
                {
                    break;
                }
                item_p = item_p->next;
            }
            if(item_p){
                d.toc.hovered_item = item_p->data;
                is_cursor_set = TRUE;
            }
        }
        else{
            d.toc.hovered_navigation_button = NULL;
            GList *list_p = d.toc.navigation_button_rects;
            while(list_p){
                Rect *button_rect = list_p->data;
                if(rect_contains_point(button_rect,
                                       event->x, event->y))
                {
                    break;
                }
                list_p = list_p->next;
            }
            if(list_p){
                d.toc.hovered_navigation_button = list_p->data;
                is_cursor_set = TRUE;
            }
        }
    }
    else{        
    }
    gdk_window_set_cursor(gtk_widget_get_window(ui.vellum),
                          is_cursor_set ? ui.pointer_cursor : ui.default_cursor);
    gtk_widget_queue_draw(ui.vellum); 
    return TRUE;
}

static gboolean
tooltip_event_callback(GtkWidget  *widget,
                       gint        x,
                       gint        y,
                       gboolean    keyboard_mode,
                       GtkTooltip *tooltip,
                       gpointer    user_data)
{
    if(keyboard_mode){
        return FALSE; 
    }
    if(ui.app_mode == ReadingMode){
        /* units */
        char *unit_tip = NULL;  
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           d.cur_page_num);
        double image_width = cairo_image_surface_get_width(d.image);
        double image_height = cairo_image_surface_get_height(d.image);
        ConvertedUnit *tooltip_cv = NULL;
        GList *list_p = meta->converted_units;
        while(list_p){
            ConvertedUnit *cv = list_p->data;
            GList *result_p = cv->find_results;
            while(result_p){
                FindResult *fr = result_p->data;
                GList *rect_p = fr->physical_layouts;
                while(rect_p){
                    Rect img_rect = map_physical_rect_to_image(rect_p->data,
                                                               meta->page_width,
                                                               meta->page_height,
                                                               image_width,
                                                               image_height,
                                                               d.image_origin_x,
                                                               d.image_origin_y);
                    if(rect_contains_point(&img_rect,
                                           x, y))
                    {
                        tooltip_cv = cv;
                        break;
                    }
                    rect_p = rect_p->next;
                }
                if(tooltip_cv){
                    break;
                }
                result_p = result_p->next;
            }
            if(tooltip_cv){
                break;
            }
            list_p = list_p->next;
        }
        if(tooltip_cv){
            unit_tip = g_strdup_printf("<span font='sans 10' foreground='#c3c3c3'>Unit</span>: %s",
                                       tooltip_cv->value_str);
        }
        /* find results */
        char *find_tip = NULL;
        list_p = meta->find_results;
        while(list_p){
            FindResult *fr = list_p->data;
            GList *rect_p = fr->physical_layouts;
            while(rect_p){
                Rect img_rect = map_physical_rect_to_image(rect_p->data,
                                                           meta->page_width,
                                                           meta->page_height,
                                                           image_width,
                                                           image_height,
                                                           d.image_origin_x,
                                                           d.image_origin_y);
                if(rect_contains_point(&img_rect,
                                       x, y))
                {
                    break;
                }
                rect_p = rect_p->next;
            }
            if(rect_p){
                break;
            }
            list_p = list_p->next;
        }
        if(list_p){
            FindResult *fr = list_p->data;
            find_tip = g_strdup_printf("<span font='sans 10' foreground='#c3c3c3'>Find</span>: %s",
                                       fr->tip);
        }
        /* links */
        char *link_tip = NULL;
        list_p = meta->links;
        while(list_p){
            Link *link = list_p->data;
            Rect img_rect = map_physical_rect_to_image(link->physical_layout,
                                                       meta->page_width,
                                                       meta->page_height,
                                                       image_width,
                                                       image_height,
                                                       d.image_origin_x,
                                                       d.image_origin_y);
            if(rect_contains_point(&img_rect,
                                   x, y))
            {
                link_tip = g_strdup_printf("<span font='sans 10' foreground='#c3c3c3'>Link</span>: %s",
                                           link->tip);
                break;
            }
            list_p = list_p->next;
        }
        char *tip_markup = NULL;
        if(unit_tip || find_tip || link_tip){   
            char *tip_markup = g_strconcat(unit_tip ? unit_tip : "",
                                           unit_tip && find_tip ? "\n\n" : "",
                                           find_tip ? find_tip : "",
                                           (unit_tip || find_tip) && link_tip ? "\n\n" : "",
                                           link_tip ? link_tip : "",
                                           NULL);
            gtk_tooltip_set_markup(GTK_TOOLTIP(tooltip),
                                   tip_markup); 
            g_free(unit_tip);
            g_free(find_tip);
            g_free(link_tip);
            g_free(tip_markup);
            return TRUE;
        }
        /* tips for page widgets */
        if(ui.is_zoom_widget_PF_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Fits page inside window(F)</span>";
        }
        else if(ui.is_zoom_widget_WF_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Stretches the page to fit width(W)</span>";
        }
        else if(ui.is_zoom_widget_IN_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Brings book closer(+)</span>";
        }
        else if(ui.is_zoom_widget_OUT_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Pushes book farther(-)</span>";
        }
        else if(ui.is_teleport_launcher_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Lets you jump to various book locations(T)</span>";
        }
        else if(ui.is_find_text_launcher_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Let's you search for textual data(CTRL + F)</span>";
        }
        else if(ui.is_toc_launcher_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Shows table of contents(CTRL + T)</span>";
        }
        if(tip_markup){
            gtk_tooltip_set_markup(GTK_TOOLTIP(tooltip),
                                   tip_markup); 
            return TRUE;
        }
    }
    else if(ui.app_mode == StartMode){
        char *tip_markup = NULL;
        if(ui.is_import_area_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Let's you choose a book(I)</span>";            
        }
        if(ui.is_continue_to_book_button_hovered){
            tip_markup = "<span font='sans 10' foreground='#c3c3c3'>Hit to continue reading book(R or Escape)</span>";
        }
        if(tip_markup){
            gtk_tooltip_set_markup(GTK_TOOLTIP(tooltip),
                                   tip_markup); 
            return TRUE;
        }
    }
    else if(ui.app_mode == TOCMode){
    }
    else{        
    }
    return FALSE; 
}

static void
destroy_app()
{
    rect_free(ui.import_area_rect);
    rect_free(ui.book_details_area_rect);
    rect_free(ui.continue_to_book_button_rect);
    rect_free(ui.app_info_area_rect);
    
    rect_free(ui.zoom_widget_PF_rect);
    rect_free(ui.zoom_widget_WF_rect);
    rect_free(ui.zoom_widget_IN_rect);
    rect_free(ui.zoom_widget_OUT_rect);
    
    rect_free(ui.teleport_launcher_rect);
    rect_free(ui.find_text_launcher_rect);
    rect_free(ui.toc_launcher_rect);
    
    g_object_unref(ui.default_cursor);
    g_object_unref(ui.text_cursor); 
    g_object_unref(ui.pointer_cursor); 
    
    if(d.go_back_stack){
        g_queue_free(d.go_back_stack);
    }
    g_regex_unref(navigation_regex);
    toc_module_destroy();
    unit_convertor_module_destroy();
    figure_module_destroy();

    teleport_widget_destroy();
    find_widget_destroy();

    gtk_widget_destroy(ui.main_window);    
}

static gboolean
on_app_quit(GtkWidget *widget,
            GdkEvent  *event,
            gpointer   data)
{
    destroy_document();
    destroy_app();
    return FALSE;
}

void
init_app(GtkApplication *app)
{
    ui.main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(ui.main_window), "readaratus");
    gtk_window_set_default_size(GTK_WINDOW(ui.main_window),
                                800, 600);
    gtk_widget_set_events(ui.main_window, 
                          GDK_STRUCTURE_MASK);
    ui.is_fullscreen = FALSE;
    g_signal_connect(G_OBJECT(ui.main_window), "window_state_event",
                     G_CALLBACK(window_state_callback), NULL);
    ui.vellum = gtk_drawing_area_new();
    gtk_widget_set_size_request(ui.vellum,
                                800, 600);
    gtk_widget_set_has_tooltip(ui.vellum,
                               TRUE);
    g_signal_connect(G_OBJECT(ui.vellum), "draw",
                     G_CALLBACK(draw_callback), NULL);
    g_signal_connect(G_OBJECT(ui.vellum), "configure-event",
                     G_CALLBACK(configure_callback), NULL);
    g_signal_connect(G_OBJECT(ui.main_window), "key_press_event",
                     G_CALLBACK(key_press_callback), NULL);
    g_signal_connect(G_OBJECT(ui.vellum), "scroll_event",
                     G_CALLBACK(scroll_event_callback), NULL);
    g_signal_connect(G_OBJECT(ui.vellum), "button_press_event",
                     G_CALLBACK(button_press_callback), NULL);
    g_signal_connect(G_OBJECT(ui.vellum), "motion_notify_event",
                     G_CALLBACK(motion_event_callback), NULL);
    g_signal_connect(G_OBJECT(ui.vellum), "query_tooltip",
                     G_CALLBACK(tooltip_event_callback), NULL);
    g_signal_connect(G_OBJECT(ui.main_window), "delete_event",
                     G_CALLBACK(on_app_quit), NULL);
    gtk_widget_set_events(ui.vellum, 
                          GDK_KEY_PRESS_MASK | 
                          GDK_SCROLL_MASK | 
                          GDK_BUTTON_PRESS_MASK |
                          GDK_POINTER_MOTION_MASK);
    /* app mode */
    ui.app_mode = StartMode;
    /* start mode widgets */
    ui.is_import_area_hovered = FALSE;
    ui.import_area_rect = rect_new();
    ui.book_details_area_rect = rect_new();
    ui.is_continue_to_book_button_hovered = FALSE;
    ui.continue_to_book_button_rect = rect_new();
    ui.app_info_area_rect = rect_new();
    ui.app_info_text = 
        "<span font='sans 12' foreground='black'><b>readaratus</b>\n"
        "A dynamic book reading system for computers\n\n"
        "Version 1.0\n"
        "Copyright © 2020 <i>readaratus team</i>\n</span>"
        "<span font='sans 12' foreground='blue'>www.readaratus.ir</span>\n\n\n\n"
        "<span font='sans 10' foreground='black'>This program is provided without any warranty and is licensed under the terms of\n"
        "GNU General Public License v3 or later.\n"
        "For extended inquiry please visit</span>\n"
        "<span font='sans 10' foreground='blue'>www.gnu.org/licenses/gpl-3.0.html</span>\n\n\n\n"
        "<span font='sans 10' foreground='black'>To get help press </span><span font='sans 10' foreground='blue'>H</span>";
    /* reading mode widgets */
    /* cursors */
    ui.default_cursor = gdk_cursor_new_from_name(gtk_widget_get_display(ui.vellum),
                                                 "default"); 
    ui.text_cursor = gdk_cursor_new_from_name(gtk_widget_get_display(ui.vellum),
                                              "text"); 
    ui.pointer_cursor = gdk_cursor_new_from_name(gtk_widget_get_display(ui.vellum),
                                                 "pointer"); 
    /* zoom widget */
    ui.is_zoom_widget_PF_hovered = FALSE;
    ui.zoom_widget_PF_rect = rect_new();
    ui.is_zoom_widget_WF_hovered = FALSE;
    ui.zoom_widget_WF_rect = rect_new();
    ui.is_zoom_widget_IN_hovered = FALSE;
    ui.zoom_widget_IN_rect = rect_new();
    ui.is_zoom_widget_OUT_hovered = FALSE;
    ui.zoom_widget_OUT_rect = rect_new();
    /* page buttons */
    ui.is_prev_page_button_hovered = FALSE;
    ui.prev_page_button_rect = rect_new();
    ui.is_next_page_button_hovered = FALSE;
    ui.next_page_button_rect = rect_new();
    /* launchers */
    ui.teleport_launcher_rect = rect_new();
    ui.find_text_launcher_rect = rect_new();
    ui.toc_launcher_rect = rect_new();

    d.go_back_stack = g_queue_new();

    gtk_container_add(GTK_CONTAINER(ui.main_window), ui.vellum);    
    gtk_widget_show_all(ui.main_window);
    gdk_window_set_cursor(gtk_widget_get_window(ui.vellum),
                          ui.default_cursor);
    pose_page_widgets();

    d.zoom_level = PageFit;
    zero_document();
    toc_module_init();
    unit_convertor_module_init();
    figure_module_init();
    ui.teleport_widget = teleport_widget_init(ui.main_window);
    g_signal_connect(G_OBJECT(ui.teleport_widget), "teleport_request_event",
                     G_CALLBACK(on_teleport_request_received), NULL);
    ui.find_widget = find_widget_init(ui.main_window);
    g_signal_connect(G_OBJECT(ui.find_widget), "find_request_event",
                     G_CALLBACK(on_find_request_received), NULL);
    GError *err = NULL;
    navigation_regex = g_regex_new(
        "^(?<command>next|prev(ious)?)\\s*(?<label>page|part|ch(apter)?|(sub)?sec(tion)?)$",
        G_REGEX_CASELESS | G_REGEX_NO_AUTO_CAPTURE,
        0,
        &err);
    if(!navigation_regex){
        g_print("navigation regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }
}