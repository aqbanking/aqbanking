/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004-2013 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "jobloadcellphone_p.h"
#include "job_l.h"


GWEN_INHERIT(AB_JOB, AB_JOBLOADCELLPHONE)





AB_JOB *AB_JobLoadCellPhone_new(AB_ACCOUNT *a) {
  AB_JOB *j;
  AB_JOBLOADCELLPHONE *aj;

  j=AB_Job_new(AB_Job_TypeLoadCellPhone, a);
  GWEN_NEW_OBJECT(AB_JOBLOADCELLPHONE, aj);
  GWEN_INHERIT_SETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j, aj,
                       AB_JobLoadCellPhone_FreeData);
  return j;

}



void GWENHYWFAR_CB AB_JobLoadCellPhone_FreeData(void *bp, void *p) {
  AB_JOBLOADCELLPHONE *aj;

  aj=(AB_JOBLOADCELLPHONE*)p;

  AB_CellPhoneProduct_free(aj->product);
  AB_CellPhoneProduct_List_free(aj->productList);
  free(aj->phoneNumber);
  AB_Value_free(aj->value);

  GWEN_FREE_OBJECT(aj);
}



void AB_JobLoadCellPhone_SetCellPhoneProduct(AB_JOB *j,
					     const AB_CELLPHONE_PRODUCT *p) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  if (aj->product)
    AB_CellPhoneProduct_free(aj->product);
  if (p)
    aj->product=AB_CellPhoneProduct_dup(p);
  else
    aj->product=NULL;
}



const AB_CELLPHONE_PRODUCT *AB_JobLoadCellPhone_GetCellPhoneProduct(const AB_JOB *j) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  return aj->product;
}



void AB_JobLoadCellPhone_SetPhoneNumber(AB_JOB *j, const char *s) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  if (aj->phoneNumber)
    free(aj->phoneNumber);
  if (s)
    aj->phoneNumber=strdup(s);
  else
    aj->phoneNumber=NULL;
}



const char *AB_JobLoadCellPhone_GetPhoneNumber(const AB_JOB *j) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  return aj->phoneNumber;
}



void AB_JobLoadCellPhone_SetValue(AB_JOB *j, const AB_VALUE *v) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  if (aj->value)
    AB_Value_free(aj->value);
  if (v)
    aj->value=AB_Value_dup(v);
  else
    aj->value=NULL;
}



const AB_VALUE *AB_JobLoadCellPhone_GetValue(const AB_JOB *j) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  return aj->value;
}



const AB_CELLPHONE_PRODUCT_LIST *AB_JobLoadCellPhone_GetCellPhoneProductList(const AB_JOB *j) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  return aj->productList;
}



void AB_JobLoadCellPhone_SetProductList(AB_JOB *j, AB_CELLPHONE_PRODUCT_LIST *l) {
  AB_JOBLOADCELLPHONE *aj;

  assert(j);
  aj=GWEN_INHERIT_GETDATA(AB_JOB, AB_JOBLOADCELLPHONE, j);
  assert(aj);

  if (aj->productList)
    AB_CellPhoneProduct_List_free(aj->productList);
  aj->productList=l;
}







