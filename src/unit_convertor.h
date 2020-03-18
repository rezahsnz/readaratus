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

#ifndef UNIT_CONVERTOR_H
#define UNIT_CONVERTOR_H

#include <glib.h>
#include "rect.h"
#include "find.h"

enum MeasurementSystem
{
	SI = 100,
	Imperial
};

enum MeasurementType
{
	Length = 1500,
	Area, 
	Volume, 
	Mass, 
	Temperature,
    Velocity
};

typedef struct Measurement Measurement;
struct Measurement
{
    enum MeasurementSystem system;
    enum MeasurementType type;
    double factor;
};

typedef struct ConvertedUnit ConvertedUnit;
struct ConvertedUnit
{
    char *whole_match;
	char *old_value;
	char *multiplier;
	char *old_unit;
	double value;
	char *unit;
    char *value_str;
    GList *find_results;
};

void 
unit_convertor_module_init(void);

void 
unit_convertor_module_destroy(void);

ConvertedUnit *
converted_unit_new(void);

void
converted_unit_free(ConvertedUnit *cv);

void
convert_units(const char *text,
			  GList      **converted_units);

#endif