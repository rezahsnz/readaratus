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

#include "teleport_widget.h"

static GtkWidget *teleport_window = NULL;
static GtkWidget *text_entry = NULL;
static GtkEntryCompletion *entry_completion = NULL;
static GtkWidget *teleport_button = NULL;

void
teleport_widget_hide(void)
{
    gtk_widget_hide(teleport_window);
}

static gboolean
on_widget_deleted(GtkWidget *widget,
                  GdkEvent  *event,
                  gpointer   data)
{
    teleport_widget_hide();
    return TRUE;
}

static void
request_teleportation(void)
{
        if(gtk_entry_get_text_length(GTK_ENTRY(text_entry))){
        const char *text = gtk_entry_get_text(GTK_ENTRY(text_entry));        
        g_signal_emit_by_name(teleport_window,
                              "teleport_request_event",
                              text);
    }   
}

static void
on_teleport_button_clicked(GtkButton *button,
                           gpointer   user_data)
{
    request_teleportation(); 
}

static void
on_text_entry_activated(GtkEntry *entry,
                        gpointer user_data)
{
    request_teleportation();
}

void
teleport_widget_show(void)
{
    gtk_widget_show_all(teleport_window);
    gtk_widget_grab_focus(text_entry);
}

static gboolean
key_press_callback(GtkWidget   *widget,
                   GdkEventKey *event,
                   gpointer     data)
{
    gboolean handled = TRUE;
    switch(event->keyval){
        case GDK_KEY_Escape:
            teleport_widget_hide();
            break;
        default:
            handled = FALSE;
    }
    return handled;
}

static void
setup_text_entry_completion(void)
{
    enum{
        VALUE_COLUMN,
        TAG_COLUMN,
        NUM_COLUMNS
    };
    GtkListStore *list_store = gtk_list_store_new(NUM_COLUMNS,
                                                  G_TYPE_STRING,
                                                  G_TYPE_STRING); 
    GtkCellArea *cell_area = gtk_cell_area_box_new();
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer),
                 "text", VALUE_COLUMN,
                 NULL);
    gtk_cell_area_add(cell_area,
                      renderer);
    // renderer = gtk_cell_renderer_text_new();
    // g_object_set(G_OBJECT(renderer),
    //              "text", TAG_COLUMN,
    //              NULL);
    // gtk_cell_area_add(cell_area,
    //                   renderer);

    entry_completion = gtk_entry_completion_new_with_area(cell_area);
    gtk_entry_completion_set_model(entry_completion,
                                   GTK_TREE_MODEL(list_store));
    gtk_entry_completion_set_text_column(entry_completion,
                                         VALUE_COLUMN);

    gtk_entry_set_completion(GTK_ENTRY(text_entry),
                             entry_completion);
}

GtkWidget *
teleport_widget_init(GtkWidget *parent)
{
    teleport_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(teleport_window),
                             FALSE);
    gtk_window_set_transient_for(GTK_WINDOW(teleport_window),
                                 GTK_WINDOW(parent));
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar),
                             "Teleport");
    gtk_header_bar_set_has_subtitle(GTK_HEADER_BAR(header_bar),
                                    FALSE);
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar),
                                         TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(teleport_window),
                            header_bar);
    g_signal_new("teleport_request_event",
                 G_TYPE_OBJECT,
                 G_SIGNAL_RUN_FIRST,
                 0, NULL, NULL,
                 g_cclosure_marshal_VOID__POINTER,
                 G_TYPE_NONE,
                 1,
                 G_TYPE_POINTER);
    g_signal_connect(G_OBJECT(teleport_window),
                     "key_press_event",
                     G_CALLBACK(key_press_callback),
                     NULL);
    g_signal_connect(G_OBJECT(teleport_window), "delete_event",
                     G_CALLBACK(on_widget_deleted), NULL);
    /* setup ui */
    GtkWidget *label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(label),
                         "Enter: '<span font_style='italic' foreground='blue'>21, VII, Chapter 3, Table 2.a, Figure 12.2, ...</span>'");
    text_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(text_entry),
                             144); 
    g_signal_connect(G_OBJECT(text_entry), "activate",
                     G_CALLBACK(on_text_entry_activated), NULL);    
    teleport_button = gtk_button_new_with_label("Teleport");
    g_signal_connect(G_OBJECT(teleport_button), "clicked",
                     G_CALLBACK(on_teleport_button_clicked), NULL);
    GtkWidget *grid_layout = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid_layout),
                             12);
    gtk_grid_set_column_spacing(GTK_GRID(grid_layout),
                                12);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    label,
                    0,
                    0,
                    3,
                    1);
    gtk_grid_attach(GTK_GRID(grid_layout),
                    text_entry,
                    0,
                    1,
                    2,
                    1);
    gtk_grid_attach_next_to(GTK_GRID(grid_layout),
                            teleport_button,
                            text_entry,
                            GTK_POS_RIGHT,                    
                            1,
                            1);
    gtk_container_add(GTK_CONTAINER(teleport_window),
                      grid_layout);
    gtk_widget_set_margin_start(grid_layout,
                                16);
    gtk_widget_set_margin_end(grid_layout,
                              16);
    gtk_widget_set_margin_top(grid_layout,
                              16);
    gtk_widget_set_margin_bottom(grid_layout,
                                 16);    
    setup_text_entry_completion();
    return teleport_window;     
}

void
teleport_widget_destroy(void)
{
    gtk_widget_destroy(teleport_window);
}

void
teleport_widget_destroy_text_completions(void)
{
    GtkListStore *list_store = GTK_LIST_STORE(gtk_entry_completion_get_model(entry_completion));
    gtk_list_store_clear(list_store);
}

void
teleport_widget_update_text_completions(GList *text_list,
                                        char *tag)
{
    GtkListStore *list_store = GTK_LIST_STORE(gtk_entry_completion_get_model(entry_completion));
    GList *text_p = text_list;
    while(text_p){
        char *text = text_p->data;
        GtkTreeIter iter;
        gtk_list_store_append(list_store,
                              &iter);
        gtk_list_store_set(list_store, &iter,
                           0, text,
                           1, tag,
                           -1);
        text_p = text_p->next;
    }
}