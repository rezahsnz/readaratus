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

#include "find_widget.h"

static GtkWidget *find_window = NULL;
static GtkWidget *text_view = NULL;
static GtkWidget *whole_words_check_button = NULL;
static GtkWidget *dualpage_check_button = NULL;
static GtkWidget *find_button = NULL;

void
find_widget_hide(void)
{
    gtk_widget_hide(find_window);
}

static gboolean
key_press_callback(GtkWidget   *widget,
                   GdkEventKey *event,
                   gpointer     data)
{
    gboolean handled = TRUE;
    switch(event->keyval){
        case GDK_KEY_Escape:
            find_widget_hide();
            break;
        default:
            handled = FALSE;
    }
    return handled;
}

static gboolean
on_widget_deleted(GtkWidget *widget,
                  GdkEvent  *event,
                  gpointer   data)
{
    find_widget_hide();
    return TRUE;
}

static void
on_find_button_clicked(GtkButton *button,
                       gpointer   user_data)
{
    GtkTextBuffer *text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    GtkTextIter iter_start, iter_end;
    gtk_text_buffer_get_iter_at_offset (text_buffer,
                                        &iter_start,
                                        0);
    gtk_text_buffer_get_iter_at_offset (text_buffer,
                                        &iter_end,
                                        -1);
    const char *text = gtk_text_buffer_get_text(text_buffer,
                                                &iter_start,
                                                &iter_end,
                                                TRUE);
    if(strlen(text)){
        FindRequestData *find_request = g_malloc(sizeof(FindRequestData));
        find_request->text = text;
        find_request->is_whole_words_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(whole_words_check_button));
        find_request->is_dualpage_checked = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dualpage_check_button));
        g_signal_emit_by_name(find_window,
                              "find_request_event",
                              find_request);
    }
}

void
find_widget_show(void)
{
    gtk_widget_show_all(find_window);
    gtk_widget_grab_focus(text_view);
}

GtkWidget *
find_widget_init(GtkWidget *parent)
{
    find_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(find_window),
                                  FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(find_window),
                                      GTK_WINDOW(parent));
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar),
                             "Find text");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar),
                                    FALSE);
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar),
                                         TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(find_window),
                            header_bar);
    g_signal_new("find_request_event",
                 G_TYPE_OBJECT,
                 G_SIGNAL_RUN_FIRST,
                 0, NULL, NULL,
                 g_cclosure_marshal_VOID__POINTER,
                 G_TYPE_NONE,
                 1,
                 G_TYPE_POINTER);
    g_signal_connect(G_OBJECT(find_window),
                     "key_press_event",
                     G_CALLBACK(key_press_callback),
                     NULL);
    g_signal_connect(G_OBJECT(find_window), "delete_event",
                     G_CALLBACK(on_widget_deleted), NULL);
    /* setup ui */    
    text_view = gtk_text_view_new();
    gtk_widget_set_tooltip_markup(text_view,
                                  "<span font='sans 10' foreground='#C3C3C3'>Put your search term here.</span>");
    whole_words_check_button = gtk_check_button_new_with_label("Whole words");    
    gtk_widget_set_tooltip_markup(whole_words_check_button,
                                  "<span font='sans 10' foreground='#C3C3C3'>Only searches for whole words.</span>");
    dualpage_check_button = gtk_check_button_new_with_label("Dualpage");
    gtk_widget_set_tooltip_markup(dualpage_check_button,
                                  "<span font='sans 10' foreground='#C3C3C3'>"
                                  "It is common for sentences to be spread across consecutive pages."
                                  " This options instructs the program to look for parts of the search"
                                  " term in page <i>X</i> and the rest in page<i>X+1</i>."
                                  "</span>");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(dualpage_check_button),
                                 TRUE);        
    find_button = gtk_button_new_with_label("Look up");
    g_signal_connect(G_OBJECT(find_button), "clicked",
                     G_CALLBACK(on_find_button_clicked), NULL);
    GtkWidget *grid_layout = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid_layout),
                             12);
    gtk_grid_set_column_spacing(GTK_GRID(grid_layout),
                                12);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    text_view,
                    0, 0,
                    6, 6);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    whole_words_check_button,
                    0, 6,
                    1, 1);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    dualpage_check_button,
                    0, 7,
                    1, 1);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    find_button,
                    5, 8,
                    1, 1);
    gtk_container_add(GTK_CONTAINER(find_window),
                      grid_layout);
    gtk_widget_set_margin_start(grid_layout,
                                16);
    gtk_widget_set_margin_end(grid_layout,
                              16);
    gtk_widget_set_margin_top(grid_layout,
                              16);
    gtk_widget_set_margin_bottom(grid_layout,
                                 16);
    return find_window;     
}

void
find_widget_destroy(void)
{
    gtk_widget_destroy(find_window);
}