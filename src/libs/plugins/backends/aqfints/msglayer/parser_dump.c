/***************************************************************************
 begin       : Fri Jun 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_normalize.h"




void AQFINTS_Parser_DumpElementTree(AQFINTS_ELEMENT *element, int indent)
{

  int i;
  const char *s;
  AQFINTS_ELEMENT *childElement;

  for (i=0; i<indent; i++)
    fprintf(stderr, " ");

  switch(AQFINTS_Element_GetElementType(element)) {
  case AQFINTS_ElementType_Group: fprintf(stderr, "GROUP"); break;
  case AQFINTS_ElementType_Deg:   fprintf(stderr, "DEG  "); break;
  case AQFINTS_ElementType_De:    fprintf(stderr, "DE   "); break;
  default:                        fprintf(stderr, "(UNK)"); break;
  }

  switch(AQFINTS_Element_GetDbType(element)) {
  case AQFINTS_ElementDataType_Int:  fprintf(stderr, " int "); break;
  case AQFINTS_ElementDataType_Char: fprintf(stderr, " char"); break;
  case AQFINTS_ElementDataType_Bin:  fprintf(stderr, " bin "); break;
  default:                           fprintf(stderr, " UNK "); break;
  }

  s=AQFINTS_Element_GetId(element);
  if (s && *s)
    fprintf(stderr, " id=%s", s?s:"(empty)");

  s=AQFINTS_Element_GetName(element);
  if (s && *s)
    fprintf(stderr, " name=%s", s?s:"(empty)");

  fprintf(stderr, " version=%d", AQFINTS_Element_GetVersion(element));

  s=AQFINTS_Element_GetRef(element);
  if (s && *s)
    fprintf(stderr, " ref=%s", s?s:"(empty)");

  s=AQFINTS_Element_GetType(element);
  if (s && *s)
    fprintf(stderr, " type=%s", s?s:"(empty)");

  fprintf(stderr, " minnum=%d", AQFINTS_Element_GetMinNum(element));

  fprintf(stderr, " maxnum=%d", AQFINTS_Element_GetMaxNum(element));

  fprintf(stderr, " minsize=%d", AQFINTS_Element_GetMinSize(element));

  fprintf(stderr, " maxsize=%d", AQFINTS_Element_GetMaxSize(element));

  fprintf(stderr, "\n");

  childElement=AQFINTS_Element_Tree2_GetFirstChild(element);
  while(childElement) {
    AQFINTS_Parser_DumpElementTree(childElement, indent+2);
    childElement=AQFINTS_Element_Tree2_GetNext(childElement);
  }
}


