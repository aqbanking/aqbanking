/***************************************************************************
    begin       : Mon Feb 25 2008
    copyright   : (C) 2008 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/*#define DEBUG_OHBCI_MODULE*/



#include "ohbci_p.h"
#include "i18n_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/padd.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/ctfile_be.h>
#include <gwenhywfar/ctplugin_be.h>
#include <gwenhywfar/ctf_context_be.h>
#include <gwenhywfar/text.h> /* DEBUG */
#include <gwenhywfar/cryptkeysym.h>
#include <gwenhywfar/cryptkeyrsa.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef OS_WIN32
# define ftruncate chsize
#endif


GWEN_INHERIT(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_RDH2)



GWEN_PLUGIN *ct_rdh2_factory(GWEN_PLUGIN_MANAGER *pm,
			     const char *modName,
			     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_Crypt_TokenRDH2_Plugin_new(pm, modName, fileName);
  assert(pl);

  return pl;
}



GWEN_PLUGIN *GWEN_Crypt_TokenRDH2_Plugin_new(GWEN_PLUGIN_MANAGER *pm,
					     const char *modName,
					     const char *fileName) {
  GWEN_PLUGIN *pl;

  pl=GWEN_Crypt_Token_Plugin_new(pm,
				 GWEN_Crypt_Token_Device_File,
				 modName,
				 fileName);

  /* set virtual functions */
  GWEN_Crypt_Token_Plugin_SetCreateTokenFn(pl,
					   GWEN_Crypt_TokenRDH2_Plugin_CreateToken);
  GWEN_Crypt_Token_Plugin_SetCheckTokenFn(pl,
					  GWEN_Crypt_TokenRDH2_Plugin_CheckToken);

  return pl;
}



GWEN_CRYPT_TOKEN* GWENHYWFAR_CB
GWEN_Crypt_TokenRDH2_Plugin_CreateToken(GWEN_PLUGIN *pl,
					const char *name) {
  GWEN_PLUGIN_MANAGER *pm;
  GWEN_CRYPT_TOKEN *ct;

  assert(pl);

  pm=GWEN_Plugin_GetManager(pl);
  assert(pm);

  ct=GWEN_Crypt_TokenRDH2_new(pm, name);
  assert(ct);

  return ct;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenRDH2_Plugin_CheckToken(GWEN_PLUGIN *pl,
				       GWEN_BUFFER *name) {
  FILE *f;
  const char *p;
  char buffer[30];
  int rv;
  unsigned int tag;

  if (GWEN_Buffer_GetUsedBytes(name)==0) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Empty name");
    return GWEN_ERROR_BAD_NAME;
  }

  p=GWEN_Buffer_GetStart(name);
  if (access(p, F_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File [%s] does not exist", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info, "File does not exist");
    return GWEN_ERROR_BAD_NAME;
  }

  if (access(p, R_OK | W_OK)) {
    DBG_ERROR(GWEN_LOGDOMAIN, "File exists but I have no writes on it");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Info,
			 "File exists but I have no writes on it");
    return GWEN_ERROR_IO;
  }

  f=fopen(p, "rb");
  if (!f) {
    DBG_ERROR(GWEN_LOGDOMAIN,
              "File exists, I have all rights but still can't open it");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "File exists, I have all rights but "
			 "still can't open it");
    return GWEN_ERROR_IO;
  }

  rv=fread(buffer, sizeof(buffer), 1, f);
  fclose(f);
  if (rv!=1) {
    DBG_INFO(GWEN_LOGDOMAIN, "This seems not to be an RDH2 keyfile (bad size)");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "This seems not to be an RDH2 keyfile "
			 "(bad size)");
    return GWEN_ERROR_NOT_SUPPORTED;
  }

  tag=((unsigned char)(buffer[1])+(((unsigned char)(buffer[0]))<<8));
  if (tag==RDH2_DISK_HEADER) {
    unsigned int version;

    version=((unsigned char)(buffer[5])+(((unsigned char)(buffer[4]))<<8));
    DBG_INFO(GWEN_LOGDOMAIN,
	     "RDH2 file (%04x) detected");
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
			 "RDH2 file detected");
    return 0;
  }

  DBG_INFO(GWEN_LOGDOMAIN,
	   "This seems not to be an RDH2 keyfile");
  GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Notice,
		       "This seems not to be an RDH2 keyfile");
  return GWEN_ERROR_NOT_SUPPORTED;
}





