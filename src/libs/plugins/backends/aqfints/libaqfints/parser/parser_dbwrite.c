/***************************************************************************
 begin       : Wed Jul 17 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "libaqfints/parser/parser_dbwrite.h"
#include "libaqfints/parser/parser_internal.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int writeDegSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeDegGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeDeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeDeGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeDeSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeCharElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeIntElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);
static int writeBinElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AQFINTS_Parser_Db_WriteSegment(AQFINTS_SEGMENT *segmentDefinition, AQFINTS_SEGMENT *segmentData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementDefinition;
  AQFINTS_ELEMENT *childDefinitionData;
  AQFINTS_ELEMENT *elementDataParent;
  const char *segCode;
  int rv;

  elementDefinition=AQFINTS_Segment_GetElements(segmentDefinition);
  assert(elementDefinition);

  segCode=AQFINTS_Segment_GetCode(segmentDefinition);

  elementDataParent=AQFINTS_Segment_GetElements(segmentData);
  if (elementDataParent==NULL) {
    elementDataParent=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementDataParent, AQFINTS_ElementType_Root);
    AQFINTS_Segment_SetElements(segmentData, elementDataParent);
  }
  assert(elementDataParent);

  childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
  if (childDefinitionData==NULL) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no children");
    return GWEN_ERROR_BAD_DATA;
  }

  rv=writeDegSequence(childDefinitionData, elementDataParent, db);
  if (rv<0) {
    DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d) [%s]", rv, segCode?segCode:"");
    return rv;
  }

  return 0;
}



int writeDegSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  while (elementDefinition) {
    int rv;

    if (AQFINTS_Element_GetElementType(elementDefinition)==AQFINTS_ElementType_Group)
      rv=writeDegGroup(elementDefinition, elementDataParent, db);
    else
      rv=writeDeg(elementDefinition, elementDataParent, db);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    elementDefinition=AQFINTS_Element_Tree2_GetNext(elementDefinition);
  }

  return 0;
}



int writeDegGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *childDefinitionData;
  int minNum;
  int maxNum;
  const char *sDbName;
  int rv;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);

  childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
  if (childDefinitionData==NULL) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no children");
    return GWEN_ERROR_BAD_DATA;
  }

  if (sDbName && *sDbName) {
    GWEN_DB_NODE *dbForGroup;
    int idx=0;

    dbForGroup=GWEN_DB_FindFirstGroup(db, sDbName);
    while (dbForGroup) {
      if (maxNum && idx>=maxNum) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d)", idx, maxNum);
        return GWEN_ERROR_BAD_DATA;
      }
      rv=writeDegSequence(childDefinitionData, elementDataParent, dbForGroup);
      if (rv<0) {
        DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      idx++;

      dbForGroup=GWEN_DB_FindNextGroup(dbForGroup, sDbName);
    }
    if (minNum && idx<minNum) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d)", idx, minNum);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    /* if no group name given the occurrence must be 1 (i.e. manNum=1, maxNum=1)
     * because with no group name we wouldn't know how to determine the number of instances
     * of the group to generate
     */
    if (minNum!=1 || maxNum!=1) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no name while minNum and/or maxNum !=1");
      return GWEN_ERROR_BAD_DATA;
    }

    rv=writeDegSequence(childDefinitionData, elementDataParent, db);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int writeDeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *childDefinitionData;
  int minNum;
  int maxNum;
  const char *sDbName;
  int rv;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);

  childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
  if (childDefinitionData==NULL) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no children");
    return GWEN_ERROR_BAD_DATA;
  }

  if (sDbName && *sDbName) {
    GWEN_DB_NODE *dbForGroup;
    int idx=0;

    dbForGroup=GWEN_DB_FindFirstGroup(db, sDbName);
    while (dbForGroup) {
      AQFINTS_ELEMENT *elementData;

      if (maxNum && idx>=maxNum) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d)", idx, maxNum);
        return GWEN_ERROR_BAD_DATA;
      }

      elementData=AQFINTS_Element_new();
      AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_Deg);
      AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
      AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);

      rv=writeDeSequence(childDefinitionData, elementData, dbForGroup);
      if (rv<0) {
        DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      idx++;

      dbForGroup=GWEN_DB_FindNextGroup(dbForGroup, sDbName);
    }
    if (minNum && idx<minNum) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d) [%s]", idx, minNum, sDbName?sDbName:"");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    AQFINTS_ELEMENT *elementData;

    /* if no deg name given the occurrence must be 0 or 1 (i.e. maxNum=1)
      * because with no group name we wouldn't know how to determine the number of instances
      * of the group to generate
      */
    if (maxNum!=1) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no name while maxNum!=1");
      return GWEN_ERROR_BAD_DATA;
    }

    elementData=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_Deg);
    AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
    AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);

    rv=writeDeSequence(childDefinitionData, elementData, db);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}




int writeDeSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  while (elementDefinition) {
    int rv;

    if (AQFINTS_Element_GetElementType(elementDefinition)==AQFINTS_ElementType_Group)
      rv=writeDeGroup(elementDefinition, elementDataParent, db);
    else
      rv=writeElement(elementDefinition, elementDataParent, db);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
    elementDefinition=AQFINTS_Element_Tree2_GetNext(elementDefinition);
  }

  return 0;
}



int writeDeGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *childDefinitionData;
  int minNum;
  int maxNum;
  const char *sDbName;
  int rv;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);

  childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
  if (childDefinitionData==NULL) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no children");
    return GWEN_ERROR_BAD_DATA;
  }

  if (sDbName && *sDbName) {
    GWEN_DB_NODE *dbForGroup;
    int idx=0;

    dbForGroup=GWEN_DB_FindFirstGroup(db, sDbName);
    while (dbForGroup) {
      if (maxNum && idx>=maxNum) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d)", idx, maxNum);
        return GWEN_ERROR_BAD_DATA;
      }
      rv=writeDeSequence(childDefinitionData, elementDataParent, dbForGroup);
      if (rv<0) {
        DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }

      idx++;

      dbForGroup=GWEN_DB_FindNextGroup(dbForGroup, sDbName);
    }
    if (minNum && idx<minNum) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d) [%s]", idx, minNum, sDbName?sDbName:"");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    /* if no group name given the occurrence must be 0 or 1 (i.e. maxNum=1)
     * because with no group name we wouldn't know how to determine the number of instances
     * of the group to generate
     */
    if (maxNum!=1) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Definition has no name while maxNum!=1");
      return GWEN_ERROR_BAD_DATA;
    }

    rv=writeDeSequence(childDefinitionData, elementDataParent, db);
    if (rv<0) {
      DBG_INFO(AQFINTS_PARSER_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}


int writeElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  const char *sType;

  sType=AQFINTS_Element_GetType(elementDefinition);
  if (sType && *sType) {
    if (AQFINTS_Parser_IsCharType(sType))
      return writeCharElement(elementDefinition, elementDataParent, db);
    else if (AQFINTS_Parser_IsIntType(sType))
      return writeIntElement(elementDefinition, elementDataParent, db);
    else if (AQFINTS_Parser_IsBinType(sType))
      return writeBinElement(elementDefinition, elementDataParent, db);
    else {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Unknown element data type \"%s\"", sType);
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "No element data type given");
    return GWEN_ERROR_BAD_DATA;
  }
}



int writeCharElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  int idx;
  int minNum;
  int maxNum;
  const char *sDbName;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);

  for (idx=0; idx<maxNum; idx++) {
    const char *s;
    AQFINTS_ELEMENT *elementData;

    s=GWEN_DB_GetCharValue(db, sDbName, idx, NULL);
    if (s==NULL)
      break;
    if (maxNum && idx>=maxNum) {
      DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d [%s])", idx, maxNum, sDbName?sDbName:"");
      return GWEN_ERROR_BAD_DATA;
    }
    elementData=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
    AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
    AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
    AQFINTS_Element_SetTextDataCopy(elementData, s);
    AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
  }

  if (minNum && idx<minNum) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d) [%s]", idx, minNum, sDbName?sDbName:"");
  }

  /* create empty DE element if no data */
  if (idx==0) {
    AQFINTS_ELEMENT *elementData;

    elementData=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
    AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
    AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
    AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
  }
  return 0;
}



int writeIntElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  int idx;
  int minNum;
  int maxNum;
  int maxSize;
  const char *sDbName;
  uint32_t eFlags;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  maxSize=AQFINTS_Element_GetMaxSize(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);
  eFlags=AQFINTS_Element_GetFlags(elementDefinition);

  for (idx=0; idx<maxNum; idx++) {
    AQFINTS_ELEMENT *elementData;

    if (GWEN_DB_ValueExists(db, sDbName, idx)) {
      int value;

      if (maxNum && idx>=maxNum) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d)", idx, maxNum);
        return GWEN_ERROR_BAD_DATA;
      }

      value=GWEN_DB_GetIntValue(db, sDbName, idx, 0);
      elementData=AQFINTS_Element_new();
      AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
      AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
      AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));

      if ((eFlags & (AQFINTS_ELEMENT_FLAGS_LEFTFILL | AQFINTS_ELEMENT_FLAGS_RIGHTFILL)) && maxSize) {
        char numbuf[64];
        int len;

        len=snprintf(numbuf, sizeof(numbuf)-1, "%d", value);
        if (len>=sizeof(numbuf)) {
          DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Buffer too small to convert integer value (would need %d bytes)", len);
          return GWEN_ERROR_INTERNAL;
        }
        numbuf[len]=0;
        if (len>maxSize) {
          DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Data bigger than allowed (%d > %d)", len, maxSize);
          return GWEN_ERROR_INVALID;
        }
        else if (len<maxSize) {
          GWEN_BUFFER *tbuf;

          tbuf=GWEN_Buffer_new(0, maxSize, 0, 1);

          if (eFlags & AQFINTS_ELEMENT_FLAGS_LEFTFILL) {
            GWEN_Buffer_FillWithBytes(tbuf, '0', maxSize-len);
            GWEN_Buffer_AppendString(tbuf, numbuf);
          }
          else if (eFlags & AQFINTS_ELEMENT_FLAGS_RIGHTFILL) {
            GWEN_Buffer_AppendString(tbuf, numbuf);
            GWEN_Buffer_FillWithBytes(tbuf, '0', maxSize-len);
          }

          AQFINTS_Element_SetTextDataCopy(elementData, GWEN_Buffer_GetStart(tbuf));
          GWEN_Buffer_free(tbuf);
        }
        else {
          AQFINTS_Element_SetTextDataCopy(elementData, numbuf);
        }

      }
      else
        AQFINTS_Element_SetDataAsInt(elementData, value);
      AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
    }
    else
      break;
  }

  if (minNum && idx<minNum) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d) [%s]", idx, minNum, sDbName?sDbName:"");
  }

  /* create empty DE element if no data */
  if (idx==0) {
    AQFINTS_ELEMENT *elementData;

    elementData=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
    AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
    AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
    AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
  }

  return 0;
}



int writeBinElement(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementDataParent, GWEN_DB_NODE *db)
{
  int idx;
  int minNum;
  int maxNum;
  const char *sDbName;

  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);
  sDbName=AQFINTS_Element_GetName(elementDefinition);

  for (idx=0; idx<maxNum; idx++) {
    if (GWEN_DB_ValueExists(db, sDbName, idx)) {
      const uint8_t *ptr;
      unsigned int len;

      if (maxNum && idx>=maxNum) {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too many elements in DB (%d < %d)", idx, maxNum);
        return GWEN_ERROR_BAD_DATA;
      }

      len=0;
      ptr=(const uint8_t *) GWEN_DB_GetBinValue(db, sDbName, idx, NULL, 0, &len);
      if (ptr && len) {
        AQFINTS_ELEMENT *elementData;

        elementData=AQFINTS_Element_new();
        AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
        AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
        AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
        AQFINTS_Element_SetDataCopy(elementData,  ptr, len);
        AQFINTS_Element_AddFlags(elementData, AQFINTS_ELEMENT_FLAGS_ISBIN);
        AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
      }
      else {
        DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Empty binary data");
        break;
      }
    }
    else
      break;
  }

  if (minNum && idx<minNum) {
    DBG_ERROR(AQFINTS_PARSER_LOGDOMAIN, "Too few elements in DB (%d < %d)", idx, minNum);
  }

  /* create empty DE element if no data */
  if (idx==0) {
    AQFINTS_ELEMENT *elementData;

    elementData=AQFINTS_Element_new();
    AQFINTS_Element_SetElementType(elementData, AQFINTS_ElementType_De);
    AQFINTS_Element_SetType(elementData, AQFINTS_Element_GetType(elementDefinition));
    AQFINTS_Element_SetTrustLevel(elementData, AQFINTS_Element_GetTrustLevel(elementDefinition));
    AQFINTS_Element_AddFlags(elementData, AQFINTS_ELEMENT_FLAGS_ISBIN);
    AQFINTS_Element_Tree2_AddChild(elementDataParent, elementData);
  }

  return 0;
}

