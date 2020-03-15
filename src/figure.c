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

#include "figure.h"

static GRegex *text_preprocessor_regex = NULL;
static GRegex *figure_caption_regex = NULL;
static GRegex *figure_reference_regex = NULL;
static GRegex *whole_match_cleaner_regex = NULL;
static GRegex *id_cleaner_regex = NULL;

void
figure_module_init(void)
{
    GError *err = NULL;
    text_preprocessor_regex = g_regex_new(
        "[0-9]+,[0-9]+",
        G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
        0,
        &err);
    if(!text_preprocessor_regex){
        g_print("regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }    

    figure_caption_regex = g_regex_new(
        "# label\n"
        "^(?<label>fig(ure)?|pic(ture)?|image|img|photo|map|box|illustration)?\\s?(\\.|-|_)?\\s?(\\(|\\[|\\{)?\\s?\n"
        "(?<id>\n"
        "# alpha-numerals: E-2, 2.A, 67.8-a_Y, ...\n"
        "(([0-9]+|[a-zA-Z])(\\.|-|_)([0-9]+|[a-zA-Z])((\\.|-|_)([0-9]+|[a-zA-Z]))*)|\n"        
        "# numerals only: 4.5\n"
        "([0-9]+((\\.|-|_)[0-9]*)*)\n"
        ")(\\s?(\\)|\\]|\\}))?",
        G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_MULTILINE | G_REGEX_NO_AUTO_CAPTURE,
        0,
        &err);
    if(!figure_caption_regex){
        g_print("figure caption regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }

    figure_reference_regex = g_regex_new(
        "# label\n"
        "\\b(?<label>fig(ure)?|pic(ture)?|image|img|photo|map|box|illustration)\\s?(-\\R+)?(\\.|-|_)?\\s?(\\(|\\[|\\{)?\\s?\n"
        "(?<id>\n"
        "# alpha-numerals: E-2, 2.A, 67.8-a_Y, ...\n"
        "(([0-9]+|[a-zA-Z])(\\.|-|_)([0-9]+|[a-zA-Z])((\\.|-|_)([0-9]+|[a-zA-Z]))*)|\n"        
        "# numerals only: 4.5\n"
        "([0-9]+((\\.|-|_)[0-9]*)*)\n"
        ")(\\s?(\\)|\\]|\\}))?",
        G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
        0,
        &err);
    if(!figure_reference_regex){
        g_print("figure reference regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }    

    whole_match_cleaner_regex = g_regex_new(
        "^(\\.|-|_)*|(\\.|-|_)*$|\\R$",
         G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
         0,
         &err);
    if(!whole_match_cleaner_regex){
        g_print("regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }   

    id_cleaner_regex = g_regex_new(
        "^(\\.|-|_)*|(\\.|-|_)*$",
         G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
         0,
         &err);
    if(!id_cleaner_regex){
        g_print("regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }    
}

void 
figure_module_destroy(void)
{
    g_regex_unref(text_preprocessor_regex);
    g_regex_unref(figure_caption_regex); 
    g_regex_unref(figure_reference_regex);
    g_regex_unref(whole_match_cleaner_regex);
    g_regex_unref(id_cleaner_regex);
}

Figure *
figure_new(void)
{
    Figure *new_fig = g_malloc(sizeof(Figure));
    new_fig->whole_match = NULL;
    new_fig->label = NULL;
    new_fig->is_label_exclusive = FALSE;
    new_fig->id = NULL;
    new_fig->is_id_complex = FALSE;
    new_fig->caption_physical_layout = NULL; 
    new_fig->page_num = -1;
    new_fig->image_id = -1;
    new_fig->image_physical_layout = NULL;
    new_fig->image = NULL;
    new_fig->captions = NULL;
    new_fig->reference = NULL;
    return new_fig;
}

Figure *
figure_copy(const Figure *figure)
{
    Figure *new_fig = figure_new();
    new_fig->whole_match = g_strdup(figure->whole_match);
    new_fig->label = figure->label ? g_strdup(figure->label) : NULL;
    new_fig->is_label_exclusive = figure->is_label_exclusive;
    new_fig->id = g_strdup(figure->id);
    new_fig->is_id_complex = figure->is_id_complex;
    return new_fig;
}

void
figure_free (Figure *fig)
{
    if(!fig){
        return;
    }
    g_free(fig->whole_match);
    g_free(fig->label);
    g_free(fig->id);
    rect_free(fig->image_physical_layout);
    GList *captions_p = fig->captions;
    while(captions_p){
        Caption *caption = captions_p->data;
        rect_free(caption->physical_layout);
        g_free(caption);
        captions_p = captions_p->next;
    }
    g_list_free(fig->captions);
    g_free(fig);
}

GList * 
extract_figure_captions (const char *text)
{
    GList *figures = NULL;
    char *filtered_text = g_regex_replace(text_preprocessor_regex,
                                          text,
                                          -1,
                                          0,
                                          "",
                                          0, NULL);
    GMatchInfo *match_info = NULL;
    g_regex_match(figure_caption_regex,
                  filtered_text,
                  0,
                  &match_info);
    while(g_match_info_matches(match_info)){
        char *whole_match = g_match_info_fetch(match_info,
                                               0);
        char *cleaned_whole_match = g_regex_replace(whole_match_cleaner_regex,
                                                    whole_match,
                                                    -1,
                                                    0,
                                                    "",
                                                    0,
                                                    NULL);
        g_free(whole_match);        
        if(g_regex_match_simple("\\R",
                                cleaned_whole_match,
                                0,
                                0))
        {
            g_free(cleaned_whole_match);
            g_match_info_next(match_info,
                              NULL);
            continue;
        } 
        Figure *fig = figure_new();
        fig->whole_match = cleaned_whole_match;
        char *label = g_match_info_fetch_named(match_info,
                                               "label");
        if(strlen(label) > 0){
            fig->label =  label;
            fig->is_label_exclusive = g_regex_match_simple("fig(ure)?|pic(ture)?|image|img|photo|illustration",
                                                           fig->label,
                                                           G_REGEX_CASELESS,
                                                           0);
        }
        else{
            fig->label = NULL;
            fig->is_label_exclusive = FALSE;
            g_free(label);
        }
        char *id = g_match_info_fetch_named(match_info,
                                            "id");
        fig->id = g_regex_replace(id_cleaner_regex,
                                  id,
                                  -1,
                                  0,
                                  "",
                                  0,
                                  NULL);
        g_free(id);        
        fig->is_id_complex = g_regex_match_simple("\\.|-|_",
                                                  fig->id,
                                                  0,
                                                  0);
        figures = g_list_append(figures,
                                fig);
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    g_free(filtered_text);    
    
    return figures;
}

GList * 
extract_figure_references (const char *text)
{
    GList *referenced_figures = NULL;
    GHashTable *id_hash = g_hash_table_new(g_str_hash,
                                           g_str_equal);
    GMatchInfo *match_info = NULL;
    g_regex_match(figure_reference_regex,
                  text,
                  0,
                  &match_info);
    while(g_match_info_matches(match_info)){
        char *label = g_match_info_fetch_named(match_info,
                                               "label");
        char *id = g_match_info_fetch_named(match_info,
                                            "id");
        char *clean_id = g_regex_replace(id_cleaner_regex,
                                         id,
                                         -1,
                                         0,
                                         "",
                                         0,
                                         NULL);
        g_free(id);        
        char *key = g_strdup_printf("%s#%s",
                                    label,
                                    clean_id);
        if(!g_hash_table_contains(id_hash,
                                  key))
        {
            g_hash_table_add(id_hash,
                             key);
            ReferencedFigure *ref_figure = g_malloc(sizeof(ReferencedFigure));
            ref_figure->label = label;
            ref_figure->id = clean_id;
            ref_figure->find_results = NULL;
            ref_figure->reference = NULL;
            referenced_figures = g_list_append(referenced_figures,
                                               ref_figure);
        }
        else{
            g_free(key);
            g_free(label);
            g_free(clean_id);
        }
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, id_hash);
    while(g_hash_table_iter_next(&iter, &key, &value)){        
        g_free((char*)key);
    }
    g_hash_table_unref(id_hash);
    return referenced_figures;
}

gboolean
are_figure_labels_equal(const char *l1,
                        const char *l2)
{
    if(!l1 && !l2){
        return TRUE;
    }
    if((!l1 && g_regex_match_simple("fig(ure)?|pic(ture)?|image|img|photo|illustration",
                                                           l2,
                                                           G_REGEX_CASELESS,
                                                           0)) ||
       (!l2 && g_regex_match_simple("fig(ure)?|pic(ture)?|image|img|photo|illustration",
                                                           l1,
                                                           G_REGEX_CASELESS,
                                                           0)))
    {
        return TRUE;
    }
    if(!l1 || !l2){
        return FALSE;
    }
    if(g_regex_match_simple("fig(ure)?",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("fig(ure)?",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("pic(ture)?",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("pic(ture)?",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("image|img",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("image|img",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("photo",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("photo",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("illustration",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("illustration",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("map",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("map",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    if(g_regex_match_simple("box",
                            l1,
                            G_REGEX_CASELESS,
                            0) &&
       g_regex_match_simple("box",
                            l2,
                            G_REGEX_CASELESS,
                            0))
    {
        return TRUE;
    }
    return FALSE;    
}
