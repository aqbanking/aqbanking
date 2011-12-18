/***************************************************************************
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
#include <gwenhywfar/xml.h>
#include <gwenhywfar/fastbuffer.h>
#include <gwenhywfar/syncio_file.h>

#include "../../../libs/aqbanking/types/bankinfo_l.h"

#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
# include <aqofxconnect/user.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef GWEN_DIR_SEPARATOR_S
  /* for gwenyhwfar < 2.5.4 */
# define GWEN_DIR_SEPARATOR '/'
# define GWEN_DIR_SEPARATOR_S "/"
#endif

#define FUZZY_SHIFT 10
#define FUZZY_THRESHOLD 850


static AB_BANKINFO_LIST *bis=0;
static GWEN_DB_NODE *dbIdx=0;

int readCSVFile(const char *fname, const char *pname, GWEN_DB_NODE *db) {
  GWEN_DB_NODE *dbParams;

  dbParams=GWEN_DB_Group_new("params");
  if (GWEN_DB_ReadFile(dbParams, pname, GWEN_DB_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Error reading profile file \"%s\"", pname);
    GWEN_DB_Group_free(dbParams);
    return -1;
  }

  if (GWEN_DB_ReadFileAs(db, fname, "csv", dbParams, GWEN_PATH_FLAGS_CREATE_GROUP)) {
    DBG_ERROR(0, "Error reading data file \"%s\"", fname);
    GWEN_DB_Group_free(dbParams);
    return -1;
  }

  GWEN_DB_Group_free(dbParams);
  return 0;
}



