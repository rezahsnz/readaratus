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

#ifndef TELEPORT_WIDGET_H
#define TELEPORT_WIDGET_H

#include <gtk/gtk.h>

void
teleport_widget_show(void);

void
teleport_widget_hide(void);


GtkWidget *
teleport_widget_init(GtkWidget *parent);

void
teleport_widget_destroy(void);

void
teleport_widget_destroy_text_completions(void);

void
teleport_widget_update_text_completions(GList *text_list,
                                        char  *tag);

#endif