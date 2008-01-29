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


#include "result_p.h"
#include "aqhbci_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(AH_RESULT, AH_Result);


AH_RESULT *AH_Result_new(int code,
                         const char *text,
                         const char *ref,
                         const char *param,
                         int isMsgResult){
  AH_RESULT *r;

  GWEN_NEW_OBJECT(AH_RESULT, r);
  GWEN_LIST_INIT(AH_RESULT, r);
  r->code=code;
  if (text)
    r->text=strdup(text);
  if (ref)
    r->ref=strdup(ref);
  if (param)
    r->param=strdup(param);
  r->isMsgResult=isMsgResult;

  return r;
}



void AH_Result_free(AH_RESULT *r){
  if (r) {
    GWEN_LIST_FINI(AH_RESULT, r);

    free(r->text);
    free(r->ref);
    free(r->param);

    GWEN_FREE_OBJECT(r);
  }
}



AH_RESULT *AH_Result_dup(const AH_RESULT *or) {
  AH_RESULT *r;

  assert(or);
  r=AH_Result_new(or->code,
		  or->text,
		  or->ref,
		  or->param,
		  or->isMsgResult);
  return r;
}



int AH_Result_GetCode(const AH_RESULT *r){
  assert(r);
  return r->code;
}



const char *AH_Result_GetText(const AH_RESULT *r){
  assert(r);
  return r->text;
}



const char *AH_Result_GetRef(const AH_RESULT *r){
  assert(r);
  return r->ref;
}



const char *AH_Result_GetParam(const AH_RESULT *r){
  assert(r);
  return r->param;
}



int AH_Result_IsError(const AH_RESULT *r){
  assert(r);
  return (r->code>=9000);
}



int AH_Result_IsWarning(const AH_RESULT *r){
  assert(r);
  return (r->code>=3000 && r->code<4000);
}



int AH_Result_IsInfo(const AH_RESULT *r){
  assert(r);
  return (r->code>=1000 && r->code<2000);
}



int AH_Result_IsOk(const AH_RESULT *r){
  assert(r);
  return (r->code<9000);
}



int AH_Result_IsMsgResult(const AH_RESULT *r){
  assert(r);
  return r->isMsgResult;
}



void AH_Result_Dump(const AH_RESULT *r, FILE *f, unsigned int insert) {
  uint32_t k;

  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Result:\n");
  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "-----------------------------------\n");
  for (k=0; k<insert; k++)
    fprintf(f, " ");
  fprintf(f, "Code     : %04d\n", r->code);
  if (r->text) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Text     : %s\n", r->text);
  }
  if (r->ref) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Reference: %s\n", r->ref);
  }
  if (r->param) {
    for (k=0; k<insert; k++)
      fprintf(f, " ");
    fprintf(f, "Parameter: %s\n", r->param);
  }
}