AB_BANKINFO *findBankInfo(AB_BANKINFO *bi,
			  const char *blz,
			  const char *location) {
  while(bi) {
    const char *lblz;
    const char *lloc;

    lblz=AB_BankInfo_GetBankId(bi);
    lloc=AB_BankInfo_GetLocation(bi);
    if (lblz && blz) {
      if (GWEN_Text_ComparePattern(lblz, blz, 0)!=-1) {
	if (!location ||
	    (location && lloc &&
	     GWEN_Text_ComparePattern(lloc, location, 0)!=-1))
	  break;
      }
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
			      const char *blz,
			      const char *location) {
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




uint32_t _word_similarity(const char *a, const char *b){
  uint32_t score = 0;
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



uint32_t _cmp_word_list(const char *s,
			GWEN_STRINGLIST *words) {
  uint32_t score = 0;
  uint32_t n = 0;
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



uint32_t _find_score(GWEN_STRINGLIST *a,
				       GWEN_STRINGLIST *b) {
  uint32_t score = 0;
  uint32_t n = 0;
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



uint32_t FuzzyCompare(const char *str1, const char *str2) {
  GWEN_STRINGLIST *a;
  GWEN_STRINGLIST *b;
  uint32_t score;

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
	  else if (strcmp(ver, "3")==0 ||
		   strcmp(ver, "3.0")==0 ||
		   strcmp(ver, "3.0.0")==0)
	    ver="3.0";
	  else if (strcmp(ver, "4")==0 ||
		   strcmp(ver, "4.0")==0 ||
		   strcmp(ver, "4.0.0")==0)
	    ver="3.0";
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

	  s=GWEN_DB_GetCharValue(dbT, "rdh6", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH6");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH6 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh7", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH7");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH7 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh8", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH8");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH8 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh9", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH9");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH9 */

	  s=GWEN_DB_GetCharValue(dbT, "rdh10", 0, "nein");
	  if (strcasecmp(s, "ja")==0) {
	    AB_BANKINFO_SERVICE *sv;

	    sv=AB_BankInfoService_new();
	    AB_BankInfoService_SetType(sv, "HBCI");
	    AB_BankInfoService_SetAddress(sv, addr);
	    AB_BankInfoService_SetPversion(sv, ver);
	    AB_BankInfoService_SetMode(sv, "RDH10");
	    AB_BankInfoService_List_Add(sv, AB_BankInfo_GetServices(bi));
	  } /* if RDH10 */

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



int readATBLZFile2(const char *fname) {
  GWEN_DB_NODE *dbData;
  GWEN_DB_NODE *dbT;
  int count=0;

  dbData=GWEN_DB_Group_new("data");
  fprintf(stdout, "Reading KIDATEN file...\n");
  if (readCSVFile(fname, "kidaten.conf", dbData)) {
    DBG_ERROR(0, "Error reading KIDATEN file \"%s\"", fname);
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


  fprintf(stdout, "Updating database...\n");
  dbT=GWEN_DB_FindFirstGroup(dbData, "bank");
  while(dbT) {
    const char *lblz;
    const char *lloc;

    lblz=GWEN_DB_GetCharValue(dbT, "bankId", 0, 0);
    lloc=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
    if (lblz && lloc) {
      AB_BANKINFO *bi;

      bi=findFirstBankInfo(lblz, 0);
      if (!bi) {
	/* new bank, add it */
	bi=AB_BankInfo_fromDb(dbT);
	if (bi) {
	  AB_BankInfo_List_Add(bi, bis);
          count++;
	}
      } /* if bank is new */
      else {
	while(bi) {
	  const char *s;

	  s=GWEN_DB_GetCharValue(dbT, "bankName", 0, 0);
	  if (s && *s)
	    AB_BankInfo_SetBankName(bi, s);
	  s=GWEN_DB_GetCharValue(dbT, "street", 0, 0);
	  if (s && *s)
	    AB_BankInfo_SetStreet(bi, s);
	  s=GWEN_DB_GetCharValue(dbT, "zipCode", 0, 0);
	  if (s && *s)
	    AB_BankInfo_SetZipcode(bi, s);
	  s=GWEN_DB_GetCharValue(dbT, "location", 0, 0);
	  if (s && *s) {
	    AB_BankInfo_SetLocation(bi, s);
	    AB_BankInfo_SetCity(bi, s);
	  }
	  count++;

	  bi=findNextBankInfo(bi, lblz, 0);
	} /* while bi */
      } /* if bank already exists */
    }
    dbT=GWEN_DB_FindNextGroup(dbT, "bank");
  }

  GWEN_DB_Group_free(dbData);
  fprintf(stdout, "Updated %d banks\n", count);
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
    int err;
    GWEN_BUFFER *tbuf;
    uint32_t len;

    tbuf=GWEN_Buffer_new(0, 256, 0, 1);
    err=GWEN_XMLNode_toBuffer(nn, tbuf, GWEN_XML_FLAGS_SIMPLE);
    if (err<0) {
      DBG_INFO(0, "here (%d)", err);
      GWEN_Buffer_free(tbuf);
      return NULL;
    }

    len=GWEN_Buffer_GetUsedBytes(tbuf);
    if (len) {
      len--;
      GWEN_Buffer_Crop(tbuf, 0, len);
    }

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



int readMSMFiles(const char *path,
                 const char *ifname,
		 const char *country) {
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *dbuf;
  uint32_t pos;
  GWEN_XMLNODE *nBanks;
  GWEN_XMLNODE *n;
  int updateCount=0;

  fprintf(stdout, "Reading GnuCash bank files...\n");
  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(pbuf, path);
  GWEN_Buffer_AppendByte(pbuf, GWEN_DIR_SEPARATOR);
  pos=GWEN_Buffer_GetPos(pbuf);

  GWEN_Buffer_AppendString(pbuf, ifname);
  nBanks=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "banks");
  if (GWEN_XML_ReadFile(nBanks,
                        GWEN_Buffer_GetStart(pbuf),
                        GWEN_XML_FLAGS_DEFAULT)) {
    DBG_ERROR(0, "Error reading file \"%s\"",
              GWEN_Buffer_GetStart(pbuf));
    GWEN_XMLNode_free(nBanks);
    return -1;
  }
  GWEN_Buffer_Crop(pbuf, 0, pos);

  n=GWEN_XMLNode_FindFirstTag(nBanks, "fil", 0, 0);
  if (n)
    n=GWEN_XMLNode_FindFirstTag(n, "fi", 0, 0);
  while(n) {
    GWEN_XMLNODE *nProvider;

    nProvider=GWEN_XMLNode_FindFirstTag(n, "prov", 0, 0);
    if (nProvider) {
      const char *guid;
      //int isCross=0;

      guid=GWEN_XMLNode_GetCharValue(nProvider, "CrossRefGuid", 0);
      if (!guid || !*guid)
	guid=GWEN_XMLNode_GetCharValue(nProvider, "guid", 0);
      else {
        //isCross=1;
      }
      if (guid) {
	FILE *f;

	GWEN_Buffer_AppendString(pbuf, "/fi/");
	GWEN_Buffer_AppendString(pbuf, guid);
	GWEN_Buffer_AppendString(pbuf, ".xml");
	/*fprintf(stderr, "Reading file \"%s\"\n", GWEN_Buffer_GetStart(pbuf));*/
	f=fopen(GWEN_Buffer_GetStart(pbuf), "r");
	if (f) {
	  GWEN_XMLNODE *nBank;
	  GWEN_XMLNODE *b;
  
	  fclose(f);
	  nBank=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "bank");
	  if (GWEN_XML_ReadFile(nBank,
				GWEN_Buffer_GetStart(pbuf),
				GWEN_XML_FLAGS_DEFAULT)) {
	    DBG_ERROR(0, "Error reading bank file \"%s\"",
		      GWEN_Buffer_GetStart(pbuf));
	    GWEN_XMLNode_free(nBank);
	    GWEN_XMLNode_free(nBanks);
	    return -1;
	  }
	  b=GWEN_XMLNode_FindFirstTag(nBank, "MSNOnlSvcInfo", 0, 0);
	  if (b)
	    b=GWEN_XMLNode_FindFirstTag(b, "ProviderSettings", 0, 0);
	  if (b) {
	    const char *s;
	    int i;
	    uint32_t uflags;

	    uflags=0;
	    i=GWEN_XMLNode_GetIntValue(b, "AcctListAvail", 0);
#ifdef AQBANKING_WITH_PLUGIN_BACKEND_AQOFXCONNECT
            if (i>0)
	      uflags|=AO_USER_FLAGS_ACCOUNT_LIST;
#endif

	    s=readCharValueXml(b, "country", dbuf);
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
	      /*fprintf(stderr, "No country in bank \"%s\", ignoring\n", guid);*/
	      /* assume us */
	      s="us";
	    }
	    if (strcasecmp(s, country)==0) {
	      AB_BANKINFO *bi;
	      const char *zipCode;
	      const char *name;
	      int addIt=1;
  
	      bi=AB_BankInfo_new();
	      AB_BankInfo_SetCountry(bi, s);
	      s=readCharValueXml(nProvider, "name", dbuf);
	      if (s && *s)
		AB_BankInfo_SetBankName(bi, s);
	      else {
		DBG_ERROR(0, "No name in bank \"%s\"", guid);
		GWEN_XMLNode_free(nBank);
		GWEN_XMLNode_free(nBanks);
		return -1;
	      }
  
	      s=readCharValueXml(b, "city", dbuf);
	      if (s && *s) {
		AB_BankInfo_SetLocation(bi, s);
		AB_BankInfo_SetCity(bi, s);
	      }
	      s=readCharValueXml(b, "address1", dbuf);
	      if (s && *s)
		AB_BankInfo_SetStreet(bi, s);
	      s=readCharValueXml(b, "zip", dbuf);
	      if (s && *s)
		AB_BankInfo_SetZipcode(bi, s);
	      s=readCharValueXml(b, "phone", dbuf);
	      if (s && *s)
		AB_BankInfo_SetPhone(bi, s);
	      s=readCharValueXml(b, "InternetMail", dbuf);
	      if (s && *s)
		AB_BankInfo_SetEmail(bi, s);
	      s=readCharValueXml(b, "website1", dbuf);
	      if (s && *s)
		AB_BankInfo_SetWebsite(bi, s);
  
	      s=readCharValueXml(b, "driverType", dbuf);
	      if (s && strcasecmp(s, "O")==0) {
		const char *pver;
		const char *server;
  
		pver=GWEN_XMLNode_GetCharValue(b, "OFXHeaderVer", 0);
		server=readCharValueXml(b, "ProviderUrl",
					dbuf);
                if (pver && *pver && server && *server &&
                    strcasecmp(server, "http://moneycentral.msn.com/cust404.htm")!=0) {
		  AB_BANKINFO_SERVICE *sv;
  
		  sv=AB_BankInfoService_new();
		  AB_BankInfoService_SetType(sv, "OFX");
		  AB_BankInfoService_SetAddress(sv, server);
		  AB_BankInfoService_SetPversion(sv, pver);
		  s=readCharValueXml(b, "fid", dbuf);
		  if (s && *s)
		      AB_BankInfoService_SetAux1(sv, s);
		  s=readCharValueXml(b, "org", dbuf);
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
			uint32_t v;
    
			v=FuzzyCompare(name, lname);
			if (v>FUZZY_THRESHOLD) {
			  fprintf(stderr,
				  "Updating existing entry for \"%s\" "
				  "(%u)\n",
				  lname, v);
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
	  else {
	    fprintf(stderr,
		    "File \"%s\" does not contain bank descriptions\n",
		    GWEN_Buffer_GetStart(pbuf));
	  }
	  GWEN_XMLNode_free(nBank);
	} /* if provider file exists */
	else {
	  fprintf(stderr, "Provider file for bank \"%s\" not found\n", guid);
	}
	GWEN_Buffer_Crop(pbuf, 0, pos);
      } /* if guid */
      else {
        fprintf(stderr, "File does not contain a GUID\n");
      }
    }
    else {
      fprintf(stderr, "File does not contain bank info.\n");
    }
    n=GWEN_XMLNode_FindNextTag(n, "fi", 0, 0);
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
  uint32_t count=0;

  f=fopen(fname, "w+");
  if (!f) {
    DBG_ERROR(0, "Error creating file \"%s\"", fname);
    return -1;
  }

  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    uint32_t pos;
    const char *s;
    char numbuf[32];

    count++;
    s=AB_BankInfo_GetBankId(bi);
    if (s) {
      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(uint32_t)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
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
  uint32_t count=0;

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
      uint32_t pos;
      char numbuf[32];

      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(uint32_t)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
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
  uint32_t count=0;

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
      uint32_t pos;
      char numbuf[32];
  
      snprintf(numbuf, sizeof(numbuf), "%08x", count);
      pos=(uint32_t)GWEN_DB_GetIntValue(dbIdx, numbuf, 0, 0);
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
  uint32_t count=0;
  GWEN_BUFFER *xbuf;
  GWEN_SYNCIO *sio;
  int rv;
  GWEN_FAST_BUFFER *fb;

  fprintf(stdout, "Saving database...\n");
  sio=GWEN_SyncIo_File_new(path, GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
		       GWEN_SYNCIO_FILE_FLAGS_READ |
		       GWEN_SYNCIO_FILE_FLAGS_WRITE |
		       GWEN_SYNCIO_FILE_FLAGS_UREAD |
		       GWEN_SYNCIO_FILE_FLAGS_UWRITE);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  fb=GWEN_FastBuffer_new(512, sio);

  GWEN_FASTBUFFER_WRITELINE(fb, rv, "# This is an automatically created file");
  if (rv>=0) {
    GWEN_FASTBUFFER_WRITELINE(fb, rv, "# All banks are separated by newlines");
  }
  if (rv>=0) {
    GWEN_FASTBUFFER_WRITELINE(fb, rv,
			      "# Please do not modify this file, "
			      "the index files rely on exact positions.");
  }
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    DBG_ERROR(0, "Error writing bank file \"%s\"", path);
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return -1;
  }

  xbuf=GWEN_Buffer_new(0, 256, 0, 1);
  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    const char *s;
    GWEN_DB_NODE *dbT;
    uint32_t pos;
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

    pos=GWEN_FastBuffer_GetBytesWritten(fb);
    snprintf(numbuf, sizeof(numbuf), "%08x", count);
    GWEN_DB_SetIntValue(dbIdx,
			GWEN_PATH_FLAGS_CREATE_VAR,
			numbuf, pos);
    if (GWEN_DB_WriteToFastBuffer(dbT,
				  fb,
				  GWEN_DB_FLAGS_QUOTE_VALUES | \
				  GWEN_DB_FLAGS_WRITE_SUBGROUPS | \
				  GWEN_DB_FLAGS_INDEND | \
				  GWEN_DB_FLAGS_ESCAPE_CHARVALUES | \
				  GWEN_DB_FLAGS_OMIT_TYPES)) {
      DBG_ERROR(0, "Error writing bank file \"%s\"", path);
      GWEN_DB_Group_free(dbT);
      GWEN_FastBuffer_free(fb);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return -1;
    }
    GWEN_FASTBUFFER_WRITELINE(fb, rv, "");
    if (rv<0) {
      DBG_ERROR_ERR(0, rv);
      DBG_ERROR(0, "Error writing bank file \"%s\"", path);
      GWEN_DB_Group_free(dbT);
      GWEN_FastBuffer_free(fb);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    GWEN_DB_Group_free(dbT);

    if (count & ~31) {
      fprintf(stdout, "%u\r", count);
    }

    bi=AB_BankInfo_List_Next(bi);
  } /* while bi */

  /* flush fast buffer */
  GWEN_FASTBUFFER_FLUSH(fb, rv);
  if (rv<0) {
    DBG_ERROR(0, "Error closing bank file \"%s\" (%d)", path, rv);
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_FastBuffer_free(fb);

  /* flush and close io */
  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_ERROR(0, "Error closing bank file \"%s\" (%d)", path, rv);
    GWEN_FastBuffer_free(fb);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_free(sio);

  fprintf(stdout, "  Written %d banks.\n", count);
  return 0;
}



int makeBankInfos(const char *path) {
  AB_BANKINFO *bi;
  uint32_t count=0;
  char numbuf[32];
  GWEN_BUFFER *dbuf;

  dbuf=GWEN_Buffer_new(0, 256, 0, 1);
  bi=AB_BankInfo_List_First(bis);
  while(bi) {
    GWEN_DB_NODE *dbT;

    count++;

    /* create path */
    GWEN_Buffer_AppendString(dbuf, path);
    GWEN_Buffer_AppendString(dbuf, GWEN_DIR_SEPARATOR_S "banks" GWEN_DIR_SEPARATOR_S);

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

    GWEN_Buffer_AppendByte(dbuf, GWEN_DIR_SEPARATOR);
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



int loadBanks(const char *fname, AB_BANKINFO_LIST *biList) {
  uint32_t count=0;
  GWEN_SYNCIO *sio;
  GWEN_FAST_BUFFER *fb;
  int rv;

  fprintf(stdout, "Loading database, this will take a few minutes ...\n");
  sio=GWEN_SyncIo_File_new(fname, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  fb=GWEN_FastBuffer_new(512, sio);

  for(;;) {
    GWEN_DB_NODE *dbT;
    AB_BANKINFO *bi;
    int pos;
    char numbuf[32];
    int rv;

    dbT=GWEN_DB_Group_new("bank");
    pos=GWEN_FastBuffer_GetBytesRead(fb);
    rv=GWEN_DB_ReadFromFastBuffer(dbT, fb,
				  GWEN_DB_FLAGS_DEFAULT |
				  GWEN_PATH_FLAGS_CREATE_GROUP|
				  GWEN_DB_FLAGS_UNTIL_EMPTY_LINE);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
        break;
      DBG_ERROR(0, "Could not load file \"%s\" (%d)", fname, rv);
      GWEN_DB_Group_free(dbT);
      GWEN_FastBuffer_free(fb);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return -1;
    }

    bi=AB_BankInfo_fromDb(dbT);
    assert(bi);
    AB_BankInfo_List_Add(bi, biList);
    GWEN_DB_Group_free(dbT);
    count++;
    snprintf(numbuf, sizeof(numbuf), "%08x", count);
    GWEN_DB_SetIntValue(dbIdx, GWEN_DB_FLAGS_OVERWRITE_VARS, numbuf, pos);
    if (count & ~31) {
      fprintf(stdout, "%u\r", count);
    }
  } /* while */
  fprintf(stdout, "\n");
  fprintf(stdout, "  Read %d banks.\n", count);

  GWEN_FastBuffer_free(fb);
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

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
    const char *blzFile;
    const char *dstFile;

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

    if (readMSMFiles(path, "bank.xml", country)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (readMSMFiles(path, "creditcard.xml", country)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (readMSMFiles(path, "brokerage.xml", country)) {
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
    uint32_t pos;

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
    if (loadBanks(srcFile, bis)) {
      fprintf(stderr, "Error loading data file.\n");
      return 2;
    }

    fprintf(stdout,
            "Writing database and index files to %s, "
            "this will take a few minutes ...\n", path);
    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dbuf, path);
    GWEN_Buffer_AppendByte(dbuf, GWEN_DIR_SEPARATOR);
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
  else if (strcasecmp(argv[1], "update")==0) {
    const char *srcFile1;
    const char *srcFile2;
    const char *destFile;
    AB_BANKINFO_LIST *updBis;
    AB_BANKINFO *curBi;

    if (argc<5) {
      fprintf(stderr,
              "Usage:\n"
              "%s update SRCFILE UPDATEFILE DESTFILE\n",
              argv[0]);
      return 1;
    }
    srcFile1=argv[2];
    srcFile2=argv[3];
    destFile=argv[4];
    dbIdx=GWEN_DB_Group_new("indexList");
    bis=AB_BankInfo_List_new();
    updBis=AB_BankInfo_List_new();
    if (loadBanks(srcFile1, bis)) {
      fprintf(stderr, "Error loading data file.\n");
      return 2;
    }
    if (loadBanks(srcFile2, updBis)) {
      fprintf(stderr, "Error loading update data file.\n");
      return 2;
    }

    curBi=AB_BankInfo_List_First(updBis);
    while(curBi) {
      const char *bankId;

      bankId=AB_BankInfo_GetBankId(curBi);
      if (bankId) {
	AB_BANKINFO *origBi;

	origBi=AB_BankInfo_List_First(bis);
	while(origBi) {
	  const char *s;

	  s=AB_BankInfo_GetBankId(origBi);
	  if (s && strcasecmp(s, bankId)==0)
            break;
	  origBi=AB_BankInfo_List_Next(origBi);
	}

	if (origBi) {
	  /* replace original */
	  AB_BankInfo_List_Del(origBi);
	  AB_BankInfo_free(origBi);
	  origBi=AB_BankInfo_dup(curBi);
	  AB_BankInfo_List_Add(origBi, bis);
	}
      }

      curBi=AB_BankInfo_List_Next(curBi);
    }

    if (saveBankInfos(destFile)) {
      return 3;
    }

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
    if (loadBanks(bankFile, bis)) {
      DBG_ERROR(0, "Error.");
      return 2;
    }

    if (GWEN_DB_WriteFile(dbIdx, "index.conf.out", GWEN_DB_FLAGS_DEFAULT)) {
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

