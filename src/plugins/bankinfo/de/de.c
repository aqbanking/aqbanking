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


#include "de_p.h"
#include <aqbanking/banking.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/text.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>



GWEN_INHERIT(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE);


AB_BANKINFO_PLUGIN *de_factory(AB_BANKING *ab, GWEN_DB_NODE *db){
  AB_BANKINFO_PLUGIN *bip;
  AB_BANKINFO_PLUGIN_DE *bde;

  bip=AB_BankInfoPlugin_new("de");
  GWEN_NEW_OBJECT(AB_BANKINFO_PLUGIN_DE, bde);
  GWEN_INHERIT_SETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE,
                       bip, bde, AB_BankInfoPluginDE_FreeData);

  bde->banking=ab;
  bde->dbData=db;
  bde->checker=AccountNumberCheck_new();
  if (!bde->checker) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "KtoBlzCheck returned an error");
    AB_BankInfoPlugin_free(bip);
    return 0;
  }
  AB_BankInfoPlugin_SetGetBankInfoFn(bip, AB_BankInfoPluginDE_GetBankInfo);
  AB_BankInfoPlugin_SetCheckAccountFn(bip, AB_BankInfoPluginDE_CheckAccount);
  return bip;
}



void AB_BankInfoPluginDE_FreeData(void *bp, void *p){
  AB_BANKINFO_PLUGIN_DE *bde;

  bde=(AB_BANKINFO_PLUGIN_DE*)p;

  if (bde->checker)
    AccountNumberCheck_delete(bde->checker);
  GWEN_FREE_OBJECT(bde);
}



AB_BANKINFO *AB_BankInfoPluginDE_GetBankInfo(AB_BANKINFO_PLUGIN *bip,
                                             const char *branchId,
                                             const char *bankId){
  AB_BANKINFO_PLUGIN_DE *bde;
  const AccountNumberCheck_Record *r;

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

  assert(bde->checker);
  r=AccountNumberCheck_findBank(bde->checker, bankId);
  if (r) {
    AB_BANKINFO *bi;

    bi=AB_BankInfo_new();
    AB_BankInfo_SetBranchId(bi, branchId);
    AB_BankInfo_SetBankId(bi, bankId);
    AB_BankInfo_SetBankName(bi, AccountNumberCheck_Record_bankName(r));
    AB_BankInfo_SetLocation(bi, AccountNumberCheck_Record_location(r));
    return bi;
  }
  DBG_INFO(AQBANKING_LOGDOMAIN, "Bank \"%s\" not found", bankId);
  return 0;
}



AB_BANKINFO_CHECKRESULT
AB_BankInfoPluginDE_CheckAccount(AB_BANKINFO_PLUGIN *bip,
                                 const char *branchId,
                                 const char *bankId,
                                 const char *accountId){
  AB_BANKINFO_PLUGIN_DE *bde;
  AccountNumberCheck_Result res;
  AB_BANKINFO_CHECKRESULT cr;

  assert(bankId);
  assert(accountId);

  assert(bip);
  bde=GWEN_INHERIT_GETDATA(AB_BANKINFO_PLUGIN, AB_BANKINFO_PLUGIN_DE, bip);
  assert(bde);

  assert(bde->checker);
  res=AccountNumberCheck_check(bde->checker,
                               bankId,
                               accountId);
  switch(res) {
  case 0:  cr=AB_BankInfoCheckResult_Ok; break;
  case 1:  cr=AB_BankInfoCheckResult_UnknownResult; break;
  case 2:  cr=AB_BankInfoCheckResult_NotOk; break;
  case 3:  cr=AB_BankInfoCheckResult_UnknownBank; break;
  default: cr=AB_BankInfoCheckResult_UnknownResult; break;
  } /* switch */

  return cr;
}








int AB_BankInfoPluginDE__ReadLine(GWEN_BUFFEREDIO *bio,
				  GWEN_STRINGLIST *sl) {
  GWEN_ERRORCODE err;
  const char *delimiters=";";

  assert(bio);

  if(!GWEN_BufferedIO_CheckEOF(bio)) {
    GWEN_BUFFER *lbuffer;
    GWEN_BUFFER *wbuffer;
    int rv;
    const char *s;

    /* read line */
    lbuffer=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_Reset(lbuffer);
    err=GWEN_BufferedIO_ReadLine2Buffer(bio, lbuffer);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(GWEN_LOGDOMAIN, err);
      GWEN_Buffer_free(lbuffer);
      return -1;
    }

    /* read columns */
    wbuffer=GWEN_Buffer_new(0, 256, 0, 1);

    s=GWEN_Buffer_GetStart(lbuffer);
    while(*s) {
      rv=GWEN_Text_GetWordToBuffer(s, delimiters, wbuffer,
				   GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
				   GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
				   GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
				   GWEN_TEXT_FLAGS_DEL_QUOTES,
				   &s);
      if (rv) {
	GWEN_Buffer_free(wbuffer);
	GWEN_Buffer_free(lbuffer);
	return rv;
      }
      GWEN_StringList_AppendString(sl, GWEN_Buffer_GetStart(wbuffer), 0, 0);
      GWEN_Buffer_Reset(wbuffer);
      if (*s) {
	if (strchr(delimiters, *s))
	  s++;
      }
    } /* while */
    GWEN_Buffer_free(wbuffer);
    GWEN_Buffer_free(lbuffer);
  }

  return 0;
}




