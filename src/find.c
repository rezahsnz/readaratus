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

#include <glib.h>
#include "find.h"

FindResult *
find_result_new()
{
    FindResult *fr = g_malloc(sizeof(FindResult));
    fr->page_num = -1;
    fr->physical_layouts = NULL;
    fr->match = NULL;
    fr->page_prefix = NULL;
    fr->page_postfix = NULL;
    fr->certainty = 0.0;
    fr->tip = NULL;
    return fr;
}

void
find_result_free(FindResult *fr)
{
    if(!fr){
        return;
    }
    GList *list_p = fr->physical_layouts;
    while(list_p){
        rect_free(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(fr->physical_layouts);
    g_free(fr->match);
    g_free(fr->tip);
    g_free(fr);

}
