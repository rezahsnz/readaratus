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

#include "find.h"
#include <math.h>

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

int
compare_poppler_rects(const void *a,
                      const void *b)
{
    int comp;
    const PopplerRectangle *rect_a = a;
    const PopplerRectangle *rect_b = b;
    if((rect_a->y1 < rect_b->y1) ||
       (rect_a->y1 == rect_b->y1 &&
        rect_a->x1 > rect_b->x1))
    {
        comp = -1;
    }
    else{
        comp = 1;
    }
    /* desc y, asc x */
    return -comp;
}


int
compare_find_results(const void *a,
                     const void *b)
{
    const FindResult *fr_a = a;
    const FindResult *fr_b = b;
    int comp = fr_a->page_num - fr_b->page_num;
    if(comp == 0){
        const Rect *rect_a = fr_a->physical_layouts->data;
        const Rect *rect_b = fr_b->physical_layouts->data;
        /* asc y, asc  x */
        if((rect_a->y1 < rect_b->y1) ||
           (rect_a->y1 == rect_b->y1 &&
            rect_a->x1 < rect_b->x1))
        {
            comp = -1;
        }
        else{
            comp = 1;
        }
    }
    return comp;    
}

static void
match_found_rects(GList  *list_p,
                  Rect   *prev_rect,
                  double  mean_line_height,
                  GList **matched_rects)
{    
    if(!list_p){
        return;
    }
    GList *candidate_rects = NULL;
    double prev_center_y = rect_center_y(prev_rect);
    GList *rects = list_p->data;
    GList *rect_p = rects;
    while(rect_p){
        Rect *rect = rect_p->data;
        double center_y = rect_center_y(rect);
        if((center_y > prev_center_y) &&
           ((center_y - prev_center_y) <= (mean_line_height * 1.9)))
        {
            candidate_rects = g_list_append(candidate_rects,
                                            rect);
        }
        rect_p = rect_p->next;
    }
    if(candidate_rects){
        candidate_rects = g_list_sort(candidate_rects,
                                      rect_xy_compare);
        Rect *rect = g_list_first(candidate_rects)->data;
        *matched_rects = g_list_append(*matched_rects,
                                       rect);
        match_found_rects(list_p->next,
                          rect,
                          mean_line_height,
                          matched_rects);
    }
    g_list_free(candidate_rects);
}

