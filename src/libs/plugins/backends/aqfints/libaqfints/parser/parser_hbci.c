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
#include "parser_normalize.h"

#include <gwenhywfar/syncio_memory.h>
#include <gwenhywfar/debug.h>

#include <ctype.h>



#define AQFINTS_PARSER_HBCI_BUFFERSIZE 1024



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int readSeg(AQFINTS_SEGMENT *targetSegment, const uint8_t *ptrBuf, uint32_t lenBuf);
static int readDeg(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int readDe(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int readString(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static int readBin(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf);
static void parseSegHeader(AQFINTS_SEGMENT *segment);

static void writeDegSequence(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf, int elementCount,
                             int *pEndOfLastNonEmptyElement);
static int writeDeg(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf);
static void writeDeSequence(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf, int elementCount,
                            int *pEndOfLastNonEmptyElement);
static int writeDe(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf);
static void writeBin(const uint8_t *ptrBuf, uint32_t lenBuf, GWEN_BUFFER *destBuf);
static void writeString(const char *s, GWEN_BUFFER *destBuf);




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

  while (lenBuf && *ptrBuf) {
    AQFINTS_SEGMENT *targetSegment;
    int rv;

    targetSegment=AQFINTS_Segment_new();

    rv=readSeg(targetSegment, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      AQFINTS_Segment_free(targetSegment);
      return rv;
    }
    parseSegHeader(targetSegment);

    AQFINTS_Parser_Segment_RemoveTrailingEmptyElements(targetSegment);

    AQFINTS_Segment_List_Add(targetSegment, targetSegmentList);

    /* store copy of segment data */
    if (lenBuf>rv) {
      if (ptrBuf[rv]!='\'') {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Segment not terminated by quotation mark");
        return GWEN_ERROR_BAD_DATA;
      }
      AQFINTS_Segment_SetDataAsCopy(targetSegment, ptrBuf, rv+1);
    }
    else {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Segment too small (no room for terminating quotation mark)");
      return GWEN_ERROR_BAD_DATA;
    }

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

  return (int)(origLenBuf-lenBuf);
}



void AQFINTS_Parser_Hbci_WriteBuffer(AQFINTS_SEGMENT_LIST *segmentList)
{
  AQFINTS_SEGMENT *segment;

  segment=AQFINTS_Segment_List_First(segmentList);
  while (segment) {
    AQFINTS_Parser_Hbci_WriteSegment(segment);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}



void AQFINTS_Parser_Hbci_WriteSegment(AQFINTS_SEGMENT *segment)
{
  AQFINTS_ELEMENT *rootElement;
  uint32_t segmentStartPos;
  int endOfLastNonEmptyElement=0;
  uint32_t pos;
  GWEN_BUFFER *destBuf;
  int rv;

  destBuf=GWEN_Buffer_new(0, 256, 0, 1);
  segmentStartPos=GWEN_Buffer_GetPos(destBuf);
  rootElement=AQFINTS_Segment_GetElements(segment);
  if (rootElement) {
    writeDegSequence(rootElement, destBuf, 0, &endOfLastNonEmptyElement);

    /* remove trailing '+'s */
    pos=GWEN_Buffer_GetPos(destBuf);
    if (pos>endOfLastNonEmptyElement) {
      uint32_t cropPos;

      cropPos=endOfLastNonEmptyElement?endOfLastNonEmptyElement:segmentStartPos;

      /*DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Crop destbuffer: %d->%d", pos, cropPos);*/
      GWEN_Buffer_Crop(destBuf, 0, cropPos);
    }
  }

  /* append segment end sign */
  GWEN_Buffer_AppendByte(destBuf, '\'');

  /* set data to segment, take over GWEN_BUFFER content */
  AQFINTS_Segment_SetData(segment, (uint8_t *) GWEN_Buffer_GetStart(destBuf), GWEN_Buffer_GetUsedBytes(destBuf));
  rv=GWEN_Buffer_Relinquish(destBuf);
  if (rv<0) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
    abort();
  }
  GWEN_Buffer_free(destBuf);
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



int readSeg(AQFINTS_SEGMENT *targetSegment, const uint8_t *ptrBuf, uint32_t lenBuf)
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


  while (lenBuf && *ptrBuf) {
    AQFINTS_ELEMENT *targetDegElement;
    int rv;

    targetDegElement=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(targetDegElement, AQFINTS_ElementType_Deg);


    rv=readDeg(targetDegElement, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
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
        return (int)(origLenBuf-lenBuf);
      }
      ptrBuf++;
      lenBuf--;
    }
  } /* while */

  DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "No delimiter at end of data");
  return GWEN_ERROR_BAD_DATA;
}



int readDeg(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  while (lenBuf && *ptrBuf) {
    AQFINTS_ELEMENT *targetDeElement;
    int rv;

    targetDeElement=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(targetDeElement, AQFINTS_ElementType_De);


    rv=readDe(targetDeElement, ptrBuf, lenBuf);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
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
        return (int)(origLenBuf-lenBuf);
      }
      ptrBuf++;
      lenBuf--;
    }
  } /* while */

  DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "No delimiter at end of data");
  return GWEN_ERROR_BAD_DATA;
}




