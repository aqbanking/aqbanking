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

#include "parser_db.h"

#include <gwenhywfar/debug.h>





#if 0
int deSequenceToDb(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT *elementData, GWEN_DB_NODE *db)
{
  while(elementDefinition) {
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
	rv=readDeSequence(childElementDefinition, elementData, dbForGroup);
      }

    }
    else {

    }
    elementDefinition=AQFINTS_Element_Tree2_GetNext(elementDefinition);
  } /* while */

}
#endif



int deToDb(AQFINTS_ELEMENT *elementDefinition, AQFINTS_ELEMENT **pElementData, GWEN_DB_NODE *db)
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

  while(elementData) {
    if (sDbName && *sDbName) {
      if (strcasecmp(sType, "AN")==0 ||
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
    idx++;
    if ((maxNum && idx==maxNum) ||
	(maxNum && idx==maxNum))
      break;

    elementData=AQFINTS_Element_Tree2_GetNext(elementData);
  }

}