GWEN_CRYPT_TOKEN *GWEN_Crypt_TokenRDH2_new(GWEN_PLUGIN_MANAGER *pm,
					   const char *name){
  GWEN_CRYPT_TOKEN *ct;
  GWEN_CRYPT_TOKEN_RDH2 *lct;

  ct=GWEN_Crypt_TokenFile_new("rdh2", name);

  GWEN_NEW_OBJECT(GWEN_CRYPT_TOKEN_RDH2, lct);
  GWEN_INHERIT_SETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_RDH2,
                       ct, lct,
                       GWEN_Crypt_TokenRDH2_FreeData);
  lct->mediumTag=GWEN_CRYPT_TOKEN_RDH2_TAG_MEDIUM3;
  lct->vminor=GWEN_CRYPT_TOKEN_RDH2_VMINOR;
  lct->cryptoTag=GWEN_CRYPT_TOKEN_RDH2_TAG_CRYPT_BF;

  /* set virtual functions */
  lct->openFn=GWEN_Crypt_Token_SetOpenFn(ct, GWEN_Crypt_TokenRDH2_Open);
  lct->closeFn=GWEN_Crypt_Token_SetCloseFn(ct, GWEN_Crypt_TokenRDH2_Close);
  lct->createFn=GWEN_Crypt_Token_SetCreateFn(ct, GWEN_Crypt_TokenRDH2_Create);
  GWEN_Crypt_Token_SetChangePinFn(ct, GWEN_Crypt_TokenRDH2_ChangePin);

  GWEN_Crypt_TokenFile_SetReadFn(ct, GWEN_Crypt_TokenRDH2_Read);
  GWEN_Crypt_TokenFile_SetWriteFn(ct, GWEN_Crypt_TokenRDH2_Write);

  return ct;
}



void GWENHYWFAR_CB GWEN_Crypt_TokenRDH2_FreeData(void *bp, void *p) {
  GWEN_CRYPT_TOKEN_RDH2 *lct;

  lct=(GWEN_CRYPT_TOKEN_RDH2*) p;
  memset(lct->password, 0, sizeof(lct->password));
  GWEN_FREE_OBJECT(lct);
}




int GWEN_Crypt_TokenRDH2__EnsurePassword(GWEN_CRYPT_TOKEN *ct,
					 int trynum,
					 int confirm,
					 uint32_t gid){
  GWEN_CRYPT_TOKEN_RDH2 *lct;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_RDH2, ct);
  assert(lct);

  if (lct->passWordIsSet==0) {
    char password[64];
    int rv;
    unsigned int pinLength=0;
    uint32_t flags;

    /* create key from password */
    memset(lct->password, 0, sizeof(lct->password));

    flags=0;
    if (trynum)
      flags|=GWEN_GUI_INPUT_FLAGS_RETRY;
    if (confirm)
      flags|=GWEN_GUI_INPUT_FLAGS_CONFIRM;
    rv=GWEN_Crypt_Token_GetPin(ct,
			       GWEN_Crypt_PinType_Access,
			       GWEN_Crypt_PinEncoding_Ascii,
                               flags,
			       (unsigned char*)password,
			       RDH2_PINMINLENGTH,
			       sizeof(password)-1,
			       &pinLength,
			       gid);
    if (rv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Error asking for PIN, aborting (%d)", rv);
      return rv;
    }

    if (strlen(password)<RDH2_PINMINLENGTH) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Your program returned a shorter PIN than instructed!");
      return GWEN_ERROR_GENERIC;
    }

    /* TODO: derive key */

    lct->passWordIsSet=1;
  }

  return 0;
}



