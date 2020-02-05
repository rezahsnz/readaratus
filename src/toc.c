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
#include <gmodule.h>
#include <glib.h>

static GRegex *toc_regex = NULL;
static GRegex *toc_extraction_regex = NULL;
static GRegex *toc_decompose_regex = NULL;
static GHashTable *toc_ids = NULL;

void
toc_module_init(void)
{
    char *pattern = 
        "^\\s?\n"
        "# item label: part, chapter, ...\n"
        "(?<label>part|ch(apter)?|(sub)?sec(tion)?)?\\s?\n"
        "(?<id>\n"
        "# digits first: 1, 1.2, 1.a, ...\n"
        "[0-9]+((\\.|-|_)([0-9]+|[a-zA-Z]))*|\n"
        "# @alpha first: a, a.1, a.a\n"
        "# named numbers up to 99\n"
        "zero|one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|\n"
        "(thir|four|fif|six|seven|eigh|nin)-?teen|\n"
        "(twenty|thirty|fourty|fifty|sixty|seventy|eighty|ninty)(-|\\s)?\n"
        "(one|two|three|four|five|six|seven|eight|nine)?|\n"
        "# roman numerals up to 99\n"
        "(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\n"
        "xxi{1,3}|xxi?v|xxvi{1,3}|xxi?x|\n"
        "xxxi{1,3}|xxxi?v|xxxvi{1,3}|xxxix|\n"
        "xli{0,3}|xli?v|xlvi{1,3}|xlix|\n"
        "li{0,3}|li?v|lvi{1,3}|li?x|\n"
        "lxi{1,3}|lxi?v|lxvi{1,3}|lxi?x|\n"
        "lxxi{1,3}|lxxi?v|lxxvi{1,3}|lxxi?x|\n"
        "lxxxi{1,3}|lxxxi?v|lxxxvi{1,3}|lxxxix|\n"
        "xci{0,3}|xci?v|xcvi{1,3}|xcix)\\b\n"
        ")";
    GError *err = NULL;
    toc_regex = g_regex_new(pattern,
                            G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
                            0,
                            &err);
    if(!toc_regex){
        g_print("toc_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        return; 
    }
    char *extraction_pattern = 
        "^\\s?\n"
        "# item label: part, chapter, ...\n"
        "(?<label>part|ch(apter)?|(sub)?sec(tion)?)?\\s?\n"
        "(?<id>\n"
        "# digits first: 1, 1.2, 1.a, ...\n"
        "[0-9]+((\\.|-|_)([0-9]+|[a-zA-Z]))*|\n"
        "# @alpha first: a, a.1, a.a\n"
        "# named numbers up to 99\n"
        "zero|one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|\n"
        "(thir|four|fif|six|seven|eigh|nin)-?teen|\n"
        "(twenty|thirty|fourty|fifty|sixty|seventy|eighty|ninty)(-|\\s)?\n"
        "(one|two|three|four|five|six|seven|eight|nine)?|\n"
        "# roman numerals up to 99\n"
        "(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\n"
        "xxi{1,3}|xxi?v|xxvi{1,3}|xxi?x|\n"
        "xxxi{1,3}|xxxi?v|xxxvi{1,3}|xxxix|\n"
        "xli{0,3}|xli?v|xlvi{1,3}|xlix|\n"
        "li{0,3}|li?v|lvi{1,3}|li?x|\n"
        "lxi{1,3}|lxi?v|lxvi{1,3}|lxi?x|\n"
        "lxxi{1,3}|lxxi?v|lxxvi{1,3}|lxxi?x|\n"
        "lxxxi{1,3}|lxxxi?v|lxxxvi{1,3}|lxxxix|\n"
        "xci{0,3}|xci?v|xcvi{1,3}|xcix)\n"
        ")";
    toc_extraction_regex = g_regex_new(extraction_pattern,        
                                       G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
                                       0,
                                       &err);
    if(!toc_extraction_regex){
        g_print("toc_extraction_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        return; 
    }
    char *decompose_pattern = 
        "^\\s*\n"
        "# item label: part, chapter, ...\n"
        "(?<label>part|ch(apter)?|(sub)?sec(tion)?)?\\b\\s*\n"
        "(?<id>\n"
        "# digits first: 1, 1.2, 1.a, ...\n"
        "[0-9]+((\\.|-|_)([0-9]+|[a-zA-Z]))*|\n"
        "# @alpha first: a, a.1, a.a\n"
        "# named numbers up to 99\n"
        "zero|one|two|three|four|five|six|seven|eight|nine|ten|eleven|twelve|\n"
        "(thir|four|fif|six|seven|eigh|nin)-?teen|\n"
        "(twenty|thirty|fourty|fifty|sixty|seventy|eighty|ninty)(-|\\s)?\n"
        "(one|two|three|four|five|six|seven|eight|nine)?|\n"
        "# roman numerals up to 99\n"
        "(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\n"
        "xxi{1,3}|xxi?v|xxvi{1,3}|xxi?x|\n"
        "xxxi{1,3}|xxxi?v|xxxvi{1,3}|xxxix|\n"
        "xli{0,3}|xli?v|xlvi{1,3}|xlix|\n"
        "li{0,3}|li?v|lvi{1,3}|li?x|\n"
        "lxi{1,3}|lxi?v|lxvi{1,3}|lxi?x|\n"
        "lxxi{1,3}|lxxi?v|lxxvi{1,3}|lxxi?x|\n"
        "lxxxi{1,3}|lxxxi?v|lxxxvi{1,3}|lxxxix|\n"
        "xci{0,3}|xci?v|xcvi{1,3}|xcix))?\\b\\s*\n"
        "(?<caption>.+\\b(?=(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\\d+)$))\n"
        "(?<page_num>(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\\d+))$";
    err = NULL;
    toc_decompose_regex = g_regex_new(decompose_pattern,
                            G_REGEX_CASELESS | G_REGEX_EXTENDED | G_REGEX_NO_AUTO_CAPTURE,
                            0,
                            &err);
    if(!toc_decompose_regex){
        g_print("toc_decompose_regex error.\ndomain: %d, \ncode: %d, \nmessage: %s\n",
                err->domain, err->code, err->message);
        return; 
    }
    toc_ids = g_hash_table_new(g_str_hash,
                               g_str_equal);
}

void
toc_module_destroy(void)
{    
    g_regex_unref(toc_regex);
    g_regex_unref(toc_extraction_regex);
    if(toc_ids){
        g_hash_table_unref(toc_ids);
    }
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

static void
walk_toc(PopplerDocument  *doc,
		 PopplerIndexIter *iter,
		 TOCItem          *toc_item,
		 int              *depth)
{
    TOCItem *previous_toc_item = NULL;
	do{
		TOCItem *new_toc_item = toc_item_new();				
		PopplerAction *action = poppler_index_iter_get_action(iter);
		switch(action->type){
			case POPPLER_ACTION_GOTO_DEST:			
	        {
	            PopplerActionGotoDest *dest_action = (PopplerActionGotoDest*) action;
                TOCItem target = toc_find_dest(doc,
                						       dest_action->dest);
                new_toc_item->offset_x = target.offset_x;
        		new_toc_item->offset_y = target.offset_y;
        		new_toc_item->page_num = target.page_num;
        		new_toc_item->title = g_strdup(dest_action->title);
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
		new_toc_item->depth = toc_item->depth + 1;
		if(new_toc_item->depth > *depth){
			*depth = new_toc_item->depth;
		}
        GMatchInfo *match_info = NULL;
        g_regex_match(toc_regex,
                      new_toc_item->title,
                      0,
                      &match_info);
        if(g_match_info_matches(match_info)){
            new_toc_item->id = g_match_info_fetch_named(match_info,
                                                        "id");
            char *label = g_match_info_fetch_named(match_info,
                                                   "label");
            if(strlen(label) > 0){
                new_toc_item->label = label;
            }
            else{
                g_free(label);
            }
        }
        g_match_info_free(match_info);
		new_toc_item->parent = toc_item;
        new_toc_item->previous = previous_toc_item;
        if(previous_toc_item){
            previous_toc_item->next = new_toc_item;
        }		
		toc_item->children = g_list_append(toc_item->children,
                                           new_toc_item);
		PopplerIndexIter *child_iter = poppler_index_iter_get_child(iter);
		if(child_iter){			
			walk_toc(doc,
					 child_iter,
					 new_toc_item,
					 depth);
			poppler_index_iter_free(child_iter);
		}
        previous_toc_item = new_toc_item;
	}while(poppler_index_iter_next(iter));
}

void
toc_dump (TOCItem *toc_item)
{
	char *indent = g_strnfill(toc_item->depth * 4,
							  ' ');
	g_print("%s%-40s(%s), page_num: %-5d, length: %-5d, depth: %-5d\n",
			indent,
			toc_item->title,
            toc_item->label ? toc_item->label : "",
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

static void 
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

/* @ heavy revisal */
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
    TOCItem target;
    target.offset_x = new_dest->change_left ? new_dest->left : 0;
	target.offset_y = new_dest->change_top ? new_dest->top : 0;
	target.page_num = new_dest->page_num - 1;
	poppler_dest_free(new_dest);
	return target;
}


static void
detect_toc_type(GList        *titles,
                enum TOCType *toc_type,
                char        **label)
{	
    *label = NULL;
	GList *matched_labels = NULL;
    GList *l1_titles_p = titles;
	while(l1_titles_p){
		GMatchInfo *match_info = NULL;
		g_regex_match(toc_regex,
					  l1_titles_p->data,
					  0,
					  &match_info);
		if(g_match_info_matches(match_info)){
			char *l = g_match_info_fetch_named(match_info,
											   "label");
            if(strlen(l) > 0){
			    matched_labels = g_list_append(matched_labels,
                                               l);
            }
            else{
                g_free(l);
            }
		}
		g_match_info_free(match_info);
        l1_titles_p = l1_titles_p->next;
	}
	if(!matched_labels){
        *toc_type = Chapter;        		
		return;
	}
	/* @ the most frequent label? */
	*label = g_strdup(matched_labels->data);
    *toc_type = toc_get_item_type(*label);
    if(*toc_type == None){
		g_print("unexpected toc label: %s\n",
				*label);        
    }
    GList *match_p = matched_labels;
    while(match_p){
        g_free((char*)match_p->data);
        match_p = match_p->next;
    }
	g_list_free(matched_labels);
}

/* 
  rearrange toc as follows:
  3 top level items: Initium, <content>, Finis
  Initium: anything before the main content, introduction, list of figures, ...
  content: parts, chapters, sections, ... which form the body of the book
  Finis:   anything after the main content: references, index, bibliography, appendices, ...
*/
static void
rearrange_toc (TOCItem *head_item,
               enum TOCType    l1_toc_type,  
               char           *main_contents_label)
{
	/* find main contents boundry */
	char *pattern;
	if(main_contents_label){
		pattern = g_strdup_printf("^\\s?%s",
							   	  main_contents_label);
	}
	else{
		pattern = g_strdup(".");
	}
	GList *start_main_content_p = head_item->children;
	while(start_main_content_p){
		TOCItem *toc_item = start_main_content_p->data;
		if(g_regex_match_simple(pattern,
			  					toc_item->title,
								G_REGEX_CASELESS,
								0))
        {
			break;
		}
        start_main_content_p = start_main_content_p->next;
	}
    GList *end_main_content_p = start_main_content_p ? start_main_content_p->next : NULL;
	while(end_main_content_p){
		TOCItem *toc_item = end_main_content_p->data;
		if(!g_regex_match_simple(pattern,
								 toc_item->title,
								 G_REGEX_CASELESS,
								 0))
        {
			end_main_content_p = end_main_content_p->prev;
			break;
		}
        end_main_content_p = end_main_content_p->next;
	}
	g_free(pattern);
	/* initium */
	TOCItem *initium_item = NULL;
    if(start_main_content_p != head_item->children){
    	TOCItem *last_initium_child_item = start_main_content_p->prev->data;
        last_initium_child_item->next = NULL;
        initium_item = toc_item_new();
    	initium_item->title = g_strdup("Initium");
    	initium_item->depth = head_item->depth + 1;
    	initium_item->parent = head_item;
        GList *item_p = head_item->children;
        while(item_p != start_main_content_p){
    		TOCItem *toc_item = item_p->data;
            if(item_p == head_item->children){
                initium_item->page_num = toc_item->page_num;
            } 
    		toc_item->parent = initium_item;    		
    		initium_item->length += toc_item->length;		
    		initium_item->children = g_list_append(initium_item->children,
                                                   toc_item);
            item_p = item_p->next;
    	}
    }
	/* main contents */
	TOCItem *main_contents_item = toc_item_new();
    switch(l1_toc_type){
        case Subsection:
            main_contents_item->title = g_strdup("Subsections");
            break;

        case Section:
            main_contents_item->title = g_strdup("Sections");
            break;

        case Chapter:
            main_contents_item->title = g_strdup("Chapters");
            break;

        case Part:
            main_contents_item->title = g_strdup("Parts");
            break;

        default:;
    }
    main_contents_item->label = g_strdup("main_contents");
	main_contents_item->depth = head_item->depth + 1;	
	main_contents_item->parent = head_item;
	GList *item_p = start_main_content_p;
	while(item_p){
		TOCItem *toc_item = item_p->data;
        if(item_p == start_main_content_p){
            toc_item->previous = NULL;
            main_contents_item->page_num = toc_item->page_num;
        } 
		toc_item->parent = main_contents_item;
		main_contents_item->length += toc_item->length;
		main_contents_item->children = g_list_append(main_contents_item->children,
                                                     toc_item);
        if(item_p == end_main_content_p){
            toc_item->next = NULL;
            break;
        }
        item_p = item_p->next;
	}
	/* finis */
	TOCItem *finis_item = NULL;
    if(end_main_content_p){
        finis_item = toc_item_new();
    	finis_item->title = g_strdup("Finis");
    	finis_item->depth = 1;
    	finis_item->parent = head_item;
        item_p = end_main_content_p->next;    	
    	while(item_p){
    		TOCItem *toc_item = item_p->data;
            if(item_p == end_main_content_p->next){
                toc_item->previous = NULL;
                finis_item->page_num = toc_item->page_num;
            }
    		toc_item->parent = finis_item;
    		finis_item->length += toc_item->length;
    		finis_item->children = g_list_append(finis_item->children,
                                                 toc_item);
            item_p = item_p->next;
    	}    	
    }
    g_list_free(head_item->children);
    head_item->children = NULL;    
    if(initium_item){
	   initium_item->next = main_contents_item;
       head_item->children = g_list_append(head_item->children,
                                           initium_item);
    }
    main_contents_item->previous = initium_item;
	main_contents_item->next = finis_item;
    head_item->children = g_list_append(head_item->children,
                                        main_contents_item);
    if(finis_item){
        finis_item->previous = main_contents_item;
        head_item->children = g_list_append(head_item->children,
                                            finis_item);
    }	
}

static const TOCItem *
get_main_contents_item(const TOCItem *head_item)
{
    TOCItem *main_contents_item = NULL;
    GList *item_p = head_item->children;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        if(toc_item->label){
            main_contents_item = toc_item;
            break;
        }
        item_p = item_p->next;
    }
    return main_contents_item;
}

static void
fix_depth (TOCItem *toc_item)
{
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;
        child_item->depth = child_item->parent->depth + 1;
        fix_depth(child_item);
        item_p = item_p->next;
    }
}

static void
fix_labels(const TOCItem *toc_item,
           char          *parent_label)
{
    GList *item_p = toc_item->children;
    while(item_p){
        TOCItem *child_item = item_p->data;
        char *child_label = NULL;
        switch(toc_get_item_type(parent_label)){
            case Part:
                child_label = child_item->depth > 2 ? "Chapter" : parent_label; 
                break;
            case Chapter:                
                child_label = child_item->depth > 2 ? "Section" : parent_label;                                                                        
                break;
            case Section:                
                child_label = child_item->depth > 2 ? "Subsection" : parent_label;
                break;
            case Subsection:
                child_label = child_item->depth > 2 ? "Chapter" : parent_label;
                break;
            default:;
        }
        child_item->label = g_strdup(child_label);
        fix_labels(child_item,
                   child_label);
        item_p = item_p->next;
    }
}

enum TOCType
toc_get_item_type (const char *label)
{
    if(!label){
        return Chapter;
    }
    enum TOCType toc_type;
    if(g_regex_match_simple("^\\s?part",
                            label,
                            G_REGEX_CASELESS,
                            0))
    {
        toc_type = Part;
    }
    else if(g_regex_match_simple("^\\s?ch(apter)?",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Chapter;
    }
    else if(g_regex_match_simple("^\\s?sec(tion)?",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Section;
    }
    else if(g_regex_match_simple("^\\s?subsec(tion)?",
                                 label,
                                 G_REGEX_CASELESS,
                                 0))
    {
        toc_type = Subsection;
    }
    else
    {
        toc_type = None;
    }
    return toc_type;
}

GList *
toc_extract_items(const char *text)
{
    GList *items = NULL;
    GMatchInfo *match_info = NULL;
    g_regex_match(toc_extraction_regex,
                  text,
                  0,
                  &match_info);
    while(g_match_info_matches(match_info)){
        TOCItem *toc_item = toc_item_new();
        toc_item->label = g_match_info_fetch_named(match_info,
                                                   "label");
        toc_item->id = g_match_info_fetch_named(match_info,
                                                "id");
        items = g_list_append(items,
                              toc_item);
        g_match_info_next(match_info,
                          NULL);
    }
    g_match_info_free(match_info);
    return items;
}


const TOCItem *
toc_search_by_id(const TOCItem *head_item,
                 const char *label,
                 const char *id)
{
    if(g_hash_table_contains(toc_ids,
                             id))
    {
        return g_hash_table_lookup(toc_ids,
                                   id);
    }
    char *escaped_title = g_regex_escape_string(id,
                                                -1);
    char *pattern = g_strdup_printf("^%s$",
                                    escaped_title); 
    g_free(escaped_title);
    TOCItem *toc_item = NULL;
    GHashTableIter iter;
    gpointer key, value;
    g_hash_table_iter_init (&iter, toc_ids);
    while (g_hash_table_iter_next (&iter, &key, &value)){
        if(g_regex_match_simple(pattern,
                                key,
                                G_REGEX_CASELESS,
                                0))
        {
            toc_item = value;
            break;
        }
    }
    g_free(pattern);
    return toc_item;
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
toc_create_hashes(TOCItem *toc_item)
{
    if(toc_item->id){
        g_hash_table_insert(toc_ids,
                            toc_item->id,
                            toc_item);
    }    
    GList *item_p = toc_item->children;
    while(item_p){
        toc_create_hashes(item_p->data);
        item_p = item_p->next; 
    }
}

void
toc_create_from_poppler_index(PopplerDocument *doc,
                              TOCItem        **head_item,
                              int             *max_toc_depth)
{
    PopplerIndexIter *iter = poppler_index_iter_new(doc);
    if(!iter){
        *head_item = NULL;
        *max_toc_depth = 0;
        return;
    }
    *head_item = toc_item_new();
    (*head_item)->depth = 0;    
    (*head_item)->length = poppler_document_get_n_pages(doc); 
    walk_toc(doc,
             iter,
             *head_item,
             max_toc_depth);
    poppler_index_iter_free(iter);  
    
    toc_calc_length(*head_item);
    GList *l1_titles = NULL;
    GList *item_p = (*head_item)->children;
    while(item_p){
        TOCItem *toc_item = item_p->data;
        l1_titles = g_list_append(l1_titles,
                                  toc_item->title);
        item_p = item_p->next;
    }
    enum TOCType l1_toc_type;
    char *l1_label;
    detect_toc_type(l1_titles,
                    &l1_toc_type,
                    &l1_label);
    g_list_free(l1_titles);
    rearrange_toc(*head_item,
                  l1_toc_type,
                  l1_label);
    fix_depth(*head_item);

    const TOCItem *main_contents_item = get_main_contents_item(*head_item);
    char *main_contetns_label = g_strndup(main_contents_item->title,
                                          strlen(main_contents_item->title) - 1);
    fix_labels(main_contents_item,
               main_contetns_label);
    (*max_toc_depth)++;    
    toc_create_hashes(*head_item);
    g_free(l1_label);
    g_free(main_contetns_label);
}

void
toc_create_from_contents_pages(GList    *page_texts,
                               TOCItem **head_item,
                               int      *max_toc_depth)
{
    const char *TOC_LINE_PATTERN = 
        "\\b(i{1,3}|i?v|vi{1,3}|i?x|xi{1,3}|xiv|xvi{1,3}|xi?x|\\d+)$";
    GList *contents_pages = NULL;
    int contents_page_num = -1;
    int page_num = 0;
    GList *text_p = page_texts;
    while(text_p){
        char *text = text_p->data;
        char **lines = g_regex_split_simple("\\R",
                                            text,
                                            0, 0);
        int num_lines = g_strv_length(lines);
        int num_matched_lines = 0;
        char **line_p = lines;
        while(*line_p){
            if(g_regex_match_simple(TOC_LINE_PATTERN,
                                    *line_p,
                                    0, 0))
            {
                num_matched_lines++;
            }
            line_p++;
        }
        g_strfreev(lines);
        if(((float)num_matched_lines / num_lines) >= 0.7){
            contents_pages = g_list_append(contents_pages,
                                           text);
            if(contents_page_num < 0){
                contents_page_num = page_num;
            }
        }
        page_num++;
        text_p = text_p->next;
    }
    if(contents_page_num < 0){
        return;
    }
    g_print("contents' pages: %d - %d\n",
            contents_page_num, contents_page_num + g_list_length(contents_pages) - 1);
    text_p = contents_pages;
    while(text_p){
        char *text = text_p->data;
        char **lines = g_regex_split_simple("\\R",
                                            text,
                                            0, 0);
        int num_lines = g_strv_length(lines);
        char **line_p = lines;
        while(*line_p){
            GMatchInfo *match_info = NULL;
            g_regex_match(toc_decompose_regex,
                          *line_p,
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
                char *caption = g_match_info_fetch_named(match_info,
                                                         "caption");
                char *page_num = g_match_info_fetch_named(match_info,
                                                          "page_num");
                g_print("label: '%s', id: '%s', caption: '%s', page_num: '%s'\n",
                        label ? label : "N/A", id ? id : "N/A", caption, page_num);
            }
            g_match_info_free(match_info);
            line_p++;
        }
        g_strfreev(lines);        
        text_p = text_p->next;
    }
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
    g_hash_table_remove_all(toc_ids);
}

void
toc_flatten_items(TOCItem *toc_item,
                  GList  **flattened_item_list)
{
    if(!toc_item){
        return;
    }    
    *flattened_item_list = g_list_append(*flattened_item_list,
                                         toc_item);
    GList *child_p = toc_item->children;
    while(child_p){
        toc_flatten_items(child_p->data,
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

static gint
compare_strings(gconstpointer a,
                gconstpointer b)
{
    return g_ascii_strcasecmp((const char *)a,
                              (const char *)b);
}

void
toc_get_labels(const TOCItem *toc_item,
               GList        **labels)
{    
    if(!toc_item){
        return;
    }
    if(toc_item->label && 
       toc_item->depth >= 2 &&
       !g_list_find_custom(*labels,
                           toc_item->label,
                           compare_strings))
    {
        *labels = g_list_append(*labels,
                               g_strdup(toc_item->label));
    }
    GList *item_p = toc_item->children;
    while(item_p){
        toc_get_labels(item_p->data,
                       labels);
        item_p = item_p->next;
    }

}
