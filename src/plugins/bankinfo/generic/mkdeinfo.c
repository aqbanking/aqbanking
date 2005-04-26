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


#include <gwenhywfar/db.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_file.h>
#include <gwenhywfar/xml.h>
#include "../../../libs/aqbanking/types/bankinfo_l.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define FUZZY_SHIFT 10
#define FUZZY_THRESHOLD 850


static AB_BANKINFO_LIST *bis=0;
static GWEN_DB_NODE *dbIdx=0;

int readCSVFile(const char *fname, const char *pname, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbParams;

  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFile(dbParams, pname,
		       GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Error reading profile file \"%s\"", pname);
    GWEN_DB_Group_free(dbParams);
    return -1;
  }

  if (GWEN_DB_ReadFileAs(db, fname, "csv", dbParams,
			 GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(0, "Error reading data file \"%s\"", fname);
    GWEN_DB_Group_free(dbParams);
    return -1;
  }

  GWEN_DB_Group_free(dbParams);
  return 0;
}



AB_BANKINFO *findBankInfo(AB_BANKINFO *bi,
			  const char *blz, const char *location) {
  while(bi) {
    const char *lblz;
    const char *lloc;

    lblz=AB_BankInfo_GetBankId(bi);
    lloc=AB_BankInfo_GetLocation(bi);
    if (lblz && lloc && blz && location) {
      if (GWEN_Text_ComparePattern(lblz, blz, 0)!=-1 &&
	  GWEN_Text_ComparePattern(lloc, location, 0)!=-1)
	break;
    }
    bi=AB_BankInfo_List_Next(bi);
  }

  return bi;
}



AB_BANKINFO *findFirstBankInfo(const char *blz, const char *location) {
  AB_BANKINFO *bi;

  bi=AB_BankInfo_List_First(bis);
  return findBankInfo(bi, blz, location);
}



AB_BANKINFO *findNextBankInfo(AB_BANKINFO *bi,
			      const char *blz, const char *location) {
  bi=AB_BankInfo_List_Next(bi);
  return findBankInfo(bi, blz, location);
}



void isoToUtf8(const char *p,
	       int size,
	       GWEN_BUFFER *buf) {
  while(*p) {
    unsigned int c;

    if (!size)
      break;

    c=(unsigned char)(*(p++));
    switch(c) {
    case 0xc4: /* AE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x84);
      break;

    case 0xe4: /* ae */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa4);
      break;

    case 0xd6: /* OE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x96);
      break;

    case 0xf6: /* oe */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb6);
      break;

    case 0xdc: /* UE */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9c);
      break;

    case 0xfc: /* ue */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xbc);
      break;

    case 0xdf: /* sz */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x9f);
      break;

    case 0xa7: /* section sign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x67);
      break;

      /* english chars */
    case 0xa3: /* pound swign */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x63);
      break;

      /* french chars */
    case 0xc7: /* C cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0x87);
      break;

    case 0xe0: /* a accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa0);
      break;

    case 0xe1: /* a accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa1);
      break;

    case 0xe2: /* a accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa2);
      break;

    case 0xe7: /* c cedille */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa7);
      break;

    case 0xe8: /* e accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa8);
      break;

    case 0xe9: /* e accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xa9);
      break;

    case 0xea: /* e accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xaa);
      break;

    case 0xec: /* i accent grave (never heard of this) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xac);
      break;

    case 0xed: /* i accent aigu (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xad);
      break;

    case 0xee: /* i accent circumflex (never heard of this, either) */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xae);
      break;

    case 0xf2: /* o accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb2);
      break;

    case 0xf3: /* o accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb3);
      break;

    case 0xf4: /* o accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb4);
      break;

    case 0xf9: /* u accent grave */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xb9);
      break;

    case 0xfa: /* u accent aigu */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xba);
      break;

    case 0xfb: /* u accent circumflex */
      GWEN_Buffer_AppendByte(buf, 0xc3);
      GWEN_Buffer_AppendByte(buf, 0xbb);
      break;

    default:
      GWEN_Buffer_AppendByte(buf, c);
    }
    if (size!=-1)
      size--;
  } /* while */
}




GWEN_TYPE_UINT32 _word_similarity(const char *a, const char *b){
  GWEN_TYPE_UINT32 score = 0;
  size_t l = 0;

  while (*a && *b) {
    if (*a == *b)
      score += 1 << FUZZY_SHIFT;
    else if (*a == b[1]) {
      score += 1 << (FUZZY_SHIFT-2); b++;
    }
    else if (a[1] == *b) {
      score += 1 << (FUZZY_SHIFT-2); a++; l++;
    }
    a++;
    b++;
    l++;
  }

  if ('\0' != *a)
    l += strlen(a);

  return score / l;
}



int _is_ascii_alnum(unsigned char c) {
  return ((c>='A' || c<='Z') ||
	  (c>='a' || c<='z') ||
	  (c>='0' || c<='9') ||
	  c>=128);
}



