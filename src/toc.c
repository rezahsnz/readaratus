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

#include "toc.h"
#include <glib.h>
#include <math.h>

static GRegex *toc_regex = NULL;

void
toc_module_init(void)
{
    char *pattern = 
        "^\n"
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
        "(I{1,3}|I?V|VI{1,3}|IX)|(XL|XC|L?X{1,3})(I{1,3}|I?V|VI{1,3}|IX)?\n"
        ")(?=(\\W*\\s|$))";
    GError *err = NULL;
    toc_regex = g_regex_new(pattern,
                            G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
                            G_REGEX_MATCH_NOTEMPTY,
                            &err);
    if(!toc_regex){
        g_print("toc_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        return; 
    }
}

void
toc_module_destroy(void)
{    
    g_regex_unref(toc_regex);
}

TOCItem *
toc_item_new(void)
{
    TOCItem *toc_item = g_malloc(sizeof(TOCItem));
    toc_item->title = NULL;
    toc_item->label = NULL;
    toc_item->id = NULL;
    toc_item->depth = -1;
    toc_item->page_num = -1;
    toc_item->length = 0;
    toc_item->rect = NULL;
    toc_item->parent = NULL;
    toc_item->next = NULL;
    toc_item->previous = NULL;
    toc_item->children = NULL;
    toc_item->offset_x = 0;
    toc_item->offset_y = 0;
    return toc_item;
}

void
toc_dump (TOCItem *toc_item)
{
	char *indent = g_strnfill(toc_item->depth * 4,
							  ' ');
	g_print("%s%-55s(%s %s), page_num: %-5d, length: %-5d, depth: %-5d\n",
			indent,
			toc_item->title,
            toc_item->label ? toc_item->label : "",
            toc_item->id ? toc_item->id : "",
			toc_item->page_num,
			toc_item->length,
			toc_item->depth);
	g_free(indent);
    GList *item_p = toc_item->children;
    while(item_p){  
		TOCItem *child = item_p->data;			
		toc_dump(child);
        item_p = item_p->next;
	}
}

void 
toc_calc_length(TOCItem *toc_item)
{
    GList *item_p = toc_item->children;
	while(item_p){		
		TOCItem *child = item_p->data;
		child->length = -child->page_num;		
		if(child->next){
			child->length += child->next->page_num;
		}
		else{
			TOCItem *parent = child->parent;
			TOCItem *head = NULL;
			while(parent && !parent->next){
				head = parent;
				parent = parent->parent;
			}
			child->length += parent ? parent->next->page_num
						  		 	: head->length;
		}
        if(child->length == 0){
            child->length = 1;
        }
		toc_calc_length(child);
        item_p = item_p->next;
	}
}

TOCItem 
toc_find_dest(PopplerDocument *doc,
		      PopplerDest *dest)
{
    PopplerDest *new_dest = NULL;
	switch(dest->type){
    	case POPPLER_DEST_NAMED:
		{
			new_dest = poppler_document_find_dest(doc,
											      dest->named_dest);
    		break;
		}
    	default:
			new_dest = poppler_dest_copy(dest);
    		break;
    }
    if(!new_dest){
        g_print("Warning: null named destination, link amy not work.\n");
    }
    TOCItem target;
    target.offset_x = new_dest ? (new_dest->change_left ? new_dest->left : 0) : 0;
	target.offset_y = new_dest ? (new_dest->change_top ? new_dest->top : 0) : 0;
	target.page_num = new_dest ? new_dest->page_num - 1 : -1;
	poppler_dest_free(new_dest);
	return target;
}

enum TOCType
toc_get_item_type (const char *label)
{
    if(!label){
        return None;
    }
    enum TOCType toc_type;
    if(g_regex_match_simple("^part\\b",
                            label,
                            G_REGEX_CASELESS,
                            0))
    {
        toc_type = Part;
    }
    else if(g_regex_match_simple("^ch(apter)?\\b",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Chapter;
    }
    else if(g_regex_match_simple("^sec(tion)?\\b",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Section;
    }
    else if(g_regex_match_simple("^subsec(tion)?\\b",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Subsection;
    }
    else{
        toc_type = None;
    }
    return toc_type;
}


int
string_index_in_list(GList      *list,
                     const char *needle,
                     gboolean    is_case_sensitive)
{
    if(!needle || !list){
        return -1;
    }
    char *escaped_needle = g_regex_escape_string(needle,
                                                 -1);
    char *pattern_needle = g_strdup_printf("^%s$",
                                           escaped_needle); 
    g_free(escaped_needle);
    GList *list_p = list;
    while(list_p){
        if(g_regex_match_simple(pattern_needle,
                                list_p->data,
                                is_case_sensitive ? 0 : G_REGEX_CASELESS,
                                0))
        {
            break;
        }

        list_p = list_p->next;
    }
    g_free(pattern_needle);
    int index = list_p ? g_list_position(list,
                                         list_p) 
                       : -1;
    return index;
}

char *
toc_infer_child_label(const char *label_parent)
{
    if(!label_parent){
        return NULL;
    }
    if(g_regex_match_simple("^part$",
                            label_parent,
                            G_REGEX_CASELESS,
                            0))
    {
        return "Chapter";
    }
    else if(g_regex_match_simple("^ch(apter)?$",
                                 label_parent,
                                 G_REGEX_CASELESS,
                                 0))
    {
        return "Section";
    }
    else if(g_regex_match_simple("^sec(tion)?$",
                                 label_parent,
                                 G_REGEX_CASELESS,
                                 0))
    {
        return "Subsection";
    }
    else if(g_regex_match_simple("^subsec(tion)?$",
                                 label_parent,
                                 G_REGEX_CASELESS,
                                 0))
    {
        return "Chapter";
    }
    else{
        return NULL;
    }
}

void
toc_fix_depth(TOCItem *toc_item)
{
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;
        child_item->depth = child_item->parent->depth + 1;
        toc_fix_depth(child_item);
        item_p = item_p->next;
    }
}

void
toc_fix_sibling_links(TOCItem *toc_item)
{
    TOCItem *previous_toc_item = NULL;
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;
        child_item->previous = previous_toc_item;
        if(previous_toc_item){
            previous_toc_item->next = child_item;
        }
        child_item->next = NULL;
        previous_toc_item = child_item;
        toc_fix_sibling_links(child_item);
        item_p = item_p->next;
    }
}

void 
toc_fix_labels(TOCItem    *toc_item,
               const char *label_parent,
               gboolean   *has_labels)
{
    if(!toc_item){
        return;
    }
    if(toc_item->title){
        GMatchInfo *match_info = NULL;
        g_regex_match(toc_regex,
                      toc_item->title,
                      0,
                      &match_info);
        if(g_match_info_matches(match_info)){
            toc_item->id = g_match_info_fetch_named(match_info,
                                                    "id");
            char *label = g_match_info_fetch_named(match_info,
                                                   "label");
            if(strlen(label) > 0){
                toc_item->label = label;
            }
            else{
                char *inferred_label = g_strdup(toc_infer_child_label(label_parent));
                toc_item->label = inferred_label ? inferred_label : g_strdup("Chapter");
            }
            if(has_labels){
                *has_labels = TRUE;
            }
        }
        else{
            toc_item->label = g_strdup(toc_infer_child_label(label_parent));
        }
        g_match_info_free(match_info);
    }
    GList *list_p = toc_item->children;
    while(list_p){
        TOCItem *toc_item_child = list_p->data;
        toc_fix_labels(toc_item_child,
                       toc_item->label,
                       has_labels);
        list_p = list_p->next;
    }
}

const TOCItem *
toc_search_by_title(const TOCItem *toc_item,
                    const char    *title)
{   
    char *escaped_title = g_regex_escape_string(toc_item->title,
                                                -1);
    char *pattern = g_strdup_printf("^%s$",
                                    escaped_title); 
    g_free(escaped_title);

    if(g_regex_match_simple(pattern,
                            title,
                            G_REGEX_CASELESS,
                            0))
    {
        g_free(pattern);
        return toc_item;
    }
    g_free(pattern);
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;
        const TOCItem *target_item = toc_search_by_title(child_item,
                                                         title);
        if(target_item){
            return target_item;
        }
        item_p = item_p->next;
    }
    return NULL;
}

static void
walk_poppler_index(PopplerDocument  *doc,
                   PopplerIndexIter *iter,
                   TOCItem          *toc_item_parent)
{    
    do{
        TOCItem *toc_item_child = toc_item_new();               
        PopplerAction *action = poppler_index_iter_get_action(iter);
        switch(action->type){
            case POPPLER_ACTION_GOTO_DEST:          
            {
                PopplerActionGotoDest *dest_action = (PopplerActionGotoDest*) action;
                TOCItem target = toc_find_dest(doc,
                                               dest_action->dest);
                toc_item_child->offset_x = target.offset_x;
                toc_item_child->offset_y = target.offset_y;
                toc_item_child->page_num = target.page_num;
                toc_item_child->title = g_strdup(dest_action->title);
                break;
            }  
            default:
            {
                PopplerActionAny *any_action = (PopplerActionAny*) action;
                g_print("unexpected action item: %s\n",
                        any_action->title);             
            }
        }
        poppler_action_free(action);
        toc_item_child->parent = toc_item_parent;       
        toc_item_parent->children = g_list_append(toc_item_parent->children,
                                                  toc_item_child);
        PopplerIndexIter *child_iter = poppler_index_iter_get_child(iter);
        if(child_iter){         
            walk_poppler_index(doc,
                               child_iter,
                               toc_item_child);
            poppler_index_iter_free(child_iter);
        }
    }while(poppler_index_iter_next(iter));
}

void
toc_fix_labels_blindly(TOCItem    *toc_item,
                       const char *label_parent)
{
    /* 
      for tocs that do not provide labels or ids, we will generate labels
      blindly:
          depth 1: chapter
              depth 2: section
                  depth 3: subsection        
    */
    if(!toc_item){
        return;
    }
    if(toc_item->title){
        /* d1 chapters are more probable */
        toc_item->label = label_parent ? g_strdup(toc_infer_child_label(label_parent))
                                       : g_strdup("Part"); 
    }
    GList *list_p = toc_item->children;
    while(list_p){
        toc_fix_labels_blindly(list_p->data,
                               toc_item->label);
        list_p = list_p->next;
    }
}

void
toc_create_from_poppler_index(PopplerDocument *doc,
                              TOCItem        **head_item)
{
    PopplerIndexIter *iter = poppler_index_iter_new(doc);
    if(!iter){
        *head_item = NULL;
        return;
    }
    *head_item = toc_item_new();
    walk_poppler_index(doc,
                       iter,
                       *head_item);
    poppler_index_iter_free(iter);
    gboolean has_labels = FALSE;
    toc_fix_labels(*head_item,
                   NULL,
                   &has_labels);
    if(!has_labels){
        /* @ try to infer labels by text analysis */
        toc_fix_labels_blindly(*head_item,
                               NULL);
        g_print("TOC is blindly labeled, bear with it.\n");
    }
    (*head_item)->depth = 0;    
    toc_fix_depth(*head_item);
    toc_fix_sibling_links(*head_item);
    (*head_item)->length = poppler_document_get_n_pages(doc); 
    toc_calc_length(*head_item);
}

int
translate_page_label(GHashTable *page_label_num_hash,
                     const char *label)
{
    int page_num = -1;
    if(g_hash_table_contains(page_label_num_hash,
                             label))
    {
        page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                       label));
    }
    else{
        GList *key_list = g_hash_table_get_keys(page_label_num_hash);
        GList *list_p = key_list;
        while(list_p){
            char *key = list_p->data;
            char *escaped = g_regex_escape_string(key,
                                                  -1);
            char *pattern = g_strdup_printf("^%s$",
                                            escaped);
            g_free(escaped);
            if(g_regex_match_simple(pattern,
                                    label,
                                    G_REGEX_CASELESS, 0))
            {
                g_free(pattern);
                page_num = GPOINTER_TO_INT(g_hash_table_lookup(page_label_num_hash,
                                                               key));
                break;
            }
            g_free(pattern);
            list_p = list_p->next;
        }
        g_list_free(key_list);
    }
    return page_num;
}

