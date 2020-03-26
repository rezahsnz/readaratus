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

#include "unit_convertor.h"
#include <math.h>

static GRegex *unit_regex = NULL;
static GRegex *comma_regex = NULL;

void
unit_convertor_module_init(void)
{
	GError *err = NULL;
	unit_regex = g_regex_new(
        "\\b\n"
        "(?<value>[-+]?[0-9]*\\,?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\s?\n"
        "(?<multiplier>hundred|k(ilo)?|thousand|(m|b)(illion)?\\s)?(-\\R+)?\n"
        "# <IMPERIAL>\n"
        "# velocity\n"
        "(?<unit>\n"
        "(in(ch(es)?)?|f(oo|ee)?t|yards?|yd|mi(les?)?)\\s?(-\\R+)?(per|/)\\s?(-\\R+)?(s(ec(ond)?s?)?|m(in(ute)?s?)?|h(ours?)?)|\n"
        "fpm|fth|fph|mp(s|m|h)|mp\\s?/\\s?h|furlongs?\\s?(per|/)\\s?fortnights?|nautical\\s?miles?|nm\\s?/\\s?h|kn(ots?)?|\n"
        "# length\n"
        "thou|inch(es)?|\"|f(oo|ee)?t|\'|yards?|yd|ch(ains?)?|fur(longs?)?|mi(les?)?|\n"
        "lea(gues?)?|fathoms?|ftm|cables?|nautical\\s?miles?|nmi|links?|rods?|\n"
        "# area\n"
        "sq(uare)?\\s?(-\\R+)?(in(ch(es)?)?|foot|feet|ft|yards?|yd|mi(les?)?)|(in|ft|yd|mi)2|acres?|perches?|roods?|\n"
        "# volume\n"
        "gi(lls?)?|pints?|quarts?|qt|gal(lons?)?|teaspoons?|tsp|tablespoons?|tbsp|\n"
        "cu(bic)?\\s?(-\\R+)?(in(ch(es)?)?|foot|feet|ft|yards?|yd)|(in|ft|yd)3\n"
        "# mass\n"
        "gr(ains?)?|drachms?|ounces?|oz|pounds?|lbs?|hundredweights?|cwt|tons?|\n"
        "# temperature\n"
        "(deg(rees?)?)?\\s?(-\\R+)?fahrenheit|\\x2109|\n"
        "# <SI>\n"
        "# velocity\n"
        "(nano|micro|milli|centi|deci|kilo)?\\s?(-\\R+)?m(etres?|eters?)?(-\\R+)?(per|/)\\s?(-\\R+)?(s(ec(ond)?s?)?|m(in(ute)?s?)?|h(ours?)?)|\n"
        "k(mh|ph|m\\s?/\\s?h)|\n"
        "# length\n"
        "(nano|micro|milli|centi|deci|kilo)\\s?(-\\R+)?(metres?|meters?)|(nm|\\x00b5m|mm|cm|m|km)|\n"
        "# area\n"
        "sq(uare)?\\s?(-\\R+)?((nano|micro|milli|centi|deci|kilo)\\s?(-\\R+)?(metres?|meters?)|(nm|\\x00b5m|mm|cm|m|km))|m2|hectares?|\n"
        "# volume\n"
        "(milli)?l(itres?|iters?)?|cu(bic)?\\s?(-\\R+)?(metres?|meters?|m)|m3|ml|\n"
        "# mass\n"
        "(nano|micro|milli)?\\s?(-\\R+)?grams?|mg|(metric)?\\s?tonnes?|mt|\n"
        "# temperature\n"
        "(deg(rees?)?)?\\s?(-\\R+)?celsius|\\x2103\n"
        ")\\b",
         G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
         0,
         &err);
	if(!unit_regex){
		g_print("regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
	}

    err = NULL;
    comma_regex = g_regex_new(",",
                              0,
                              0,
                              &err);
    if(!comma_regex){
        g_print("comma_regex error. pattern: %s \ndomain: %d, \ncode: %d, \nmessage: %s\n",
                ",",
                err->domain, 
                err->code, 
                err->message);
    }
}

void
unit_convertor_module_destroy(void)
{
    g_regex_unref(unit_regex);
    g_regex_unref(comma_regex);
}

ConvertedUnit *
converted_unit_new(void)
{
    ConvertedUnit *cv = g_malloc(sizeof(ConvertedUnit));
    cv->whole_match = NULL;
    cv->old_value = NULL;
    cv->multiplier = NULL;
    cv->old_unit = NULL;
    cv->value = -1;
    cv->unit = NULL;
    cv->value_str = NULL;
    cv->find_results = NULL;
    return cv;
}

void
converted_unit_free(ConvertedUnit *cv)
{
    if(!cv){
        return;
    }
    g_free(cv->whole_match);
    g_free(cv->old_value);
    g_free(cv->multiplier);
    g_free(cv->old_unit);
    g_free(cv->unit);
    g_free(cv->value_str);
    GList *list_p = cv->find_results;
    while(list_p){
        find_result_free(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(cv->find_results);
    g_free(cv);
}

static double
get_multiplier (const char *multiplier_str)
{
    double multiplier = 1.0;
    if(g_regex_match_simple("^hundred$",
                            multiplier_str,
                            G_REGEX_CASELESS,
                            0))
    {
        multiplier = 1e+2;
    }
    else if(g_regex_match_simple("^(k(ilo)?|thousand)$",
                            multiplier_str,
                            G_REGEX_CASELESS,
                            0))
    {
        multiplier = 1e+3;
    }
    else if(g_regex_match_simple("^m(illion)?$",
                            multiplier_str,
                            G_REGEX_CASELESS,
                            0))
    {
        multiplier = 1e+6;
    }
    else if(g_regex_match_simple("^b(illion)?$",
                            multiplier_str,
                            G_REGEX_CASELESS,
                            0))
    {
        multiplier = 1e+9;
    }
    return multiplier;
}

static Measurement
get_target_measurement (const char *unit)
{
    Measurement measurement;
    /* imerial to si */
    /* length, base unit is meters */
    if(g_regex_match_simple("^thou$",
                            unit,
                            G_REGEX_CASELESS,
                            0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 0.0000254;
    }
    else if(g_regex_match_simple("^(inch(es)?|\")$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 0.0254;
    }
    else if(g_regex_match_simple("^(f(oo|ee)?t|\')$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 0.3048;
    }
    else if(g_regex_match_simple("^(yards?|yd)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 0.9144;
    }
    else if(g_regex_match_simple("^ch(ains?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 20.1168;
    }
    else if(g_regex_match_simple("^fur(longs?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 201.168;
    }
    else if(g_regex_match_simple("^mi(les?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 1609.344;
    }
    else if(g_regex_match_simple("^lea(gues?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 4828.032;
    }
    else if(g_regex_match_simple("^(fathoms?|ftm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 1.852;
    }
    else if(g_regex_match_simple("^cables?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 185.2;
    }
    else if(g_regex_match_simple("^(nautical(\\R|\\s)?miles?|nmi)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 1852;
    }
    else if(g_regex_match_simple("^links?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 0.201168;
    }
    else if(g_regex_match_simple("^rods?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Length;
        measurement.factor = 5.0292;
    }
    /* area, base unit is square meters */
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(in(ch(es)?)?|\")$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 0.00064516;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(f(oo|ee)?t|\')$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 0.092903;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(yards?|yd)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 0.836127;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?mi(les?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 2.59e+6;
    }
    else if(g_regex_match_simple("^perchs?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 25.29285264;
    }
    else if(g_regex_match_simple("^roods?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 1011.7141056;
    }
    else if(g_regex_match_simple("^acres?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Area;
        measurement.factor = 4046.86;
    }
    /* volume, base unit is liters */
    else if(g_regex_match_simple("^gi(lls?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 0.1420653125;
    }
    else if(g_regex_match_simple("^pints?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 0.56826125;
    }
    else if(g_regex_match_simple("^(quarts?|qt)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 1.1365225;
    }
    else if(g_regex_match_simple("^gal(lons?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 4.54609;
    }
    else if(g_regex_match_simple("^(teaspoons?|tsp)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 0.00492892159375;
    }
    else if(g_regex_match_simple("^(tablespoons?|tbsp)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {        
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 0.01478676478125;
    }
    else if(g_regex_match_simple("^cu(bic)?(\\R|\\s)?(in(ch(es)?)?|in3)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 0.016387064;
    }
    else if(g_regex_match_simple("^(cu(bic)?(\\R|\\s)?(f(oo|ee)?t|\')|ft3)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 28.316846592;
    }
    else if(g_regex_match_simple("^(cu(bic)?(\\R|\\s)?(yards?|yd)|yd3)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Volume;
        measurement.factor = 746.554857984;
    }
    /* mass, base unit is grams */
    else if(g_regex_match_simple("^gr(ains?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 0.06479891;
    }
    else if(g_regex_match_simple("^drachms?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 1.7718451953125;
    }
    else if(g_regex_match_simple("^(ounces?|oz)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 28.349523125;
    }
    else if(g_regex_match_simple("^(pounds?|lbs?)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 453.59237;
    }
    else if(g_regex_match_simple("^stone$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 6350.29318;
    }
    else if(g_regex_match_simple("^(hundredweights?|cwt)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 5.0802345443e+4;
    }
    else if(g_regex_match_simple("^tons?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Mass;
        measurement.factor = 1.016047e+6;
    }   
    /* temperature */
    else if(g_regex_match_simple("^((deg(rees?)?)?\\s?fahrenheit|\\x2109)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Temperature;
        measurement.factor = 1;
    }
    /* velocity */
    /* base unit is m/s */
    else if(g_regex_match_simple("^in(ch(es)?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 2.54e-2;        
    }
    else if(g_regex_match_simple("^in(ch(es)?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 4.233333333e-4;
    }
    else if(g_regex_match_simple("^in(ch(es)?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 7.05556e-6;    
    }
    else if(g_regex_match_simple("^f(oo|ee)?t\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 0.3048;        
    }
    else if(g_regex_match_simple("^(f(oo|ee)?t\\s?(per|/)\\s?m(in(ute)?s?)?|fpm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 5.08e-3;
    }
    else if(g_regex_match_simple("^(f(oo|ee)?t\\s?(per|/)\\s?h(ours?)?|fth|fph)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor =  8.466667e-5;    
    }
    else if(g_regex_match_simple("^((yards?|yd)\\s?(per|/)\\s?s(ec(ond)?s?)?|yds)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 0.9;        
    }
    else if(g_regex_match_simple("^((yards?|yd)\\s?(per|/)\\s?m(in(ute)?s?)?|ydm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 1.5e-2;
    }
    else if(g_regex_match_simple("^((yards?|yd)\\s?(per|/)\\s?h(ours?)?|ydh)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 2.5e-4;    
    }
    else if(g_regex_match_simple("^(mi(les?)?\\s?(per|/)\\s?s(ec(ond)?s?)?|mps)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 1609.344;        
    }
    else if(g_regex_match_simple("^(mi(les?)?\\s?(per|/)\\s?m(in(ute)?s?)?|mpm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 26.8224;
    }
    else if(g_regex_match_simple("^(mi(les?)?\\s?(per|/)\\s?h(ours?)?|mph|mp\\s?/\\s?h)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 0.44704;    
    }
    else if(g_regex_match_simple("^(n(autical\\s?)?m(iles?)?|nm\\s?/\\s?h|kn(ots?)?)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 0.514772;    
    }
    
    else if(g_regex_match_simple("^furlongs?(per|/)fortnights?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = SI;
        measurement.type = Velocity;
        measurement.factor = 1.663095e-4;    
    }
    /* si to imperial */
    /* length, base unit is feet */
    else if(g_regex_match_simple("^(nano(\\R|\\s)?(metres?|meters?)|nm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 / 1e+9;
    }
    else if(g_regex_match_simple("^(micro(\\R|\\s)?(metres?|meters?)|\\x00b5m)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 / 1e+6;
    }
    else if(g_regex_match_simple("^(milli(\\R|\\s)?(metres?|meters?)|mm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 / 1e+3;
    }
    else if(g_regex_match_simple("^(centi(\\R|\\s)?(metres?|meters?)|cm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 / 1e+2;
    }
    else if(g_regex_match_simple("^deci(\\R|\\s)?(metres?|meters?)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 / 1e+1;
    }
    else if(g_regex_match_simple("^m(etres?|eters?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084;
    }
    else if(g_regex_match_simple("^(kilo(metres?|meters?)|km)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Length;
        measurement.factor = 3.28084 * 1e+3;
    }
    /* area, base unit is square feet */
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(nano(\\R|\\s)?(metres?|meters?)|nm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 / 1e+18;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(micro(\\R|\\s)?(metres?|meters?)|\\x00b5m)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 / 1e+12;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(milli(\\R|\\s)?(metres?|meters?)|mm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 / 1e+6;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(centi(\\R|\\s)?(metres?|meters?)|cm)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 / 1e+4;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(metres?|meters?|m)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639;
    }
    else if(g_regex_match_simple("^sq(uare)?(\\R|\\s)?(kilo(\\R|\\s)?(metres?|meters?)|km)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 * 1e+6;
    }
    else if(g_regex_match_simple("^hectares?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Area;
        measurement.factor = 10.7639 * 1e+4;
    }
    /* volume, base unit is cubic feet */
    else if(g_regex_match_simple("^(milli(\\R|\\s)?l(itres?|iters?)?|ml)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Volume;
        measurement.factor = 0.035314667 / 1e+3;
    }
    else if(g_regex_match_simple("^l(itres?|iters?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Volume;
        measurement.factor = 0.035314667;
    }
    else if(g_regex_match_simple("^(cu(bic)?(\\R|\\s)?m(etres?|eters?)?|m3)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Volume;
        measurement.factor = 0.035314667 * 1e+3;
    }
    /* mass, base unit is pounds */
    else if(g_regex_match_simple("^nano(\\R|\\s)?grams?$", 
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 / 1e+9;
    }
    else if(g_regex_match_simple("^micro(\\R|\\s)?grams?$", 
                                 unit,
                                 G_REGEX_CASELESS,
                                 0)){
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 / 1e+6;
    }
    else if(g_regex_match_simple("^(milli(\\R|\\s)?grams?|mg)$", 
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 / 1e+3;
    }
    else if(g_regex_match_simple("^g(rams?)?$", 
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 ;
    }
    else if(g_regex_match_simple("^kg$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0)){
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 * 1e+3;
    }
    else if(g_regex_match_simple("^((metric)?(\\R|\\s)?ton(nes?)?|mt)$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 * 1e+6;
    }
    else if(g_regex_match_simple("^kt$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Mass;
        measurement.factor = 0.00220462 * 1e+9;
    }
    /* temperature */
    else if(g_regex_match_simple("^(deg(rees?)?)?(\\R|\\s)?celsius|\\x2103$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Temperature;
        measurement.factor = 1;
    }
    /* velocity, base unit is ft/s */
    else if(g_regex_match_simple("^nano?\\s?m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895e-9;
    }
    else if(g_regex_match_simple("^nano?\\s?m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.4680665e-11;
    }
    else if(g_regex_match_simple("^nano?\\s?m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134442e-13;
    }
    else if(g_regex_match_simple("^micro?\\s?m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895e-6;
    }
    else if(g_regex_match_simple("^micro?\\s?m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.4680665e-8;
    }
    else if(g_regex_match_simple("^micro?\\s?m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134442e-10;
    }
    else if(g_regex_match_simple("^milli?\\s?m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895e-3;
    }
    else if(g_regex_match_simple("^milli?\\s?m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.4680665e-5;
    }
    else if(g_regex_match_simple("^milli?\\s?m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134442e-7;
    }
    else if(g_regex_match_simple("^centi?\\s?m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895e-2;
    }
    else if(g_regex_match_simple("^centi?\\s?m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.4680665e-4;
    }
    else if(g_regex_match_simple("^centi?\\s?m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134442e-6;
    }
    else if(g_regex_match_simple("^deci?\\s?m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.2808399e-1;
    }
    else if(g_regex_match_simple("^deci?\\s?m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.468066e-3;
    }
    else if(g_regex_match_simple("^deci?\\s?m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134e-5;
    }
    else if(g_regex_match_simple("^m(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895;
    }
    else if(g_regex_match_simple("m(etres?|eters?)?\\s?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 5.4680665e-2;
    }
    else if(g_regex_match_simple("m(etres?|eters?)?\\s?(per|/)\\s?h(ours?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 9.1134442e-4;
    }
    else if(g_regex_match_simple("^kilom(etres?|eters?)?\\s?(per|/)\\s?s(ec(ond)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 3.280839895e+3;
    }
    else if(g_regex_match_simple("^kilom(etres?|eters?)?(per|/)\\s?m(in(ute)?s?)?$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 54.680664917;
    }
    else if(g_regex_match_simple("^(kilom(etres?|eters?)?(per|/)\\s?h(ours?)?|k(mh|ph|m\\s?/\\s?h))$",
                                 unit,
                                 G_REGEX_CASELESS,
                                 0))
    {
        measurement.system = Imperial;
        measurement.type = Velocity;
        measurement.factor = 0.911344415;
    }

    return measurement;
}

static
ConvertedUnit *
humanize (const char *value_str,
		  const char *multiplier_str,
		  const char *unit)
{
	double multiplier = get_multiplier(multiplier_str);
	Measurement target_measurement = get_target_measurement(unit);
	
	double value = g_ascii_strtod(value_str,
								  NULL);
    gboolean is_neg = value < 0;
    value = fabs(value) * multiplier * target_measurement.factor;
	ConvertedUnit *cv = converted_unit_new();
    switch (target_measurement.system) {
    case SI:
        switch (target_measurement.type) {
        case Length:
            if(value <= 1e-9){
                cv->value = value * 1e+9;
                cv->unit = g_strdup("nm");
            }
            else if(value > 1e-9 && value <= 1e-6){
                cv->value = value * 1e+6;
                cv->unit = g_strdup("micro-m");
            }
            else if(value > 1e-6 && value <= 1e-2){
                cv->value = value * 1e+3;
                cv->unit = g_strdup("mm");
            }
            else if(value > 1e-2 && value < 1){
                cv->value = value * 1e+2;
                cv->unit = g_strdup("cm");
            }
            else if(value >= 1 && value < 1e+3 ){
                cv->value = value;
                cv->unit = g_strdup("m");
            }
            else if(value >= 1e+3){
                cv->value = value / 1e+3;
                cv->unit = g_strdup("km");
            }
            break;
        case Area:
            if(value < 1){
                cv->value = value * 1e+4;
                cv->unit = g_strdup("sqcm");
            }
            else if(value >= 1 && value < 1e+4){
                cv->value = value;
                cv->unit = g_strdup("sqm");
            }
            else{
                cv->value = value / 1e+4;
                cv->unit = g_strdup("hectares");
            }
            break;
        case Volume:
            if(value < 1){
                cv->value = value * 1e+3;
                cv->unit = g_strdup("ml");
            }
            else if(value >= 1 && value < 1000){
                cv->value = value;
                cv->unit = g_strdup("L");
            }
            else{
                cv->value = value / 1e+3;
                cv->unit = g_strdup("m3");
            }
            break;
        case Mass:            
            if(value < 1){
                cv->value = value * 1e+3;
                cv->unit = g_strdup("mg");
            }
            else if(value >= 1 && value < 1e+3){
                cv->value = value;
                cv->unit = g_strdup("g");
            }
            else if(value >= 1e+3 && value < 1e+6){
                cv->value = value / 1e+3;
                cv->unit = g_strdup("kg");
            }
            else{
                cv->value = value / 1e+6;
                cv->unit = g_strdup("mt");
            }
            break;
        case Temperature:
            cv->value = 9.0 / 5.0 * value + 32;
            cv->unit = g_strdup("celsius");
            break;
        case Velocity:
            if(value < 0.0027777778){
                cv->value = value * 100;
                cv->unit = g_strdup("cm/s");
            }
            if(value > 0.0027777778 && value < 0.27777778){
                cv->value = value;
                cv->unit = g_strdup("m/s");                
            }
            else{
                cv->value = value * 3.6;
                cv->unit = g_strdup("km/h");
            }
        }
        break;
    case Imperial:
        switch (target_measurement.type) {
        case Length:
            if(value < 1){
                cv->value = value * 8;
                cv->unit = g_strdup("inches");
            }
            else if(value >= 1 && value < 5280){
                cv->value = value;
                cv->unit = g_strdup("feet");
            }
            else{
                cv->value = value / 5280;
                cv->unit = g_strdup("miles");
            }
            break;
        case Area:
            if(value < 1){
                cv->value = value * 144;
                cv->unit = g_strdup("sq inches");
            }
            else if( value >= 1 && value < 43560){
                cv->value = value;
                cv->unit = g_strdup("sqft");
            }
            else{   
                cv->value = value / 2.778e+7;
                cv->unit = g_strdup("sq miles");
            }
            break;
        case Volume:
            if(value < 0.000578704){
                cv->value = value * 1728;
                cv->unit = g_strdup("in3");
            }
            else if(value >= 0.000578704 && value < 0.16054365){
                cv->value = value * 6.22883546;
                cv->unit = g_strdup("gal");
            }
            else{
                cv->value = value;
                cv->unit = g_strdup("ft3");
            }
            break;            
        case Mass:
            if(value < 0.0625){
                cv->value = value * 16;
                cv->unit = g_strdup("oz");
            }
            else if(value >= 0.0625 && value < 2240){
                cv->value = value;
                cv->unit = g_strdup("lb");
            }
            else{
                cv->value = value / 2240;
                cv->unit = g_strdup("tons");
            }
            break;
        case Temperature:
            cv->value = 5.0 / 9.0 * (value - 32);
            cv->unit = g_strdup("fahrenheit");
            break; 
        case Velocity:
            if(value < 0.122222225){
                cv->value = value * 12;
                cv->unit = g_strdup("in/s");
            }
            if(value > 0.122222225 && value < 1.4666667){
                cv->value = value;
                cv->unit = g_strdup("ft/s");                
            }
            else{
                cv->value = value * 0.68181818;
                cv->unit = g_strdup("mp/h");
            }
        }
        break;
    }
    cv->value *= is_neg ? -1 : 1;
    if(cv->value > -1.0 && cv->value < 1.0){
        cv->value_str = g_strdup_printf("%.6f %s",
                                    cv->value,
                                    cv->unit);        
    }
    else{
        cv->value_str = g_strdup_printf("%'.2f %s",
                                        cv->value,
                                        cv->unit);    
    }
    return cv;
}	

static int 
compare_units(gconstpointer a,
              gconstpointer b)
{
    const ConvertedUnit *cv_a = a;
    char *key_a = g_strdup_printf("%s %s %s",
                                  cv_a->old_value,
                                  cv_a->multiplier,
                                  cv_a->old_unit);
    const ConvertedUnit *cv_b = b;
    char *key_b = g_strdup_printf("%s %s %s",
                                  cv_b->old_value,
                                  cv_b->multiplier,
                                  cv_b->old_unit);
    int diff = strlen(key_a) - strlen(key_b);
    g_free(key_a);
    g_free(key_b);
    return -diff;
}

void
convert_units(const char *text,
              GList     **converted_units)
{   
    GHashTable *unit_hash = g_hash_table_new(g_str_hash,
                                             g_str_equal);
    *converted_units = NULL;
    GMatchInfo *match_info = NULL;
    g_regex_match(unit_regex,
                  text,
                  0,
                  &match_info);
    while(g_match_info_matches(match_info)){
        char *value = g_match_info_fetch_named(match_info,
                                               "value");
        char *multiplier = g_match_info_fetch_named(match_info,
                                                    "multiplier");
        multiplier = g_strchomp(multiplier);
        char *unit = g_match_info_fetch_named(match_info,
                                              "unit");
        char *key = g_strdup_printf("%s %s %s",
                                    value,
                                    multiplier,
                                    unit);
        if(!g_hash_table_contains(unit_hash,
                                  key) &&
           !g_regex_match_simple("^0+$",
                                 value,
                                 0, 0))
        {
            char *comma_less_value = g_regex_replace(comma_regex,
                                                     value,
                                                     -1,
                                                     0,
                                                     "",
                                                     0, NULL);
            ConvertedUnit *cv = humanize(comma_less_value,
                                         multiplier,
                                         unit);            
            cv->whole_match = g_match_info_fetch(match_info,
                                                 0);
            // g_print("unit: '%s'\n", cv->whole_match);
            cv->old_value = value;
            cv->multiplier = multiplier;
            cv->old_unit = unit;
            *converted_units = g_list_insert_sorted(*converted_units,
                                                    cv,
                                                    compare_units);
            g_free(comma_less_value);
            g_hash_table_add(unit_hash,
                             key);
        }
        else{
            g_free(key);
            g_free(value);
            g_free(unit);
            g_free(multiplier);
        }
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    GList *keys = g_hash_table_get_keys(unit_hash);
    GList *list_p = keys;
    while(list_p){
        g_free((char*)list_p->data);
        list_p = list_p->next;
    }
    g_list_free(keys);
    g_hash_table_unref(unit_hash);
}