int AB_BankInfoPluginDE__ReadFromColumn4(AB_BANKINFO *bi,
					 GWEN_STRINGLIST *sl) {
  const char *s;
  const char *t;
  GWEN_STRINGLISTENTRY *se;
  AB_BANKINFO_SERVICE *bis;

  /* 4th column: location */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  AB_BankInfo_SetLocation(bi, s);

  /* 5th column: RZ */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 6th column: Organization */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;

  /* 7th column: Hostname */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  bis=AB_BankInfoService_new();
  AB_BankInfoService_SetType(bis, "HBCI");
  AB_BankInfoService_SetSuffix(bis, "3000");
  AB_BankInfoService_SetMode(bis, "tcp"); /* normal RDH/DDV service */
  AB_BankInfoService_SetAddress(bis, s);
  t=s;

  /* 8th column: IP address */
  se=GWEN_StringListEntry_Next(se);
  if (!se) {
    AB_BankInfoService_free(bis);
    return 0;
  }
  s=GWEN_StringListEntry_Data(se);
  assert(s);

  if (!*t)
    /* no hostname, use the IP address instead */
    AB_BankInfoService_SetAddress(bis, s);

  /* add service */
  AB_BankInfoService_List_Add(bis, AB_BankInfo_GetServices(bi));

  /* 9th column: HBCI version */
  se=GWEN_StringListEntry_Next(se);
  if (!se) {
    AB_BankInfoService_free(bis);
    return 0;
  }
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  AB_BankInfoService_SetPversion(bis, s);

  /* 10th column: DDV */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 11th column: RDH-1 */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 12th column: RDH-2 */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 13th column: RDH-3 */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 14th column: RDH-4 */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  /* 15th column: RDH-5 */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;

  /* 16th column: PIN/TAN URL */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  if (*s) {
    bis=AB_BankInfoService_new();
    AB_BankInfoService_SetType(bis, "HBCI");
    AB_BankInfoService_SetSuffix(bis, "443");
    AB_BankInfoService_SetMode(bis, "ssl"); /* normal PIN/TAN service */
    AB_BankInfoService_SetAddress(bis, s);
  }

  /* add service */
  AB_BankInfoService_List_Add(bis, AB_BankInfo_GetServices(bi));

  /* 17th column: PIN/TAN Version */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  if (*s) {
    AB_BankInfoService_SetPversion(bis, s);
  }

  return 0;
}



int AB_BankInfoPluginDE__ReadFromColumn3(AB_BANKINFO *bi,
					 GWEN_STRINGLIST *sl) {
  const char *s;
  GWEN_STRINGLISTENTRY *se;

  /* 3rd column: Bank name */
  se=GWEN_StringListEntry_Next(se);
  if (!se)
    return 0;
  s=GWEN_StringListEntry_Data(se);
  assert(s);
  AB_BankInfo_SetBankName(bi, s);

  return AB_BankInfoPluginDE__ReadFromColumn4(bi, sl);
}



