#ifndef TOC_SYNTHESIS
#define TOC_SYNTHESIS

#include <glib.h>
#include <poppler/glib/poppler.h>
#include "toc.h"

int
string_index_in_list(GList      *list,
                     const char *needle,
                     gboolean    is_case_sensitive);

void
toc_create_from_contents_pages(PopplerDocument *doc,
                               GPtrArray       *page_meta_list,
                               GHashTable      *page_label_num_hash,
                               TOCItem        **head_item);

#endif