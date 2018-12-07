/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "generic_p.h"
#include "i18n_l.h"

#include <aqbanking/banking_be.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/syncio_file.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC)



AB_BANKINFO_PLUGIN *AB_BankInfoPluginGENERIC_new(AB_BANKING *ab,
                                                 const char *country){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_GENERIC *bde;

  assert(country);
  bip=AB_BankInfoPlugin_new(country);
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_GENERIC, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                       bip, bde, AB_BankInfoPluginGENERIC_FreeData);

  bde->banking=ab;
  bde->country=strdup(country);
  AB_BankInfoPlugin_SetGetBankInfoFn(bip, AB_BankInfoPluginGENERIC_GetBankInfo);
  AB_BankInfoPlugin_SetGetBankInfoByTemplateFn(bip,
                                               AB_BankInfoPluginGENERIC_SearchbyTemplate);

  return bip;
}



void GWENHYWFAR_CB AB_BankInfoPluginGENERIC_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_GENERIC *bde;

  bde=(AB_BANKINFO_PLUGIN_GENERIC*)p;
  free(bde->country);
  if (bde->dataDir) free(bde->dataDir);

  GWEN_FREE_OBJECT(bde);
}



void AB_BankInfoPluginGENERIC__GetDataDir(AB_BANKINFO_PLUGIN *bip,
                                          GWEN_BUFFER *pbuf) {
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  int gotit=0;

  assert(pbuf);

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  if (bde->dataDir) {
    gotit=1;
    GWEN_Buffer_AppendString(pbuf, bde->dataDir);
  }
  else {
    GWEN_STRINGLIST *sl;

    sl=AB_Banking_GetGlobalDataDirs();
    if (sl) {
      GWEN_STRINGLISTENTRY *se;
      GWEN_BUFFER *buf;

      buf=GWEN_Buffer_new(0, 256, 0, 1);
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
        FILE *f;
        const char *s;
        unsigned int pos;

        s=GWEN_StringListEntry_Data(se);
        GWEN_Buffer_AppendString(buf, s);
	GWEN_Buffer_AppendString(buf,
				 DIRSEP
				 "aqbanking"
				 DIRSEP
				 "bankinfo"
				 DIRSEP);
        GWEN_Buffer_AppendString(buf, bde->country);
        pos=GWEN_Buffer_GetPos(buf);
        GWEN_Buffer_AppendString(buf, DIRSEP);
        GWEN_Buffer_AppendString(buf, "banks.data");
        f=fopen(GWEN_Buffer_GetStart(buf), "r");
        if (f) {
          fclose(f);
          GWEN_Buffer_Crop(buf, 0, pos);
          bde->dataDir=strdup(GWEN_Buffer_GetStart(buf));
          GWEN_Buffer_AppendBuffer(pbuf, buf);
          gotit=1;
          break;
        }
        GWEN_Buffer_Reset(buf);
        se=GWEN_StringListEntry_Next(se);
      }
      GWEN_Buffer_free(buf);
    }
    GWEN_StringList_free(sl);
  }
  assert(gotit);
}