GWEN_STRINGLIST *_make_word_list(const char *str) {
  GWEN_STRINGLIST *sl;

  sl=GWEN_StringList_new();
  while(*str) {
    const char *p;
    size_t size;

    while (*str && !_is_ascii_alnum(*str))
      str++;
    p=str;
    while (*str && _is_ascii_alnum(*str)) {
      str++;
    }

    size=(str-p);
    if (*p) {
      char *wptr;
      char *t;
      size_t i;

      wptr=(char*)malloc(size+1);
      t=wptr;
      for (i=0; i<size; i++)
	*(t++)=tolower(*(p++));
      *t=0;
      /* expand some abbreviations */
      if (strcasecmp(wptr, "FCU")==0) {
	GWEN_StringList_AppendString(sl, "FEDERAL", 0, 0);
	GWEN_StringList_AppendString(sl, "CREDIT", 0, 0);
	GWEN_StringList_AppendString(sl, "UNION", 0, 0);
        free(wptr);
      }
      else if (strcasecmp(wptr, "1ST")==0) {
	GWEN_StringList_AppendString(sl, "FIRST", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "TR")==0) {
	GWEN_StringList_AppendString(sl, "TRUST", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "BK")==0) {
	GWEN_StringList_AppendString(sl, "BANK", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "CO")==0) {
	GWEN_StringList_AppendString(sl, "COMPANY", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "CORP")==0) {
	GWEN_StringList_AppendString(sl, "CORPORATION", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "DEPT")==0) {
	GWEN_StringList_AppendString(sl, "DEPARTMENT", 0, 0);
	free(wptr);
      }
      else if (strcasecmp(wptr, "NAT")==0 ||
	       strcasecmp(wptr, "NATL")==0) {
	GWEN_StringList_AppendString(sl, "NATIONAL", 0, 0);
	free(wptr);
      }
      else {
	GWEN_StringList_AppendString(sl, wptr, 1, 0); /* take this string */
      }
    }
  } /* while */

  return sl;
}



GWEN_TYPE_UINT32 _cmp_word_list(const char *s,
					  GWEN_STRINGLIST *words) {
  GWEN_TYPE_UINT32 score = 0;
  GWEN_TYPE_UINT32 n = 0;
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(words);
  while(se) {
    const char *p;

    n++;
    p=GWEN_StringListEntry_Data(se);
    assert(p);

    if (strcmp(s, p)==0)
      return 1 << FUZZY_SHIFT;
    else
      score+=_word_similarity(s, p);
    se=GWEN_StringListEntry_Next(se);
  }

  return (n>0)?(score/n):0;
}



GWEN_TYPE_UINT32 _find_score(GWEN_STRINGLIST *a,
				       GWEN_STRINGLIST *b) {
  GWEN_TYPE_UINT32 score = 0;
  GWEN_TYPE_UINT32 n = 0;
  GWEN_STRINGLISTENTRY *se;

  se=GWEN_StringList_FirstEntry(a);
  while(se) {
    const char *p;

    n++;
    p=GWEN_StringListEntry_Data(se);
    assert(p);

    score+=_cmp_word_list(p, b);

    se=GWEN_StringListEntry_Next(se);
  }

  return (n>0)?(score/n):0;
}



GWEN_TYPE_UINT32 FuzzyCompare(const char *str1, const char *str2) {
  GWEN_STRINGLIST *a;
  GWEN_STRINGLIST *b;
  GWEN_TYPE_UINT32 score;

  a=_make_word_list(str1);
  b=_make_word_list(str2);

  score=(_find_score(a, b) + _find_score(b, a))/2;

  GWEN_StringList_free(b);
  GWEN_StringList_free(a);

  return score;
}






