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




void AQFINTS_Parser_Normalize_Segment(AQFINTS_SEGMENT *segment)
{
  AQFINTS_ELEMENT_TREE *elementTree;

  elementTree=AQFINTS_Segment_GetElements(segment);
  if (elementTree) {
    AQFINTS_ELEMENT *element;

    /* check top-level elements: if DE, prepend a DEG before it */
    element=AQFINTS_Element_Tree_GetFirst(elementTree);
    while(element) {
      AQFINTS_ELEMENT *nextElement;

      nextElement=AQFINTS_Element_Tree_GetNext(element);

      if (AQFINTS_Element_GetElementType(element)==AQFINTS_ElementType_De) {
        AQFINTS_ELEMENT *elementDeg;

        elementDeg=AQFINTS_Element_new();
        AQFINTS_Element_SetElementType(elementDeg, AQFINTS_ElementType_Deg);
        AQFINTS_Element_Tree_Replace(element, elementDeg);
        AQFINTS_Element_Tree_AddChild(elementDeg, element);
      }

      element=nextElement;
    }
  }
}