AB_BANKINFO *AB_BankInfoPluginGENERIC__ReadBankInfo(AB_BANKINFO_PLUGIN *bip,
                                                    const char *num){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  GWEN_BUFFER *pbuf;
  AB_BANKINFO *bi;
  GWEN_DB_NODE *dbT;
  GWEN_SYNCIO *sio;
  uint32_t pos;
  int rv;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  /* get position */
  assert(strlen(num)==8);
  if (1!=sscanf(num, "%08x", &pos)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Invalid index");
    return 0;
  }

  /* get path */
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "banks.data");

  sio=GWEN_SyncIo_File_new(GWEN_Buffer_GetStart(pbuf), GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(pbuf);
    return NULL;
  }

  /* seek position */
  DBG_VERBOUS(0, "Seeking to %08x (%d)", pos, pos);
  if ((int64_t)-1==GWEN_SyncIo_File_Seek(sio, pos, GWEN_SyncIo_File_Whence_Set)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "seek(%s, %u): %s",
	      GWEN_Buffer_GetStart(pbuf),
              pos,
	      strerror(errno));
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return NULL;
  }

  /* read data */
  dbT=GWEN_DB_Group_new("bank");
  rv=GWEN_DB_ReadFromIo(dbT, sio,
			GWEN_DB_FLAGS_DEFAULT |
			GWEN_PATH_FLAGS_CREATE_GROUP|
			GWEN_DB_FLAGS_UNTIL_EMPTY_LINE);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Could not load file \"%s\" (%d)", GWEN_Buffer_GetStart(pbuf), rv);
    GWEN_DB_Group_free(dbT);
    GWEN_Buffer_free(pbuf);
    return 0;
  }

  bi=AB_BankInfo_fromDb(dbT);
  assert(bi);
  GWEN_DB_Group_free(dbT);
  GWEN_Buffer_free(pbuf);

  return bi;
}



AB_BANKINFO *AB_BankInfoPluginGENERIC_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                                  const char *branchId,
                                                  const char *bankId){
  return AB_BankInfoPluginGENERIC__SearchbyCode(bip, bankId);
}



AB_BANKINFO *AB_BankInfoPluginGENERIC__SearchbyCode(AB_BANKINFO_PLUGIN *bip,
                                                    const char *bankId){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  GWEN_BUFFER *pbuf;
  FILE *f;
  char lbuf[512];

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "blz.idx");
  f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
  if (!f) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "fopen(%s): %s",
             GWEN_Buffer_GetStart(pbuf),
             strerror(errno));
    GWEN_Buffer_free(pbuf);
    return 0;
  }

  while(!feof(f)) {
    unsigned char *p;

    lbuf[0]=0;
    p=(unsigned char*)fgets(lbuf, sizeof(lbuf), f);
    if (p) {
      char *blz=0;
      char *num=0;
      unsigned int i;

      i=strlen(lbuf);
      if (lbuf[i-1]==10)
        lbuf[i-1]=0;
      blz=(char*)p;
      while(*p && *p!='\t')
        p++;
      assert(*p=='\t');
      *p=0;
      p++;
      num=(char*)p;
      if (strcasecmp(blz, bankId)==0) {
        AB_BANKINFO *bi;

        bi=AB_BankInfoPluginGENERIC__ReadBankInfo(bip, num);
        fclose(f);
        GWEN_Buffer_free(pbuf);
        return bi;
      }
    }
  }
  fclose(f);
  DBG_INFO(AQBANKING_LOGDOMAIN, "Bank %s not found", bankId);
  return 0;
}



int AB_BankInfoPluginGENERIC__AddById(AB_BANKINFO_PLUGIN *bip,
                                      const char *bankId,
                                      AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  GWEN_BUFFER *pbuf;
  FILE *f;
  char lbuf[512];
  uint32_t count=0;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "blz.idx");
  f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
  if (!f) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "fopen(%s): %s",
             GWEN_Buffer_GetStart(pbuf),
             strerror(errno));
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  while(!feof(f)) {
    unsigned char *p;

    lbuf[0]=0;
    p=(unsigned char*)fgets(lbuf, sizeof(lbuf), f);
    if (p) {
      char *blz=0;
      char *num=0;
      unsigned int i;

      i=strlen(lbuf);
      if (lbuf[i-1]==10)
        lbuf[i-1]=0;
      blz=(char*)p;
      while(*p && *p!='\t')
        p++;
      assert(*p=='\t');
      *p=0;
      p++;
      num=(char*)p;
      i=strlen(lbuf);
      if (GWEN_Text_ComparePattern(blz, bankId, 0)!=-1) {
        AB_BANKINFO *bi;

        bi=AB_BankInfoPluginGENERIC__ReadBankInfo(bip, num);
        if (bi) {
          AB_BankInfo_List2_PushBack(bl, bi);
          count++;
        }
      }
    }
  } /* while ! feof */
  fclose(f);
  if (!count) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bank %s not found", bankId);
    return GWEN_ERROR_NOT_FOUND;
  }
  return 0;
}



