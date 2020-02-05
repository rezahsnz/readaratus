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

#ifndef ROMAN_NUMERAL
#define ROMAN_NUMERAL

#include <glib.h>

gboolean
roman_is_valid(const char *roman);

const char *
roman_from_decimal(int decimal);

int
roman_to_decimal(const char *roman);

const char *
roman_previous(const char *roman);

const char *
roman_next(const char *roman);

int 
roman_compare(const char *r1,
              const char *r2);

#endif