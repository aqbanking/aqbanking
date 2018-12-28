/***************************************************************************
    begin       : Sat Mar 08 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "eu_p.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/misc.h>


GWEN_LIST_FUNCTIONS(EB_EU, EB_Eu)


#define SETSTRING(xName) \
  assert(eu); \
  free(eu->xName); \
  if (s) eu->xName=strdup(s); \
  else eu->xName=NULL;



EB_EU *EB_Eu_new() {
  EB_EU *eu;

  GWEN_NEW_OBJECT(EB_EU, eu);
  GWEN_LIST_INIT(EB_EU, eu);

  return eu;
}



void EB_Eu_free(EB_EU *eu) {
  if (eu) {
    free(eu->version);
    free(eu->jobType);
    free(eu->signaturePtr);
    free(eu->userId);
    free(eu->originalFileName);
    GWEN_Time_free(eu->creationTime);
    GWEN_Time_free(eu->signatureTime);
    GWEN_FREE_OBJECT(eu);
  }
}



EB_EU *EB_Eu_dup(const EB_EU *oldEu) {
  GWEN_BUFFER *buf;
  EB_EU *eu;

  buf=GWEN_Buffer_new(0, 512, 0, 1);
  EB_Eu_toBuffer(oldEu, buf);
  eu=EB_Eu_fromBuffer((const uint8_t*)GWEN_Buffer_GetStart(buf),
		      GWEN_Buffer_GetUsedBytes(buf));
  GWEN_Buffer_free(buf);
  return eu;
}



void copyTrimmedString(const uint8_t *p, uint32_t l, char **pDst) {
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 128, 0, 1);
  GWEN_Buffer_AppendBytes(buf, (const char*)p, l);
  GWEN_Text_CondenseBuffer(buf);
  *pDst=strdup(GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
}



EB_EU *EB_Eu_fromBuffer(const uint8_t *p, uint32_t l) {
  if (l<512) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Too few bytes, not a complete EU (%d)", l);
    return NULL;
  }
  else {
    EB_EU *eu;
    const uint8_t *s;
    char *t;

    eu=EB_Eu_new();
    s=p;
    copyTrimmedString(s, 4, &(eu->version));
    s+=4;
    copyTrimmedString(s, 4, &t);
    if (t) {
      sscanf(t, "%d", &(eu->modLen));
      free(t);
    }
    s+=4;
    copyTrimmedString(s, 3, &(eu->jobType));
    s+=3;
    EB_Eu_SetSignature(eu, s, 128);
    s+=128;
    copyTrimmedString(s, 8, &(eu->userId));
    s+=8;
    copyTrimmedString(s, 128, &(eu->originalFileName));
    s+=128;
    copyTrimmedString(s, 16, &t);
    if (t) {
      eu->creationTime=GWEN_Time_fromString(t, "YYYYMMDD hhmmss");
      free(t);
    }
    s+=16;
    copyTrimmedString(s, 16, &t);
    if (t) {
      eu->signatureTime=GWEN_Time_fromString(t, "YYYYMMDD hhmmss");
      free(t);
    }
    s+=16;

    return eu;
  }
}



void EB_Eu_toBuffer(const EB_EU *eu, GWEN_BUFFER *buf) {
  int l;
  char numbuf[16];

  /* version */
  if (eu->version) {
    l=strlen(eu->version);
    if (l)
      GWEN_Buffer_AppendString(buf, eu->version);
  }
  else
    l=0;
  if (l<4)
    GWEN_Buffer_FillWithBytes(buf, 32, 4-l);

  /* length of modulus */
  snprintf(numbuf, sizeof(numbuf)-1, "%d", eu->modLen);
  numbuf[sizeof(numbuf)-1]=0;
  l=strlen(numbuf);
  if (l)
    GWEN_Buffer_AppendString(buf, numbuf);
  if (l<4)
    GWEN_Buffer_FillWithBytes(buf, 32, 4-l);

  /* job type */
  if (eu->jobType) {
    l=strlen(eu->jobType);
    if (l)
      GWEN_Buffer_AppendString(buf, eu->jobType);
  }
  else
    l=0;
  if (l<3)
    GWEN_Buffer_FillWithBytes(buf, 32, 3-l);

  /* signature */
  if (eu->signatureLen<128)
    GWEN_Buffer_FillWithBytes(buf, 0, 128-eu->signatureLen);
  if (eu->signaturePtr)
    GWEN_Buffer_AppendBytes(buf,
			    (const char*)eu->signaturePtr,
			    eu->signatureLen);

  /* user id */
  if (eu->userId) {
    l=strlen(eu->userId);
    if (l)
      GWEN_Buffer_AppendString(buf, eu->userId);
  }
  else
    l=0;
  if (l<8)
    GWEN_Buffer_FillWithBytes(buf, 32, 8-l);

  /* original file */
  if (eu->originalFileName) {
    l=strlen(eu->originalFileName);
    if (l)
      GWEN_Buffer_AppendString(buf, eu->originalFileName);
  }
  else
    l=0;
  if (l<128)
    GWEN_Buffer_FillWithBytes(buf, 32, 128-l);

  /* creation time */
  if (eu->creationTime)
    GWEN_Time_toString(eu->creationTime, "YYYYMMDD hhmmss ", buf);
  else
    GWEN_Buffer_FillWithBytes(buf, 32, 16);

  /* signature time */
  if (eu->signatureTime)
    GWEN_Time_toString(eu->signatureTime, "YYYYMMDD hhmmss ", buf);
  else
    GWEN_Buffer_FillWithBytes(buf, 32, 16);

  /* free use field */
  GWEN_Buffer_FillWithBytes(buf, 0, 8);

  /* RFU */
  GWEN_Buffer_FillWithBytes(buf, 0, 197);
}



