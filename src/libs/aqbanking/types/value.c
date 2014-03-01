/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "value_p.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/buffer.h>

#include <assert.h>
#ifdef HAVE_LOCALE_H
# include <locale.h>
#endif

#include <ctype.h>


#define AB_VALUE_STRSIZE 256



GWEN_LIST_FUNCTIONS(AB_VALUE, AB_Value)




AB_VALUE *AB_Value_new(void) {
  AB_VALUE *v;

  GWEN_NEW_OBJECT(AB_VALUE, v);
  GWEN_LIST_INIT(AB_VALUE, v);
  mpq_init(v->value);
  return v;
}



void AB_Value_free(AB_VALUE *v) {
  if (v) {
    mpq_clear(v->value);
    free(v->currency);
    GWEN_LIST_FINI(AB_VALUE, v);
    GWEN_FREE_OBJECT(v);
  }
}



AB_VALUE *AB_Value_dup(const AB_VALUE *ov) {
  AB_VALUE *v;

  assert(ov);
  v=AB_Value_new();
  mpq_set(v->value, ov->value);
  if (ov->currency)
    v->currency=strdup(ov->currency);

  return v;
}



AB_VALUE *AB_Value_fromDouble(double i) {
  GWEN_BUFFER *nbuf;
  AB_VALUE *v;
  int rv;

  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_Text_DoubleToBuffer(i, nbuf);
  assert(rv==0);
  v=AB_Value_fromString(GWEN_Buffer_GetStart(nbuf));
  GWEN_Buffer_free(nbuf);
  return v;
}

AB_VALUE *AB_Value_fromInt(long int num, long int denom) {
  AB_VALUE *v;

  v=AB_Value_new();
  mpq_set_si(v->value, num, denom);

  return v;
}


static int AB_Value_determineDecimalComma(const char *s) {
  int len;
  int i;

  len=strlen(s);
  for (i=len-1; i>=0; i--) {
    if (s[i]==',' || s[i]=='.')
      return (int) (s[i]);
  }

  return 0;
}



AB_VALUE *AB_Value_fromString(const char *s) {
  AB_VALUE *v;
  const char *currency=NULL;
  int conversion_succeeded = 1;	// assume conversion will succeed
  char *tmpString=NULL;
  char *p;
  char *t;
  char decimalComma;
  int isNeg=0;

  if( !s ) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Attempt to convert a NULL value");
    return NULL;
  }

  tmpString=strdup(s);
  p=tmpString;

  while(*p && *p<33)
    p++;

  if (*p=='-') {
    isNeg=1;
    p++;
  }
  else if (*p=='+') {
    p++;
  }

  t=strchr(p, ':');
  if (t) {
    currency=t+1;
    *t=0;
  }

  /* remove thousand's comma */
  decimalComma=AB_Value_determineDecimalComma(p);
  if (decimalComma) {
    char *s1, *d1;

    s1=p;
    d1=p;
    while(*s1) {
      register char c;

      c=*(s1++);
      if (isdigit(c) || c=='/')
	*(d1++)=c;
      else if (c==decimalComma)
        /* always use '.' as decimal comma */
	*(d1++)='.';
    }
    *d1=0;
  }

  v=AB_Value_new();

  t=strchr(p, '.');
  if (t) {
    // remove comma and calculate denominator
    unsigned long denominator = 1;
    char *next;
    do {
      next=t+1;
      *t=*next;
      if (*next != 0)
        denominator *= 10;
      t++;
    } while (*next);

    // set denominator to the calculated value
    mpz_set_ui(mpq_denref(v->value), denominator);

    // set numerator to the resulting integer string without comma
    if (mpz_set_str(mpq_numref(v->value), p, 10) == -1) {
      conversion_succeeded = 0;
    }
  }
  else {
    /*DBG_ERROR(0, "Scanning this value: %s\n", p);*/
    conversion_succeeded = (gmp_sscanf(p, "%Qu", v->value) == 1);
  }

  /* set currency (if any) */
  if (currency)
    v->currency=strdup(currency);

  /* temporary string no longer needed */
  free(tmpString);

  if (!conversion_succeeded) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "[%s] is not a valid value", s);
    AB_Value_free(v);
    return NULL;
  }

  if (isNeg)
    mpq_neg(v->value, v->value);


  return v;
}



