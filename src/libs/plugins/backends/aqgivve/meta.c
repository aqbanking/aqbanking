/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#include "meta_p.h"
#include <stdlib.h>
#include <aqbanking/banking.h>
#include "provider.h"



AG_META *AG_META_new()
{
  AG_META *m = (AG_META *)malloc(sizeof(AG_META));
  m->current_page = -1;
  m->page_size = -1;
  m->total_entries = -1;
  m->total_pages = -1;

  return m;
}



void AG_META_free(AG_META *meta)
{
  free(meta);
}



int AG_META_GetCurrentPage(AG_META *meta)
{
  if (meta) {
    return meta->current_page;
  }
  else {
    return -1;
  }
}



void AG_META_SetCurrentPage(AG_META *meta, int cp)
{
  if (meta) {
    meta->page_size = cp;
  }
}



int AG_META_GetPageSize(AG_META *meta)
{
  if (meta) {
    return meta->page_size;
  }
  else {
    return -1;
  }
}



void AG_META_SetPageSize(AG_META *meta, int ps)
{
  if (meta) {
    meta->page_size = ps;
  }
}



int AG_META_GetTotalEntries(AG_META *meta)
{
  if (meta) {
    return meta->total_entries;
  }
  else {
    return -1;
  }
}



void AG_META_SetTotalEntries(AG_META *meta, int te)
{
  if (meta) {
    meta->total_entries = te;
  }
}



int AG_META_GetTotalPages(AG_META *meta)
{
  if (meta) {
    return meta->total_pages;
  }
  else {
    return -1;
  }
}



void AG_META_SetTotalPages(AG_META *meta, int tp)
{
  if (meta) {
    meta->page_size = tp;
  }
}



AG_META *AG_META_FromJsonElem(GWEN_JSON_ELEM *meta_elem)
{
  AG_META *m = NULL;

  if (!meta_elem) {
    return NULL;
  }

  int type = GWEN_JsonElement_GetType(meta_elem);
  const char *val = GWEN_JsonElement_GetData(meta_elem);


  if ((type == GWEN_JSON_ELEMTYPE_STRING) && (strcmp(val, "null") == 0)) {
    return NULL;
  }

  if (type == GWEN_JSON_ELEMTYPE_OBJECT) {

    m = AG_META_new();

    GWEN_JSON_ELEM *json_total_entries = GWEN_JsonElement_GetElementByPath(meta_elem, "total_entries", 0);
    if (json_total_entries) {
      AG_META_SetTotalEntries(m, atoi(GWEN_JsonElement_GetData(json_total_entries)));
    }

    GWEN_JSON_ELEM *json_current_page = GWEN_JsonElement_GetElementByPath(meta_elem, "current_page", 0);
    if (json_current_page) {
      AG_META_SetCurrentPage(m, atoi(GWEN_JsonElement_GetData(json_current_page)));
    }

    GWEN_JSON_ELEM *json_page_size = GWEN_JsonElement_GetElementByPath(meta_elem, "page_size", 0);
    if (json_page_size) {
      AG_META_SetPageSize(m, atoi(GWEN_JsonElement_GetData(json_page_size)));
    }

    GWEN_JSON_ELEM *json_total_pages = GWEN_JsonElement_GetElementByPath(meta_elem, "total_pages", 0);
    if (json_total_pages) {
      AG_META_SetTotalPages(m, atoi(GWEN_JsonElement_GetData(json_total_pages)));
    }

  }

  return m;
}