static void
find_rects_of_text(PageMeta *meta,
                   GRegex   *regex,
                   gboolean  is_dualpage,
                   gboolean  is_whole_words,
                   GList   **find_results)
{
    int poppler_find_options = 0;
    poppler_find_options |= is_whole_words ? POPPLER_FIND_WHOLE_WORDS_ONLY : 0;
    if(poppler_find_options == 0){
        poppler_find_options = POPPLER_FIND_DEFAULT;
    }

    GMatchInfo *match_info = NULL;
    g_regex_match(regex,
                  meta->text,
                  0,
                  &match_info);
    const char *pattern = g_regex_get_pattern(regex);
    GList *already_matched_rects = NULL;
    gboolean is_flattend = FALSE;
    while(g_match_info_matches(match_info)){
        char *match = g_match_info_fetch(match_info,
                                         0);
        char **tokens = g_regex_split_simple("\\R",
                                             match,
                                             0,
                                             0);
        int num_tokens = g_strv_length(tokens);
        /* a: flatten match */
        if(!is_flattend){
            char *flattened_match = g_strjoinv(" ",
                                               tokens);
            GList *pop_rects = poppler_page_find_text_with_options(meta->page,
                                                                   flattened_match,
                                                                   (PopplerFindFlags)poppler_find_options);
            GList *rect_p = pop_rects;
            while(rect_p){
                PopplerRectangle *pr = rect_p->data;
                pr->y1 = meta->page_height - pr->y1;
                pr->y2 = meta->page_height - pr->y2;
                Rect *rect = rect_from_poppler_rectangle(pr);
                poppler_rectangle_free(pr);
                double rect_cy = rect_center_y(rect);
                if(is_dualpage){
                    gboolean is_valid = TRUE;
                    /* prefix: rect must be rightmost */
                    if(pattern[0] != '^'){
                        for(int i = 0; i < meta->num_layouts; i++){
                            Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                                i);
                            double text_cy = rect_center_y(text_rect);
                            if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                               text_rect->x1 >= rect->x2)
                            {
                                is_valid = FALSE;
                                break;
                            }
                            if(text_cy - rect_cy > 2 * meta->mean_line_height){
                                break;
                            }
                        }                        
                    }
                    /* postfix: rect must be leftmost */
                    else{
                        for(int i = 0; i < meta->num_layouts; i++){
                            Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                                i);
                            double text_cy = rect_center_y(text_rect);
                            if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                               text_rect->x1 < rect->x1)
                            {
                                is_valid = FALSE;
                                break;
                            }
                            if(text_cy - rect_cy > 2 * meta->mean_line_height){
                                break;
                            }
                        } 
                    }
                    if(!is_valid){
                        rect_free(rect);
                        rect_p = rect_p->next;  
                        continue;
                    }
                }
                FindResult *find_result = find_result_new();
                find_result->page_num = meta->page_num;
                find_result->match = g_strdup(flattened_match);
                find_result->physical_layouts = g_list_append(find_result->physical_layouts,
                                                              rect);
                *find_results = g_list_append(*find_results,
                                              find_result);
                already_matched_rects = g_list_append(already_matched_rects,
                                                      rect);
                rect_p = rect_p->next;                    
            }
            g_free(flattened_match);
            g_list_free(pop_rects);
            is_flattend = TRUE;
        }
        if(num_tokens == 1){
            g_free(match);
            g_strfreev(tokens);
            g_match_info_next(match_info,
                              NULL);
            continue;
        }
        /* b: split match by 'new line' and perform line matching */
        GList *list_of_rects = NULL;
        char **token_p = tokens;
        while(*token_p){
            GList *rects = NULL;
            GList *pop_rects = poppler_page_find_text_with_options(meta->page,
                                                                   *token_p,
                                                                   (PopplerFindFlags)poppler_find_options);
            if(!pop_rects){                   
                g_print("poppler failed to find: '%s' part of '%s'\n",
                        *token_p, match); 
            }          
            pop_rects = g_list_sort(pop_rects,
                                    compare_poppler_rects);
            GList *rect_p = pop_rects;                
            while(rect_p){
                PopplerRectangle *pr = rect_p->data;
                pr->y1 = meta->page_height - pr->y1;
                pr->y2 = meta->page_height - pr->y2;
                Rect *rect = rect_from_poppler_rectangle(pr);
                poppler_rectangle_free(pr);
                GList *already_p = already_matched_rects;
                while(already_p){
                    if(rects_have_intersection(rect,
                                               already_p->data)){
                        break;
                    }
                    already_p = already_p->next;
                }
                if(!already_p){
                    rects = g_list_append(rects,
                                          rect);
                }
                else{
                    rect_free(rect);
                }
                rect_p = rect_p->next;
            }
            g_list_free(pop_rects);
            if(rects){         
                /* at most one rect per line: the first line contains the
                   rightmost rect and subsequent lines contain lefmost rects */
                GList *one_per_line_rects = NULL;
                GList *seen_lines = NULL;
                rect_p = rects;
                while(rect_p){
                    if(g_list_find(seen_lines,
                                   rect_p->data))
                    {
                        rect_p = rect_p->next;
                        continue;
                    }
                    Rect *rect_base = rect_p->data;
                    GList *same_line_rects = NULL;
                    same_line_rects = g_list_append(same_line_rects,
                                                    rect_base);
                    double base_center_y = rect_center_y(rect_base);
                    GList *next_p = rect_p->next;
                    while(next_p){
                        if(g_list_find(seen_lines,
                                       next_p->data))
                        {
                            next_p = next_p->next;
                            continue;
                        }
                        Rect *rect_next = next_p->data;
                        double next_center_y = rect_center_y(rect_next);
                        if(fabs(next_center_y - base_center_y) < 0.05 * meta->mean_line_height){
                            same_line_rects = g_list_append(same_line_rects,
                                                            rect_next);
                        }
                        next_p = next_p->next;
                    }
                    if(list_of_rects){
                        /* leftmost rect */                                                                                
                        one_per_line_rects = g_list_append(one_per_line_rects,
                                                           g_list_first(same_line_rects)->data);
                    }
                    else{
                        /* rightmost rect*/
                        gboolean is_postfix = is_dualpage && (pattern[0] != '^');
                        one_per_line_rects = g_list_append(one_per_line_rects,
                                                           is_postfix ? g_list_first(same_line_rects)->data
                                                                      : g_list_last(same_line_rects)->data);
                    }
                    seen_lines = g_list_concat(seen_lines,
                                               same_line_rects);
                    rect_p = rect_p->next;
                }
                list_of_rects = g_list_append(list_of_rects,
                                              one_per_line_rects);
                g_list_free(seen_lines);
                g_list_free(rects);
            }
            token_p++;
        }
        if(g_list_length(list_of_rects) == num_tokens){
            /* remove rects from list until the max size of any list is <= the smallest list,
               only retain vertically close rects */
            GList *cur_list_p = list_of_rects;
            while(cur_list_p){
                if(cur_list_p->next){
                    GList *cur_rects = cur_list_p->data;  
                    int cur_list_size = g_list_length(cur_rects);                                                                      
                    GList *next_rects = cur_list_p->next->data;
                    int next_list_size = g_list_length(next_rects);
                    if(cur_list_size == next_list_size){
                        cur_list_p = cur_list_p->next;
                        continue;
                    }
                    GList *edited_large_rects = NULL;
                    GList *small_rects = cur_list_size < next_list_size ? cur_rects : next_rects;
                    GList *large_rects =  cur_list_size > next_list_size ? cur_rects : next_rects;
                    double dist_sign = small_rects == cur_rects ? 1 : -1;
                    GList *small_p = small_rects;
                    while(small_p){
                        Rect *rect_small = small_p->data;
                        double small_center_y = rect_center_y(rect_small);
                        double min_dist = meta->page_height;
                        Rect *closest_rect = NULL;
                        GList *large_p = large_rects;
                        while(large_p){
                            Rect *rect_large = large_p->data;
                            if(!g_list_find(edited_large_rects,
                                            rect_large))
                            {
                                double large_center_y = rect_center_y(rect_large);
                                double dist = dist_sign * (small_center_y - large_center_y);
                                if(dist < min_dist && (fabs(dist) > meta->mean_line_height * 0.05)){
                                    min_dist = small_center_y - large_center_y;
                                    closest_rect = rect_large;
                                }
                            }
                            large_p = large_p->next;
                        }
                        edited_large_rects = g_list_append(edited_large_rects,
                                                           closest_rect);
                        small_p = small_p->next;
                    }
                    if(large_rects == cur_rects){
                        cur_list_p->data = edited_large_rects;
                    }
                    else{
                        cur_list_p->next->data = edited_large_rects;
                    }
                    GList *large_p = large_rects;
                    while(large_p){
                        if(!g_list_find(edited_large_rects,
                                        large_p->data))
                        {
                            rect_free(large_p->data);
                        }
                        large_p = large_p->next;
                    }
                    g_list_free(large_rects);
                }
                cur_list_p = cur_list_p->next;
            }
            GList *rect_p = list_of_rects->data;
            while(rect_p){
                Rect *rect = rect_p->data;   
                double rect_cy = rect_center_y(rect);                   
                /* make sure the first rect is right-most except for multipage postfix rects */
                gboolean is_rightmost = TRUE;
                if((num_tokens > 1) && !(is_dualpage && pattern[0] == '^')){
                    for(int i = 0; i < meta->num_layouts; i++){
                        Rect *text_rect = g_ptr_array_index(meta->physical_text_layouts,
                                                            i);
                        double text_cy = rect_center_y(text_rect);
                        if(fabs(text_cy - rect_cy) < (meta->mean_line_height * 0.05) &&
                           text_rect->x1 >= rect->x2)
                        {
                            is_rightmost = FALSE;
                            break;
                        }
                        if(text_cy - rect_cy > 2 * meta->mean_line_height){
                            break;
                        }
                    }
                }           
                if(!is_rightmost){
                    rect_p = rect_p->next;
                    continue;
                }      
                GList *matched_rects = NULL;
                matched_rects = g_list_append(matched_rects,
                                              rect);
                match_found_rects(list_of_rects->next,
                                  rect,
                                  meta->mean_line_height,
                                  &matched_rects);
                if(g_list_length(matched_rects) == num_tokens){
                    FindResult *find_result = find_result_new();
                    find_result->page_num = meta->page_num;
                    find_result->match = g_strdup(match);
                    GList *ma_rect_p = matched_rects;        
                    while(ma_rect_p){
                        Rect *fr_rect = ma_rect_p->data;
                        find_result->physical_layouts = g_list_append(find_result->physical_layouts,
                                                                      rect_copy(fr_rect));
                        ma_rect_p = ma_rect_p->next;
                    }       
                    *find_results = g_list_append(*find_results,
                                                  find_result);                        
                }
                g_list_free(matched_rects);                    
                rect_p = rect_p->next;
            }
        }
        GList *list_p = list_of_rects;
        while(list_p){
            GList *rects = list_p->data;
            GList *rect_p = rects;
            while(rect_p){
                rect_free(rect_p->data);
                rect_p = rect_p->next;
            }
            g_list_free(rects);
            list_p = list_p->next;
        }
        g_list_free(list_of_rects);            
        g_strfreev(tokens);
        g_free(match);
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    g_list_free(already_matched_rects);
    if(is_dualpage){
        *find_results = g_list_sort(*find_results,
                                    compare_find_results);
        GList *single_p = (pattern[0] == '^') ? g_list_first(*find_results)
                                              : g_list_last(*find_results);
        GList *rem_list = g_list_remove_link(*find_results,
                                             single_p);
        *find_results = single_p;
        GList *list_p = rem_list;
        while(list_p){
            find_result_free(list_p->data);
            list_p = list_p->next;
        }
        g_list_free(rem_list);
    }
}

