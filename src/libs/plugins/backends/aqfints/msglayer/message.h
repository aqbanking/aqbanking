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


#include "msglayer/parser/parser.h"
#include "msglayer/parser/segment.h"
#include "msglayer/keyname.h"

#include <gwenhywfar/buffer.h>



typedef struct AQFINTS_MESSAGE AQFINTS_MESSAGE;


/** @name Constructor, Destructor
 *
 */
/*@{*/
AQFINTS_MESSAGE *AQFINTS_Message_new(AQFINTS_PARSER *parser);
void AQFINTS_Message_free(AQFINTS_MESSAGE *msg);
/*@}*/


/** @name Cryptography
 *
 */
/*@{*/
AQFINTS_KEYNAME_LIST *AQFINTS_Message_GetSignerList(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_AddSigner(AQFINTS_MESSAGE *msg, AQFINTS_KEYNAME *keyName);

AQFINTS_KEYNAME *AQFINTS_Message_GetCrypter(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_SetCrypter(AQFINTS_MESSAGE *msg, AQFINTS_KEYNAME *keyName);
/*@}*/


/** @name Segments
 *
 */
/*@{*/
AQFINTS_SEGMENT_LIST *AQFINTS_Message_GetSegmentList(const AQFINTS_MESSAGE *msg);
void AQFINTS_Message_AddSegment(AQFINTS_MESSAGE *msg, AQFINTS_SEGMENT *segment);
/*@}*/




#endif