int GWEN_Crypt_TokenRDH2__DecodeBankData(GWEN_CRYPT_TOKEN *ct,
					 TLV *bankTlv) {
  GWEN_CRYPT_TOKEN_RDH2 *lct;
  const uint8_t *p;
  int size;
  GWEN_BUFFER *dbuf;
  uint16_t signSeq=0;
  uint8_t keyStatus=0;
  uint32_t nextContextId=1;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_RDH2, ct);
  assert(lct);

  p=Tlv_GetTagData(bankTlv);
  size=Tlv_GetTagLength(bankTlv);
  if (size<2) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Tag too small to contain any subtag");
    return GWEN_ERROR_BAD_DATA;
  }

  /* create static buffer */
  dbuf=GWEN_Buffer_new((char*)p, size, size, 0);
  GWEN_Buffer_SubMode(dbuf, GWEN_BUFFER_MODE_DYNAMIC);

  while(GWEN_Buffer_GetBytesLeft(dbuf)) {
    TLV *tlv;

    tlv=Tlv_fromBuffer(dbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad file (no TLV)");
      return GWEN_ERROR_BAD_DATA;
    }

    switch(Tlv_GetTagType(tlv)) {
    case RDH2_USER_DATA_INFO: {
      GWEN_CRYPT_TOKEN_CONTEXT *fct;
      GWEN_BUFFER *tbuf;
      const char *pp;
      const char *t;
      uint8_t serviceType;

      fct=GWEN_CTF_Context_new();
      tbuf=GWEN_Buffer_new(0, 128, 0, 1);
      pp=Tlv_GetTagData(tlv);

      /* skip country */
      pp+=3;

      /* bank code */
      GWEN_Buffer_AppendBytes(tbuf, pp, 30);
      GWEN_Text_CondenseBuffer(tbuf);
      t=GWEN_Buffer_GetStart(tbuf);
      if (*t)
	GWEN_Crypt_Token_Context_SetServiceId(fct, t);
      GWEN_Buffer_Reset(tbuf);
      pp+=30;

      /* bank name */
      GWEN_Buffer_AppendBytes(tbuf, pp, 60);
      GWEN_Text_CondenseBuffer(tbuf);
      t=GWEN_Buffer_GetStart(tbuf);
      if (*t)
	GWEN_Crypt_Token_Context_SetPeerName(fct, t);
      GWEN_Buffer_Reset(tbuf);
      pp+=60;

      /* user id */
      GWEN_Buffer_AppendBytes(tbuf, pp, 30);
      GWEN_Text_CondenseBuffer(tbuf);
      t=GWEN_Buffer_GetStart(tbuf);
      if (*t)
	GWEN_Crypt_Token_Context_SetUserId(fct, t);
      GWEN_Buffer_Reset(tbuf);
      pp+=30;

      /* skip customer id */
      pp+=30;

      /* system id */
      GWEN_Buffer_AppendBytes(tbuf, pp, 50);
      GWEN_Text_CondenseBuffer(tbuf);
      t=GWEN_Buffer_GetStart(tbuf);
      if (*t)
	GWEN_Crypt_Token_Context_SetSystemId(fct, t);
      GWEN_Buffer_Reset(tbuf);
      pp+=50;

      /* service type */
      serviceType=*pp;
      pp++;

      /* com address */
      GWEN_Buffer_AppendBytes(tbuf, pp, 50);
      GWEN_Text_CondenseBuffer(tbuf);
      t=GWEN_Buffer_GetStart(tbuf);
      if (*t)
	GWEN_Crypt_Token_Context_SetAddress(fct, t);
      GWEN_Buffer_Reset(tbuf);
      pp+=50;

      switch(serviceType) {
      case 3: /* pin/tan */
	GWEN_Crypt_Token_Context_SetPort(fct, 443);
	break;
      case 2: /* BTX */
      case 1: /* real HBCI */
      default:
	GWEN_Crypt_Token_Context_SetPort(fct, 3000);
	break;
      }

      /* signature counter */
      signSeq=pp[0]+(pp[1]<<8);
      pp+=2;

      /* key status */
      keyStatus=*pp;
      pp++;

      /* insert more here */

      /* all info read */
      GWEN_Buffer_free(tbuf);

      /* finalize context */
      GWEN_Crypt_Token_Context_SetId(fct, nextContextId++);
      GWEN_Crypt_Token_Context_SetSignKeyId(fct, 0x01);
      GWEN_Crypt_Token_Context_SetDecipherKeyId(fct, 0x02);
      GWEN_Crypt_Token_Context_SetVerifyKeyId(fct, 0x03);
      GWEN_Crypt_Token_Context_SetEncipherKeyId(fct, 0x04);
      GWEN_Crypt_Token_Context_SetAuthSignKeyId(fct, 0x05);
      GWEN_Crypt_Token_Context_SetAuthVerifyKeyId(fct, 0x06);

      /* add context */
      GWEN_Crypt_TokenFile_AddContext(ct, fct);

      break;
    }

    case RDH2_USER_DATA_KEY: {
      DBG_ERROR(0, "TODO: Read user keys");
      break;
    }

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", Tlv_GetTagType(tlv));
      break;
    }

    Tlv_free(tlv);
  }

  return 0;
}



