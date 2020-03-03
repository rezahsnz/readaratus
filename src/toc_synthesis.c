#include "toc_synthesis.h"

#include "find.h"
#include "page_meta.h"
#include "roman_numeral.h"
#include <math.h>
#include <poppler/glib/poppler.h>

typedef struct
{
    char *match;
    char *label;
    char *id;
    char *caption;
    char *page_label;
}TOCLine;

typedef struct
{
    PageMeta *meta;
    char **lines;
    int num_lines;
    int num_labels;
    double score;
}ContentsPage;

static void
align_unprocessed_lines(GPtrArray *page_meta_list,
                        GHashTable *unprocessed_line_hash,
                        GList     **aligned_line_list)
{
    /* 
       align incorrectly separated lines of text by simple geograhical
       checks(comparing Y-centers with thresholding)
    */
    *aligned_line_list = NULL;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init(&iter, unprocessed_line_hash);
    while(g_hash_table_iter_next(&iter, &key, &value)){
        int page_num = GPOINTER_TO_INT(key);
        PageMeta *meta = g_ptr_array_index(page_meta_list,
                                           page_num);
        const double ALIGNMENT_THRESHOLD = meta->mean_line_height * 0.25;
        GList *unprocessed_line_list = value;
        GList *already_matched_list = NULL;
        GList *list_p = unprocessed_line_list;
        while(list_p){
            if(g_list_find(already_matched_list,
                           list_p))
            {
                list_p = list_p->next;
                continue;
            }
            GList *matched_line_list = NULL,
                  *matched_rect_list = NULL;
            char *line = list_p->data;            
            GList *rect_list = NULL;
            GList *pop_list = poppler_page_find_text_with_options(meta->page,
                                                                  line,
                                                                  POPPLER_FIND_WHOLE_WORDS_ONLY);
            GList *pop_p = pop_list;
            while(pop_p){
                Rect *rect = rect_from_poppler_rectangle(pop_p->data);
                rect->y1 = meta->page_height - rect->y1;
                rect->y2 = meta->page_height - rect->y2;                
                rect_list = g_list_append(rect_list,
                                          rect);
                pop_p = pop_p->next;
            }
            g_list_free(pop_list);
            GList *list_other_p = list_p->next;
            while(list_other_p){
                if(g_list_find(already_matched_list,
                               list_other_p))
                {
                    list_other_p = list_other_p->next;
                    continue;
                }
                char *line_other = list_other_p->data;
                GList *rect_other_list = NULL;
                GList *pop_other_list = poppler_page_find_text_with_options(meta->page,
                                                                            line_other,
                                                                            POPPLER_FIND_WHOLE_WORDS_ONLY);
                GList *pop_other_p = pop_other_list;
                while(pop_other_p){
                    Rect *rect_other = rect_from_poppler_rectangle(pop_other_p->data);
                    rect_other->y1 = meta->page_height - rect_other->y1;
                    rect_other->y2 = meta->page_height - rect_other->y2;                
                    rect_other_list = g_list_append(rect_other_list,
                                                    rect_other);
                    pop_other_p = pop_other_p->next;
                }
                g_list_free(pop_other_list);
                gboolean is_matched = FALSE;
                GList *rect_p = rect_list;
                while(!is_matched && rect_p){
                    Rect *rect = rect_p->data;
                    double rect_cy = rect_center_y(rect);
                    GList *rect_other_p = rect_other_list;
                    while(!is_matched && rect_other_p){
                        Rect *rect_other = rect_other_p->data;
                        double rect_other_cy = rect_center_y(rect_other);
                        if(fabs(rect_cy - rect_other_cy) < ALIGNMENT_THRESHOLD){
                            if(!matched_line_list){
                                matched_line_list = g_list_append(matched_line_list,
                                                                  list_p);
                                matched_rect_list = g_list_append(matched_rect_list,
                                                                  rect_copy(rect));
                            }
                            matched_line_list = g_list_append(matched_line_list,
                                                              list_other_p);
                            matched_rect_list = g_list_append(matched_rect_list,
                                                              rect_copy(rect_other));
                            is_matched = TRUE;
                        }
                        rect_other_p = rect_other_p->next;
                    }                    
                    rect_p = rect_p->next;
                }
                g_list_free_full(rect_other_list,
                                 (GDestroyNotify)rect_free);
                list_other_p = list_other_p->next;
            }
            g_list_free_full(rect_list,
                             (GDestroyNotify)rect_free);
            if(matched_line_list){
                already_matched_list = g_list_concat(already_matched_list,
                                                     matched_line_list);
                GHashTable *rect_string_hash = g_hash_table_new(g_direct_hash,
                                                                g_direct_equal);
                for(int i = 0; i < g_list_length(matched_line_list); i++){
                    Rect *rect = g_list_nth_data(matched_rect_list,
                                                 i);
                    GList *line_string_p = g_list_nth_data(matched_line_list,
                                                           i);
                    g_hash_table_insert(rect_string_hash,
                                        rect,
                                        line_string_p->data);
                }
                matched_rect_list = g_list_sort(matched_rect_list,
                                                rect_x_compare);
                GString *toc_line = g_string_new(NULL);
                GList *rect_p = matched_rect_list;
                while(rect_p){
                    Rect *rect = rect_p->data;
                    char *string = g_hash_table_lookup(rect_string_hash,
                                                       rect);
                    toc_line = g_string_append(toc_line,
                                               string);
                    if(rect_p->next){
                        toc_line = g_string_append(toc_line,
                                                   " ");
                    }
                    rect_p = rect_p->next;
                }
                *aligned_line_list = g_list_append(*aligned_line_list,
                                                   toc_line->str);
                g_string_free(toc_line,
                              FALSE);
                g_hash_table_unref(rect_string_hash);
                g_list_free_full(matched_rect_list,
                                 (GDestroyNotify)rect_free);
            }
            list_p = list_p->next;
        }
        g_list_free(already_matched_list);
    }    
}

