/***************************************************************************
 begin       : Fri Jul 19 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "message_p.h"

#include "aqfints.h"
#include "parser/parser_xml.h"
#include "parser/parser_hbci.h"
#include "parser/parser_normalize.h"
#include "parser/parser_dbwrite.h"

#include <gwenhywfar/debug.h>

#include <inttypes.h>




/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void _moveSegmentsBetweenLists(AQFINTS_SEGMENT_LIST *segmentList1, AQFINTS_SEGMENT_LIST *segmentList2);
static void _moveSegmentsByCodeBetweenLists(AQFINTS_SEGMENT_LIST *segmentList1, AQFINTS_SEGMENT_LIST *segmentList2,
                                            const char *code);





/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */


AQFINTS_MESSAGE *AQFINTS_Message_new(void)
{
  AQFINTS_MESSAGE *msg;

  GWEN_NEW_OBJECT(AQFINTS_MESSAGE, msg);
  msg->signerList=AQFINTS_KeyDescr_List_new();
  msg->segmentList=AQFINTS_Segment_List_new();

  return msg;
}



void AQFINTS_Message_free(AQFINTS_MESSAGE *msg)
{
  if (msg) {
    free(msg->dialogId);
    free(msg->tanJobCode);
    AQFINTS_Segment_List_free(msg->segmentList);
    AQFINTS_KeyDescr_List_free(msg->signerList);
    AQFINTS_KeyDescr_free(msg->crypter);
    GWEN_FREE_OBJECT(msg);
  }
}



uint32_t AQFINTS_Message_GetFlags(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->flags;
}



void AQFINTS_Message_SetFlags(AQFINTS_MESSAGE *msg, uint32_t f)
{
  assert(msg);
  msg->flags=f;
}



void AQFINTS_Message_AddFlags(AQFINTS_MESSAGE *msg, uint32_t f)
{
  assert(msg);
  msg->flags|=f;
}



void AQFINTS_Message_SubFlags(AQFINTS_MESSAGE *msg, uint32_t f)
{
  assert(msg);
  msg->flags&=~f;
}



int AQFINTS_Message_GetMessageNumber(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->messageNumber;
}



void AQFINTS_Message_SetMessageNumber(AQFINTS_MESSAGE *msg, int v)
{
  assert(msg);
  msg->messageNumber=v;
}



int AQFINTS_Message_GetRefMessageNumber(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->refMessageNumber;
}



void AQFINTS_Message_SetRefMessageNumber(AQFINTS_MESSAGE *msg, int v)
{
  assert(msg);
  msg->refMessageNumber=v;
}



const char *AQFINTS_Message_GetDialogId(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->dialogId;
}



void AQFINTS_Message_SetDialogId(AQFINTS_MESSAGE *msg, const char *s)
{
  assert(msg);
  if (msg->dialogId)
    free(msg->dialogId);
  if (s)
    msg->dialogId=strdup(s);
  else
    msg->dialogId=NULL;
}



int AQFINTS_Message_GetHbciVersion(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->hbciVersion;
}



void AQFINTS_Message_SetHbciVersion(AQFINTS_MESSAGE *msg, int i)
{
  assert(msg);
  msg->hbciVersion=i;
}




const char *AQFINTS_Message_GetTanJobCode(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->tanJobCode;
}



void AQFINTS_Message_SetTanJobCode(AQFINTS_MESSAGE *msg, const char *s)
{
  assert(msg);
  if (msg->tanJobCode)
    free(msg->tanJobCode);
  if (s)
    msg->tanJobCode=strdup(s);
  else
    msg->tanJobCode=NULL;
}




AQFINTS_KEYDESCR_LIST *AQFINTS_Message_GetSignerList(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->signerList;
}



int AQFINTS_Message_HasAnySigner(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  if (msg->signerList && AQFINTS_KeyDescr_List_GetCount(msg->signerList)>0)
    return 1;

  return 0;
}



void AQFINTS_Message_AddSigner(AQFINTS_MESSAGE *msg, AQFINTS_KEYDESCR *keyDescr)
{
  uint32_t uid;

  assert(msg);
  uid=AQFINTS_KeyDescr_GetUniqueUserId(keyDescr);

  if (AQFINTS_Message_FindSigner(msg, uid)) {
    DBG_WARN(AQFINTS_LOGDOMAIN, "Signer %" PRIx32 "already exists, not adding", uid);
  }
  else
    AQFINTS_KeyDescr_List_Add(keyDescr, msg->signerList);
}



AQFINTS_KEYDESCR *AQFINTS_Message_FindSigner(AQFINTS_MESSAGE *msg, uint32_t uniqueUserId)
{
  if (msg->signerList) {
    AQFINTS_KEYDESCR *keyDescr;

    keyDescr=AQFINTS_KeyDescr_List_First(msg->signerList);
    while (keyDescr) {
      if (AQFINTS_KeyDescr_GetUniqueUserId(keyDescr)==uniqueUserId)
        return keyDescr;

      keyDescr=AQFINTS_KeyDescr_List_Next(keyDescr);
    }
  }

  return NULL;
}