GList *
find_text(const GPtrArray *metae,
          const char      *find_term,
          int              start_page,
          int              pages_length,
          gboolean         is_dualpage,
          gboolean         is_whole_words)
{ 
    if(!metae || !find_term || strlen(find_term) == 0 ||
       start_page < 0 || pages_length <= 0)
    {
        return NULL;
    }   
    GError *err = NULL;  
    GRegex *nl_ws_regex = g_regex_new(
        "\\s+|\\R+",
        0, 0,
        &err);
    if(!nl_ws_regex){
        g_print("nl_ws_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    } 
    char *no_nl_ws_term = g_regex_replace(nl_ws_regex,
                                          find_term,
                                          -1,
                                          0,
                                          " ",
                                          0,
                                          NULL);
    GRegex *trimmer_regex = g_regex_new(
        "^\\s+|\\s+$",
        0,
        0,
        &err);
    if(!trimmer_regex){
        g_print("trimmer_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }
    char *cleaned_term = g_regex_replace(trimmer_regex,
                                         no_nl_ws_term,
                                         -1,
                                         0,
                                         "",
                                         0,
                                         NULL);
    g_free(no_nl_ws_term);
    g_regex_unref(nl_ws_regex);
    g_regex_unref(trimmer_regex);
    gboolean term_has_whitespace = g_regex_match_simple("\\s",
                                                        cleaned_term,
                                                        0, 0);
    char *temp = cleaned_term;
    cleaned_term = g_regex_escape_string(temp,
                                         -1);
    g_free(temp);
    int term_len = strlen(cleaned_term);
    GString *dashed_term = g_string_sized_new(term_len * 7);
    for(int i = 0; i < strlen(cleaned_term); i++){
        dashed_term = g_string_append_c(dashed_term,
                                        cleaned_term[i]);
        if((i == strlen(cleaned_term) - 1) ||
           (cleaned_term[i] == '\\'))         
        {
            continue;
        }
        if(cleaned_term[i] != ' '){
            if(cleaned_term[i] != '-' && cleaned_term[i + 1] != '-'){
                dashed_term = g_string_append(dashed_term,
                                              "-?");
            }
            if(cleaned_term[i + 1] != ' ' && cleaned_term[i + 1] != '-'){
                dashed_term = g_string_append(dashed_term,
                                              "\\R*");
            }
        }        
    }       
    GRegex *whitespace_regex = g_regex_new(
        "\\s+",
        0,
        0,
        &err);
    if(!whitespace_regex){
        g_print("whitespace_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }    
    err = NULL;
    char *pattern = g_regex_replace_literal(whitespace_regex,
                                            dashed_term->str,
                                            -1,
                                            0,
                                            "(\\R|\\s)+",
                                            0,
                                            &err);
    if(err){
        g_print("whitespace_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);   
    }
    g_regex_unref(whitespace_regex);
    g_string_free(dashed_term,
                  TRUE);
    if(is_whole_words){
        temp = pattern;
        pattern = g_strdup_printf("\\b%s\\b",
                                  pattern);
        g_free(temp);
    }
    err = NULL;
    GRegex *multiline_regex = g_regex_new(
        pattern,
        G_REGEX_CASELESS,
        0,
        &err);
    if(!multiline_regex){
        g_print("multiline_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
    }          
    
    GList *find_results = NULL;
    for(int page_num = start_page; page_num < start_page + pages_length; page_num++){
        PageMeta *meta = g_ptr_array_index(metae,
                                           page_num);              
        /* 1: tokenize the term with word-wraps(dashes followd by newline)" and try again.
              if 'adios' is requested, we try to find
              {'a-\ndios', 'ad-\niso', 'adi-\nos', 'adio-\ns'}           
           2: each whitespace might be a newline so we suppose whitespaces are newlines and try again.
              if 'adios pegasus camus' is requested, we try to find
              {'adios\npegasus camus', 'adios\npegasus\ncamus', 'adios pegasus\ncamus'}
        */  
        if(strlen(meta->text) >= term_len){
            GList *results = NULL;
            find_rects_of_text(meta,
                               multiline_regex,
                               FALSE,
                               is_whole_words,
                               &results);
            find_results = g_list_concat(find_results,
                                         results);
        }
    } 
    g_regex_unref(multiline_regex);
    g_free(pattern);    
    if(!is_dualpage || !term_has_whitespace){
        g_free(cleaned_term);
        return find_results;
    } 
    GList *multipage_prefix_regex_list = NULL;
    GList *multipage_postfix_regex_list = NULL;
    char **tokens = g_regex_split_simple("\\s+",
                                         cleaned_term,
                                         0,
                                         0);
    int num_tokens = g_strv_length(tokens);
    for(int i = 0; i < num_tokens - 1; i++){
        GString *prefix_pattern = g_string_new("");
        for(int j = 0; j <= i; j++){   
            prefix_pattern = g_string_append(prefix_pattern,
                                             *(tokens + j));
            if(j < i){
                prefix_pattern = g_string_append(prefix_pattern,
                                                 "(\\R|\\s)+");
            }
        }
        if(is_whole_words){
            prefix_pattern = g_string_prepend(prefix_pattern,
                                              "\\b");
        }
        prefix_pattern = g_string_append(prefix_pattern,
                                         "$");
        err = NULL;
        GRegex *prefix_regex = g_regex_new(
            prefix_pattern->str,
            G_REGEX_CASELESS | G_REGEX_MULTILINE,
            0,
            &err);
        if(!prefix_regex){
            g_print("prefix_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                    err->domain, err->code, err->message);
        }    
        multipage_prefix_regex_list = g_list_append(multipage_prefix_regex_list,
                                                    prefix_regex);
        g_string_free(prefix_pattern,
                      FALSE);  

        GString *postfix_pattern = g_string_new("^");
        for(int k = i + 1; k < num_tokens; k++){
            postfix_pattern = g_string_append(postfix_pattern,
                                              *(tokens + k));
            if(k < num_tokens - 1){
                postfix_pattern = g_string_append(postfix_pattern,
                                                  "(\\R|\\s)+");
            }
        }
        if(is_whole_words){
            postfix_pattern = g_string_append(postfix_pattern,
                                              "\\b");
        }
        err = NULL;
        GRegex *postfix_regex = g_regex_new(
            postfix_pattern->str,
            G_REGEX_CASELESS | G_REGEX_MULTILINE,
            0,
            &err);
        if(!postfix_regex){
            g_print("postfix_regex error.\ndomain:  %d, \ncode: %d, \nmessage: %s\n",
                    err->domain, err->code, err->message);
        }
        multipage_postfix_regex_list = g_list_append(multipage_postfix_regex_list,
                                                     postfix_regex);
        g_string_free(postfix_pattern,
                      FALSE);
    }   
    if(g_list_length(multipage_prefix_regex_list) != g_list_length(multipage_postfix_regex_list)){
        g_print("the number of prefix and postfix regexes are not the same.\n");
    }
    /* 3: multipage search. a page might end with some terms and the next page begin with the 
          rest of the terms.
          if 'adios pegasus camus' is requested, we try to find the first set of terms in the
          current page and the rest of the terms in the next page.
    */
    int multipage_regex_num = g_list_length(multipage_prefix_regex_list);
    for(int page_num = start_page; page_num < start_page + pages_length - 1; page_num++){
        PageMeta *meta_prefix = g_ptr_array_index(metae,
                                                  page_num);
        PageMeta *meta_postfix = g_ptr_array_index(metae,
                                                   page_num + 1);
        for(int i = 0; i < multipage_regex_num; i++){
            GRegex *prefix_regex = g_list_nth_data(multipage_prefix_regex_list,
                                                   i);
            GList *find_results_prefix = NULL;       
            find_rects_of_text(meta_prefix,
                               prefix_regex,
                               TRUE,
                               is_whole_words,
                               &find_results_prefix);            
            if(find_results_prefix){         
                FindResult *find_result_prefix = find_results_prefix->data;
                /* make sure prefix result does not intersect previous find results */
                GList *already_p = find_results;
                while(already_p){
                    FindResult *fr_already = already_p->data;
                    if((fr_already->page_num == meta_prefix->page_num) &&
                       rect_lists_intersect(fr_already->physical_layouts,
                                            find_result_prefix->physical_layouts))
                    {
                        break;
                    }
                    already_p = already_p->next;
                }
                if(already_p){
                    find_result_free(find_result_prefix);
                    g_list_free(find_results_prefix);
                    continue;
                }            

                GRegex *postfix_regex = g_list_nth_data(multipage_postfix_regex_list,
                                                        i);
                GList *find_results_postfix = NULL;
                find_rects_of_text(meta_postfix,
                                   postfix_regex,
                                   TRUE,
                                   is_whole_words,
                                   &find_results_postfix);                
                if(find_results_postfix){
                    FindResult *find_result_postfix = find_results_postfix->data;
                    /* make sure postfix result does not intersect previous find results */
                    GList *already_p = find_results;
                    while(already_p){
                        FindResult *fr_already = already_p->data;
                        if((fr_already->page_num == meta_postfix->page_num) &&
                           rect_lists_intersect(fr_already->physical_layouts,
                                                find_result_postfix->physical_layouts))
                        {
                            break;
                        }
                        already_p = already_p->next;
                    }
                    if(already_p){
                        find_result_free(find_result_prefix);
                        g_list_free(find_results_prefix);
                        find_result_free(find_result_postfix);
                        g_list_free(find_results_postfix);
                        continue;
                    }
                    find_result_prefix->page_postfix = find_result_postfix;
                    find_result_postfix->page_prefix = find_result_prefix;                    
                    find_results = g_list_append(find_results,
                                                 find_result_prefix);
                    find_results = g_list_append(find_results,
                                                 find_result_postfix);
                    g_list_free(find_results_prefix);
                    g_list_free(find_results_postfix);
                    /*g_print("multipage find result: %d: '%s', %d: '%s'\n",
                            page_num, find_result_prefix->match,
                            page_num + 1, find_result_postfix->match);*/
                }
                else{
                    find_result_free(find_result_prefix);                                       
                    g_list_free(find_results_prefix);
                }
            }
        }
    }
    GList *list_p = multipage_prefix_regex_list;
    while(list_p){
        g_regex_unref(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(multipage_prefix_regex_list);
    list_p = multipage_postfix_regex_list;
    while(list_p){
        g_regex_unref(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(multipage_postfix_regex_list);
    g_free(cleaned_term);        
    return find_results;
}