static void
decompose_lines(GList *line_list,
                GList **toc_line_list,
                GList **unprocessed_line_list)
{
    /* 
       decompose a line of text into separate toc items.
       'chapter II homology 610' >
       {label: 'chapter', id: 'II', caption: 'homology', page_label: '610'}
    */    
    static GRegex *discovery_regex = NULL;
    if(!discovery_regex){
        const char *discovery_pattern = 
            "^\\s*\n"
            "# item label: part, chapter, ...\n"
            "(?<label>part|ch(apter)?|(sub)?sec(tion)?)?\\s*\n"
            "(?<id>\n"
            "# digits first: 1, 1.2, 1.a, ...\n"
            "[0-9]+((\\.|-|_)([0-9]+|[a-zA-Z]))*|\n"
            "# @alpha first: a, a.1, a.a\n"
            "# named numbers up to 99\n"
            "zero|one|two|three|four|five|six|seven|eight|nine|\n"
            "ten|eleven|twelve|(thir|four|fif|six|seven|eigh|nin)-?teen|\n"
            "((twenty|thirty|fourty|fifty|sixty|seventy|eighty|ninty)(-|\\s*)?)\n"
            "(one|two|three|four|five|six|seven|eight|nine)?|\n"
            "# roman numerals up to 99\n"
            "(I{1,3}|I?V|VI{1,3}|IX)|(XL|XC|L?X{1,3})(I{1,3}|I?V|VI{1,3}|IX)?)?\\b\\s*\n"
            "(?<caption>.+\\b(?=((I{1,3}|I?V|VI{1,3}|IX)|(XL|XC|L?X{1,3})(I{1,3}|I?V|VI{1,3}|IX)?|\\d+)$))\n"
            "(?<page_label>((I{1,3}|I?V|VI{1,3}|IX)|(XL|XC|L?X{1,3})(I{1,3}|I?V|VI{1,3}|IX)?|\\d+))$";
        GError *err = NULL;
        discovery_regex = g_regex_new(discovery_pattern,
                                      G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
                                      G_REGEX_MATCH_NOTEMPTY,
                                      &err);
        if(!discovery_regex){
            g_print("discovery_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                    err->domain, err->code, err->message);
            return; 
        }
    }
    *toc_line_list = NULL;
    if(unprocessed_line_list){
        *unprocessed_line_list = NULL;
    }
    GList *list_p = line_list;
    while(list_p){
        char *line = list_p->data;
        GMatchInfo *match_info = NULL;
        g_regex_match(discovery_regex,
                      line,
                      0,
                      &match_info);
        if(g_match_info_matches(match_info)){
            char *label = g_match_info_fetch_named(match_info,
                                                   "label");
            if(strlen(label) == 0){
                g_free(label);
                label = NULL;
            }            
            char *id = g_match_info_fetch_named(match_info,
                                                "id");
            if(strlen(id) == 0){
                g_free(id);
                id = NULL;
            }
            TOCLine *toc_line = g_malloc(sizeof(TOCLine)); 
            toc_line->match = g_match_info_fetch(match_info,
                                                 0);
            toc_line->label = label;
            toc_line->id = id;
            toc_line->caption = g_match_info_fetch_named(match_info,
                                                         "caption");
            toc_line->page_label = g_match_info_fetch_named(match_info,
                                                            "page_label");
            *toc_line_list = g_list_append(*toc_line_list,
                                           toc_line);
        }
        else{        
            if(unprocessed_line_list){    
                *unprocessed_line_list = g_list_append(*unprocessed_line_list,
                                                       g_strdup(line));
            }
        }
        g_match_info_free(match_info);
        list_p = list_p->next;
    }
}

static int
compare_toc_lines(const void *a,
                  const void *b)
{
    const TOCLine *toc_line_a = a;
    const TOCLine *toc_line_b = b;
    if(roman_is_valid(toc_line_a->page_label)){
        if(roman_is_valid(toc_line_b->page_label)){
            return roman_to_decimal(toc_line_a->page_label) -
                   roman_to_decimal(toc_line_b->page_label);
        }
        else{
            return -1;
        }
    }
    else{
        if(roman_is_valid(toc_line_b->page_label)){            
            return +1;
        }
        else{
            char *end_ptr = NULL;
            int page_num_a = g_ascii_strtoll(toc_line_a->page_label,
                                             &end_ptr,
                                             10);
            end_ptr = NULL;
            int page_num_b = g_ascii_strtoll(toc_line_b->page_label,
                                             &end_ptr,
                                             10);
            return page_num_a - page_num_b;
        }
    }    
}

static int
compare_contents_page_groups(const void *a,
                             const void *b)
{
    double weighted_score_a = 0.0,
           weighted_score_b = 0.0;
    int len = 0;
    const GList *list_p = a;
    while(list_p){        
        ContentsPage *contents_page = list_p->data;
        weighted_score_a += contents_page->score * 0.65 + 
            ((float)contents_page->num_labels / contents_page->num_lines) * 0.35;
        list_p = list_p->next;
        len++;
    }
    weighted_score_a /= len;
    len = 0;
    list_p = b;
    while(list_p){        
        ContentsPage *contents_page = list_p->data;
        weighted_score_b += contents_page->score * 0.65 + 
            ((float)contents_page->num_labels / contents_page->num_lines) * 0.35;
        list_p = list_p->next;
        len++;
    }
    weighted_score_b /= len;
    return weighted_score_a - weighted_score_b;
}

static void
treeize_toc_list(GHashTable *page_label_num_hash,
                 GList      *label_list,
                 TOCItem   **parent,
                 GList      *list_p,
                 const char *label_parent)
{    
    if(!list_p){
        return;
    }    
    TOCLine *toc_line = list_p->data;
    int my_label_index = string_index_in_list(label_list,
                                              toc_line->label,
                                              FALSE);
    TOCItem *actual_parent = *parent;
    char *inferred_label = NULL;
    if(my_label_index < 0){
        if(!toc_line->id){
            treeize_toc_list(page_label_num_hash,
                             label_list,
                             parent,
                             list_p->next,
                             label_parent);
            return;
        }
        inferred_label = toc_infer_child_label(label_parent);
    }
    else{        
        while(my_label_index <= string_index_in_list(label_list,
                                                     actual_parent->label,
                                                     FALSE))
        {
            actual_parent = actual_parent->parent;
        }
        inferred_label = toc_line->label;
    }
    TOCItem *toc_item_child = toc_item_new();
    toc_item_child->title = g_strconcat(inferred_label, " ",
                                        toc_line->id ? toc_line->id : "",
                                        toc_line->id ? " " :  "",
                                        toc_line->caption,
                                        NULL);
    toc_item_child->label = g_strdup(inferred_label);
    toc_item_child->id = toc_line->id ? g_strdup(toc_line->id) : NULL;
    toc_item_child->page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                                   toc_line->page_label));
    toc_item_child->parent = actual_parent;
    actual_parent->children = g_list_append(actual_parent->children,
                                            toc_item_child);
    treeize_toc_list(page_label_num_hash,
                     label_list,
                     &toc_item_child,
                     list_p->next,
                     toc_line->label);    
}

