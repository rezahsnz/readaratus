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

#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

#include <gtk/gtk.h>

typedef struct FindRequestData FindRequestData;
struct FindRequestData
{
    const char *text;
    gboolean is_whole_words_checked;
    gboolean is_dualpage_checked;
};

void
find_widget_show(void);

void
find_widget_hide(void);

GtkWidget *
find_widget_init(GtkWidget *parent);

void
find_widget_destroy(void);

#endif