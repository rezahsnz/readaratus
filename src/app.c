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
#include <cairo.h>
#include <pango/pangocairo.h>
#include <math.h>
#include "find.h"
#include "toc_synthesis.h"
#include "teleport_widget.h"
#include "find_widget.h"
#include "page_meta.h"
#include "unit_convertor.h"
#include "roman_numeral.h"

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
/* darkblue: #, (254, 90, 29) */
static const double blue_r = 0,
                    blue_g = 0,
                    blue_b = 1;

static const double dashed_style[2] = {8, 5};
static const int num_dashes = 2;
static const double toc_navigation_panel_height = 72;

static void 
pose_page_widgets(void)
{
    double widget_width = gtk_widget_get_allocated_width(ui.vellum);
    double widget_height = gtk_widget_get_allocated_height(ui.vellum);
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
    const double panel_button_size = widget_width > 800 ? 47 : 33,
                 panel_margin = widget_width > 800 ? 12 : 8;
    const double panel_width = 2 * panel_margin + 
        2 * (panel_button_size + panel_margin) + 7 * panel_button_size;
    const double panel_height = 2 * panel_margin + 2.75 * panel_button_size;
    ui.panel_rect->x1 = widget_width / 2 - panel_width / 2;
    ui.panel_rect->y1 = widget_height - panel_height - panel_margin;
    ui.panel_rect->x2 = ui.panel_rect->x1 + panel_width;
    ui.panel_rect->y2 = ui.panel_rect->y1 + panel_height;
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
        double hidden_width = image_width - widget_width;
        d.image_origin_x = -MIN(fabs(progress_x * image_width), hidden_width / 2.0);
    }
    if(image_height <= widget_height){
        d.image_origin_y = widget_height / 2.0 - image_height / 2.0;
    }
    else{
        double hidden_height = image_height - widget_height;        
        d.image_origin_y = -MIN(fabs(progress_y * image_height), hidden_height);
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
    if(d.toc.head_item){
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
    d.preserved_progress_x = 0.0;
    d.preserved_progress_y = 0.0;
    goto_page(d.cur_page_num + 1,
              0, 0);
}

static void
previous_page(void)
{
    d.preserved_progress_x = 0.0;
    d.preserved_progress_y = 0.0;
    goto_page(d.cur_page_num - 1,
              0, 0.999);
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
        double my_dx = dx,
               my_dy = dy; 
        switch(d.zoom_level){
            case PageFit:
                my_dx = my_dy = 0;
                break;
            case WidthFit:
                /* vertical only */
                my_dx = 0;
                if(my_dy + d.image_origin_y > 0){
                    my_dy = 0;
                }
                if(abs(my_dy + d.image_origin_y) > hidden_portion_height){
                    my_dy = 0;
                }
                break;
            default:
                /* vertical */
                if(my_dy + d.image_origin_y > 0){
                    my_dy = 0;
                }
                if(abs(my_dy + d.image_origin_y) > hidden_portion_height){
                    my_dy = 0;
                }
                /* horizontal */
                if(my_dx + d.image_origin_x > 0){
                    my_dx = 0;
                }
                if(abs(my_dx + d.image_origin_x) > hidden_portion_width){
                    my_dx = 0;
                }
                break;
        }
        if(my_dx != 0.0 || my_dy != 0.0){
            d.image_origin_x += my_dx;
            d.image_origin_y += my_dy;
            d.preserved_progress_x = (d.image_origin_x) / cairo_image_surface_get_width(d.image);
            d.preserved_progress_y = (d.image_origin_y) / cairo_image_surface_get_height(d.image);
            gtk_widget_queue_draw(ui.vellum);
        }
        else{
            /* semi continous mode */
            if(dy > 0){
                previous_page();
            }
            else if(dy < 0){
                next_page();
            }
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

static void
load_text_layouts(PageMeta *meta)
{
    meta->num_layouts = 0;
    meta->physical_text_layouts = NULL;
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
load_links()
{
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        meta->links = NULL;
        GList *link_mappings = poppler_page_get_link_mapping(meta->page);
        GList *link_p = link_mappings;
        while(link_p){
            PopplerLinkMapping *link_mapping = link_p->data;
            Link *link = g_malloc(sizeof(Link));
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
                    PageMeta *meta_target = g_ptr_array_index(d.metae,
                                                              target.page_num);
                    if(meta_target->page_label->label){
                        link->tip = g_strdup_printf("Page '%s'",
                                                    meta_target->page_label->label);
                    }
                    else{
                        link->tip = g_strdup_printf("Page(index): '%d'",
                                                    target.page_num);
                    }
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
            GList *find_results = find_text(d.metae,
                                            cv->whole_match,
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
    /* 1: check if document provides TOC */
     toc_create_from_poppler_index(d.doc,
                                   &d.toc.head_item);    
    /* 2: locate contents/index/toc pages */
    if(!d.toc.head_item){
        g_print("Document provides no index, trying to synthesize TOC from the contents' pages.\n");
        toc_create_from_contents_pages(d.doc,
                                       d.metae,
                                       d.page_label_num_hash,
                                       &d.toc.head_item);
    }
    /* 3: scan all pages and create TOC */
    if(!d.toc.head_item){
    }

    if(d.toc.head_item){
        char *title = poppler_document_get_title(d.doc);
        d.toc.head_item->title = 
            (!title || strlen(title) == 0) ? g_strdup("Head")
                                           : title;
        toc_flatten(d.toc.head_item,
                    &d.toc.flattened_items);        
        GList *list_p = d.toc.flattened_items;
        while(list_p){
            TOCItem *toc_item = list_p->data;
            if(toc_item->label && string_index_in_list(d.toc.labels,
                                                       toc_item->label,
                                                       FALSE) < 0)
            {
                d.toc.labels = g_list_append(d.toc.labels,
                                             g_strdup(toc_item->label));
                d.toc.navigation_button_rects = g_list_append(d.toc.navigation_button_rects,
                                                              rect_new());
                d.toc.navigation_button_rects = g_list_append(d.toc.navigation_button_rects,
                                                              rect_new());
            }
            list_p = list_p->next;
        }           
    }
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
create_image_for_figure(PageMeta *meta,
                        Figure   *figure)
{
    /*
      create the image that figure represents.
      render the target page to a suitable resolution and then crop the image
      out of it.
    */
    const double MIN_PAGE_WIDTH_PX = 640;
    cairo_surface_t *first_image_surface = poppler_page_get_image(meta->page,
                                                                  figure->image_id);
    double page_render_width = cairo_image_surface_get_width(first_image_surface) * 
        (meta->page_width / rect_width(figure->image_physical_layout));
    page_render_width = MAX(MIN_PAGE_WIDTH_PX, page_render_width);
    double page_render_height = meta->aspect_ratio * page_render_width;
    cairo_surface_destroy(first_image_surface);
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
load_figures(void)
{        
    gboolean labels_are_exclusive = FALSE,
             ids_are_complex = FALSE;
    GList *all_figures = NULL;
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        GList *image_mappings = poppler_page_get_image_mapping(meta->page);
        /* tiny image = noise */
        GList *image_mappings_p = image_mappings;
        while(image_mappings_p){
            GList *next = image_mappings_p->next;
            PopplerImageMapping *img = image_mappings_p->data;
            double img_width = fabs(img->area.x2 - img->area.x1),
                   img_height = fabs(img->area.y2 - img->area.y1);
            if(img_width < meta->mean_line_height ||
               img_height < meta->mean_line_height)
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
        g_list_free_full(fig_list,
                         (GDestroyNotify)figure_free);
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
            closest_figure->image = create_image_for_figure(meta,
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
        } 
        figure_list_p = figure_list_p->next;
    }
    g_list_free_full(g_hash_table_get_keys(processed_images_hash_table),
                     (GDestroyNotify)g_free);
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
resolve_referenced_figures(void)
{
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
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
            for(int ref_page_num = 0; ref_page_num < d.num_pages; ref_page_num++){
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
                    char *needle = g_strdup_printf("%s %s",
                                                   ref_figure->label,
                                                   ref_figure->id);
                    ref_figure->find_results = find_text(d.metae,
                                                         needle,
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
        poppler_page_get_size(meta->page,
                              &meta->page_width,
                              &meta->page_height);
        meta->aspect_ratio = meta->page_height / meta->page_width;
        meta->text = poppler_page_get_text(meta->page);        
        load_text_layouts(meta);
        meta->links = NULL;
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
fix_page_labels(void)
{
    /* 
       Page labels are 'lone' numerals that are the most distant(wrt Y-center)
       objects to the center of the page. Once they are found, a frequency
       count is performed and the most frequent diff(between 0-based page_num
       and label) is chosen as the value that gets subtracted from the
       page_num of each page to get the actual page label.
       Initial pages of books are usually labeled with roman numerals. If any 
       page is found to be labeled this way, it is used as a reference for the
       labeling of its neighbour pages.
    */
    GError *err = NULL;
    GRegex *page_label_regex = NULL;
    const char *pattern = 
        "# roman range: 1-99\n"
        "^((XC|XL|L?X{0,3})(IX|IV|V?I{0,3})|\\d+)\\b|\n"
        "\\b((XC|XL|L?X{0,3})(IX|IV|V?I{0,3})|\\d+)$";
    page_label_regex = g_regex_new(pattern,
                                   G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_MULTILINE | G_REGEX_NO_AUTO_CAPTURE,
                                   G_REGEX_MATCH_NOTEMPTY,
                                   &err);
    if(!page_label_regex){
        g_print("page_label_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        return; 
    }    
    GHashTable *diff_freq_hash = g_hash_table_new(g_direct_hash,
                                                  g_direct_equal);
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        meta->page_label = g_malloc(sizeof(PageLabel));
        meta->page_label->label = NULL;
        meta->page_label->physical_layout = NULL;

        double min_dist = meta->page_height;
        char *page_label_str = NULL;
        GMatchInfo *match_info = NULL;
        g_regex_match(page_label_regex,
                      meta->text,
                      0,
                      &match_info);
        while(g_match_info_matches(match_info)){
            char *match = g_match_info_fetch(match_info,
                                             0);
            GList *pop_list = poppler_page_find_text_with_options(meta->page,
                                                                  match,
                                                                  POPPLER_FIND_WHOLE_WORDS_ONLY);
            GList *list_p = pop_list;
            while(list_p){
                Rect *rect = rect_from_poppler_rectangle(list_p->data);
                rect->y1 = meta->page_height - rect->y1;
                rect->y2 = meta->page_height - rect->y2;
                double center_y = rect_center_y(rect);
                double dist = MIN(center_y, meta->page_height - center_y);
                if((dist < min_dist) &&
                   (dist < 0.35 * meta->page_height))
                {
                    page_label_str = match;
                    min_dist = dist;
                }
                else{
                    rect_free(rect);
                }
                list_p = list_p->next;
            }
            g_list_free(pop_list);
            if(page_label_str != match){
                g_free(match);
            }
            g_match_info_next(match_info,
                              NULL);
        }
        g_match_info_free(match_info); 
        if(!page_label_str){
            continue;
        }                
        meta->page_label->label = page_label_str;
        char *end_ptr = NULL;
        int page_label_decimal = g_ascii_strtoll(page_label_str,
                                                 &end_ptr,
                                                 10);
        if((page_label_decimal == 0) && (page_label_str == end_ptr)){
            continue;
        }
        int diff = page_num - page_label_decimal;
        int freq = 0;        
        if(g_hash_table_contains(diff_freq_hash,
                                 GINT_TO_POINTER(diff)))
        {
            freq = GPOINTER_TO_INT(g_hash_table_lookup(diff_freq_hash,
                                                       GINT_TO_POINTER(diff)));
        }
        freq += 1;
        g_hash_table_insert(diff_freq_hash,
                            GINT_TO_POINTER(diff),
                            GINT_TO_POINTER(freq));
    }
    g_regex_unref(page_label_regex);
    int max_freq = -1, actual_diff = 0;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, diff_freq_hash);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        int diff = GPOINTER_TO_INT(key);
        int freq = GPOINTER_TO_INT(value);
        if(freq > max_freq){
            max_freq = freq;
            actual_diff = diff;
        }
    }
    g_hash_table_unref(diff_freq_hash);
    /* at least half of pages should suggest the same diff, otherwise simple
       index-labeling will be used. */
    if(max_freq < d.num_pages / 2){
        for(int page_num = 0; page_num < d.num_pages; page_num++){
            PageMeta *meta = g_ptr_array_index(d.metae,
                                               page_num);
            g_free(meta->page_label->label);
            meta->page_label->label = g_strdup_printf("%d",
                                                      page_num + 1);
            g_hash_table_insert(d.page_label_num_hash,
                                meta->page_label->label,
                                GINT_TO_POINTER(page_num));
        }
        g_print("Majority of the pages provide no labels, using page index instead.\n");
        return;
    }
    /* find the first roman-labeled page starting from the last possible
       roman-labeled page (because first pages of books are usually un-labeled) */
    int frl_page_num = actual_diff;
    for(; frl_page_num >= 0; frl_page_num--){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           frl_page_num);        
        if(roman_to_decimal(meta->page_label->label) > -1){
            break;
        }
    }
    if(frl_page_num >= 0){
        PageMeta *frl_meta = g_ptr_array_index(d.metae,
                                               frl_page_num);
        /* label pages before the first roman-labeled page */
        int pre_frl_page_num = frl_page_num - 1;
        const char *pre_frl_label = roman_previous(frl_meta->page_label->label);
        gboolean is_upper = g_ascii_isupper(frl_meta->page_label->label[0]);
        while(pre_frl_page_num >= 0 && pre_frl_label){
            PageMeta *pre_frl_meta = g_ptr_array_index(d.metae,
                                                       pre_frl_page_num);
            g_free(pre_frl_meta->page_label->label);
            pre_frl_meta->page_label->label = is_upper ? g_strdup(pre_frl_label)
                                                       : g_ascii_strdown(pre_frl_label,
                                                                         -1);
            pre_frl_label = roman_previous(pre_frl_label);            
            pre_frl_page_num--;
        }
        /* if no more roman numerals exist, unlabel the remaining pages */
        for(int page_num = 0; page_num < pre_frl_page_num; page_num++){
            PageMeta *pre_roman_meta = g_ptr_array_index(d.metae,
                                                         page_num);
            g_free(pre_roman_meta->page_label->label);
            pre_roman_meta->page_label->label = NULL;
        }
        /* label pages after the first roman labeled page */
        int post_frl_page_num = frl_page_num + 1;
        const char *post_frl_label = roman_next(frl_meta->page_label->label);
        while(post_frl_page_num <= actual_diff && post_frl_label){
            PageMeta *post_frl_meta = g_ptr_array_index(d.metae,
                                                        post_frl_page_num);
            g_free(post_frl_meta->page_label->label);
            post_frl_meta->page_label->label = is_upper ? g_strdup(post_frl_label)
                                                        : g_ascii_strdown(post_frl_label,
                                                                         -1);
            post_frl_label = roman_next(post_frl_label);
            post_frl_page_num++;
        }
        /* if no more roman numerals exist, unlabel the remaining pages */
        for(int page_num = post_frl_page_num; page_num <= actual_diff; page_num++){
            PageMeta *post_roman_meta = g_ptr_array_index(d.metae,
                                                          page_num);
            g_free(post_roman_meta->page_label->label);
            post_roman_meta->page_label->label = NULL;
        }
    }
    else{
        /* in case no roman labels are found, unlabel the pages */
        for(int page_num = 0; page_num <= actual_diff; page_num++){
            PageMeta *unlabeled_meta = g_ptr_array_index(d.metae,
                                                         page_num);
            g_free(unlabeled_meta->page_label->label);
            unlabeled_meta->page_label->label = NULL;
        }
    }
    /* assign numeric labels to decimally labelled pages */
    for(int page_num = actual_diff + 1; page_num < d.num_pages; page_num++)
    {
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        g_free(meta->page_label->label);
        meta->page_label->label = g_strdup_printf("%d",
                                                  page_num - actual_diff);
    }
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        if(!meta->page_label->label){
            continue;
        }
        g_hash_table_insert(d.page_label_num_hash,
                            meta->page_label->label,
                            GINT_TO_POINTER(page_num));
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
    d.toc.labels = NULL;
    d.toc.flattened_items = NULL;
    d.toc.navigation_button_rects = NULL;
    d.toc.where = NULL;
    d.toc.origin_x = 0;
    d.toc.origin_y = 0;
    d.find_details.selected_p = NULL;
    d.find_details.find_results = NULL;
    d.find_details.max_results = 0;
    d.find_details.max_results_page_num = -1;    
}

static void
setup_text_completions(void)
{
    GList *text_list = NULL;
    /* page num/label */
    for(int page_num = 0; page_num < d.num_pages; page_num++){
        PageMeta *meta = g_ptr_array_index(d.metae,
                                           page_num);
        text_list = g_list_append(text_list,
                                  g_strdup(meta->page_label->label));
    }
    teleport_widget_update_text_completions(text_list,
                                            "Page");
    g_list_free_full(text_list,
                     (GDestroyNotify)g_free);
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
    g_list_free_full(text_list,
                     (GDestroyNotify)g_free);
    text_list = NULL;
    /* toc items */
    list_p = d.toc.flattened_items;
    while(list_p){
        TOCItem *toc_item = list_p->data;
        if(toc_item->depth > 0){            
            text_list = g_list_append(text_list,
                                      g_strdup(toc_item->title));
        }
        list_p = list_p->next;
    }
    teleport_widget_update_text_completions(text_list,
                                            "TOC");
    g_list_free_full(text_list,
                     (GDestroyNotify)g_free);
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
    g_list_free_full(text_list,
                     (GDestroyNotify)g_free);
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
        g_list_free_full(meta->converted_units,
                         (GDestroyNotify)converted_unit_free);
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
            g_list_free_full(ref_figure->find_results,
                             (GDestroyNotify)find_result_free);
            g_free(ref_figure);
            list_p = list_p->next;
        }
        g_list_free(meta->referenced_figures);
        g_object_unref(meta->page);     
        if(meta->page_label){
            g_free(meta->page_label->label);        
            rect_free(meta->page_label->physical_layout);
        }
        g_free(meta->page_label);
        g_free(meta);
    }
    g_ptr_array_unref(d.metae);
    cairo_surface_destroy(d.image);
    g_list_free_full(d.find_details.find_results,
                     (GDestroyNotify)find_result_free);
    toc_destroy(d.toc.head_item);
    g_list_free_full(d.toc.labels,
                     (GDestroyNotify)g_free);    
    g_list_free_full(d.toc.navigation_button_rects,
                     (GDestroyNotify)rect_free);
    g_list_free(d.toc.flattened_items);
    g_list_free(d.find_details.find_results);
    if(d.go_back_stack){
        gpointer *p = g_queue_pop_head(d.go_back_stack);
        while(p){
            g_free((GoBack*)p);
            p = g_queue_pop_head(d.go_back_stack);
        }
    }
    g_hash_table_remove_all(d.page_label_num_hash);
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
                                        "/home/reza/book");
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
    g_print("Fixing page labels...\n");
    fix_page_labels();
    g_print("Processing links...\n");
    load_links();
    g_print("Loading TOC...\n");
    load_toc();
    g_print("Converting units...\n");
    load_units();
    g_print("Loading figures...\n");
    load_figures();
    resolve_referenced_figures();
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
previous_subsection(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Subsection &&
           toc_item->previous &&
           toc_get_item_type(toc_item->previous->label) == Subsection)
        {
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *subsection_item = item_p->data;        
        goto_toc_item_page(subsection_item->previous);
    }
    g_list_free(where);    
}

static void
next_subsection(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Subsection &&
           toc_item->next && toc_get_item_type(toc_item->next->label) == Subsection)
        {
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
    }
    g_list_free(where);
}

static void
previous_section(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Section &&
           toc_item->previous &&
           toc_get_item_type(toc_item->previous->label) == Section){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *section_item = item_p->data;
        goto_toc_item_page(section_item->previous);
    }
    g_list_free(where);    
}

static void
next_section(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Section &&
           toc_item->next && toc_get_item_type(toc_item->next->label) == Section)
        {
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
    }    
    g_list_free(where);
}

static void
previous_chapter(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Chapter &&
           toc_item->previous &&
           toc_get_item_type(toc_item->previous->label) == Chapter)
        {
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *chapter_item = item_p->data;
        goto_toc_item_page(chapter_item->previous);
    }
    g_list_free(where);    
}

static void
next_chapter(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Chapter &&
           toc_item->next && toc_get_item_type(toc_item->next->label) == Chapter)
        {
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
    }
    g_list_free(where);
}

static void
previous_part(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
               d.toc.head_item,
               &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Part &&
           toc_item->previous &&
           toc_get_item_type(toc_item->previous->label) == Part){
            break;
        }
        item_p = item_p->next;
    }
    if(item_p){
        TOCItem *part_item = item_p->data;
        goto_toc_item_page(part_item->previous);
    } 
    g_list_free(where);    
}

