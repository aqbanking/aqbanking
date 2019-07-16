/***************************************************************************
 begin       : Sun Jul 07 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "parser_dbread.h"

#include <gwenhywfar/debug.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static int readDe(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db);
static int readDeSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db);
static int readDeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db);
static int readDeGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db);
static int readDegSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db);
static int readSeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementData, GWEN_DB_NODE *db);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


int AQFINTS_Parser_Db_ReadSegment(AQFINTS_SEGMENT *definitionSegment, AQFINTS_SEGMENT *dataSegment, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementDefinition;
  AQFINTS_ELEMENT *elementData;
  int rv;

  elementDefinition=AQFINTS_Segment_GetElements(definitionSegment);
  elementData=AQFINTS_Segment_GetElements(dataSegment);
  rv=readSeg(elementDefinition, elementData, db);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int readSeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementData, GWEN_DB_NODE *db)
{
  const char *sDbName;

  sDbName=AQFINTS_Element_GetName(elementDefinition);

  if (elementData) {
    AQFINTS_ELEMENT *childDefinitionData;
    AQFINTS_ELEMENT *childElementData;
    int rv;
    GWEN_DB_NODE *dbForDeg;

    childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
    if (childDefinitionData==NULL) {
      DBG_ERROR(0, "DEG Definition has no children");
      return GWEN_ERROR_BAD_DATA;
    }

    childElementData=AQFINTS_Element_Tree2_GetFirstChild(elementData);
    if (childElementData==NULL) {
      DBG_ERROR(0, "DEG Data has no children");
      return GWEN_ERROR_BAD_DATA;
    }

    if (sDbName && *sDbName)
      dbForDeg=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, sDbName);
    else
      dbForDeg=db;
    rv=readDegSequence(childDefinitionData, &childElementData, dbForDeg);
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }

    if (childElementData) {
      DBG_ERROR(0, "Too many data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
      return GWEN_ERROR_BAD_DATA;
    }
  }
  else {
    DBG_ERROR(0, "No data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
    return GWEN_ERROR_BAD_DATA;
  }

  return 0;
}



int readDegSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementData;

  elementData=*pElementData;

  while(elementDefinition) {
    const char *sDbName;
    int minNum;

    sDbName=AQFINTS_Element_GetName(elementDefinition);
    minNum=AQFINTS_Element_GetMinNum(elementDefinition);

    if (AQFINTS_Element_GetElementType(elementDefinition)==AQFINTS_ElementType_Group) {
      AQFINTS_ELEMENT *childElementDefinition;

      childElementDefinition=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
      if (childElementDefinition) {
	const char *sDbName;
	GWEN_DB_NODE *dbForGroup;
	int rv;

	sDbName=AQFINTS_Element_GetName(elementDefinition);
	if (sDbName && *sDbName)
	  dbForGroup=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, sDbName);
	else
	  dbForGroup=db;
	rv=readDegSequence(childElementDefinition, &elementData, dbForGroup);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return rv;
	}
      }
    } /* if group */
    else {
      int rv;

      if (elementData) {
	rv=readDeg(elementDefinition, &elementData, db);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return rv;
	}
      }
      else {
	if (minNum>0) {
	  DBG_ERROR(0, "Too few data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
	  return GWEN_ERROR_BAD_DATA;
	}
      }
    } /* if not group */
    elementDefinition=AQFINTS_Element_Tree2_GetNext(elementDefinition);
  } /* while */

  *pElementData=elementData;
  return 0;
}



int readDeg(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementData;
  int idx=0;
  int minNum;
  int maxNum;
  const char *sDbName;

  elementData=*pElementData;
  sDbName=AQFINTS_Element_GetName(elementDefinition);
  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);

  DBG_ERROR(0, "Reading DEG (name=%s)", sDbName?sDbName:"(unnamed)");

  while(elementData) {
    AQFINTS_ELEMENT *childDefinitionData;
    AQFINTS_ELEMENT *childElementData;
    int rv;
    GWEN_DB_NODE *dbForDeg;

    childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
    if (childDefinitionData==NULL) {
      DBG_ERROR(0, "DEG Definition has no children");
      return GWEN_ERROR_BAD_DATA;
    }

    childElementData=AQFINTS_Element_Tree2_GetFirstChild(elementData);
    if (childElementData==NULL) {
      DBG_ERROR(0, "DEG Data has no children");
      return GWEN_ERROR_BAD_DATA;
    }

    if (sDbName && *sDbName)
      dbForDeg=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, sDbName);
    else
      dbForDeg=db;
    DBG_ERROR(0, "Entering sequence for DEG (name=%s)", sDbName?sDbName:"(unnamed)");
    rv=readDeSequence(childDefinitionData, &childElementData, dbForDeg);
    DBG_ERROR(0, "Left sequence for DEG (name=%s)", sDbName?sDbName:"(unnamed)");
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }

    if (childElementData) {
      DBG_ERROR(0, "Too many data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
      return GWEN_ERROR_BAD_DATA;
    }

    elementData=AQFINTS_Element_Tree2_GetNext(elementData);
    idx++;
    if (maxNum && idx>=maxNum)
      break;
  }

  if (minNum && idx<minNum) {
    DBG_ERROR(0, "Too few data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
    return GWEN_ERROR_BAD_DATA;
  }

  *pElementData=elementData;
  return 0;
}