int GWENHYWFAR_CB 
GWEN_Crypt_TokenRDH2_Read(GWEN_CRYPT_TOKEN *ct, int fd, uint32_t gid){
  GWEN_CRYPT_TOKEN_RDH2 *lct;
  GWEN_BUFFER *rbuf;
  unsigned char c;
  GWEN_TAG16 *tlv;
  int i;

  assert(ct);
  lct=GWEN_INHERIT_GETDATA(GWEN_CRYPT_TOKEN, GWEN_CRYPT_TOKEN_RDH2, ct);
  assert(lct);

  rbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  /* read file into rbuf */
  while(1) {
    char buffer[256];
    int rv;

    rv=read(fd, buffer, sizeof(buffer));
    if (rv==-1) {
      DBG_ERROR(GWEN_LOGDOMAIN, "read: %s", strerror(errno));
      return GWEN_ERROR_IO;
    }
    if (rv==0)
      break;
    GWEN_Buffer_AppendBytes(rbuf, buffer, rv);
  }

  if (GWEN_Buffer_GetUsedBytes(rbuf)<4) {
    DBG_ERROR(GWEN_LOGDOMAIN, "This seems not to be an RDH2 key file");
    GWEN_Buffer_free(rbuf);
    return -1;
  }

  GWEN_Buffer_Rewind(rbuf);

  while(GWEN_Buffer_GetBytesLeft(rbuf)) {
    const uint8_t *p;
    TLV *tlv;
    unsigned int l;

    tlv=Tlv_fromBuffer(rbuf, 0);
    if (!tlv) {
      DBG_ERROR(GWEN_LOGDOMAIN, "Bad file (no TLV)");
      return GWEN_ERROR_BAD_DATA;
    }

    p=Tlv_GetTagData(tlv);

    switch(Tlv_GetTagType(tlv)) {
    case RDH2_DISK_HEADER:
      if (Tlv_GetTagLength(tlv)<26) {
	DBG_ERROR(GWEN_LOGDOMAIN, "Bad file (disk header too short)");
	return GWEN_ERROR_BAD_DATA;
      }
      lct->version=(p[0+1]<<8)+p[0];
      memmove(lct->salt, p+2, 20);
      lct->iterations=
	p[2+20]+
	(p[2+20+1]<<8)+
	(p[2+20+2]<<16)+
	(p[2+20+3]<<24);
      break;

    case RDH2_USER_DATA:
      break;

    case RDH2_BANK_KEY:
      break;

    case RDH2_DATE:
      break;

    case RDH2_MAC:
      break;

    default:
      DBG_WARN(GWEN_LOGDOMAIN, "Unknown tag %02x", Tlv_GetTagType(tlv));
      break;
    } /* switch */

    Tlv_free(tlv);
  } /* while */
  GWEN_Buffer_free(rbuf);

  lct->justCreated=0;

  return 0;
}