int readDBBFile(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading Deutsche Bundesbank file...\n");
  if (readCSVFile(fname, "dbb.conf", dbData)) {
    DBG_ERROR(0, "Error reading DBB file \"%s\"", fname);
    GWEN_DB_Group_free(dbData);
    return -1;
  }

  fprintf(stdout, "Building database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;
    const char *lloc;

    lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    lloc=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
    if (lloc && lblz && isdigit(*lblz)) {
      if (strcasecmp(lblz, "0")!=0) {
	AB_BANKINFO *bi;

	bi=AB_BankInfo_fromDb(dbT);
        assert(bi);
        AB_BankInfo_SetCity(bi, lloc);

	AB_BankInfo_List_Add(bi, bis);
	count++;
      }
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Found %d banks\n", count);
  return 0;
}



int readHBCIFile(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading HBCI file...\n");
  if (readCSVFile(fname, "hbci.conf", dbData)) {
    DBG_ERROR(0, "Error reading HBCI file \"%s\"", fname);
    GWEN_DB_Group_free(dbData);
    return -1;
  }

  fprintf(stdout, "Updating database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;
    const char *lloc;

    lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    lloc=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
    if (lblz && lloc) {
      AB_BANKINFO *bi;

      bi=findFirstBankInfo(lblz, lloc);
      while(bi) {
	const char *addr;
	const char *ver;

	count++;
	addr=GWEN_DB_GetCharValue(dbT, "hostName", 0, 0);
        if (addr==0)
	  addr=GWEN_DB_GetCharValue(dbT, "hostIp", 0, 0);
	ver=GWEN_DB_GetCharValue(dbT, "hbciVersion", 0, 0);
	if (ver) {
	  /* normalize version */
	  if (strcmp(ver, "2")==0 ||
	      strcmp(ver, "2.01")==0 ||
	      strcmp(ver, "2.0.1")==0)
	    ver="2.01";
	  else if (strcmp(ver, "2.1")==0 ||
		   strcmp(ver, "2.10")==0 ||
		   strcmp(ver, "2.1.0")==0)
	    ver="2.10";
	  else if (strcmp(ver, "2.2")==0 ||
		   strcmp(ver, "2.20")==0 ||
		   strcmp(ver, "2.2.0")==0)
	    ver="2.20";
	}
	if (addr) {
	  const char *s;

	  s=GWEN_DB_GetCharValue(dbT, "ddv", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "DDV");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if DDV */

	  s=GWEN_DB_GetCharValue(dbT, "rdh1", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH1");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH1 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh2", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH2");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH2 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh3", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH3");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH3 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh4", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH4");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH4 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh5", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH5");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH5 */

	  s=GWEN_DB_GetCharValue(dbT, "pinTanUrl", 0, 0);
	  if (s && *s) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, s);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "PINTAN");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* id PINTAN */
	} /* if addr */

	bi=findNextBankInfo(bi, lblz, lloc);
      }

    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Updated %d banks\n", count);
  return 0;
}



int readATBLZFile(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading Austrian Bank file...\n");
  if (readCSVFile(fname, "atblz.conf", dbData)) {
    DBG_ERROR(0, "Error reading ATBLZ file \"%s\"", fname);
    GWEN_DB_Group_free(dbData);
    return -1;
  }

  fprintf(stdout, "Building database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;

    lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    if (lblz && isdigit(*lblz)) {
      if (strcasecmp(lblz, "0")!=0) {
	AB_BANKINFO *bi;

	bi=AB_BankInfo_fromDb(dbT);
        assert(bi);
	AB_BankInfo_List_Add(bi, bis);
	count++;
      }
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Found %d banks\n", count);
  return 0;
}



int readFedAchDirFile(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading Federal Reserve Bank file...\n");
  if (readCSVFile(fname, "fedachdir.conf", dbData)) {
    DBG_ERROR(0, "Error reading FedAchDir file \"%s\"", fname);
    GWEN_DB_Group_free(dbData);
    return -1;
  }

  fprintf(stdout, "Building database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;
    const char *lloc;
    int recType;

    recType=GWEN_DB_GetIntValue(dbT, "xRecordType", 0, 1);
    if (recType==2)
      lblz=GWEN_DB_GetCharValue(dbT, "xNewBankId", 0, 0);
    else
      lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    lloc=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
    if (lloc && lblz && isdigit(*lblz)) {
      AB_BANKINFO *bi;
      GWEN_BUFFER *tbuf;
      const char *s;

      /* compose phone number */
      tbuf=GWEN_Buffer_new(0, 32, 0, 1);
      s=GWEN_DB_GetCharValue(dbT, "xPhoneAreaCode", 0, 0);
      if (s) {
	GWEN_Buffer_AppendString(tbuf, s);
	GWEN_Buffer_AppendByte(tbuf, '-');
      }
      s=GWEN_DB_GetCharValue(dbT, "xPhonePrefix", 0, 0);
      if (s)
	GWEN_Buffer_AppendString(tbuf, s);
      s=GWEN_DB_GetCharValue(dbT, "xPhoneSuffix", 0, 0);
      if (s)
	GWEN_Buffer_AppendString(tbuf, s);
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "phone", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      /* compose zipcode */
      s=GWEN_DB_GetCharValue(dbT, "xZipCode", 0, 0);
      if (s) {
	GWEN_Buffer_AppendString(tbuf, s);
	s=GWEN_DB_GetCharValue(dbT, "xZipExt", 0, 0);
	if (s) {
	  GWEN_Buffer_AppendByte(tbuf, '-');
	  GWEN_Buffer_AppendString(tbuf, s);
	}
	GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			     "zipCode", GWEN_Buffer_GetStart(tbuf));
	GWEN_Buffer_Reset(tbuf);
      }
      bi=AB_BankInfo_fromDb(dbT);
      assert(bi);
      AB_BankInfo_SetBankId(bi, lblz);
      AB_BankInfo_SetCity(bi, lloc);
      AB_BankInfo_List_Add(bi, bis);
      GWEN_Buffer_free(tbuf);
      count++;
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Found %d banks\n", count);
  return 0;
}




const char *readCharValueXml(GWEN_XMLNODE *n, const char *name,
                             GWEN_BUFFER *dbuf) {
  GWEN_XMLNODE *nn;

  GWEN_Buffer_Reset(dbuf);
  nn=GWEN_XMLNode_FindFirstTag(n, name, 0, 0);
  if (nn) {
    GWEN_BUFFEREDIO *bio;
    GWEN_ERRORCODE err;
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    bio=GWEN_BufferedIO_Buffer2_new(tbuf, 0);
    GWEN_BufferedIO_SetWriteBuffer(bio, 0, 512);
    if (GWEN_XMLNode_WriteToStream(nn, bio, GWEN_XML_FLAGS_SIMPLE)) {
      DBG_ERROR(0, "Error writing data to buffer");
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    err=GWEN_BufferedIO_Close(bio);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    GWEN_BufferedIO_free(bio);
    if (GWEN_Text_UnescapeXmlToBuffer(GWEN_Buffer_GetStart(tbuf), dbuf)) {
      DBG_ERROR(0, "Error decoding XML buffer");
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    GWEN_Buffer_free(tbuf);

    return GWEN_Buffer_GetStart(dbuf);
  }
  return 0;
}



int readMSMFiles(const char *path, const char *country) {
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *dbuf;
  GWEN_TYPE_UINT32 pos;
  GWEN_XMLNODE *nBanks;
  GWEN_XMLNODE *n;
  int updateCount=0;

  fprintf(stdout, "Reading GnuCash bank files...\n");
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, path);
  GWEN_Buffer_AppendByte(pbuf, '/');
  pos=GWEN_Buffer_GetPos(pbuf);

  GWEN_Buffer_AppendString(pbuf, "banks.xml");
  nBanks=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "banks");
  if (GWEN_XML_ReadFile(nBanks,
                        GWEN_Buffer_GetStart(pbuf),
                        GWEN_XML_FLAGS_DEFAULT |
                        GWEN_XML_FLAGS_IGNORE_INCLUDE
                       )) {
    DBG_ERROR(0, "Error reading file \"%s\"",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_XMLNode_free(nBanks);
    return -1;
  }
  GWEN_Buffer_Crop(pbuf, 0, pos);

  n=GWEN_XMLNode_FindFirstTag(nBanks, "banks", 0, 0);
  if (n)
    n=GWEN_XMLNode_FindFirstTag(n, "bank:bank", 0, 0);
  while(n) {
    const char *guid;
    int isCross=0;

    guid=GWEN_XMLNode_GetCharValue(n, "bank:crossrefguid", 0);
    if (!guid || !*guid)
      guid=GWEN_XMLNode_GetCharValue(n, "bank:guid", 0);
    else
      isCross=1;
    if (guid) {
      FILE *f;

      GWEN_Buffer_AppendString(pbuf, guid);
      GWEN_Buffer_AppendString(pbuf, ".xml");
      f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
      if (f) {
        GWEN_XMLNODE *nBank;
        GWEN_XMLNODE *b;

        fclose(f);
        nBank=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "bank");
        if (GWEN_XML_ReadFile(nBank,
                              GWEN_Buffer_GetStart(pbuf),
                              GWEN_XML_FLAGS_DEFAULT |
                              GWEN_XML_FLAGS_IGNORE_INCLUDE)) {
          DBG_ERROR(0, "Error reading bank file \"%s\"",
                    GWEN_Buffer_GetStart(pbuf));
          GWEN_XMLNode_free(nBank);
          GWEN_XMLNode_free(nBanks);
          return -1;
        }
        b=GWEN_XMLNode_FindFirstTag(nBank, "providers", 0, 0);
        if (b)
          b=GWEN_XMLNode_FindFirstTag(b, "provider:provider", 0, 0);
        if (b) {
          const char *s;

          s=readCharValueXml(b, "provider:country", dbuf);
          if (s && *s) {
            if (strcasecmp(s, "USA")==0 ||
                strcasecmp(s, "US")==0 ||
                strcasecmp(s, "Uni")==0 ||
                strcasecmp(s, "United States")==0)
              s="us";
            else if (strcasecmp(s, "CAN")==0 ||
                     strcasecmp(s, "CA")==0 ||
                     strcasecmp(s, "Canada")==0)
              s="ca";
            else {
              DBG_ERROR(0, "Unknown country \"%s\" in bank \"%s\"", s, guid);
              GWEN_XMLNode_free(nBank);
              GWEN_XMLNode_free(nBanks);
              return -1;
            }
          }
          else {
            fprintf(stderr, "No country in bank \"%s\", ignoring\n", guid);
            s="none";
          }
	  if (strcasecmp(s, country)==0) {
	    AB_BANKINFO *bi;
	    const char *zipCode;
	    const char *name;
	    int addIt=1;

            bi=AB_BankInfo_new();
            AB_BankInfo_SetCountry(bi, s);
            s=readCharValueXml(n, "bank:name", dbuf);
            if (s && *s)
              AB_BankInfo_SetBankName(bi, s);
            else {
              DBG_ERROR(0, "No name in bank \"%s\"", guid);
              GWEN_XMLNode_free(nBank);
              GWEN_XMLNode_free(nBanks);
              return -1;
            }

            s=readCharValueXml(b, "provider:city", dbuf);
            if (s && *s) {
              AB_BankInfo_SetLocation(bi, s);
              AB_BankInfo_SetCity(bi, s);
            }
            s=readCharValueXml(b, "provider:address1", dbuf);
            if (s && *s)
              AB_BankInfo_SetStreet(bi, s);
            s=readCharValueXml(b, "provider:region", dbuf);
            if (s && *s)
              AB_BankInfo_SetRegion(bi, s);
            s=readCharValueXml(b, "provider:postcode", dbuf);
            if (s && *s)
              AB_BankInfo_SetZipcode(bi, s);
            s=readCharValueXml(b, "provider:phone", dbuf);
            if (s && *s)
              AB_BankInfo_SetPhone(bi, s);
            s=readCharValueXml(b, "provider:email", dbuf);
            if (s && *s)
              AB_BankInfo_SetEmail(bi, s);
            s=readCharValueXml(b, "provider:website", dbuf);
            if (s && *s)
              AB_BankInfo_SetWebsite(bi, s);

            s=readCharValueXml(n, "bank:driver", dbuf);
            if (s && strcasecmp(s, "O")==0) {
              const char *pver;
              const char *server;

              pver=GWEN_XMLNode_GetCharValue(b, "provider:ofxheaderver", 0);
              server=readCharValueXml(b, "provider:ofxserver",
                                      dbuf);
              if (pver && *pver && server && *server) {
                AB_BANKINFO_SERVICE *sv;

                sv=AB_BankInfoService_new();
                AB_BankInfoService_SetType(sv, "OFX");
                AB_BankInfoService_SetAddress(sv, server);
                AB_BankInfoService_SetPversion(sv, pver);
                s=readCharValueXml(b, "provider:fid", dbuf);
                if (s && *s)
                    AB_BankInfoService_SetAux1(sv, s);
                s=readCharValueXml(b, "provider:org", dbuf);
                if (s && *s)
                    AB_BankInfoService_SetAux2(sv, s);

                AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
              }
	    }

	    name=AB_BankInfo_GetBankName(bi);
	    zipCode=AB_BankInfo_GetZipcode(bi);
	    if (name && *name && zipCode && *zipCode) {
	      AB_BANKINFO *tbi;
	      char zbuf[32];
	      const char *x;
	      char *y;

	      /* make compact zip code */
	      assert(strlen(zipCode)<sizeof(zbuf));
	      x=zipCode;
              y=zbuf;
	      while(*x) {
		if (*x=='-')
		  break;
		else if (isdigit(*x))
		  *(y++)=*x;
		x++;
	      } /* while */
	      *y=0;
	      zipCode=zbuf;
	      tbi=AB_BankInfo_List_First(bis);
	      while(tbi) {
		const char *lname;
		const char *lzip;
                const char *lblz;

		lblz=AB_BankInfo_GetBankId(tbi);
		if (lblz && *lblz) {
		  lname=AB_BankInfo_GetBankName(tbi);
		  lzip=AB_BankInfo_GetZipcode(tbi);
  
		  if (lname && *lname && lzip && *lzip) {
		    char lzbuf[32];
  
		    /* make compact zip code */
		    assert(strlen(lzip)<sizeof(lzbuf));
		    x=lzip;
		    y=lzbuf;
		    while(*x) {
		      if (*x=='-')
			break;
		      else if (isdigit(*x))
			*(y++)=*x;
		      x++;
		    } /* while */
		    *y=0;
		    lzip=lzbuf;
		    if (strcasecmp(zipCode, lzip)==0) {
		      GWEN_TYPE_UINT32 v;
  
		      v=FuzzyCompare(name, lname);
		      if (v>FUZZY_THRESHOLD) {
			fprintf(stderr,
				"Updated existing entry for \"%s\" ("
				GWEN_TYPE_TMPL_UINT32
				")\n", lname, v);
			break;
		      }
		    }
		  }
		}
		tbi=AB_BankInfo_List_Next(tbi);
	      } /* while existing entries */
	      if (tbi) {
		const char *s1, *s2;

		/* merge in new info */
		s1=AB_BankInfo_GetEmail(bi);
		s2=AB_BankInfo_GetEmail(tbi);
		if ((s1 && *s1) && (!s2 || !*s2))
		  AB_BankInfo_SetEmail(tbi, s1);
                s1=AB_BankInfo_GetWebsite(bi);
		s2=AB_BankInfo_GetWebsite(tbi);
		if ((s1 && *s1) && (!s2 || !*s2))
		  AB_BankInfo_SetWebsite(tbi, s1);

		/* move new service entries to end of existing ones */
		AB_BankInfoService_List_AddList(AB_BankInfo_GetServices(tbi),
						AB_BankInfo_GetServices(bi));

		addIt=0;
	      }
	    }
	    if (addIt)
	      AB_BankInfo_List_Add(bi, bis);
	    else {
	      AB_BankInfo_free(bi);
	      updateCount++;
	    }
          } /* if matching country */
        } /* if provider */
        GWEN_XMLNode_free(nBank);
      } /* if provider file exists */
      else {
        DBG_WARN(0, "Provider file for bank \"%s\" not found", guid);
      }
      GWEN_Buffer_Crop(pbuf, 0, pos);
    } /* if guid */
    n=GWEN_XMLNode_FindNextTag(n, "bank:bank", 0, 0);
  }

  fprintf(stdout, "%d banks updated.\n", updateCount);

  return 0;
}



int readBcFile(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading BC Bankenstamm file...\n");
  if (readCSVFile(fname, "bcbankenstamm.conf", dbData)) {
    DBG_ERROR(0, "Error reading BC Bankenstamm file \"%s\"", fname);
    GWEN_DB_Group_free(dbData);
    return -1;
  }

  if (GWEN_DB_WriteFile(dbData,
                        "out.conf",
                        GWEN_DB_FLAGS_QUOTE_VALUES | \
                        GWEN_DB_FLAGS_WRITE_SUBGROUPS | \
                        GWEN_DB_FLAGS_INDEND | \
                        GWEN_DB_FLAGS_ADD_GROUP_NEWLINES | \
                        GWEN_DB_FLAGS_ESCAPE_CHARVALUES | \
                        GWEN_DB_FLAGS_OMIT_TYPES)) {
    DBG_ERROR(0, "Error writing bank file");
    return -1;
  }

  fprintf(stdout, "Building database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;
    const char *lloc;

    lblz=GWEN_DB_GetCharValue(dbT, "xNewBankId", 0, 0);
    if (!lblz || !*lblz)
      lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    lloc=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
    if (lloc && *lloc && lblz && isdigit(*lblz)) {
      AB_BANKINFO *bi;
      GWEN_BUFFER *tbuf;
      const char *s;

      /* compose phone number */
      tbuf=GWEN_Buffer_new(0, 32, 0, 1);
      s=GWEN_DB_GetCharValue(dbT, "xCountryPrefix", 0, 0);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendByte(tbuf, '-');
      }
      s=GWEN_DB_GetCharValue(dbT, "xPhone", 0, 0);
      if (s && *s)
        GWEN_Buffer_AppendString(tbuf, s);
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                           "phone", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      /* compose fax number */
      s=GWEN_DB_GetCharValue(dbT, "xCountryPrefix", 0, 0);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        GWEN_Buffer_AppendByte(tbuf, '-');
      }
      s=GWEN_DB_GetCharValue(dbT, "xFax", 0, 0);
      if (s && *s)
	GWEN_Buffer_AppendString(tbuf, s);
      GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
			   "fax", GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_Reset(tbuf);

      /* compose bank code */
      s=GWEN_DB_GetCharValue(dbT, "xNewBankId", 0, 0);
      if (s && *s) {
        GWEN_Buffer_AppendString(tbuf, s);
        /*
        GWEN_Buffer_AppendString(tbuf, "0000");
        */
        GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "bankId", GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_Reset(tbuf);
      }
      else {
        s=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
        assert(s);
        GWEN_Buffer_AppendString(tbuf, s);
        /*
        s=GWEN_DB_GetCharValue(dbT, "xFilialId", 0, 0);
        if (s)
          GWEN_Buffer_AppendString(tbuf, s);
        */
        GWEN_DB_SetCharValue(dbT, GWEN_DB_FLAGS_OVERWRITE_VARS,
                             "bankId", GWEN_Buffer_GetStart(tbuf));
        GWEN_Buffer_Reset(tbuf);
      }

      bi=AB_BankInfo_fromDb(dbT);
      assert(bi);
      AB_BankInfo_SetCity(bi, lloc);
      AB_BankInfo_List_Add(bi, bis);
      GWEN_Buffer_free(tbuf);
      count++;
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Found %d banks\n", count);
  return 0;
}





int makeIndexBlz(const char *fname) {
  AB_BANKINFO *bi;
  FILE *f;
  GWEN_TYPE_UINT32 count=0;

  f=fopen(fname, "w+");
  if (!f) {
    DBG_ERROR(0, "Error creating file \"%s\"", fname);
    return -1;
  }

  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    GWEN_TYPE_UINT32 pos;
    const char *s;
    char numbuf[32];

    count++;
    s=AB_BankInfo_GetBankId(bi);
    if (s) {
      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
      if (pos==0 && count!=1) {
        DBG_ERROR(0, "No index given for \"%s\" (%d)", numbuf, count);
        fclose(f);
        return -1;
      }
      fprintf(f, "%s\t%08x\n", s, pos);
    }
    bi=AB_BankInfo_List_Next(bi);
  }

  if (fclose(f)) {
    DBG_ERROR(0, "Error closing file \"%s\"", fname);
    return -1;
  }

  return 0;
}



int makeIndexBic(const char *fname) {
  AB_BANKINFO *bi;
  FILE *f;
  GWEN_TYPE_UINT32 count=0;

  f=fopen(fname, "w+");
  if (!f) {
    DBG_ERROR(0, "Error creating file \"%s\"", fname);
    return -1;
  }

  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    const char *s;

    count++;
    s=AB_BankInfo_GetBic(bi);
    if (s && *s) {
      GWEN_TYPE_UINT32 pos;
      char numbuf[32];

      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
      if (pos==0 && count!=1) {
        DBG_ERROR(0, "No index given for \"%s\" (%d)", numbuf, count);
        fclose(f);
        return -1;
      }
      fprintf(f, "%s\t%08x\n", s, pos);
    }
    bi=AB_BankInfo_List_Next(bi);
  }

  if (fclose(f)) {
    DBG_ERROR(0, "Error closing file \"%s\"", fname);
    return -1;
  }

  return 0;
}



int makeIndexNameAndLoc(const char *fname) {
  AB_BANKINFO *bi;
  FILE *f;
  GWEN_TYPE_UINT32 count=0;

  f=fopen(fname, "w+");
  if (!f) {
    DBG_ERROR(0, "Error creating file \"%s\"", fname);
    return -1;
  }

  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    const char *name;
    const char *loc;

    count++;
    name=AB_BankInfo_GetBankName(bi);
    loc=AB_BankInfo_GetLocation(bi);
    if (name && *name && loc && *loc) {
      GWEN_TYPE_UINT32 pos;
      char numbuf[32];
  
      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
      if (pos==0 && count!=1) {
        DBG_ERROR(0, "No index given for \"%s\" (%d)", numbuf, count);
        fclose(f);
        return -1;
      }
      fprintf(f, "%s\t%s\t%08x\n", name, loc, pos);
    }
    bi=AB_BankInfo_List_Next(bi);
  }

  if (fclose(f)) {
    DBG_ERROR(0, "Error closing file \"%s\"", fname);
    return -1;
  }

  return 0;
}



int saveBankInfos(const char *path) {
  AB_BANKINFO *bi;
  GWEN_TYPE_UINT32 count=0;
  GWEN_BUFFER *xbuf;
  GWEN_BUFFEREDIO *bio;
  int fd;
  GWEN_ERRORCODE err;

  fprintf(stdout, "Saving database...\n");
  fd=open(path, O_RDWR | O_CREAT | O_TRUNC,
          S_IRUSR | S_IWUSR
#ifdef S_IRGRP
          |S_IRGRP
#endif
#ifdef S_IROTH
          |S_IROTH
#endif
          );
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s", path, strerror(errno));
    return -1;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);

  err=GWEN_BufferedIO_WriteLine(bio,
                                "# This is an automatically created file");
  if (GWEN_Error_IsOk(err))
    err=GWEN_BufferedIO_WriteLine(bio,
                                  "# All banks are separated by newlines");
  if (GWEN_Error_IsOk(err))
    err=GWEN_BufferedIO_WriteLine(bio,
                                  "# Please do not modify this file, "
                                  "the index files rely on exact positions.");
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    DBG_ERROR(0, "Error writing bank file \"%s\"", path);
    return -1;
  }

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    const char *s;
    GWEN_DB_NODE *dbT;
    GWEN_TYPE_UINT32 pos;
    char numbuf[32];

    count++;

    /* some conversions to UTF8 */
    s=AB_BankInfo_GetBankName(bi);
    assert(s);
    isoToUtf8(s, strlen(s), xbuf);
    AB_BankInfo_SetBankName(bi, GWEN_Buffer_GetStart(xbuf));
    GWEN_Buffer_Reset(xbuf);

    s=AB_BankInfo_GetLocation(bi);
    if (s) {
      isoToUtf8(s, strlen(s), xbuf);
      AB_BankInfo_SetLocation(bi, GWEN_Buffer_GetStart(xbuf));
      AB_BankInfo_SetCity(bi, GWEN_Buffer_GetStart(xbuf));
      GWEN_Buffer_Reset(xbuf);
    }

    /* create DB */
    dbT=GWEN_DB_Group_new("bank");
    AB_BankInfo_toDb(bi, dbT);

    pos=GWEN_BufferedIO_GetBytesWritten(bio);
    snprintf(numbuf, sizeof(numbuf), "%08x", count);
    GWEN_DB_SetIntValue(dbIdx, GWEN_DB_FLAGS_OVERWRITE_VARS,
                        numbuf, pos);
    if (GWEN_DB_WriteToStream(dbT, bio,
                              GWEN_DB_FLAGS_QUOTE_VALUES | \
                              GWEN_DB_FLAGS_WRITE_SUBGROUPS | \
                              GWEN_DB_FLAGS_INDEND | \
                              GWEN_DB_FLAGS_ESCAPE_CHARVALUES | \
                              GWEN_DB_FLAGS_OMIT_TYPES)) {
      DBG_ERROR(0, "Error writing bank file \"%s\"", path);
      GWEN_DB_Group_free(dbT);
      return -1;
    }
    err=GWEN_BufferedIO_WriteLine(bio, "");
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR_ERR(0, err);
      DBG_ERROR(0, "Error writing bank file \"%s\"", path);
      GWEN_DB_Group_free(dbT);
      return -1;
    }
    GWEN_DB_Group_free(dbT);

    if (count & ~31) {
      fprintf(stdout, GWEN_TYPE_TMPL_UINT32"\r", count);
    }

    bi=AB_BankInfo_List_Next(bi);
  } /* while bi */

  err=GWEN_BufferedIO_Close(bio);
  GWEN_BufferedIO_free(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_ERROR_ERR(0, err);
    DBG_ERROR(0, "Error closing bank file \"%s\"", path);
    return -1;
  }
  fprintf(stdout, "  Written %d banks.\n", count);
  return 0;
}



