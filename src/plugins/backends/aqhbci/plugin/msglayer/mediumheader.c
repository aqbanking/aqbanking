/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/misc.h>

#include "mediumheader_p.h"
#include "aqhbci_l.h"
#include <aqhbci/aqhbci.h>
#include <gwenhywfar/debug.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <strings.h>



AH_MEDIUMHEADER *AH_MediumHeader_new(const char *mtype,
                                     GWEN_TYPE_UINT32 vmajor,
                                     GWEN_TYPE_UINT32 vminor){
  AH_MEDIUMHEADER *mh;

  assert(mtype);
  GWEN_NEW_OBJECT(AH_MEDIUMHEADER, mh);
  mh->typName=strdup(mtype);
  mh->vmajor=vmajor;
  mh->vminor=vminor;

  return mh;
}



void AH_MediumHeader_free(AH_MEDIUMHEADER *mh){
  if (mh) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Destroying AH_MEDIUMHEADER");
    free(mh->typName);
    GWEN_FREE_OBJECT(mh);
  }
}



AH_MEDIUMHEADER *AH_MediumHeader_FromBuffer(GWEN_BUFFER *hbuf) {
  AH_MEDIUMHEADER *mh;
  const char *p;
  const char *p2;
  GWEN_TYPE_UINT32 size;
  GWEN_TYPE_UINT32 vmajor;
  GWEN_TYPE_UINT32 vminor;
  char *typname;

  p=GWEN_Buffer_GetStart(hbuf);
  size=GWEN_Buffer_GetUsedBytes(hbuf);
  if (size<4) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Too few bytes, not a medium header");
    return 0;
  }
  if (strncasecmp(p, AH_MEDIUMHEADER_MAGIC, 4)!=0) {
    DBG_DEBUG(AQHBCI_LOGDOMAIN, "Bad magic value, not a medium header");
    return 0;
  }

  p+=4;
  if (*p!=':') {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad medium header (missing ':')");
    return 0;
  }
  p++;

  /* get type name */
  p2=p;
  while(*p2 && *p2!=':') p2++;
  if (!*p2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad medium header (missing type name)");
    return 0;
  }
  if (*p2!=':') {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Bad medium header (missing ':')");
    return 0;
  }
  typname=(char*)malloc(p2-p+1);
  memmove(typname, p, p2-p);
  typname[p2-p]=0;
  p=p2+1;

  /* get vmajor and vminor */
  if (sscanf(p, "%u:%u", &vmajor, &vminor)!=2) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "Bad medium header (missing version information)");
    free(typname);
    return 0;
  }

  while(*p && *p!='\n') p++;

  GWEN_Buffer_SetPos(hbuf, p-GWEN_Buffer_GetStart(hbuf)+1);
  mh=AH_MediumHeader_new(typname, vmajor, vminor);
  free(typname);
  return mh;
}



void AH_MediumHeader_ToBuffer(AH_MEDIUMHEADER *mh,
                              GWEN_BUFFER *hbuf) {
  char numbuf[32];

  GWEN_Buffer_AppendString(hbuf, AH_MEDIUMHEADER_MAGIC);
  GWEN_Buffer_AppendByte(hbuf, ':');
  GWEN_Buffer_AppendString(hbuf, mh->typName);
  GWEN_Buffer_AppendByte(hbuf, ':');
  snprintf(numbuf, sizeof(numbuf)-1, "%d:%d", mh->vmajor, mh->vminor);
  GWEN_Buffer_AppendString(hbuf, numbuf);
  GWEN_Buffer_AppendByte(hbuf, '\n');
}



const char *AH_MediumHeader_GetMediumType(const AH_MEDIUMHEADER *mh) {
  assert(mh);
  return mh->typName;
}



GWEN_TYPE_UINT32 AH_MediumHeader_GetMajorVersion(const AH_MEDIUMHEADER *mh){
  assert(mh);
  return mh->vmajor;
}



GWEN_TYPE_UINT32 AH_MediumHeader_GetMinorVersion(const AH_MEDIUMHEADER *mh){
  assert(mh);
  return mh->vminor;
}