static void 
toc_item_free_recursive(TOCItem *toc_item)
{
    if(!toc_item){
        return;
    }
    GList *list_p = toc_item->children;
    while(list_p){
        toc_item_free_recursive(list_p->data);
        list_p = list_p->next;
    }
    g_list_free(toc_item->children);
    g_free(toc_item->title);
    g_free(toc_item->label);
    g_free(toc_item->id);
    rect_free(toc_item->rect);
    g_free(toc_item);
}

void
toc_destroy(TOCItem *head_item)
{
    if(!head_item){
        return;
    }
    toc_item_free_recursive(head_item);
}

void
toc_flatten(TOCItem *toc_item,
            GList  **flattened_item_list)
{
    if(!toc_item){
        return;
    }    
    *flattened_item_list = g_list_append(*flattened_item_list,
                                         toc_item);
    GList *child_p = toc_item->children;
    while(child_p){
        toc_flatten(child_p->data,
                    flattened_item_list);
        child_p = child_p->next;
    }
}

void
toc_where_am_i(int      page_num,
               TOCItem *toc_item,
               GList  **where)
{
    if(!toc_item){
        return;
    }
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;             
        if(page_num >= child_item->page_num &&
           page_num < child_item->page_num + child_item->length)
        {
            *where = g_list_append(*where,
                                   child_item);
            toc_where_am_i(page_num,
                           child_item,
                           where);
            break;
        }
        item_p = item_p->next;
    }    
}