int makeBankInfos(const char *path) {
  AB_BANKINFO *bi;
  GWEN_TYPE_UINT32 count=0;
  char numbuf[32];
  GWEN_BUFFER *dbuf;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    GWEN_DB_NODE *dbT;

    count++;

    /* create path */
    GWEN_Buffer_AppendString(dbuf, path);
    GWEN_Buffer_AppendString(dbuf, "/banks/");

    snprintf(numbuf, sizeof(numbuf), "%04x", count);
    GWEN_Buffer_AppendBytes(dbuf, numbuf, 2);
    if (GWEN_Directory_GetPath(GWEN_Buffer_GetStart(dbuf),
                               GWEN_PATH_FLAGS_CHECKROOT |
                               GWEN_DIR_FLAGS_PUBLIC_PATH |
                               GWEN_DIR_FLAGS_PUBLIC_NAME)) {
      DBG_ERROR(0, "Error creating path \"%s\"",
		GWEN_Buffer_GetStart(dbuf));
      /* GWEN_DB_Group_free(dbT); -- not yet initialized */
      GWEN_Buffer_free(dbuf);
      return -1;
    }

    GWEN_Buffer_AppendByte(dbuf, '/');
    GWEN_Buffer_AppendBytes(dbuf, numbuf+2, 2);
    GWEN_Buffer_AppendString(dbuf, ".bank");

    /* create DB */
    dbT=GWEN_DB_Group_new("bank");
    AB_BankInfo_toDb(bi, dbT);

    /* write file */
    if (GWEN_DB_WriteFile(dbT,
			  GWEN_Buffer_GetStart(dbuf),
			  GWEN_DB_FLAGS_QUOTE_VALUES | \
			  GWEN_DB_FLAGS_WRITE_SUBGROUPS | \
			  GWEN_DB_FLAGS_INDEND | \
			  GWEN_DB_FLAGS_ADD_GROUP_NEWLINES | \
			  GWEN_DB_FLAGS_ESCAPE_CHARVALUES | \
			  GWEN_DB_FLAGS_OMIT_TYPES)) {
      DBG_ERROR(0, "Error writing bank file \"%s\"",
		GWEN_Buffer_GetStart(dbuf));
      GWEN_DB_Group_free(dbT);
      GWEN_Buffer_free(dbuf);
      return -1;
    }
#ifndef OS_WIN32
    if (chmod(GWEN_Buffer_GetStart(dbuf),
              S_IRUSR | S_IWUSR
# ifdef S_IRGRP
              | S_IRGRP
# endif
# ifdef S_IROTH
              | S_IROTH
# endif
             )) {
      DBG_ERROR(0, "chmod(%s): %s",
                GWEN_Buffer_GetStart(dbuf),
                strerror(errno));
      GWEN_DB_Group_free(dbT);
      GWEN_Buffer_free(dbuf);
      return -1;
    }
#endif

    GWEN_Buffer_Reset(dbuf);

    GWEN_DB_Group_free(dbT);

    bi=AB_BankInfo_List_Next(bi);
  }

  fprintf(stdout, "  Written %d banks\n", count);
  return 0;
}