const char *AB_Value_GetCurrency(const AB_VALUE *v){
  assert(v);
  return v->currency;
}



void AB_Value_SetCurrency(AB_VALUE *v, const char *s){
  assert(v);
  free(v->currency);
  if (s) 
    v->currency=strdup(s);
  else
    v->currency=0;
}



AB_VALUE *AB_Value_fromDb(GWEN_DB_NODE *db){
  AB_VALUE *vc;
  const char *p;

  /* read and parse value */
  p=GWEN_DB_GetCharValue(db, "value", 0, 0);
  if (!p)
    return NULL;
  vc=AB_Value_fromString(p);
  if (vc==NULL)
    return NULL;

  /* read currency (if any) */
  p=GWEN_DB_GetCharValue(db, "currency", 0, "EUR");
  if (p)
    AB_Value_SetCurrency(vc, p);
  return vc;
}



int AB_Value_toDb(const AB_VALUE *v, GWEN_DB_NODE *db) {
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 128, 0, 1);
  AB_Value__toString(v, buf);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "value", GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
  if (v->currency)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "currency", v->currency);
  return 0;
}



int AB_Value_toDbFloat(const AB_VALUE *v, GWEN_DB_NODE *db) {
  GWEN_BUFFER *buf;

  buf=GWEN_Buffer_new(0, 128, 0, 1);
  AB_Value_toHumanReadableString2(v, buf, 2, 0);
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
		       "value", GWEN_Buffer_GetStart(buf));
  GWEN_Buffer_free(buf);
  if (v->currency)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 "currency", v->currency);
  return 0;
}



void AB_Value__toString(const AB_VALUE *v, GWEN_BUFFER *buf) {
  int rv;
  uint32_t size;
  char *p;

  assert(v);
  GWEN_Buffer_AllocRoom(buf, AB_VALUE_STRSIZE);
  p=GWEN_Buffer_GetPosPointer(buf);
  size=GWEN_Buffer_GetMaxUnsegmentedWrite(buf);
  rv=gmp_snprintf(p, size, "%Qi", v->value);
  assert(rv<size);
  GWEN_Buffer_IncrementPos(buf, rv+1);
  GWEN_Buffer_AdjustUsedBytes(buf);
}



int AB_Value_GetNumDenomString(const AB_VALUE *v,
			       char *buffer,
			       uint32_t buflen) {
  int rv;

  assert(v);

  rv=gmp_snprintf(buffer, buflen, "%Qu", v->value);
  if (rv<0 || rv>=buflen) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Buffer too small");
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  return 0;
}



void AB_Value_toString(const AB_VALUE *v, GWEN_BUFFER *buf) {
  assert(v);
  AB_Value__toString(v, buf);
  if (v->currency) {
    GWEN_Buffer_AppendString(buf, ":");
    GWEN_Buffer_AppendString(buf, v->currency);
  }
}



void AB_Value_toHumanReadableString(const AB_VALUE *v,
                                     GWEN_BUFFER *buf,
                                     int prec) {
  AB_Value_toHumanReadableString2(v, buf, prec?prec:2, 0);
}



void AB_Value_toHumanReadableString2(const AB_VALUE *v,
				     GWEN_BUFFER *buf,
				     int prec,
				     int withCurrency) {
  char numbuf[128];
  double num;
  int rv;
#ifdef HAVE_SETLOCALE
  const char *orig_locale = setlocale(LC_NUMERIC, NULL);
  char *currentLocale = strdup(orig_locale ? orig_locale : "C");
  setlocale(LC_NUMERIC, "C");
#endif

  num=AB_Value_GetValueAsDouble(v);
  rv=snprintf(numbuf, sizeof(numbuf), "%.*f",
	      prec, num);

#ifdef HAVE_SETLOCALE
  setlocale(LC_NUMERIC, currentLocale);
  free(currentLocale);
#endif

  if (rv<1 || rv>=sizeof(numbuf)) {
    assert(0);
  }
  GWEN_Buffer_AppendString(buf, numbuf);

  if (v->currency && withCurrency) {
    GWEN_Buffer_AppendString(buf, " ");
    GWEN_Buffer_AppendString(buf, v->currency);
  }
}