static void
find_toc_target_position(GPtrArray *page_meta_list,
                         TOCItem *toc_item)
{
    /*
      find position of a toc item's heading in its page.      
    */
    if(!toc_item){
        return;
    }
    GList *list_p = toc_item->children;
    while(list_p){
        find_toc_target_position(page_meta_list,
                                 list_p->data);
        list_p = list_p->next;
    }
    if(toc_item->page_num < 0 || !toc_item->title){
        return;
    }
    /* 1: look up title */
    GList *find_results = NULL;
        find_results = find_text(page_meta_list,
                                toc_item->title,
                                toc_item->page_num,
                                1,
                                FALSE,
                                TRUE);
    find_results = g_list_sort(find_results,
                               compare_find_results);
    double target_y1 = -1.0,
           tallest = -1.0;
    list_p = find_results;
    while(list_p){
        FindResult *fr = list_p->data;
        Rect *rect_first = g_list_first(fr->physical_layouts)->data,
             *rect_last  = g_list_last(fr->physical_layouts)->data;
        double t = fabs(rect_last->y2 - rect_first->y1);
        if(t > tallest){
            tallest = t;
            target_y1 = rect_first->y1;
        }
        list_p = list_p->next;
    }
    /* 2: remove label and id from title and try again */
    if(target_y1 < 0 && (toc_item->label || toc_item->id)){
        char *needle_without_label = NULL;
        if(toc_item->label){
            char *pattern = g_strdup_printf("^%s\\b\\s*",
                                            toc_item->label);
            GError *err = NULL;
            GRegex *regex = g_regex_new(pattern,
                                        G_REGEX_CASELESS,
                                        G_REGEX_MATCH_NOTEMPTY,
                                        &err);
            if(!regex){
                g_print("label_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                        err->domain, err->code, err->message);
                return; 
            }
            needle_without_label = g_regex_replace(regex,
                                                   toc_item->title,
                                                   -1, 0,
                                                   "",
                                                   0, NULL);
            g_regex_unref(regex);
            g_free(pattern);
        }
        char *needle = NULL;
        if(toc_item->id){
            char *pattern = g_strdup_printf("^%s\\b\\s*\\W*",
                                            toc_item->id);
            GError *err = NULL;
            GRegex *regex = g_regex_new(pattern,
                                        G_REGEX_CASELESS,
                                        G_REGEX_MATCH_NOTEMPTY,
                                        &err);
            if(!regex){
                g_print("label_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                        err->domain, err->code, err->message);
                return; 
            }
            needle = g_regex_replace(regex,
                                     needle_without_label ? needle_without_label
                                                          : toc_item->label,
                                     -1, 0,
                                     "",
                                     0, NULL);
            g_regex_unref(regex);
            g_free(pattern);
        }
        g_free(needle_without_label);
        find_results = NULL;
        find_results = find_text(page_meta_list,
                                 needle,
                                 toc_item->page_num,
                                 1,
                                 FALSE,
                                 TRUE);
        g_free(needle);        
        find_results = g_list_sort(find_results,
                                   compare_find_results);
        GList *list_p = find_results;
        while(list_p){
            FindResult *fr = list_p->data;
            Rect *rect_first = g_list_first(fr->physical_layouts)->data,
                 *rect_last  = g_list_last(fr->physical_layouts)->data;
            double t = fabs(rect_last->y2 - rect_first->y1);
            if(t > tallest){
                tallest = t;
                target_y1 = rect_first->y1;
            }
            list_p = list_p->next;
        }
    }
    g_list_free_full(find_results,
                     (GDestroyNotify)find_result_free);                         
    if(target_y1 > 0){
        PageMeta *meta = g_ptr_array_index(page_meta_list,
                                           toc_item->page_num);
        toc_item->offset_y = meta->page_height - target_y1;
    }
}

void
toc_create_from_contents_pages(PopplerDocument *doc,
                               GPtrArray       *page_meta_list,
                               GHashTable      *page_label_num_hash,
                               TOCItem        **head_item)
{
    /*
      scan document's initial pages for toc contents. if a page happens to
      contain toc contents, try to extract toc related data from the page.
      besides valid data, each page might also contain separate lines that
      are incorrectly scattered by the pdf engine. to recycle scattered
      lines, a geographical line alignment procedure is employed. finally,
      the extracted toc lines are turned into a tree-like toc structure.
    */
    const int MIN_CONTENTS_PAGES = 18;
    const int NUM_CONTENTS_PAGES = MAX(floor(page_meta_list->len * 0.05),
                                       MIN_CONTENTS_PAGES);
    GList *contents_page_group_list = NULL;
    int most_recent_contents_page = -2;
    for(int page_num = 0; page_num < NUM_CONTENTS_PAGES; page_num++){
        PageMeta *meta = g_ptr_array_index(page_meta_list,
                                           page_num);
        char **lines = g_regex_split_simple("\\R",
                                            meta->text,
                                            0, 0);
        int num_lines = g_strv_length(lines);
        int num_labels = 0;
        int num_matched_lines = 0;
        char **line_p = lines;
        while(*line_p){
            if(g_regex_match_simple("\\b((I{1,3}|I?V|VI{1,3}|IX)|(XL|XC|L?X{1,3})(I{1,3}|I?V|VI{1,3}|IX)?|\\d+)$",
                                    *line_p,
                                    G_REGEX_CASELESS,
                                    0))
            {
                num_matched_lines++;
            }
            if(g_regex_match_simple("^(part|ch(apter)?|(sub)?sec(tion)?)\\b",
                                    *line_p,
                                    G_REGEX_CASELESS,
                                    0))
            {
                num_labels++;
            }
            line_p++;
        }
        float score = (float)num_matched_lines / num_lines;
        if(score == 1.0){
            g_print("Possible noisy TOC due to complete match score at page %s.\n",
                    meta->page_label->label);
        }
        if(score >= 0.7){
            ContentsPage *contents_page = g_malloc(sizeof(ContentsPage));
            contents_page->meta = meta;
            contents_page->lines = lines;
            contents_page->num_lines = num_lines;
            contents_page->score = score;
            contents_page->num_labels = num_labels;
            if(meta->page_num - most_recent_contents_page == 1){ 
                GList *contents_page_group = g_list_last(contents_page_group_list)->data;
                contents_page_group = g_list_append(contents_page_group,
                                                    contents_page);               
            }
            else{                
                GList *contents_page_group = NULL;
                contents_page_group = g_list_append(contents_page_group,
                                                    contents_page); 
                contents_page_group_list = g_list_append(contents_page_group_list,
                                                         contents_page_group); 
            }
            most_recent_contents_page = meta->page_num;
        }
        else{
            g_strfreev(lines);
        }
    }
    if(!contents_page_group_list){
        return;
    }
    contents_page_group_list = g_list_sort(contents_page_group_list,
                                           compare_contents_page_groups);
    g_print("page groups likely to contain toc data:\n");
    GList *group_p = contents_page_group_list;
    while(group_p){
        g_print("{");
        GList *list_p = group_p->data;
        while(list_p){
            ContentsPage *contents_page = list_p->data;
            g_print("'%s'%s",
                    contents_page->meta->page_label->label,
                    list_p->next ? ", " : "");
            list_p = list_p->next;
        }
        g_print("}\n");
        group_p = group_p->next;
    }
    GList *final_toc_line_list = NULL;
    group_p = g_list_last(contents_page_group_list);
    while(group_p){
        GHashTable *unprocessed_line_hash = g_hash_table_new(g_direct_hash,
                                                             g_direct_equal);       
        GList *toc_line_list = NULL;  
        int total_lines_of_group = 0;
        GList *contents_p = group_p->data;
        while(contents_p){
            ContentsPage *contents_page = contents_p->data; 
            total_lines_of_group += contents_page->num_lines;       
            GList *line_list = NULL;
            char **line_p = contents_page->lines;
            while(*line_p){
                line_list = g_list_append(line_list,
                                          g_strdup(*line_p));
                line_p++;
            }
            GList *toc_line_list_paged = NULL,
                  *unprocessed_line_list_paged = NULL;
            decompose_lines(line_list,
                            &toc_line_list_paged,
                            &unprocessed_line_list_paged);        
            g_list_free_full(line_list,
                             (GDestroyNotify)g_free);
            toc_line_list = g_list_concat(toc_line_list,
                                          toc_line_list_paged);     
            g_hash_table_insert(unprocessed_line_hash,
                                GINT_TO_POINTER(contents_page->meta->page_num),
                                unprocessed_line_list_paged);    
            contents_p = contents_p->next;
        }
        GList *aligned_line_list = NULL;
        align_unprocessed_lines(page_meta_list,
                                unprocessed_line_hash,
                                &aligned_line_list);
        GList *unprocessed_line_list = g_hash_table_get_values(unprocessed_line_hash);                                   
        GList *list_p = unprocessed_line_list;
        while(list_p){
            g_list_free_full(list_p->data,
                             (GDestroyNotify)g_free);
            list_p = list_p->next;
        }
        g_list_free(unprocessed_line_list);
        g_hash_table_unref(unprocessed_line_hash);
        GList *scavenged_toc_line_list = NULL;
        decompose_lines(aligned_line_list,
                        &scavenged_toc_line_list,
                        NULL);
        g_list_free_full(aligned_line_list,
                         (GDestroyNotify)g_free);
        toc_line_list = g_list_concat(toc_line_list,
                                      scavenged_toc_line_list);
        toc_line_list = g_list_sort(toc_line_list,
                                    compare_toc_lines);
        /* valid toc lines points to valid page labels */
        list_p = toc_line_list;
        while(list_p){
            TOCLine *toc_line = list_p->data;            
            int page_num = translate_page_label(page_label_num_hash,
                                                toc_line->page_label);
            GList *next = list_p->next;
            if(page_num < 0){                
                g_free(toc_line->label);
                g_free(toc_line->id);
                g_free(toc_line->caption);
                g_free(toc_line->page_label);
                g_free(toc_line);
                toc_line_list = g_list_remove_link(toc_line_list,
                                                   list_p);
                g_list_free(list_p);
            }
            list_p = next;
        }
        if(g_list_length(toc_line_list) >= total_lines_of_group * 0.5){
            final_toc_line_list = toc_line_list;
            break;
        }
        list_p = toc_line_list;
        while(list_p){
            TOCLine *toc_line = list_p->data;
            g_free(toc_line->label);
            g_free(toc_line->id);
            g_free(toc_line->caption);
            g_free(toc_line->page_label);
            g_free(toc_line);
            g_free(toc_line);
            list_p = list_p->next;
        }
        g_list_free(toc_line_list);
        group_p = group_p->prev;
    }
    group_p = contents_page_group_list;
    while(group_p){
        GList *contents_page_list = group_p->data;
        GList *contents_p = contents_page_list;
        while(contents_p){
            ContentsPage *contents_page = contents_p->data;
            g_strfreev(contents_page->lines);
            g_free(contents_page);
            contents_p = contents_p->next;
        }
        g_list_free(contents_page_list);
        group_p = group_p->next;
    }   
    g_list_free(contents_page_group_list);
    /* find distinct labels */
    GList *label_list = NULL; 
    GList *main_contents_start_p  = NULL,
          *finis_start_p = NULL;
    GList *list_p = final_toc_line_list;
    while(list_p){
        TOCLine *toc_line = list_p->data;
        if(toc_line->label && string_index_in_list(label_list,
                                                   toc_line->label,
                                                   TRUE) < 0)
        {
            label_list = g_list_append(label_list,
                                       toc_line->label);
        }
        if(!main_contents_start_p && (toc_line->label || toc_line->id)){
            main_contents_start_p = list_p;
        }
        if(!finis_start_p && main_contents_start_p && (!toc_line->label && !toc_line->id)){
            finis_start_p = list_p;
        }
        list_p = list_p->next;
    }    
    (*head_item) = toc_item_new();
    if(label_list){
        if(final_toc_line_list != main_contents_start_p){
            list_p = final_toc_line_list;
            while(list_p != main_contents_start_p){
                TOCLine *toc_line = list_p->data;
                TOCItem *child_item = toc_item_new();
                child_item->title = g_strdup(toc_line->caption);
                child_item->page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                                           toc_line->page_label));
                child_item->parent = *head_item;
                (*head_item)->children = g_list_append((*head_item)->children,
                                                       child_item);
                list_p = list_p->next;
            }
        }
        /* main contents */
        TOCItem *main_contents_item = toc_item_new(); 
        treeize_toc_list(page_label_num_hash,
                         label_list,
                         &main_contents_item,
                         main_contents_start_p,
                         NULL);
        TOCItem *main_contents_1st_child = main_contents_item->children->data;
        main_contents_item->title = g_strconcat(main_contents_1st_child->label,
                                                "s",
                                                NULL);
        main_contents_item->page_num = main_contents_1st_child->page_num;
        main_contents_item->parent = *head_item;
        (*head_item)->children = g_list_append((*head_item)->children,
                                               main_contents_item);
        if(finis_start_p){
            list_p = finis_start_p;
            while(list_p){
                TOCLine *toc_line = list_p->data;
                TOCItem *child_item = toc_item_new();
                child_item->title = g_strchomp(g_strdup(toc_line->caption));
                child_item->page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                                           toc_line->page_label));
                child_item->parent = *head_item;
                (*head_item)->children = g_list_append((*head_item)->children,
                                                       child_item);
                list_p = list_p->next;
            }            
        }
    }
    else{
        /* no labels were found so we would have a 'linear' toc */
        GList *list_p = final_toc_line_list;
        while(list_p){
            TOCLine *toc_line = list_p->data;
            TOCItem *child_item = toc_item_new();
            child_item->title = g_strdup(toc_line->match);
            child_item->page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                                       toc_line->page_label));
            child_item->parent = *head_item;
            (*head_item)->children = g_list_append((*head_item)->children,
                                                   child_item);
            list_p = list_p->next;
        }
        toc_fix_labels(*head_item,
                       NULL);
    }
    list_p = final_toc_line_list;
    while(list_p){
        TOCLine *toc_line = list_p->data;
        g_free(toc_line->match);
        g_free(toc_line->label);
        g_free(toc_line->id);
        g_free(toc_line->caption);
        g_free(toc_line->page_label);
        g_free(toc_line);
        list_p = list_p->next;
    }
    g_list_free(final_toc_line_list);
    (*head_item)->depth = 0;  
    toc_fix_depth(*head_item);
    toc_fix_sibling_links(*head_item);
    (*head_item)->length = poppler_document_get_n_pages(doc);
    toc_calc_length(*head_item);
    find_toc_target_position(page_meta_list,
                             *head_item);
}