static void
next_part(void)
{
    if(!d.toc.head_item){
        return;
    }
    GList *where = NULL;
    toc_where_am_i(d.cur_page_num,
                   d.toc.head_item,
                   &where);
    if(!where){
        return;
    }
    GList *item_p = where;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_get_item_type(toc_item->label) == Part &&
           toc_item->next && toc_get_item_type(toc_item->next->label) == Part)
        {
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
    }
    g_list_free(where);
}

static void
teleport(const char *term)
{
    char *object_name = g_strdup(term);
    g_strstrip(object_name);
    /* object is page label */
    int page_num = translate_page_label(d.page_label_num_hash,
                                        object_name);
    if(page_num >= 0){
        go_back_save();
        goto_page(page_num,
                  0, 0);
        g_free(object_name);
        return;
    }
    /* object is a navigation request: next/prev page, part, chapter, etc... */
    GError *err = NULL;
    GRegex *navigation_regex = g_regex_new(
        "^(?<command>next|prev(ious)?)\\b\\s*(?<label>page|part|ch(apter)?|(sub)?sec(tion)?)$",
        G_REGEX_CASELESS | G_REGEX_NO_AUTO_CAPTURE,
        0,
        &err);
    if(!navigation_regex){
        g_print("navigation regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }
    GMatchInfo *match_info = NULL;
    g_regex_match(navigation_regex,
                  object_name,
                  G_REGEX_MATCH_NOTEMPTY,
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
    if(d.toc.head_item){
        const TOCItem *target_item = toc_search_by_title(d.toc.head_item,
                                                         object_name);
        if(target_item){
            goto_toc_item_page(target_item);
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
    d.find_details.max_results = 0;
    d.find_details.max_results_page_num = -1;
}

static void
show_panel(void)
{
    ui.is_panel_hovered = TRUE;
    gtk_widget_queue_draw(ui.vellum);
}

static void
on_find_request_received(GtkWidget *sender,
                         gpointer   user_data)
{            
    FindRequestData *find_request = user_data;
    if(d.metae){
        destroy_find_results();
        gtk_widget_queue_draw(ui.vellum);     
        d.find_details.find_results = find_text(d.metae,
                                                find_request->text,
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
        for(i = 0; i < d.num_pages; i++){
            PageMeta *meta = g_ptr_array_index(d.metae,
                                               i);
            if(!meta->find_results){
                continue;
            }
            int num_results = g_list_length(meta->find_results);
            if(num_results > d.find_details.max_results){
                d.find_details.max_results = num_results;
                d.find_details.max_results_page_num = i;
            }
        }
        find_next();
    }    
    g_free(find_request);
    show_panel();
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
                                            meta->page_label->label);
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
    /* active referenced figure */
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
    /* panel */
    if(!ui.is_panel_hovered || meta->active_referenced_figure){
        return;
    }
    const double panel_button_size = widget_width > 800 ? 47 : 33,
                 panel_margin = widget_width > 800 ? 12 : 8;    
    cairo_rectangle(cr,
                    ui.panel_rect->x1, ui.panel_rect->y1,
                    rect_width(ui.panel_rect), rect_height(ui.panel_rect));
    cairo_set_source_rgba(cr,
                          silver_r, silver_r, silver_r, 0.9);
    cairo_fill(cr);
    Rect tools_label_rect;
    tools_label_rect.x1 = ui.panel_rect->x1 + panel_margin;
    tools_label_rect.y1 = ui.panel_rect->y1 + panel_margin;
    tools_label_rect.x2 = tools_label_rect.x1 + 3 * (panel_button_size + panel_margin) - panel_margin;
    tools_label_rect.y2 = tools_label_rect.y1 + panel_button_size * 0.25;
    draw_text(cr,
              "<span font='sans 8' foreground='#222222'><b>Tools</b></span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &tools_label_rect);
    const double active_buttone_tone = 0.2666,
                 normal_button_tone = 0.4666;
    /* teleport launcher */
    ui.teleport_launcher_rect->x1 = ui.panel_rect->x1 + panel_margin;
    ui.teleport_launcher_rect->y1 = tools_label_rect.y2 + panel_margin;
    ui.teleport_launcher_rect->x2 = ui.teleport_launcher_rect->x1 + panel_button_size;
    ui.teleport_launcher_rect->y2 = ui.teleport_launcher_rect->y1 + panel_button_size;
    double button_tone = ui.is_teleport_launcher_hovered ? active_buttone_tone
                                                         : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_move_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.1,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.6);
    cairo_line_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.1,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.7);
    cairo_line_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.40,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.8);
    cairo_line_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.6,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.8);
    cairo_line_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.9,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.7);
    cairo_line_to(cr,
                  ui.teleport_launcher_rect->x1 + panel_button_size * 0.9,
                  ui.teleport_launcher_rect->y1 + panel_button_size * 0.6);
    cairo_close_path(cr);    
    cairo_fill(cr);
    cairo_rectangle(cr,
                    ui.teleport_launcher_rect->x1 + panel_button_size * 0.25,
                    ui.teleport_launcher_rect->y1 + panel_button_size * 0.15,
                    panel_button_size * 0.5, panel_button_size * 0.15);
    cairo_rectangle(cr,
                    ui.teleport_launcher_rect->x1 + panel_button_size * 0.325,
                    ui.teleport_launcher_rect->y1 + panel_button_size * 0.35,
                    panel_button_size * 0.35, panel_button_size * 0.075);
    cairo_rectangle(cr,
                    ui.teleport_launcher_rect->x1 + panel_button_size * 0.4,
                    ui.teleport_launcher_rect->y1 + panel_button_size * 0.475,
                    panel_button_size * 0.2, panel_button_size * 0.035);
    cairo_fill(cr);
    /* find launcher */
    ui.find_text_launcher_rect->x1 = ui.teleport_launcher_rect->x2 + panel_margin;
    ui.find_text_launcher_rect->y1 = ui.teleport_launcher_rect->y1;
    ui.find_text_launcher_rect->x2 = ui.find_text_launcher_rect->x1 + panel_button_size;
    ui.find_text_launcher_rect->y2 = ui.find_text_launcher_rect->y1 + panel_button_size;
    button_tone = ui.is_find_text_launcher_hovered ? active_buttone_tone
                                                   : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_set_line_width(cr,
                         2);
    cairo_arc(cr,
              ui.find_text_launcher_rect->x1 + panel_button_size * 0.25,
              ui.find_text_launcher_rect->y1 + panel_button_size * 0.5,
              panel_button_size * 0.15,
              0, 2 * M_PI);
    cairo_stroke(cr);
    cairo_arc(cr,
          ui.find_text_launcher_rect->x1 + panel_button_size * 0.75,
          ui.find_text_launcher_rect->y1 + panel_button_size * 0.5,
          panel_button_size * 0.15,
          0, 2 * M_PI);
    cairo_stroke(cr);
    cairo_set_line_width(cr,
                         1);
    cairo_rectangle(cr,
                    ui.find_text_launcher_rect->x1 + panel_button_size * 0.4,
                    ui.find_text_launcher_rect->y1 + panel_button_size * 0.475,
                    panel_button_size * 0.2, panel_button_size * 0.05);
    cairo_fill(cr);
    cairo_move_to(cr,
                  ui.find_text_launcher_rect->x1 + panel_button_size * 0.1,
                  ui.find_text_launcher_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.find_text_launcher_rect->x1 + panel_button_size * 0.03,
                  ui.find_text_launcher_rect->y1 + panel_button_size * 0.53);
    cairo_stroke(cr);
    cairo_move_to(cr,
                  ui.find_text_launcher_rect->x1 + panel_button_size * 0.9,
                  ui.find_text_launcher_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.find_text_launcher_rect->x1 + panel_button_size * 0.97,
                  ui.find_text_launcher_rect->y1 + panel_button_size * 0.53);
    cairo_stroke(cr);
    /* toc launcher */
    ui.toc_launcher_rect->x1 = ui.find_text_launcher_rect->x2 + panel_margin;
    ui.toc_launcher_rect->y1 = ui.find_text_launcher_rect->y1;
    ui.toc_launcher_rect->x2 = ui.toc_launcher_rect->x1 + panel_button_size;
    ui.toc_launcher_rect->y2 = ui.toc_launcher_rect->y1 + panel_button_size;
    button_tone = ui.is_toc_launcher_hovered ? active_buttone_tone
                                             : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_rectangle(cr,
                    ui.toc_launcher_rect->x1 + panel_button_size * 0.1,
                    ui.toc_launcher_rect->y1 + panel_button_size * 0.1,
                    panel_button_size * 0.25, panel_button_size * 0.15);
    cairo_rectangle(cr,
                    ui.toc_launcher_rect->x1 + panel_button_size * 0.35,
                    ui.toc_launcher_rect->y1 + panel_button_size * 0.3,
                    panel_button_size * 0.25, panel_button_size * 0.15);
    cairo_rectangle(cr,
                    ui.toc_launcher_rect->x1 + panel_button_size * 0.6,
                    ui.toc_launcher_rect->y1 + panel_button_size * 0.5,
                    panel_button_size * 0.25, panel_button_size * 0.15);
    cairo_rectangle(cr,
                    ui.toc_launcher_rect->x1 + panel_button_size * 0.35,
                    ui.toc_launcher_rect->y1 + panel_button_size * 0.7,
                    panel_button_size * 0.25, panel_button_size * 0.15);    
    cairo_fill(cr);
    cairo_move_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.225,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.25);
    cairo_line_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.225,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.775);
    cairo_stroke(cr);
    cairo_move_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.225,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.375);
    cairo_line_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.35,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.375);
    cairo_stroke(cr);
    cairo_move_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.225,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.775);
    cairo_line_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.35,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.775);
    cairo_stroke(cr);
    cairo_move_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.475,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.45);
    cairo_line_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.475,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.575);
    cairo_line_to(cr,
                  ui.toc_launcher_rect->x1 + panel_button_size * 0.6,
                  ui.toc_launcher_rect->y1 + panel_button_size * 0.575);
    cairo_stroke(cr);
    /* zoom */
    Rect zoom_label_rect;
    zoom_label_rect.x1 = ui.panel_rect->x2 - panel_margin - 4 * (panel_button_size + panel_margin) + panel_margin;
    zoom_label_rect.y1 = ui.panel_rect->y1 + panel_margin;
    zoom_label_rect.x2 = zoom_label_rect.x1 + 4 * (panel_button_size + panel_margin) - panel_margin;
    zoom_label_rect.y2 = zoom_label_rect.y1 + panel_button_size * 0.25;
    draw_text(cr,
              "<span font='sans 8' foreground='#222222'><b>Zoom</b></span>",
              PANGO_ALIGN_CENTER,
              PANGO_ALIGN_CENTER,
              &zoom_label_rect);
    /* zoom out */
    ui.zoom_widget_OUT_rect->x1 = ui.panel_rect->x2 - panel_button_size - panel_margin;
    ui.zoom_widget_OUT_rect->y1 = zoom_label_rect.y2 + panel_margin;
    ui.zoom_widget_OUT_rect->x2 = ui.zoom_widget_OUT_rect->x1 + panel_button_size;
    ui.zoom_widget_OUT_rect->y2 = ui.zoom_widget_OUT_rect->y1 + panel_button_size;
    button_tone = ui.is_zoom_widget_OUT_hovered ? active_buttone_tone
                                                : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_rectangle(cr,
                    ui.zoom_widget_OUT_rect->x1 + panel_button_size * 0.35,
                    ui.zoom_widget_OUT_rect->y1 + panel_button_size * 0.475,
                    panel_button_size * 0.3, panel_button_size * 0.05);
    cairo_fill(cr);
    /* zoom in */
    ui.zoom_widget_IN_rect->x1 = ui.zoom_widget_OUT_rect->x1 - panel_button_size - panel_margin;
    ui.zoom_widget_IN_rect->y1 = ui.zoom_widget_OUT_rect->y1;
    ui.zoom_widget_IN_rect->x2 = ui.zoom_widget_IN_rect->x1 + panel_button_size;
    ui.zoom_widget_IN_rect->y2 = ui.zoom_widget_IN_rect->y1 + panel_button_size;
    button_tone = ui.is_zoom_widget_IN_hovered ? active_buttone_tone
                                                : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_rectangle(cr,
                    ui.zoom_widget_IN_rect->x1 + panel_button_size * 0.35,
                    ui.zoom_widget_IN_rect->y1 + panel_button_size * 0.475,
                    panel_button_size * 0.3, panel_button_size * 0.05);
    cairo_rectangle(cr,
                    ui.zoom_widget_IN_rect->x1 + panel_button_size * 0.475,
                    ui.zoom_widget_IN_rect->y1 + panel_button_size * 0.35,
                    panel_button_size * 0.05, panel_button_size * 0.3);
    cairo_fill(cr);
    /* width fit */
    ui.zoom_widget_WF_rect->x1 = ui.zoom_widget_IN_rect->x1 - panel_button_size - panel_margin;
    ui.zoom_widget_WF_rect->y1 = ui.zoom_widget_IN_rect->y1;
    ui.zoom_widget_WF_rect->x2 = ui.zoom_widget_WF_rect->x1 + panel_button_size;
    ui.zoom_widget_WF_rect->y2 = ui.zoom_widget_WF_rect->y1 + panel_button_size;
    button_tone = (ui.is_zoom_widget_WF_hovered || d.zoom_level == WidthFit)
                  ? active_buttone_tone : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_move_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.3,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.35);
    cairo_line_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.1,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.3,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.65);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_move_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.7,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.35);
    cairo_line_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.9,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.7,
                  ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.65);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_rectangle(cr,
                    ui.zoom_widget_WF_rect->x1 + panel_button_size * 0.25,
                    ui.zoom_widget_WF_rect->y1 + panel_button_size * 0.475,
                    panel_button_size * 0.45, panel_button_size * 0.05);
    cairo_fill(cr);
    /* page fit */
    ui.zoom_widget_PF_rect->x1 = ui.zoom_widget_WF_rect->x1 - panel_button_size - panel_margin;
    ui.zoom_widget_PF_rect->y1 = ui.zoom_widget_WF_rect->y1;
    ui.zoom_widget_PF_rect->x2 = ui.zoom_widget_PF_rect->x1 + panel_button_size;
    ui.zoom_widget_PF_rect->y2 = ui.zoom_widget_PF_rect->y1 + panel_button_size;
    button_tone = (ui.is_zoom_widget_PF_hovered || d.zoom_level == PageFit)
                  ? active_buttone_tone : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_rectangle(cr,
                    ui.zoom_widget_PF_rect->x1 + panel_button_size * 0.315,
                    ui.zoom_widget_PF_rect->y1 + panel_button_size * 0.25,
                    panel_button_size * 0.37, panel_button_size * 0.5);    
    cairo_set_line_width(cr,
                         2);
    cairo_stroke(cr);       
    cairo_set_line_width(cr,
                         1);
    /* prev page button */
    ui.prev_page_button_rect->x1 = ui.panel_rect->x1 + panel_margin;
    ui.prev_page_button_rect->y1 = ui.panel_rect->y2 - panel_button_size - panel_margin;
    ui.prev_page_button_rect->x2 = ui.prev_page_button_rect->x1 + panel_button_size;
    ui.prev_page_button_rect->y2 = ui.prev_page_button_rect->y1 + panel_button_size;
    button_tone = ui.is_prev_page_button_hovered ? active_buttone_tone
                                                 : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_move_to(cr,
                  ui.prev_page_button_rect->x1 + panel_button_size * 0.85,
                  ui.prev_page_button_rect->y1 + panel_button_size * 0.15);
    cairo_line_to(cr,
                  ui.prev_page_button_rect->x1 + panel_button_size * 0.15,
                  ui.prev_page_button_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.prev_page_button_rect->x1 + panel_button_size * 0.85,
                  ui.prev_page_button_rect->y1 + panel_button_size * 0.85);
    cairo_close_path(cr);
    cairo_fill(cr);
    /* next page button */
    ui.next_page_button_rect->x1 = ui.panel_rect->x2 - panel_button_size - panel_margin;
    ui.next_page_button_rect->y1 = ui.prev_page_button_rect->y1;
    ui.next_page_button_rect->x2 = ui.next_page_button_rect->x1 + panel_button_size;
    ui.next_page_button_rect->y2 = ui.next_page_button_rect->y1 + panel_button_size;
    button_tone = ui.is_next_page_button_hovered ? active_buttone_tone
                                                 : normal_button_tone;
    cairo_set_source_rgba(cr,
                          button_tone, button_tone, button_tone, 1);
    cairo_move_to(cr,
                  ui.next_page_button_rect->x1 + panel_button_size * 0.15,
                  ui.next_page_button_rect->y1 + panel_button_size * 0.15);
    cairo_line_to(cr,
                  ui.next_page_button_rect->x1 + panel_button_size * 0.85,
                  ui.next_page_button_rect->y1 + panel_button_size * 0.5);
    cairo_line_to(cr,
                  ui.next_page_button_rect->x1 + panel_button_size * 0.15,
                  ui.next_page_button_rect->y1 + panel_button_size * 0.85);
    cairo_close_path(cr);
    cairo_fill(cr);
    /* reading progress */
    Rect reading_progress_rect;
    reading_progress_rect.x1 = ui.prev_page_button_rect->x2 + panel_margin;
    reading_progress_rect.y1 = ui.prev_page_button_rect->y1;
    reading_progress_rect.x2 = ui.next_page_button_rect->x1 - panel_margin;
    reading_progress_rect.y2 = ui.next_page_button_rect->y2;
    cairo_rectangle(cr,
                    reading_progress_rect.x1,
                    reading_progress_rect.y1,
                    rect_width(&reading_progress_rect),
                    rect_height(&reading_progress_rect));
    cairo_set_source_rgba(cr,
                          normal_button_tone, normal_button_tone, normal_button_tone, 0.8);
    cairo_fill(cr);
    double progress = ((double)(d.cur_page_num + 1) / d.num_pages);  
    Rect reading_progress_data_rect;
    reading_progress_data_rect.x1 = reading_progress_rect.x1 + 2;                    
    reading_progress_data_rect.y1 = reading_progress_rect.y1 + 2;
    reading_progress_data_rect.x2 = reading_progress_data_rect.x1 + rect_width(&reading_progress_rect) - 4;
    reading_progress_data_rect.y2 = reading_progress_data_rect.y1 + panel_button_size - 4;
    cairo_rectangle(cr,
                    reading_progress_data_rect.x1,
                    reading_progress_data_rect.y1,
                    progress * rect_width(&reading_progress_data_rect),
                    rect_height(&reading_progress_data_rect));
    cairo_set_source_rgba(cr,
                          active_buttone_tone, active_buttone_tone, active_buttone_tone, 0.8);
    cairo_fill(cr);    
    if(meta->page_label->label){
        PageMeta *last_page_meta = g_ptr_array_index(d.metae,
                                                     d.num_pages - 1);
        char *last_page_label = (last_page_meta->page_label->label)
         ? g_strdup_printf("%s", last_page_meta->page_label->label)
         : g_strdup_printf("%d", d.num_pages);
        char *reading_progress_text = g_strdup_printf("<span font='sans 10' foreground='black'>%s of %s</span>",
                                                      meta->page_label->label,
                                                      last_page_label);
        draw_text(cr,
                  reading_progress_text,
                  PANGO_ALIGN_CENTER,
                  PANGO_ALIGN_CENTER,
                  &reading_progress_rect);
        g_free(last_page_label);
        g_free(reading_progress_text);
    }
    /* find results profile */
    if(d.find_details.find_results){
        double data_box_width = rect_width(&reading_progress_data_rect),
               data_box_height = rect_height(&reading_progress_data_rect);
        double page_bar_width = (data_box_width / d.num_pages);
        for(int i = 0; i < d.num_pages; i++){
            PageMeta *meta = g_ptr_array_index(d.metae,
                                               i);
            if(!meta->find_results){
                continue;
            }
            cairo_rectangle(cr,
                            reading_progress_data_rect.x1 + i * page_bar_width,
                            reading_progress_data_rect.y2,
                            page_bar_width,
                            -(double)g_list_length(meta->find_results) / d.find_details.max_results *
                            data_box_height);            
        }
        cairo_set_source_rgba(cr,
                              giants_orange_r, giants_orange_g, giants_orange_b, 0.8);
        cairo_fill(cr);
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
    if(!d.toc.head_item){
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
        if(!d.toc.head_item){
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
        /* panel */
        ui.is_panel_hovered = rect_contains_point(ui.panel_rect,
                                                  event->x, event->y);
        if(ui.is_panel_hovered){
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
        }        
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
            unit_tip = g_strdup_printf("<span font='sans 10' >~= %s</span>",
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
            find_tip = g_strdup_printf("<span font='sans 10' >%s</span>",
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
                link_tip = g_strdup_printf("<span font='sans 10' >Link</span>: %s",
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
            tip_markup = "<span font='sans 10' >Fits page inside window(F)</span>";
        }
        else if(ui.is_zoom_widget_WF_hovered){
            tip_markup = "<span font='sans 10' >Stretches the page to fit width(W)</span>";
        }
        else if(ui.is_zoom_widget_IN_hovered){
            tip_markup = "<span font='sans 10' >Brings book closer(+)</span>";
        }
        else if(ui.is_zoom_widget_OUT_hovered){
            tip_markup = "<span font='sans 10' >Pushes book farther(-)</span>";
        }
        else if(ui.is_teleport_launcher_hovered){
            tip_markup = "<span font='sans 10' >Lets you jump to various book locations(T)</span>";
        }
        else if(ui.is_find_text_launcher_hovered){
            tip_markup = "<span font='sans 10' >Let's you search for textual data(CTRL + F)</span>";
        }
        else if(ui.is_toc_launcher_hovered){
            tip_markup = "<span font='sans 10' >Shows table of contents(CTRL + T)</span>";
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
            tip_markup = "<span font='sans 10' >Let's you choose a book(I)</span>";            
        }
        if(ui.is_continue_to_book_button_hovered){
            tip_markup = "<span font='sans 10' >Hit to continue reading book(R or Escape)</span>";
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
    rect_free(ui.panel_rect);
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
    
    g_hash_table_unref(d.page_label_num_hash);
    if(d.go_back_stack){
        g_queue_free(d.go_back_stack);
    }
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
    const double widget_width = 800,
                 widget_height = 600;
    ui.main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(ui.main_window), "readaratus");
    gtk_window_set_default_size(GTK_WINDOW(ui.main_window),
                                widget_width, widget_height);
    gtk_widget_set_events(ui.main_window, 
                          GDK_STRUCTURE_MASK);
    ui.is_fullscreen = FALSE;
    g_signal_connect(G_OBJECT(ui.main_window), "window_state_event",
                     G_CALLBACK(window_state_callback), NULL);
    ui.vellum = gtk_drawing_area_new();
    gtk_widget_set_size_request(ui.vellum,
                                widget_width, widget_height);
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
        "Version 2.0\n"
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
    /* panel */
    ui.panel_rect = rect_new();
    ui.is_panel_hovered = FALSE;
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

    d.page_label_num_hash = g_hash_table_new(g_str_hash,
                                             g_str_equal);
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
}