int readDeGroup(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementData;
  int idx=0;
  int minNum;
  int maxNum;
  const char *sDbName;

  elementData=*pElementData;
  sDbName=AQFINTS_Element_GetName(elementDefinition);
  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);

  DBG_ERROR(0, "Reading group (name=%s)", sDbName?sDbName:"(unnamed)");

  while(elementData) {
    AQFINTS_ELEMENT *childDefinitionData;
    int rv;
    GWEN_DB_NODE *dbForDeg;

    childDefinitionData=AQFINTS_Element_Tree2_GetFirstChild(elementDefinition);
    if (childDefinitionData==NULL) {
      DBG_ERROR(0, "DEG Definition has no children");
      return GWEN_ERROR_BAD_DATA;
    }

    if (sDbName && *sDbName)
      dbForDeg=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_CREATE_GROUP, sDbName);
    else
      dbForDeg=db;
    DBG_ERROR(0, "Entering sequence for group (name=%s)", sDbName?sDbName:"(unnamed)");
    rv=readDeSequence(childDefinitionData, &elementData, dbForDeg);
    DBG_ERROR(0, "Left sequence for group (name=%s)", sDbName?sDbName:"(unnamed)");
    if (rv<0) {
      DBG_INFO(0, "here (%d)", rv);
      return rv;
    }
#if 0
    if (elementData)
      elementData=AQFINTS_Element_Tree2_GetNext(elementData);
#endif
    idx++;
    if (maxNum && idx>=maxNum)
      break;
  }

  if (minNum && idx<minNum) {
    DBG_ERROR(0, "Too few data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
    return GWEN_ERROR_BAD_DATA;
  }

  *pElementData=elementData;
  return 0;
}



int readDeSequence(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementData;

  DBG_ERROR(0, "Reading DE sequence");
  elementData=*pElementData;

  while(elementDefinition) {
    const char *sDbName;
    int minNum;

    sDbName=AQFINTS_Element_GetName(elementDefinition);
    minNum=AQFINTS_Element_GetMinNum(elementDefinition);

    DBG_ERROR(0, "Handling DE sequence element (name=%s)", sDbName?sDbName:"(unnamed)");

    if (AQFINTS_Element_GetElementType(elementDefinition)==AQFINTS_ElementType_Group) {
      int rv;

      rv=readDeGroup(elementDefinition, &elementData, db);
      if (rv<0) {
        DBG_INFO(0, "here (%d)", rv);
        return rv;
      }
    } /* if group */
    else {
      int rv;

      if (elementData) {
	rv=readDe(elementDefinition, &elementData, db);
	if (rv<0) {
	  DBG_INFO(0, "here (%d)", rv);
	  return rv;
	}
      }
      else {
	if (minNum>0) {
	  DBG_ERROR(0, "Too few data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
	  return GWEN_ERROR_BAD_DATA;
	}
      }
    } /* if not group */
    elementDefinition=AQFINTS_Element_Tree2_GetNext(elementDefinition);
  } /* while */

  *pElementData=elementData;
  return 0;
}



int readDe(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
{
  AQFINTS_ELEMENT *elementData;
  int idx=0;
  int minNum;
  int maxNum;
  const char *sDbName;
  const char *sType;

  elementData=*pElementData;
  sDbName=AQFINTS_Element_GetName(elementDefinition);
  sType=AQFINTS_Element_GetType(elementDefinition);
  minNum=AQFINTS_Element_GetMinNum(elementDefinition);
  maxNum=AQFINTS_Element_GetMaxNum(elementDefinition);

  DBG_ERROR(0, "Reading DE (name=%s)", sDbName?sDbName:"(unnamed)");

  while(elementData) {
    if (sDbName && *sDbName) {
      if (strcasecmp(sType, "AN")==0 ||
	  strcasecmp(sType, "float")==0 ||
	  strcasecmp(sType, "alpha")==0 ||
	  strcasecmp(sType, "ascii")==0) {
	const char *sData;

	sData=AQFINTS_Element_GetDataAsChar(elementData, NULL);
	if (sData && *sData)
	  GWEN_DB_SetCharValue(db, 0, sDbName, sData);
      }
      else if (strcasecmp(sType, "num")==0) {
	GWEN_DB_SetIntValue(db, 0, sDbName, AQFINTS_Element_GetDataAsInt(elementData, 0));
      }
      else if (strcasecmp(sType, "bin")==0) {
	const uint8_t *val;
	uint32_t valSize;

	val=AQFINTS_Element_GetDataPointer(elementData);
	valSize=AQFINTS_Element_GetDataLength(elementData);
	if (val && valSize)
	  GWEN_DB_SetBinValue(db, 0, sDbName, (const void*) val, valSize);
      }
      else {
	DBG_ERROR(0, "Unknown data type \"%s\"", sType);
	return GWEN_ERROR_BAD_DATA;
      }
    }

    elementData=AQFINTS_Element_Tree2_GetNext(elementData);
    idx++;
    if (maxNum && idx>=maxNum)
      break;
  }
  DBG_ERROR(0, "Read DE %d times (name=%s, ptr=%p)", idx, sDbName?sDbName:"(unnamed)", (void*)elementData);

  if (minNum && idx<minNum) {
    DBG_ERROR(0, "Too few data elements for definition element \"%s\"", sDbName?sDbName:"unnamed");
    return GWEN_ERROR_BAD_DATA;
  }

  *pElementData=elementData;
  return 0;
}




