/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_generic_p.h"
#include "ofxxmlctx_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>




AIO_OFX_GROUP *AIO_OfxGroup_Generic_new(const char *groupName,
					AIO_OFX_GROUP *parent,
					GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;

  /* create base group */
  g=AIO_OfxGroup_new(groupName, parent, ctx);
  assert(g);

  /* set virtual functions */
  AIO_OfxGroup_SetEndTagFn(g, AIO_OfxGroup_Generic_EndTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_Generic_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_Generic_EndSubGroup);

  return g;
}



int AIO_OfxGroup_Generic_EndTag(AIO_OFX_GROUP *g, const char *tagName) {
  assert(g);

  if (strcasecmp(AIO_OfxGroup_GetGroupName(g), tagName)!=0) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Tag [%s] does not close [%s], ignoring",
	     tagName, AIO_OfxGroup_GetGroupName(g));
    /*return GWEN_ERROR_BAD_DATA;*/
    return 0;
  }

  /* always end this tag */
  return 1;
}



int AIO_OfxGroup_Generic_AddData(AIO_OFX_GROUP *g, const char *data) {
  assert(g);

  /* just ignore the data */
  return 0;
}



int AIO_OfxGroup_Generic_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg){
  assert(g);

  /* just ignore the end of sub group */
  return 0;
}






AB_ACCOUNT_TYPE AIO_OfxGroup_Generic_AccountTypeFromString(const char *s) {
  AB_ACCOUNT_TYPE t;
  
  if (strcasecmp(s, "CHECKING")==0)
    t=AB_AccountType_Checking;
  else if (strcasecmp(s, "SAVINGS")==0)
    t=AB_AccountType_Savings;
  else if (strcasecmp(s, "MONEYMRKT")==0)
    t=AB_AccountType_MoneyMarket;
  else if (strcasecmp(s, "INVESTMENT")==0) t=AB_AccountType_Investment; /*INVESTMENT String added by SRB 4/23/09*/
  else if (strcasecmp(s, "CREDITLINE")==0)
    t=AB_AccountType_Bank;
  else if (strcasecmp(s, "BANK")==0)       /* not a real code */
    t=AB_AccountType_Bank;
  else if (strcasecmp(s, "CREDITCARD")==0) /* not a real code */
    t=AB_AccountType_CreditCard;
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,
	     "Unknown account type [%s], assuming bank account", s);
    t=AB_AccountType_Bank;
  }

  return t;
}