int loadBanks(const char *fname) {
  GWEN_TYPE_UINT32 count=0;
  GWEN_BUFFEREDIO *bio;
  int fd;

  fprintf(stdout, "Loading database, this will take a few minutes ...\n");
  fd=open(fname, O_RDONLY | O_EXCL);
  if (fd==-1) {
    DBG_ERROR(0, "open(%s): %s", fname, strerror(errno));
    return -1;
  }
  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  while(!GWEN_BufferedIO_CheckEOF(bio)) {
    GWEN_DB_NODE *dbT;
    AB_BANKINFO *bi;
    int pos;
    char numbuf[32];

    dbT=GWEN_DB_Group_new("bank");
    pos=GWEN_BufferedIO_GetBytesRead(bio);
    if (GWEN_DB_ReadFromStream(dbT, bio,
                               GWEN_DB_FLAGS_DEFAULT |
                               GWEN_DB_FLAGS_STOP_ON_EMPTY_LINE |
                               GWEN_PATH_FLAGS_CREATE_GROUP)) {
      DBG_ERROR(0, "Could not load file \"%s\"", fname);
      GWEN_DB_Group_free(dbT);
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      return -1;
    }

    bi=AB_BankInfo_fromDb(dbT);
    assert(bi);
    AB_BankInfo_List_Add(bi, bis);
    GWEN_DB_Group_free(dbT);
    count++;
    snprintf(numbuf, sizeof(numbuf), "%08x", count);
    GWEN_DB_SetIntValue(dbIdx, GWEN_DB_FLAGS_OVERWRITE_VARS, numbuf, pos);
  } /* while */
  fprintf(stdout, "  Read %d banks.\n", count);

  return 0;
}