int AB_BankInfoPluginGENERIC__AddByBic(AB_BANKINFO_PLUGIN *bip,
                                       const char *bic,
                                       AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  GWEN_BUFFER *pbuf;
  FILE *f;
  char lbuf[512];
  uint32_t count=0;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "bic.idx");
  f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
  if (!f) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "fopen(%s): %s",
             GWEN_Buffer_GetStart(pbuf),
             strerror(errno));
    GWEN_Buffer_free(pbuf);
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  while(!feof(f)) {
    unsigned char *p;

    lbuf[0]=0;
    p=(unsigned char*)fgets(lbuf, sizeof(lbuf), f);
    if (p) {
      char *key=0;
      char *num=0;
      unsigned int i;

      i=strlen(lbuf);
      if (lbuf[i-1]==10)
        lbuf[i-1]=0;
      key=(char*)p;
      while(*p && *p!='\t')
        p++;
      assert(*p=='\t');
      *p=0;
      p++;
      num=(char*)p;
      if (GWEN_Text_ComparePattern(key, bic, 0)!=-1) {
        AB_BANKINFO *bi;

        bi=AB_BankInfoPluginGENERIC__ReadBankInfo(bip, num);
        if (bi) {
          AB_BankInfo_List2_PushBack(bl, bi);
          count++;
        }
      }
    }
  } /* while ! feof */
  fclose(f);
  if (!count) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bank %s not found", bic);
    return GWEN_ERROR_NOT_FOUND;
  }
  return 0;
}



int AB_BankInfoPluginGENERIC__AddByNameAndLoc(AB_BANKINFO_PLUGIN *bip,
                                              const char *name,
                                              const char *loc,
                                              AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  GWEN_BUFFER *pbuf;
  FILE *f;
  char lbuf[512];
  uint32_t count=0;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  if (name==0)
    name="*";
  if (loc==0)
    loc="*";

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "namloc.idx");
  f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
  if (!f) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "fopen(%s): %s",
             GWEN_Buffer_GetStart(pbuf),
             strerror(errno));
    GWEN_Buffer_free(pbuf);
    DBG_ERROR(AQBANKING_LOGDOMAIN, "namloc index file not available");
    return GWEN_ERROR_NOT_AVAILABLE;
  }

  while(!feof(f)) {
    unsigned char *p;

    lbuf[0]=0;
    p=(unsigned char*)fgets(lbuf, sizeof(lbuf), f);
    if (p) {
      char *key1=0;
      char *key2=0;
      char *num=0;
      unsigned int i;

      i=strlen(lbuf);
      if (lbuf[i-1]==10)
        lbuf[i-1]=0;
      key1=(char*)p;
      while(*p && *p!='\t')
        p++;
      assert(*p=='\t');
      *p=0;
      p++;
      key2=/* GCC4 pointer-signedness fix: */ (char*) p;
      while(*p && *p!='\t')
        p++;
      assert(*p=='\t');
      *p=0;
      p++;
      num=(char*)p;
      if (GWEN_Text_ComparePattern(key1, name, 0)!=-1 &&
          GWEN_Text_ComparePattern(key2, loc, 0)!=-1) {
        AB_BANKINFO *bi;

        bi=AB_BankInfoPluginGENERIC__ReadBankInfo(bip, num);
	if (bi) {
          AB_BankInfo_List2_PushBack(bl, bi);
          count++;
        }
      }
    }
  } /* while ! feof */
  fclose(f);
  if (!count) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Bank %s/%s not found", name, loc);
    return GWEN_ERROR_NOT_FOUND;
  }
  return 0;
}



