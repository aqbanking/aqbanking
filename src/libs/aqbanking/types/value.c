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



AB_VALUE *AB_Value_fromString(const char *s) {
  AB_VALUE *v;
  const char *currency=NULL;
  int rv;
  char *tmpString=NULL;
  char *p;
  char *t;
  int isNeg=0;

#ifdef HAVE_SETLOCALE
  const char *orig_locale = setlocale(LC_NUMERIC, NULL);
  char *currentLocale = strdup(orig_locale ? orig_locale : "C");

  setlocale(LC_NUMERIC,"C");
#endif

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

  v=AB_Value_new();
  t=strchr(p, ',');
  if (t)
    *t='.';

  if (strchr(p, '.')) {
    mpf_t v1;

    mpf_init(v1);
    if (mpf_set_str(v1, p, 10)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "[%s] is not a valid value", s);
      AB_Value_free(v);
#ifdef HAVE_SETLOCALE
      setlocale(LC_NUMERIC, currentLocale);
      free(currentLocale);
#endif
      return NULL;
    }
    mpq_set_f(v->value, v1);
    mpf_clear(v1);
    rv=1;
  }
  else {
    /*DBG_ERROR(0, "Scanning this value: %s\n", p);*/
    rv=gmp_sscanf(p, "%Qu", v->value);
  }

#ifdef HAVE_SETLOCALE
  setlocale(LC_NUMERIC, currentLocale);
  free(currentLocale);
#endif

  /* set currency (if any) */
  if (currency)
    v->currency=strdup(currency);

  /* temporary string no longer needed */
  free(tmpString);

  if (rv!=1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "[%s] is not a valid value", s);
    AB_Value_free(v);
    return NULL;
  }

  /* canonicalize */
  mpq_canonicalize(v->value);

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
  char numbuf[128];
  double num;
  int rv;
#ifdef HAVE_SETLOCALE
  const char *orig_locale = setlocale(LC_NUMERIC, NULL);
  char *currentLocale = strdup(orig_locale ? orig_locale : "C");
  setlocale(LC_NUMERIC, "C");
#endif

  num=AB_Value_GetValueAsDouble(v);
  rv=snprintf(numbuf, sizeof(numbuf), "%.*lf",
              prec?prec:2, num);

#ifdef HAVE_SETLOCALE
  setlocale(LC_NUMERIC, currentLocale);
  free(currentLocale);
#endif

  if (rv<1 || rv>=sizeof(numbuf)) {
    assert(0);
  }
  GWEN_Buffer_AppendString(buf, numbuf);

  if (v->currency) {
    GWEN_Buffer_AppendString(buf, " ");
    GWEN_Buffer_AppendString(buf, v->currency);
  }
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
  rv=snprintf(numbuf, sizeof(numbuf), "%.*lf",
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
  return mpq_get_d(v->value);
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