int main(int argc, char **argv) {
  if (argc<2) {
    fprintf(stderr,
            "Usage:\n"
            "%s COMMAND\n",
            argv[0]);
    return 1;
  }

  if (strcasecmp(argv[1], "build")==0) {
    const char *dbbFile, *hbciFile, *dstFile;

    if (argc<5) {
      fprintf(stderr,
              "Usage:\n"
              "%s build DBB-file HBCI-file DESTFILE\n",
              argv[0]);
      return 1;
    }
    dbbFile=argv[2];
    hbciFile=argv[3];
    dstFile=argv[4];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");
    if (readDBBFile(dbbFile)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (readHBCIFile(hbciFile)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (saveBankInfos(dstFile)) {
      return 3;
    }
  }

  else if (strcasecmp(argv[1], "build-at")==0) {
    const char *blzFile, *dstFile;

    if (argc<4) {
      fprintf(stderr,
              "Usage:\n"
              "%s build-at BLZ-file DESTFILE\n",
              argv[0]);
      return 1;
    }
    blzFile=argv[2];
    dstFile=argv[3];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");
    if (readATBLZFile(blzFile)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (saveBankInfos(dstFile)) {
      return 3;
    }
  }
  else if (strcasecmp(argv[1], "build-ch")==0) {
    const char *blzFile, *dstFile;

    if (argc<4) {
      fprintf(stderr,
              "Usage:\n"
              "%s build-ch BLZ-file DESTFILE\n",
              argv[0]);
      return 1;
    }
    blzFile=argv[2];
    dstFile=argv[3];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");
    if (readBcFile(blzFile)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (saveBankInfos(dstFile)) {
      return 3;
    }
  }
  else if (strcasecmp(argv[1], "build-am")==0) {
    const char *path, *country, *src2File, *dstFile;

    if (argc<6) {
      fprintf(stderr,
              "Usage:\n"
	      "%s build-am SRCPATH COUNTRY FEDACHDIRFILE DESTFILE\n",
	      argv[0]);
      return 1;
    }
    path=argv[2];
    country=argv[3];
    src2File=argv[4];
    dstFile=argv[5];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");

    if (strcasecmp(country, "us")==0) {
      if (readFedAchDirFile(src2File)) {
	return 3;
      }
    }

    if (readMSMFiles(path, country)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (saveBankInfos(dstFile)) {
      return 3;
    }
  }
  else if (strcasecmp(argv[1], "install")==0) {
    const char *path;
    const char *srcFile;
    GWEN_BUFFER *dbuf;
    GWEN_TYPE_UINT32 pos;

    if (argc<4) {
      fprintf(stderr,
              "Usage:\n"
              "%s install SRCFILE DESTDIR\n",
              argv[0]);
      return 1;
    }
    srcFile=argv[2];
    path=argv[3];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");
    if (loadBanks(srcFile)) {
      fprintf(stderr, "Error loading data file.\n");
      return 2;
    }

    fprintf(stdout,
            "Writing database and index files to %s, "
            "this will take a few minutes ...\n", path);
    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dbuf, path);
    GWEN_Buffer_AppendByte(dbuf, '/');
    pos=GWEN_Buffer_GetPos(dbuf);

    GWEN_Buffer_AppendString(dbuf, "banks.data");
    if (saveBankInfos(GWEN_Buffer_GetStart(dbuf))) {
      fprintf(stderr, "Error saving data files.\n");
      return 3;
    }
    GWEN_Buffer_Crop(dbuf, 0, pos);

    fprintf(stdout, "- writing BLZ index...\n");
    GWEN_Buffer_AppendString(dbuf, "blz.idx");
    if (makeIndexBlz(GWEN_Buffer_GetStart(dbuf))) {
      fprintf(stderr, "Error saving index file.\n");
      GWEN_Buffer_free(dbuf);
      return 3;
    }
    GWEN_Buffer_Crop(dbuf, 0, pos);

    fprintf(stdout, "- writing BIC index...\n");
    GWEN_Buffer_AppendString(dbuf, "bic.idx");
    if (makeIndexBic(GWEN_Buffer_GetStart(dbuf))) {
      fprintf(stderr, "Error saving index file.\n");
      GWEN_Buffer_free(dbuf);
      return 3;
    }
    GWEN_Buffer_Crop(dbuf, 0, pos);

    fprintf(stdout, "- writing NAMLOC index...\n");
    GWEN_Buffer_AppendString(dbuf, "namloc.idx");
    if (makeIndexNameAndLoc(GWEN_Buffer_GetStart(dbuf))) {
      fprintf(stderr, "Error saving index file.\n");
      GWEN_Buffer_free(dbuf);
      return 3;
    }
    GWEN_Buffer_free(dbuf);
  }
  else if (strcasecmp(argv[1], "debug")==0) {
    const char *bankFile;

    if (argc<3) {
      fprintf(stderr,
              "Usage:\n"
              "%s debug BLZ-file\n",
              argv[0]);
      return 1;
    }
    bankFile=argv[2];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");
    if (loadBanks(bankFile)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (GWEN_DB_WriteFile(dbIdx,
                          "index.conf.out",
                          GWEN_DB_FLAGS_DEFAULT)) {
      DBG_ERROR(0, "Error writing index file");
      return -1;
    }
  }
  else if (strcasecmp(argv[1], "debug2")==0) {
    const char *bankFile;

    if (argc<3) {
      fprintf(stderr,
              "Usage:\n"
              "%s debug BLZ-file\n",
              argv[0]);
      return 1;
    }
    bankFile=argv[2];
    bis=AB_BankInfo_List_new();
    dbIdx=GWEN_DB_Group_new("indexList");

    if (readFedAchDirFile(bankFile)) {
      return 3;
    }
    if (saveBankInfos("us.out")) {
      return 3;
    }
  }

  return 0;
}