int AB_BankInfoPluginGENERIC__CmpTemplate(AB_BANKINFO *bi,
                                          const AB_BANKINFO *tbi,
                                          uint32_t flags) {
  const char *s;
  const char *t;
  
  if (flags & AB_BANKINFO_GENERIC__FLAGS_BRANCHID) {
    s=AB_BankInfo_GetBranchId(bi);
    t=AB_BankInfo_GetBranchId(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  
  if (flags & AB_BANKINFO_GENERIC__FLAGS_BANKID) {
    s=AB_BankInfo_GetBankId(bi);
    t=AB_BankInfo_GetBankId(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_BIC) {
    s=AB_BankInfo_GetBic(bi);
    t=AB_BankInfo_GetBic(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_BANKNAME) {
    s=AB_BankInfo_GetBankName(bi);
    t=AB_BankInfo_GetBankName(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_LOCATION) {
    s=AB_BankInfo_GetLocation(bi);
    t=AB_BankInfo_GetLocation(tbi);
    if (!t || !*t)
      t=AB_BankInfo_GetCity(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_ZIPCODE) {
    s=AB_BankInfo_GetZipcode(bi);
    t=AB_BankInfo_GetZipcode(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_REGION) {
    s=AB_BankInfo_GetRegion(bi);
    t=AB_BankInfo_GetRegion(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_PHONE) {
    s=AB_BankInfo_GetPhone(bi);
    t=AB_BankInfo_GetPhone(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_FAX) {
    s=AB_BankInfo_GetFax(bi);
    t=AB_BankInfo_GetFax(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_EMAIL) {
    s=AB_BankInfo_GetEmail(bi);
    t=AB_BankInfo_GetEmail(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }
  if (flags & AB_BANKINFO_GENERIC__FLAGS_WEBSITE) {
    s=AB_BankInfo_GetWebsite(bi);
    t=AB_BankInfo_GetWebsite(tbi);
    if (s && *s)
      if (GWEN_Text_ComparePattern(s, t, 0)==-1)
        return 0;
  }

  return 1;
}



int AB_BankInfoPluginGENERIC_AddByTemplate(AB_BANKINFO_PLUGIN *bip,
                                           AB_BANKINFO *tbi,
                                           AB_BANKINFO_LIST2 *bl,
                                           uint32_t flags){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  uint32_t count=0;
  uint32_t i=0;
  GWEN_BUFFER *pbuf;
  uint32_t progressId;
  GWEN_SYNCIO *sio;
  int rv;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  /* get path */
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  AB_BankInfoPluginGENERIC__GetDataDir(bip, pbuf);
  GWEN_Buffer_AppendString(pbuf, DIRSEP "banks.data");

  /* open file */
  sio=GWEN_SyncIo_File_new(GWEN_Buffer_GetStart(pbuf), GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  progressId=GWEN_Gui_ProgressStart(GWEN_GUI_PROGRESS_DELAY |
				    GWEN_GUI_PROGRESS_ALLOW_EMBED |
				    GWEN_GUI_PROGRESS_SHOW_PROGRESS |
				    GWEN_GUI_PROGRESS_SHOW_ABORT,
				    I18N("Scanning bank database..."),
				    NULL,
				    GWEN_GUI_PROGRESS_NONE,
				    0);

  for (;;) {
    GWEN_DB_NODE *dbT;
    AB_BANKINFO *bi;

    if (i & ~63) {
      if (GWEN_Gui_ProgressAdvance(progressId, GWEN_GUI_PROGRESS_NONE)==
	  GWEN_ERROR_USER_ABORTED) {
	DBG_INFO(GWEN_LOGDOMAIN, "User aborted");
	GWEN_Gui_ProgressEnd(progressId);
	DBG_ERROR(AQBANKING_LOGDOMAIN,
		  "Aborted by user");
	GWEN_SyncIo_Disconnect(sio);
	GWEN_SyncIo_free(sio);
	GWEN_Buffer_free(pbuf);
	return GWEN_ERROR_USER_ABORTED;
      }
    }

    dbT=GWEN_DB_Group_new("bank");
    if (GWEN_DB_ReadFromIo(dbT, sio,
			   GWEN_DB_FLAGS_DEFAULT |
			   GWEN_PATH_FLAGS_CREATE_GROUP |
			   GWEN_DB_FLAGS_UNTIL_EMPTY_LINE)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not read from file \"%s\"",
                GWEN_Buffer_GetStart(pbuf));
      GWEN_Gui_ProgressEnd(progressId);
      GWEN_DB_Group_free(dbT);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      GWEN_Buffer_free(pbuf);
      return GWEN_ERROR_GENERIC;
    }

    bi=AB_BankInfo_fromDb(dbT);
    assert(bi);
    if (AB_BankInfoPluginGENERIC__CmpTemplate(bi, tbi, flags)==1) {
      count++;
      AB_BankInfo_List2_PushBack(bl, bi);
    }
    else {
      AB_BankInfo_free(bi);
    }
    GWEN_DB_Group_free(dbT);
    i++;
  } /* while */

  GWEN_Gui_ProgressEnd(progressId);

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);
  GWEN_Buffer_free(pbuf);

  if (count==0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No matching bank found");
    return GWEN_ERROR_NOT_FOUND;
  }

  return 0;
}



int AB_BankInfoPluginGENERIC_SearchbyTemplate(AB_BANKINFO_PLUGIN *bip,
                                              AB_BANKINFO *tbi,
                                              AB_BANKINFO_LIST2 *bl){
  AB_BANKINFO_PLUGIN_GENERIC *bde;
  uint32_t flags;
  int rv;
  const char *s;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_GENERIC,
                           bip);
  assert(bde);

  /* this is just to speed up often needed tests */
  flags=0;
  s=AB_BankInfo_GetBranchId(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_BRANCHID;
  s=AB_BankInfo_GetBankId(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_BANKID;
  s=AB_BankInfo_GetBic(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_BIC;
  s=AB_BankInfo_GetBankName(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_BANKNAME;
  s=AB_BankInfo_GetLocation(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_LOCATION;
  s=AB_BankInfo_GetStreet(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_STREET;
  s=AB_BankInfo_GetZipcode(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_ZIPCODE;
  s=AB_BankInfo_GetCity(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_CITY;
  s=AB_BankInfo_GetRegion(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_REGION;
  s=AB_BankInfo_GetPhone(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_PHONE;
  s=AB_BankInfo_GetFax(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_FAX;
  s=AB_BankInfo_GetEmail(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_EMAIL;
  s=AB_BankInfo_GetWebsite(tbi);
  if (s && *s)
    flags|=AB_BANKINFO_GENERIC__FLAGS_WEBSITE;

  if (flags==AB_BANKINFO_GENERIC__FLAGS_BIC)
    rv=AB_BankInfoPluginGENERIC__AddByBic(bip,
                                          AB_BankInfo_GetBic(tbi),
                                          bl);
  else if ((flags & ~AB_BANKINFO_GENERIC__FLAGS_BRANCHID)==
           AB_BANKINFO_GENERIC__FLAGS_BANKID)
    rv=AB_BankInfoPluginGENERIC__AddById(bip,
                                         AB_BankInfo_GetBankId(tbi),
                                         bl);
  else if (flags==(AB_BANKINFO_GENERIC__FLAGS_BANKNAME|
                   AB_BANKINFO_GENERIC__FLAGS_LOCATION) ||
           flags==AB_BANKINFO_GENERIC__FLAGS_BANKNAME ||
           flags==AB_BANKINFO_GENERIC__FLAGS_LOCATION) {
    rv=AB_BankInfoPluginGENERIC__AddByNameAndLoc(bip,
                                                 AB_BankInfo_GetBankName(tbi),
                                                 AB_BankInfo_GetLocation(tbi),
                                                 bl);
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No quick search implemented for these flags (%08x)", flags);
    rv=GWEN_ERROR_NOT_AVAILABLE;
  }
  if (rv==GWEN_ERROR_NOT_AVAILABLE) {
    rv=AB_BankInfoPluginGENERIC_AddByTemplate(bip, tbi, bl, flags);
  }

  return rv;
}











