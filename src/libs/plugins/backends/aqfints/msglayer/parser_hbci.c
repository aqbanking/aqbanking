/***************************************************************************
 begin       : Fri Jul 04 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "parser_hbci.h"

#include <gwenhywfar/syncio_memory.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>



#define AQFINTS_PARSER_HBCI_BUFFERSIZE 1024



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int segRead(AQFINTS_SEGMENT *targetSegment, const uint8_t *ptrBuf, uint32_t lenBuf);
static int degRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int deRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int stringRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int binRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static void parseSegHeader(AQFINTS_SEGMENT *segment);




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */




int AQFINTS_Parser_Hbci_ReadBuffer(AQFINTS_SEGMENT_LIST *targetSegmentList,
                                   const uint8_t *ptrBuf,
                                   uint32_t lenBuf)
{
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  while(lenBuf && *ptrBuf) {
    AQFINTS_SEGMENT *targetSegment;
    int rv;

    targetSegment=AQFINTS_Segment_new();


    rv=segRead(targetSegment, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      AQFINTS_Segment_free(targetSegment);
      return rv;
    }
    parseSegHeader(targetSegment);
    AQFINTS_Segment_List_Add(targetSegment, targetSegmentList);

    /* advance pointer and size */
    lenBuf-=rv;
    ptrBuf+=rv;

    if (lenBuf) {
      if (*ptrBuf!='\'') {
        /* DE was terminated, but not by "'", error */
        return GWEN_ERROR_BAD_DATA;
      }
      ptrBuf++;
      lenBuf--;
    }
  } /* while */

  return (int) (origLenBuf-lenBuf);
}



void parseSegHeader(AQFINTS_SEGMENT *segment)
{
  AQFINTS_ELEMENT *element;

  element=AQFINTS_Segment_GetElements(segment);
  if (element) {
    AQFINTS_ELEMENT *deg1;

    deg1=AQFINTS_Element_Tree2_GetFirstChild(element);
    if (deg1) {
      AQFINTS_ELEMENT *de;

      de=AQFINTS_Element_Tree2_GetFirstChild(deg1);
      if (de) {
        const char *s;
        int i;

        /* read segment code */
        s=AQFINTS_Element_GetDataAsChar(de, NULL);
        if (s && *s)
          AQFINTS_Segment_SetCode(segment, s);

        /* read segment number */
        de=AQFINTS_Element_Tree2_GetNext(de);
        if (de) {
          i=AQFINTS_Element_GetDataAsInt(de, 0);
          AQFINTS_Segment_SetSegmentNumber(segment, i);

          /* read segment version */
          de=AQFINTS_Element_Tree2_GetNext(de);
          if (de) {
            i=AQFINTS_Element_GetDataAsInt(de, 0);
            AQFINTS_Segment_SetSegmentVersion(segment, i);

            /* read reference segment number */
            de=AQFINTS_Element_Tree2_GetNext(de);
            if (de) {
              i=AQFINTS_Element_GetDataAsInt(de, 0);
              AQFINTS_Segment_SetRefSegmentNumber(segment, i);
            } /* if fourth de */
          } /* if third de */
        } /* if second de */
      } /* if first de */
    } /* if deg1 */
  } /* if element */
}



int segRead(AQFINTS_SEGMENT *targetSegment, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  AQFINTS_ELEMENT *targetElement;
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  targetElement=AQFINTS_Segment_GetElements(targetSegment);
  if (targetElement==NULL) {
    targetElement=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(targetElement, AQFINTS_ElementType_Root);
    AQFINTS_Segment_SetElements(targetSegment, targetElement);
  }


  while(lenBuf && *ptrBuf) {
    AQFINTS_ELEMENT *targetDegElement;
    int rv;

    targetDegElement=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(targetDegElement, AQFINTS_ElementType_Deg);


    rv=degRead(targetDegElement, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      AQFINTS_Element_free(targetDegElement);
      return rv;
    }
    AQFINTS_Element_Tree2_AddChild(targetElement, targetDegElement);

    /* advance pointer and size */
    lenBuf-=rv;
    ptrBuf+=rv;

    if (lenBuf) {
      if (*ptrBuf!='+') {
        /* DE was terminated, but not by ':', so a higher syntax element ended */
        return (int) (origLenBuf-lenBuf);
      }
      ptrBuf++;
      lenBuf--;
    }
  } /* while */

  DBG_ERROR(0, "No delimiter at end of data");
  return GWEN_ERROR_BAD_DATA;
}



int degRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  while(lenBuf && *ptrBuf) {
    AQFINTS_ELEMENT *targetDeElement;
    int rv;

    targetDeElement=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(targetDeElement, AQFINTS_ElementType_De);


    rv=deRead(targetDeElement, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      AQFINTS_Element_free(targetDeElement);
      return rv;
    }
    AQFINTS_Element_Tree2_AddChild(targetElement, targetDeElement);

    /* advance pointer and size */
    lenBuf-=rv;
    ptrBuf+=rv;

    if (lenBuf) {
      if (*ptrBuf!=':') {
        /* DE was terminated, but not by ':', so a higher syntax element ended */
        return (int) (origLenBuf-lenBuf);
      }
      ptrBuf++;
      lenBuf--;
    }
  } /* while */

  DBG_ERROR(0, "No delimiter at end of data");
  return GWEN_ERROR_BAD_DATA;
}




int deRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  if (lenBuf) {
    if (*ptrBuf=='@') {
      int rv;

      rv=binRead(targetElement, ptrBuf, lenBuf);
      if (rv<0) {
        DBG_INFO(0, "here (%d)", rv);
        return rv;
      }
      return rv;
    }
    else {
      int rv;

      rv=stringRead(targetElement, ptrBuf, lenBuf);
      if (rv<0) {
        DBG_INFO(0, "here (%d)", rv);
        return rv;
      }
      return rv;
    }
  }
  DBG_ERROR(0, "Empty data buffer");
  return GWEN_ERROR_NO_DATA;
}



int stringRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;
  GWEN_BUFFER *destBuf;

  origLenBuf=lenBuf;
  destBuf=GWEN_Buffer_new(0, 256, 0, 1);

  while(*ptrBuf && lenBuf) {
    switch(*ptrBuf) {
    case '\'':
    case '+':
    case ':':
      /* end of segment, DEG or DE reached */
      if (GWEN_Buffer_GetUsedBytes(destBuf))
        AQFINTS_Element_SetTextDataCopy(targetElement, GWEN_Buffer_GetStart(destBuf));
      GWEN_Buffer_free(destBuf);
      return (int) (origLenBuf-lenBuf);
    case '?':
      /* escape character */
      ptrBuf++;
      lenBuf--;
      if (lenBuf && *ptrBuf)
        GWEN_Buffer_AppendByte(destBuf, *ptrBuf);
      else {
        DBG_ERROR(0, "Premature end of data (question mark was last character)");
        GWEN_Buffer_free(destBuf);
        return GWEN_ERROR_BAD_DATA;
      }
      break;
    default:
      GWEN_Buffer_AppendByte(destBuf, *ptrBuf);
    } /* switch */

    ptrBuf++;
    lenBuf--;
  } /* while */

  DBG_ERROR(0, "No delimiter at end of data");
  GWEN_Buffer_free(destBuf);
  return GWEN_ERROR_BAD_DATA;
}



int binRead(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  if (lenBuf && *ptrBuf=='@') {
    uint32_t lenBinary=0;

    ptrBuf++;
    lenBuf--;

    while(lenBuf && *ptrBuf && isdigit(*ptrBuf)) {
      lenBinary*=10;
      lenBinary+=((*ptrBuf)-'0');
      ptrBuf++;
      lenBuf--;
    }
    if (lenBuf && *ptrBuf=='@') {
      ptrBuf++;
      lenBuf--;

      if (lenBuf<lenBinary) {
        DBG_ERROR(0, "Invalid size of binary data (%lu < %lu)",
                  (unsigned long int) lenBuf,
                  (unsigned long int) lenBinary);
        return GWEN_ERROR_BAD_DATA;
      }

      if (lenBinary)
        AQFINTS_Element_SetDataCopy(targetElement, ptrBuf, lenBinary);
      AQFINTS_Element_AddFlags(targetElement, AQFINTS_ELEMENT_FLAGS_ISBIN);
      ptrBuf+=lenBinary;
      lenBuf-=lenBinary;
      return (int) (origLenBuf-lenBuf);
    }
  }

  DBG_ERROR(0, "Error in binary data spec");
  return GWEN_ERROR_BAD_DATA;
}