void AQFINTS_Message_ClearSigners(AQFINTS_MESSAGE *msg)
{
  assert(msg);
  if (msg->signerList) {
    AQFINTS_KeyDescr_List_Clear(msg->signerList);
  }
}



AQFINTS_KEYDESCR *AQFINTS_Message_GetCrypter(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->crypter;
}



void AQFINTS_Message_SetCrypter(AQFINTS_MESSAGE *msg, AQFINTS_KEYDESCR *keyDescr)
{
  assert(msg);
  if (msg->crypter)
    AQFINTS_KeyDescr_free(msg->crypter);
  msg->crypter=keyDescr;
}



AQFINTS_SEGMENT_LIST *AQFINTS_Message_GetSegmentList(const AQFINTS_MESSAGE *msg)
{
  assert(msg);
  return msg->segmentList;
}



void AQFINTS_Message_AddSegment(AQFINTS_MESSAGE *msg, AQFINTS_SEGMENT *segment)
{
  assert(msg);
  assert(segment);
  AQFINTS_Segment_List_Add(segment, msg->segmentList);
}



void AQFINTS_Message_Reenumerate(AQFINTS_MESSAGE *msg)
{
  int segNum;
  AQFINTS_SEGMENT *segment;

  assert(msg);

  /* first segment number is 2 + number of signers (1 x HNHBK, n x HNSHK) */
  segNum=AQFINTS_KeyDescr_List_GetCount(msg->signerList)+2;
  segment=AQFINTS_Segment_List_First(msg->segmentList);
  while (segment) {
    AQFINTS_Segment_SetSegmentNumber(segment, segNum++);
    segment=AQFINTS_Segment_List_Next(segment);
  }
}



int AQFINTS_Message_GetFirstSegNum(const AQFINTS_MESSAGE *msg)
{
  AQFINTS_SEGMENT *segment;

  assert(msg);

  segment=AQFINTS_Segment_List_First(msg->segmentList);
  if (segment)
    return AQFINTS_Segment_GetSegmentNumber(segment);
  return 0;
}



int AQFINTS_Message_GetLastSegNum(const AQFINTS_MESSAGE *msg)
{
  AQFINTS_SEGMENT *segment;

  assert(msg);

  segment=AQFINTS_Segment_List_Last(msg->segmentList);
  if (segment)
    return AQFINTS_Segment_GetSegmentNumber(segment);
  return 0;
}



void AQFINTS_Message_MoveResultSegsToFront(AQFINTS_MESSAGE *msg)
{
  AQFINTS_SEGMENT_LIST *tmpSegmentList;

  tmpSegmentList=AQFINTS_Segment_List_new();

  _moveSegmentsBetweenLists(msg->segmentList, tmpSegmentList);
  _moveSegmentsByCodeBetweenLists(tmpSegmentList, msg->segmentList, "HIRMG");
  _moveSegmentsByCodeBetweenLists(tmpSegmentList, msg->segmentList, "HIRMS");
  _moveSegmentsBetweenLists(tmpSegmentList, msg->segmentList);

  AQFINTS_Segment_List_free(tmpSegmentList);
  AQFINTS_Message_Reenumerate(msg);
}



void _moveSegmentsBetweenLists(AQFINTS_SEGMENT_LIST *segmentList1, AQFINTS_SEGMENT_LIST *segmentList2)
{
  AQFINTS_SEGMENT *segment;

  while ((segment=AQFINTS_Segment_List_First(segmentList1))) {
    AQFINTS_Segment_List_Del(segment);
    AQFINTS_Segment_List_Add(segment, segmentList2);
  }
}



void _moveSegmentsByCodeBetweenLists(AQFINTS_SEGMENT_LIST *segmentList1, AQFINTS_SEGMENT_LIST *segmentList2,
                                     const char *code)
{

  AQFINTS_SEGMENT *segment;
  /* move all HIRMG segments back to old list */
  segment=AQFINTS_Segment_List_First(segmentList1);
  while (segment) {
    AQFINTS_SEGMENT *nextSegment;

    nextSegment=AQFINTS_Segment_List_Next(segment);
    if (strcasecmp(AQFINTS_Segment_GetCode(segment), code)==0) {
      AQFINTS_Segment_List_Del(segment);
      AQFINTS_Segment_List_Add(segment, segmentList2);
    }
    segment=nextSegment;
  }
}



uint32_t AQFINTS_Message_SampleSegmentFlags(const AQFINTS_MESSAGE *msg)
{
  uint32_t flags=0;

  if (msg->segmentList)
    flags=AQFINTS_Segment_List_SampleFlags(msg->segmentList);
  return flags;
}