int EB_Eu_toDb(const EB_EU *eu, GWEN_DB_NODE *db) {
  assert(eu);
  if (eu->version)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "version", eu->version);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "modLen", eu->modLen);
  if (eu->jobType)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "jobType", eu->jobType);
  if (eu->signaturePtr && eu->signatureLen)
    GWEN_DB_SetBinValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			"signature",
			eu->signaturePtr,
			eu->signatureLen);
  if (eu->userId)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "userId", eu->userId);
  if (eu->originalFileName)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "originalFileName", eu->originalFileName);
  if (eu->creationTime) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "creationTime");
    GWEN_Time_toDb(eu->creationTime, dbT);
  }
  if (eu->signatureTime) {
    GWEN_DB_NODE *dbT;

    dbT=GWEN_DB_GetGroup(db, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "signatureTime");
    GWEN_Time_toDb(eu->signatureTime, dbT);
  }

  return 0;
}



EB_EU *EB_Eu_fromDb(GWEN_DB_NODE *db) {
  EB_EU *eu;
  const char *s;
  const void *p;
  unsigned int l;
  GWEN_DB_NODE *dbT;

  eu=EB_Eu_new();
  s=GWEN_DB_GetCharValue(db, "version", 0, NULL);
  if (s)
    eu->version=strdup(s);
  s=GWEN_DB_GetCharValue(db, "jobType", 0, NULL);
  if (s)
    eu->jobType=strdup(s);
  eu->modLen=GWEN_DB_GetIntValue(db, "modLen", 0, 1024);
  p=GWEN_DB_GetBinValue(db, "signature", 0, NULL, 0, &l);
  if (p && l)
    EB_Eu_SetSignature(eu, p, l);
  s=GWEN_DB_GetCharValue(db, "userId", 0, NULL);
  if (s)
    eu->userId=strdup(s);
  s=GWEN_DB_GetCharValue(db, "originalFileName", 0, NULL);
  if (s)
    eu->originalFileName=strdup(s);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "creationTime");
  if (dbT)
    eu->creationTime=GWEN_Time_fromDb(dbT);
  dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "signatureTime");
  if (dbT)
    eu->signatureTime=GWEN_Time_fromDb(dbT);

  return eu;
}



const char *EB_Eu_GetVersion(const EB_EU *eu) {
  assert(eu);
  return eu->version;
}



void EB_Eu_SetVersion(EB_EU *eu, const char *s) {
  SETSTRING(version);
}



int EB_Eu_GetModLen(const EB_EU *eu) {
  assert(eu);
  return eu->modLen;
}



void EB_Eu_SetModLen(EB_EU *eu, int i) {
  assert(eu);
  eu->modLen=i;
}



const char *EB_Eu_GetJobType(const EB_EU *eu) {
  assert(eu);
  return eu->jobType;
}



void EB_Eu_SetJobType(EB_EU *eu, const char *s) {
  SETSTRING(jobType);
}



const uint8_t *EB_Eu_GetSignaturePtr(const EB_EU *eu) {
  assert(eu);
  return eu->signaturePtr;
}



uint32_t EB_Eu_GetSignatureLen(const EB_EU *eu) {
  assert(eu);
  return eu->signatureLen;
}



void EB_Eu_SetSignature(EB_EU *eu, const uint8_t *p, uint32_t l) {
  assert(eu);
  if (eu->signaturePtr && eu->signatureLen)
    free(eu->signaturePtr);
  if (p && l) {
    eu->signaturePtr=(uint8_t*)malloc(l);
    assert(eu->signaturePtr);
    memmove(eu->signaturePtr, p, l);
    eu->signatureLen=l;
  }
  else {
    eu->signaturePtr=NULL;
    eu->signatureLen=0;
  }
}



const char *EB_Eu_GetUserId(const EB_EU *eu) {
  assert(eu);
  return eu->userId;
}



void EB_Eu_SetUserId(EB_EU *eu, const char *s) {
  SETSTRING(userId);
}



const char *EB_Eu_GetOriginalFileName(const EB_EU *eu) {
  assert(eu);
  return eu->originalFileName;
}



void EB_Eu_SetOriginalFileName(EB_EU *eu, const char *s) {
  SETSTRING(originalFileName);
}



const GWEN_TIME *EB_Eu_GetCreationTime(const EB_EU *eu) {
  assert(eu);
  return eu->creationTime;
}



void EB_Eu_SetCreationTime(EB_EU *eu, const GWEN_TIME *ti) {
  assert(eu);
  GWEN_Time_free(eu->creationTime);
  if (ti)
    eu->creationTime=GWEN_Time_dup(ti);
  else
    eu->creationTime=NULL;
}



const GWEN_TIME *EB_Eu_GetSignatureTime(const EB_EU *eu) {
  assert(eu);
  return eu->signatureTime;
}



void EB_Eu_SetSignatureTime(EB_EU *eu, const GWEN_TIME *ti) {
  assert(eu);
  GWEN_Time_free(eu->signatureTime);
  if (ti)
    eu->signatureTime=GWEN_Time_dup(ti);
  else
    eu->signatureTime=NULL;
}









