/*
 * Copyright © 2020 Reza Hasanzadeh
 * Special thanks to 'Dive Into Python 3' book.
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

#include "roman_numeral.h"

static const int ROMAN_ZOO_MAX = 100;

static char *ROMAN_ZOO[] = {
    "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX",
    "X", "XI", "XII", "XIII", "XIV", "XV", "XVI", "XVII", "XVIII", "XIX",
    "XX","XXI", "XXII", "XXIII", "XXIV", "XXV", "XXVI", "XXVII", "XXVIII", "XXIX",
    "XXX", "XXXI", "XXXII", "XXXIII", "XXXIV", "XXXV", "XXXVI", "XXXVII", "XXXVIII", "XXXIX",
    "XL", "XLI", "XLII", "XLIII", "XLIV", "XLV", "XLVI", "XLVII", "XLVIII", "XLIX",
    "L","LI", "LII", "LIII", "LIV", "LV", "LVI", "LVII", "LVIII", "LIX",
    "LX","LXI", "LXII", "LXIII", "LXIV", "LXV", "LXVI", "LXVII", "LXVIII", "LXIX",
    "LXX", "LXXI", "LXXII", "LXXIII", "LXXIV", "LXXV", "LXXVI", "LXXVII", "LXXVIII", "LXXIX", 
    "LXXX", "LXXXI", "LXXXII", "LXXXIII", "LXXXIV", "LXXXV", "LXXXVI", "LXXXVII", "LXXXVIII", "LXXXIX", 
    "XCI", "XCII", "XCIII", "XCIV", "XCV", "XCVI", "XCVII", "XCVIII", "XCIX", "XCX",
    "C"
}; 

gboolean
roman_is_valid(const char *roman)
{    
    if(!roman){
        return FALSE;
    }
    return g_regex_match_simple(
        "^M{0,3}(CM|CD|D?C{0,3})(XC|XL|L?X{0,3})(IX|IV|V?I{0,3})$",
        roman,
        G_REGEX_CASELESS,
        G_REGEX_MATCH_NOTEMPTY);
}

const char *
roman_from_decimal(int decimal)
{
    if(decimal < 1 || decimal > ROMAN_ZOO_MAX){
        return NULL;
    }
    return ROMAN_ZOO[decimal - 1];
}

int
roman_to_decimal(const char *roman)
{
    if(!roman_is_valid(roman)){
        return -1;
    }
    char *roman_pattern = g_strdup_printf("^%s$",
                                          roman);
    int decimal = 0;
    for(; decimal < ROMAN_ZOO_MAX; decimal++){
        if(g_regex_match_simple(roman_pattern,
                                ROMAN_ZOO[decimal],
                                G_REGEX_CASELESS,
                                0))
        {
            break;
        }
    }
    g_free(roman_pattern);
    return decimal < ROMAN_ZOO_MAX ? decimal + 1 : -1;
}

const char *
roman_previous(const char *roman)
{
    int decimal = roman_to_decimal(roman);
    if(decimal < 0){
        return NULL;
    }
    return decimal > 1 ? ROMAN_ZOO[decimal - 2] : NULL;
}

const char *
roman_next(const char *roman)
{
    int decimal = roman_to_decimal(roman) + 1;
    if(decimal > ROMAN_ZOO_MAX){
        return NULL;
    }
    return ROMAN_ZOO[decimal - 1];
}

int 
roman_compare(const char *r1,
              const char *r2)
{
    return roman_to_decimal(r1) - roman_to_decimal(r2);
}