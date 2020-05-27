/***************************************************************************
 begin       : Fri Jul 19 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQFINTS_MESSAGE_H
#define AQFINTS_MESSAGE_H


//#include "msglayer/parser/parser.h"
#include "parser/segment.h"
#include "msglayer/keydescr.h"

#include <gwenhywfar/buffer.h>



#define AQFINTS_MESSAGE_FLAGS_DELAYED_VERIFY 0x00000001


typedef struct AQFINTS_MESSAGE AQFINTS_MESSAGE;


/** @name Constructor, Destructor
 *
 */
/*@{*/
AQFINTS_MESSAGE *AQFINTS_Message_new(void);
void AQFINTS_Message_free(AQFINTS_MESSAGE *msg);
/*@}*/


int AQFINTS_Message_GetMessageNumber(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetMessageNumber(AQFINTS_MESSAGE *msg, int v);

int AQFINTS_Message_GetRefMessageNumber(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetRefMessageNumber(AQFINTS_MESSAGE *msg, int v);


const char *AQFINTS_Message_GetDialogId(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetDialogId(AQFINTS_MESSAGE *msg, const char *s);


int AQFINTS_Message_GetHbciVersion(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetHbciVersion(AQFINTS_MESSAGE *msg, int i);



/** @name Cryptography
 *
 */
/*@{*/
AQFINTS_KEYDESCR_LIST *AQFINTS_Message_GetSignerList(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_AddSigner(AQFINTS_MESSAGE *msg, AQFINTS_KEYDESCR *keyDescr);

void AQFINTS_Message_ClearSigners(AQFINTS_MESSAGE *msg);

AQFINTS_KEYDESCR *AQFINTS_Message_FindSigner(AQFINTS_MESSAGE *msg, uint32_t uniqueUserId);


AQFINTS_KEYDESCR *AQFINTS_Message_GetCrypter(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetCrypter(AQFINTS_MESSAGE *msg, AQFINTS_KEYDESCR *keyDescr);
/*@}*/


/** @name Segments
 *
 */
/*@{*/
AQFINTS_SEGMENT_LIST *AQFINTS_Message_GetSegmentList(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_AddSegment(AQFINTS_MESSAGE *msg, AQFINTS_SEGMENT *segment);
/*@}*/


void AQFINTS_Message_Reenumerate(AQFINTS_MESSAGE *msg);
void AQFINTS_Message_MoveResultSegsToFront(AQFINTS_MESSAGE *msg);


int AQFINTS_Message_GetFirstSegNum(const AQFINTS_MESSAGE *msg);
int AQFINTS_Message_GetLastSegNum(const AQFINTS_MESSAGE *msg);


/**
 * Code of the TAN-relevant segment (i.e. code of the segment for which a tan is needed).
 */
const char *AQFINTS_Message_GetTanJobCode(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetTanJobCode(AQFINTS_MESSAGE *msg, const char *s);


uint32_t AQFINTS_Message_GetFlags(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetFlags(AQFINTS_MESSAGE *msg, uint32_t f);
void AQFINTS_Message_AddFlags(AQFINTS_MESSAGE *msg, uint32_t f);
void AQFINTS_Message_SubFlags(AQFINTS_MESSAGE *msg, uint32_t f);


#endif