AB_BANKINFO *AB_BankInfoPluginDE__SearchbyCode(AB_BANKINFO_PLUGIN *bip,
					       const char *bankId){
  GWEN_BUFFEREDIO *bio;
  int fd;

  fd=open(AB_BANKINFO_DE_DATAFILE, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "open(%s): %s",
	      AB_BANKINFO_DE_DATAFILE,
	      strerror(errno));
    return 0;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  assert(bio);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  while (!GWEN_BufferedIO_CheckEOF(bio)) {
    GWEN_STRINGLIST *sl;
    GWEN_STRINGLISTENTRY *se;

    /* read line into string list */
    sl=GWEN_StringList_new();
    if (AB_BankInfoPluginDE__ReadLine(bio, sl)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in bank data file");
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return 0;
    }

    /* first column: number */
    se=GWEN_StringList_FirstEntry(sl);
    if (se) {
      /* 2nd column: bank code */
      se=GWEN_StringListEntry_Next(se);
      if (se) {
	const char *s;

	s=GWEN_StringListEntry_Data(se);
	assert(s);
	/* found a valid line, compare it */
	if (*s && strcasecmp(s, bankId)==0) {
	  AB_BANKINFO *bi;

	  /* match, read the rest of it */
	  bi=AB_BankInfo_new();

	  /* read remaining data */
	  AB_BankInfo_SetCountry(bi, "de");
	  AB_BankInfo_SetBankId(bi, s);

	  if (AB_BankInfoPluginDE__ReadFromColumn3(bi, sl)) {
	    DBG_INFO(AQBANKING_LOGDOMAIN, "Error in bank data file");
	    AB_BankInfo_free(bi);
	    GWEN_BufferedIO_Abandon(bio);
	    GWEN_BufferedIO_free(bio);
	    return 0;
	  }

	  GWEN_StringList_free(sl);
	  GWEN_BufferedIO_Close(bio);
	  GWEN_BufferedIO_free(bio);
	  return bi;
	} /* if match */
      } /* if 2nd entry */
    } /* if 1st entry */
    GWEN_StringList_free(sl);
  } /* while */

  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  return 0;
}



int AB_BankInfoPluginDE_SearchbyTemplate(AB_BANKINFO_PLUGIN *bip,
					 AB_BANKINFO *tbi,
					 AB_BANKINFO_LIST2 *bl) {
  GWEN_BUFFEREDIO *bio;
  int fd;
  int lines;
  int found;

  fd=open(AB_BANKINFO_DE_DATAFILE, O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "open(%s): %s",
	      AB_BANKINFO_DE_DATAFILE,
	      strerror(errno));
    return AB_ERROR_NO_DATA;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  assert(bio);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  lines=0;
  found=0;
  while (!GWEN_BufferedIO_CheckEOF(bio)) {
    GWEN_STRINGLIST *sl;
    GWEN_STRINGLISTENTRY *se;

    /* read line into string list */
    sl=GWEN_StringList_new();
    if (AB_BankInfoPluginDE__ReadLine(bio, sl)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in bank data file");
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return AB_ERROR_BAD_CONFIG_FILE;
    }

    if (lines) {
      /* first column: number */
      se=GWEN_StringList_FirstEntry(sl);
      if (se) {
	/* 2nd column: bank code */
	se=GWEN_StringListEntry_Next(se);
	if (se) {
	  AB_BANKINFO *bi;
	  const char *s;
	  const char *p;
	  int match;
  
	  s=GWEN_StringListEntry_Data(se);
	  assert(s);
	  if (*s) {
	    /* found a valid line, read rest */
	    bi=AB_BankInfo_new();
    
	    /* read remaining data */
	    AB_BankInfo_SetCountry(bi, "de");
	    AB_BankInfo_SetBankId(bi, s);
    
	    if (AB_BankInfoPluginDE__ReadFromColumn3(bi, sl)) {
	      DBG_INFO(AQBANKING_LOGDOMAIN, "Error in bank data file");
	      AB_BankInfo_free(bi);
	      GWEN_BufferedIO_Abandon(bio);
	      GWEN_BufferedIO_free(bio);
	      return AB_ERROR_BAD_CONFIG_FILE;
	    }
    
	    /* preset */
	    match=1;
    
	    /* check bank id */
	    p=AB_BankInfo_GetBankId(tbi);
	    if (p) {
	      s=AB_BankInfo_GetBankId(bi);
	      if (s) {
		if (-1==GWEN_Text_ComparePattern(s, p, 0))
		  match=0;
	      }
	    }
    
	    /* check bank name */
	    p=AB_BankInfo_GetBankName(tbi);
	    if (p) {
	      s=AB_BankInfo_GetBankName(bi);
	      if (s) {
		if (-1==GWEN_Text_ComparePattern(s, p, 0))
		  match=0;
	      }
	    }
    
	    /* check location */
	    p=AB_BankInfo_GetLocation(tbi);
	    if (p) {
	      s=AB_BankInfo_GetLocation(bi);
	      if (s) {
		if (-1==GWEN_Text_ComparePattern(s, p, 0))
		  match=0;
	      }
	    }
    
	    /* add it or free it */
	    if (match) {
              found++;
	      AB_BankInfo_List2_PushBack(bl, bi);
	    }
	    else
	      AB_BankInfo_free(bi);
	  } /* if 2nd column contain data */
	} /* if 2nd entry */
      } /* if 1st entry */
    } /* if lines */
    lines++;
    GWEN_StringList_free(sl);
  } /* while */

  GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);

  if (!found) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No matching entry found");
    return AB_ERROR_NOT_FOUND;
  }
  return 0;
}












