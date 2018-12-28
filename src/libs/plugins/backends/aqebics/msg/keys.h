/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_KEYS_H
#define AQEBICS_MSG_KEYS_H


#include <aqebics/aqebics.h>

#include <gwenhywfar/cryptkey.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/ct.h>

#include <libxml/tree.h>


EB_RC EB_Key_toBin(const GWEN_CRYPT_KEY *k,
                   const char *userId,
		   const char *version,
                   int keySize,
                   GWEN_BUFFER *buf);

EB_RC EB_Key_fromBin(GWEN_CRYPT_KEY **k,
		     const char *version,
		     char *bufUserId,
		     unsigned int lenUserId,
                     const char *p, unsigned int bsize);


EB_RC EB_Key_toXml(GWEN_CRYPT_KEY *k, xmlNodePtr node);
EB_RC EB_Key_fromXml(GWEN_CRYPT_KEY **k, xmlNodePtr node);

int EB_Key_Info_toXml(const GWEN_CRYPT_TOKEN_KEYINFO *ki, xmlNodePtr node);
EB_RC EB_Key_Info_ReadXml(GWEN_CRYPT_TOKEN_KEYINFO *ki, xmlNodePtr node);

EB_RC EB_Key_Info_toBin(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			const char *userId,
			const char *version,
			int keySize,
			GWEN_BUFFER *buf);


int EB_Key_BuildHashSha1(const GWEN_CRYPT_KEY *k, GWEN_BUFFER *hbuf, int encode64);
int EB_Key_BuildHashSha256(const GWEN_CRYPT_KEY *k, GWEN_BUFFER *hbuf, int encode64);

int EB_Key_Info_BuildHashSha1(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
			      GWEN_BUFFER *hbuf,
			      int encode64);

int EB_Key_Info_BuildHashSha256(const GWEN_CRYPT_TOKEN_KEYINFO *ki,
				GWEN_BUFFER *hbuf,
				int encode64);


int EB_Key_Info_BuildSigHash_Rmd160(const GWEN_CRYPT_TOKEN_KEYINFO *ki, GWEN_BUFFER *hbuf);

int EB_Key_Info_BuildSigHash_Sha256(const GWEN_CRYPT_TOKEN_KEYINFO *ki, GWEN_BUFFER *hbuf);


#endif