double AB_Value_GetValueAsDouble(const AB_VALUE *v) {
  assert(v);
  if (mpz_fits_slong_p(mpq_numref(v->value)) && mpz_fits_slong_p(mpq_denref(v->value))) {
    return (double) (mpz_get_d(mpq_numref(v->value)) / mpz_get_d(mpq_denref(v->value)));
  }
  else {
    return mpq_get_d(v->value);
  }
}



void AB_Value_SetValueFromDouble(AB_VALUE *v, double i) {
  assert(v);
  mpq_set_d(v->value, i);
}



void AB_Value_SetZero(AB_VALUE *v) {
  assert(v);
  mpq_clear(v->value);
  mpq_init(v->value);
}



int AB_Value_IsZero(const AB_VALUE *v) {
  assert(v);
  return (mpq_sgn(v->value)==0);
}



int AB_Value_IsNegative(const AB_VALUE *v) {
  assert(v);
  return (mpq_sgn(v->value)<0);
}



int AB_Value_IsPositive(const AB_VALUE *v) {
  assert(v);
  return (mpq_sgn(v->value)>=0);
}



int AB_Value_Compare(const AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);

  return mpq_cmp(v1->value, v2->value);
}

int AB_Value_Equal(const AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);

  return mpq_equal(v1->value, v2->value);
}



int AB_Value_AddValue(AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);

  mpq_add(v1->value, v1->value, v2->value);
  return 0;
}



int AB_Value_SubValue(AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);
  mpq_sub(v1->value, v1->value, v2->value);
  return 0;
}



int AB_Value_MultValue(AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);

  mpq_mul(v1->value, v1->value, v2->value);
  return 0;
}



int AB_Value_DivValue(AB_VALUE *v1, const AB_VALUE *v2) {
  assert(v1);
  assert(v2);

  mpq_div(v1->value, v1->value, v2->value);
  return 0;
}



int AB_Value_Negate(AB_VALUE *v) {
  assert(v);
  mpq_neg(v->value, v->value);
  return 0;
}



void AB_Value_Dump(const AB_VALUE *v, FILE *f, unsigned int indent) {
  unsigned int i;

  for (i=0; i<indent; i++)
    fprintf(f, " ");
  fprintf(f, "Value: ");
  if (v) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 128, 0, 1);
    AB_Value_toHumanReadableString(v, nbuf, 2);
    gmp_fprintf(f, "%Qi (%s)\n", v->value, GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
  }
  else
    fprintf(f, "[none]\n");
}



AB_VALUE_LIST *AB_Value_List_dup(const AB_VALUE_LIST *stl) {
  if (stl) {
    AB_VALUE_LIST *nl;
    AB_VALUE *e;

    nl=AB_Value_List_new();
    e=AB_Value_List_First(stl);
    while(e) {
      AB_VALUE *ne;

      ne=AB_Value_dup(e);
      assert(ne);
      AB_Value_List_Add(ne, nl);
      e=AB_Value_List_Next(e);
    } /* while (e) */
    return nl;
  }
  else
    return 0;
}



long int AB_Value_Num(const AB_VALUE *v) {
  assert(v);
  return mpz_get_si(mpq_numref(v->value));
}



long int AB_Value_Denom(const AB_VALUE *v) {
  assert(v);
  return mpz_get_si(mpq_denref(v->value));
}



void AB_Value_toHbciString(const AB_VALUE *v, GWEN_BUFFER *buf) {
  GWEN_BUFFER *tbuf;
  char *p;
  int l;

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
  AB_Value_toHumanReadableString2(v, tbuf, 2, 0);

  /* convert decimal komma */
  p=GWEN_Buffer_GetStart(tbuf);
  while(*p) {
    if (*p=='.') {
      *p=',';
      break;
    }
    p++;
  }

  /* remove trailing zeroes */
  p=GWEN_Buffer_GetStart(tbuf);
  l=strlen(GWEN_Buffer_GetStart(tbuf));
  if (l>0 && strchr(p, ',')!=NULL) {
    l--;
    while(l>0 && p[l]=='0') {
      p[l]=0;
      l--;
    }
  }

  GWEN_Buffer_AppendString(buf, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}