int readDe(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  if (lenBuf) {
    if (*ptrBuf=='@') {
      int rv;

      rv=readBin(targetElement, ptrBuf, lenBuf);
      if (rv<0) {
        DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      return rv;
    }
    else {
      int rv;

      rv=readString(targetElement, ptrBuf, lenBuf);
      if (rv<0) {
        DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
      return rv;
    }
  }
  DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Empty data buffer");
  return GWEN_ERROR_NO_DATA;
}



int readString(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;
  GWEN_BUFFER *destBuf;

  origLenBuf=lenBuf;
  destBuf=GWEN_Buffer_new(0, 256, 0, 1);

  while (*ptrBuf && lenBuf) {
    switch (*ptrBuf) {
    case '\'':
    case '+':
    case ':':
      /* end of segment, DEG or DE reached */
      if (GWEN_Buffer_GetUsedBytes(destBuf))
        AQFINTS_Element_SetTextDataCopy(targetElement, GWEN_Buffer_GetStart(destBuf));
      GWEN_Buffer_free(destBuf);
      return (int)(origLenBuf-lenBuf);
    case '?':
      /* escape character */
      ptrBuf++;
      lenBuf--;
      if (lenBuf && *ptrBuf)
        GWEN_Buffer_AppendByte(destBuf, *ptrBuf);
      else {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Premature end of data (question mark was last character)");
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

  DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "No delimiter at end of data");
  GWEN_Buffer_free(destBuf);
  return GWEN_ERROR_BAD_DATA;
}



int readBin(AQFINTS_ELEMENT *targetElement, const uint8_t *ptrBuf, uint32_t lenBuf)
{
  uint32_t origLenBuf;

  origLenBuf=lenBuf;

  if (lenBuf && *ptrBuf=='@') {
    uint32_t lenBinary=0;

    ptrBuf++;
    lenBuf--;

    while (lenBuf && *ptrBuf && isdigit(*ptrBuf)) {
      lenBinary*=10;
      lenBinary+=((*ptrBuf)-'0');
      ptrBuf++;
      lenBuf--;
    }
    if (lenBuf && *ptrBuf=='@') {
      ptrBuf++;
      lenBuf--;

      if (lenBuf<lenBinary) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Invalid size of binary data (%lu < %lu)",
                  (unsigned long int) lenBuf,
                  (unsigned long int) lenBinary);
        return GWEN_ERROR_BAD_DATA;
      }

      if (lenBinary)
        AQFINTS_Element_SetDataCopy(targetElement, ptrBuf, lenBinary);
      AQFINTS_Element_AddFlags(targetElement, AQFINTS_ELEMENT_FLAGS_ISBIN);
      ptrBuf+=lenBinary;
      lenBuf-=lenBinary;
      return (int)(origLenBuf-lenBuf);
    }
  }

  DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Error in binary data spec");
  return GWEN_ERROR_BAD_DATA;
}



void writeDegSequence(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf, int elementCount, int *pEndOfLastNonEmptyElement)
{
  AQFINTS_ELEMENT *childElement;

  childElement=AQFINTS_Element_Tree2_GetFirstChild(element);
  while (childElement) {
    if (elementCount)
      GWEN_Buffer_AppendByte(destBuf, '+');

    if (AQFINTS_Element_GetElementType(childElement)==AQFINTS_ElementType_Group)
      writeDegSequence(childElement, destBuf, elementCount, pEndOfLastNonEmptyElement);
    else {
      int rv;

      rv=writeDeg(childElement, destBuf);
      if (rv>0)
        *pEndOfLastNonEmptyElement=GWEN_Buffer_GetPos(destBuf);
    }
    elementCount++;
    childElement=AQFINTS_Element_Tree2_GetNext(childElement);
  }
}



int writeDeg(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf)
{
  uint32_t elementStartPos;
  uint32_t elementSize;
  int endOfLastNonEmptyElement=0;
  uint32_t pos;

  elementStartPos=GWEN_Buffer_GetPos(destBuf);
  writeDeSequence(element, destBuf, 0, &endOfLastNonEmptyElement);

  /* remove trailing ':'s */
  pos=GWEN_Buffer_GetPos(destBuf);
  if (pos>endOfLastNonEmptyElement) {
    uint32_t cropPos;

    cropPos=endOfLastNonEmptyElement?endOfLastNonEmptyElement:elementStartPos;

    DBG_DEBUG(AQFINTS_PARSER_LOGDOMAIN, "Crop destbuffer: %d->%d", pos, cropPos);
    GWEN_Buffer_Crop(destBuf, 0, cropPos);
  }

  /* set size and pos */
  elementSize=GWEN_Buffer_GetPos(destBuf)-elementStartPos;
  return (elementSize>0)?1:0;
}



void writeDeSequence(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf, int elementCount, int *pEndOfLastNonEmptyElement)
{
  AQFINTS_ELEMENT *childElement;

  childElement=AQFINTS_Element_Tree2_GetFirstChild(element);
  while (childElement) {
    if (elementCount)
      GWEN_Buffer_AppendByte(destBuf, ':');

    if (AQFINTS_Element_GetElementType(childElement)==AQFINTS_ElementType_Group)
      writeDeSequence(childElement, destBuf, elementCount, pEndOfLastNonEmptyElement);
    else {
      int rv;

      rv=writeDe(childElement, destBuf);
      if (rv>0) {
        *pEndOfLastNonEmptyElement=GWEN_Buffer_GetPos(destBuf);
        DBG_DEBUG(AQFINTS_PARSER_LOGDOMAIN, "Nonempty element ends at %d", GWEN_Buffer_GetPos(destBuf));
      }
      else {
        DBG_DEBUG(AQFINTS_PARSER_LOGDOMAIN, "Empty element ends at %d", GWEN_Buffer_GetPos(destBuf));
      }
    }
    elementCount++;
    childElement=AQFINTS_Element_Tree2_GetNext(childElement);
  }
}



int writeDe(AQFINTS_ELEMENT *element, GWEN_BUFFER *destBuf)
{
  uint32_t elementStartPos;
  uint32_t elementSize;

  elementStartPos=GWEN_Buffer_GetPos(destBuf);

  if (AQFINTS_Element_GetFlags(element) & AQFINTS_ELEMENT_FLAGS_ISBIN) {
    uint32_t lenBuf;
    const uint8_t *ptrBuf;

    lenBuf=AQFINTS_Element_GetDataLength(element);
    ptrBuf=AQFINTS_Element_GetDataPointer(element);
    if (lenBuf && ptrBuf)
      writeBin(ptrBuf, lenBuf, destBuf);
  }
  else {
    const char *s;

    s=AQFINTS_Element_GetDataAsChar(element, NULL);
    if (s && *s)
      writeString(s, destBuf);
  }

  elementSize=GWEN_Buffer_GetPos(destBuf)-elementStartPos;
  return (elementSize>0)?1:0;
}



void writeBin(const uint8_t *ptrBuf, uint32_t lenBuf, GWEN_BUFFER *destBuf)
{
  char numbuf[32];
  int i;

  i=snprintf(numbuf, sizeof(numbuf)-1, "%u", (unsigned int) lenBuf);
  assert(i<sizeof(numbuf));

  GWEN_Buffer_AppendByte(destBuf, '@');
  GWEN_Buffer_AppendString(destBuf, numbuf);
  GWEN_Buffer_AppendByte(destBuf, '@');
  GWEN_Buffer_AppendBytes(destBuf, (const char *) ptrBuf, lenBuf);
}



void writeString(const char *s, GWEN_BUFFER *destBuf)
{
  if (s) {
    while (*s) {
      if (NULL!=strchr("+:@?'", *s))
        GWEN_Buffer_AppendByte(destBuf, '?');    /* prepend by '?' */
      GWEN_Buffer_AppendByte(destBuf, *s);
      s++;
    }
  }
}


