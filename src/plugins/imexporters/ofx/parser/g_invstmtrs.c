/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 comments    :Stephen R. Besch
 email       :sbesch@acsu.buffalo.edu

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "g_invstmtrs_p.h"
#include "ofxxmlctx_l.h"
#include "i18n_l.h"

#include "g_generic_l.h"
#include "g_ignore_l.h"

#include "g_invacc_l.h"               /*SRB 4/22/09*/
#include "g_invposlist_l.h"
#include "g_invtranlist_l.h"          /*SRB 4/22/09*/
#include <aqbanking/accstatus.h>

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/gui.h>



GWEN_INHERIT(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS)

/*This code parallels the code in g_stmtrs with the exception that there are quite a few more items to
 ignore and more items to handle. Since there are data items that we want to use, we need to define
 virtual functions for adding data items. And, since some of the subgroups return data, we will need
 to watch for end-tags so we can dispose of the data when the group closes.*/

AIO_OFX_GROUP *AIO_OfxGroup_INVSTMTRS_new(const char *groupName,
					  AIO_OFX_GROUP *parent,
					  GWEN_XML_CONTEXT *ctx) {
  AIO_OFX_GROUP *g;
  AIO_OFX_GROUP_INVSTMTRS *xg;

  /* create base group */
  g=AIO_OfxGroup_Generic_new(groupName, parent, ctx);
  assert(g);

  GWEN_NEW_OBJECT(AIO_OFX_GROUP_INVSTMTRS, xg);
  assert(xg);
  GWEN_INHERIT_SETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g, xg,
                       AIO_OfxGroup_INVSTMTRS_FreeData);

  /* set virtual functions */
  AIO_OfxGroup_SetStartTagFn(g, AIO_OfxGroup_INVSTMTRS_StartTag);
  AIO_OfxGroup_SetAddDataFn(g, AIO_OfxGroup_INVSTMTRS_AddData);
  AIO_OfxGroup_SetEndSubGroupFn(g, AIO_OfxGroup_INVSTMTRS_EndSubGroup);
  return g;
}

GWENHYWFAR_CB
void AIO_OfxGroup_INVSTMTRS_FreeData(void *bp, void *p) {
  AIO_OFX_GROUP_INVSTMTRS *xg;

  xg=(AIO_OFX_GROUP_INVSTMTRS*)p;
  assert(xg);
  free(xg->currency);
  free(xg->currentElement);
  GWEN_FREE_OBJECT(xg);
}

/*There are 2 data items and 7 subgroups (INVACCTFROM, INVACCTTO, INVTRANLIST, INVPOSLIST, INVOOLIST, INCOME, INVBAL)
 Original code handled 3 of the 7 groups. I've added the INVTRANLIST group to the mix.*/

int AIO_OfxGroup_INVSTMTRS_StartTag(AIO_OFX_GROUP *g,
				    const char *tagName) {
  AIO_OFX_GROUP_INVSTMTRS *xg;
  GWEN_XML_CONTEXT *ctx;
  AIO_OFX_GROUP *gNew=NULL;

/*First, get the data and context.*/

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
  assert(xg);
  ctx=AIO_OfxGroup_GetXmlContext(g);

  free(xg->currentElement);                                  /*Get rid of the old contents*/
  xg->currentElement=NULL;

/*Handle the data tags first We only need to make the current element's value match the tag.*/

  if (strcasecmp(tagName, "CURDEF")==0 ||
      strcasecmp(tagName, "DTASOF")==0) {
    xg->currentElement=strdup(tagName);
  }

/*Then handle the groups.*/

  else if (strcasecmp(tagName, "INVACCTFROM")==0 ||
	   strcasecmp(tagName, "INVACCTTO")==0)
    gNew=AIO_OfxGroup_INVACC_new(tagName, g, ctx);
  else if (strcasecmp(tagName, "INVTRANLIST")==0)
    gNew=AIO_OfxGroup_INVTRANLIST_new(tagName, g, ctx); /*SRB 4/22/09*/
  else if (strcasecmp(tagName, "INVPOSLIST")==0)
    gNew=AIO_OfxGroup_INVPOSLIST_new(tagName, g, ctx);
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN,"Ignoring group [%s]", tagName);
    gNew=AIO_OfxGroup_Ignore_new(tagName, g, ctx);
  }
  if (gNew) {
    AIO_OfxXmlCtx_SetCurrentGroup(ctx, gNew);
    GWEN_XmlCtx_IncDepth(ctx);
  }
  return 0;
}



/*Even though we look for the DTASOF tag above (we must, so that it can be distinguished from a group),
 nothing is done with the data. The only tag we preocess is the currency definition.*/

int AIO_OfxGroup_INVSTMTRS_AddData(AIO_OFX_GROUP *g, const char *data) {
  AIO_OFX_GROUP_INVSTMTRS *xg;
  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
  assert(xg);

/*If the last start tag defined a "currentElement", then see if we recognize it.*/

  if (xg->currentElement) {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;

/*Always make sure that there are no weird or extra characters in the string data.*/

    buf=GWEN_Buffer_new(0, strlen(data), 0, 1);
    rv=AIO_OfxXmlCtx_SanitizeData(AIO_OfxGroup_GetXmlContext(g), data, buf);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(buf);
      return rv;
    }
    s=GWEN_Buffer_GetStart(buf);
    if (*s) {                                                   /*If there is actually a string there, then*/
      DBG_INFO(AQBANKING_LOGDOMAIN, "AddData: %s=[%s]", xg->currentElement, s);
      if (strcasecmp(xg->currentElement, "CURDEF")==0) {        /*See if it was following a CURDEF*/
	      free(xg->currency);                                     /*If so, then remove any debris*/
        xg->currency=strdup(s);                                 /*and dup the string into xg->currency*/
      }
      else {                                                    /*All other tags are ignored!*/
	      DBG_INFO(AQBANKING_LOGDOMAIN,
                "Ignoring data for unknown element [%s]",
		            xg->currentElement);
      }
    }
    GWEN_Buffer_free(buf);
  }
  return 0;
}

/*We need to watch for the ending of our 3 groups. Ignore INVACCTTO for now.*/

int AIO_OfxGroup_INVSTMTRS_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg) {
  AIO_OFX_GROUP_INVSTMTRS *xg;
  const char *s;
  GWEN_XML_CONTEXT *ctx;

/*Set up pointers to INVSTMTRS group data*/

  assert(g);
  xg=GWEN_INHERIT_GETDATA(AIO_OFX_GROUP, AIO_OFX_GROUP_INVSTMTRS, g);
  assert(xg);
  ctx=AIO_OfxGroup_GetXmlContext(g);
  assert(ctx);

  s=AIO_OfxGroup_GetGroupName(sg);                            /*Pointer to the group name*/

/*First look for the INVACCTFROM group. This is quite simple and is in fact nearly identical to the code for
 BANKACCTFROM in g_stmtrn.c. ....What about INVACCTTO? */

  if (strcasecmp(s, "INVACCTFROM")==0) {
    AB_IMEXPORTER_ACCOUNTINFO *ai;
    const char *s;
    DBG_INFO(AQBANKING_LOGDOMAIN, "Importing account %s/%s", AIO_OfxGroup_INVACC_GetBrokerId(sg), AIO_OfxGroup_INVACC_GetAccId(sg));
    ai=AB_ImExporterAccountInfo_new();                        /*Create the AccountInfo Structure*/
    assert(ai);                                               /*Validate creation*/
    s=AIO_OfxGroup_INVACC_GetBrokerId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetBankCode(ai, s);       /*Install Broker ID*/
    s=AIO_OfxGroup_INVACC_GetAccId(sg);
    if (s)
      AB_ImExporterAccountInfo_SetAccountNumber(ai, s);  /*And account number*/

/* and set currency if there is one */

    if (xg->currency) AB_ImExporterAccountInfo_SetCurrency(ai, xg->currency);

/* set account type, if known */

    s=AIO_OfxGroup_INVACC_GetAccType(sg);
    if (!s) s="INVESTMENT"; /* Investment is a real code now?--- SRB 4/22/09*/
    if (s) {
      AB_ACCOUNT_TYPE t;
      t=AIO_OfxGroup_Generic_AccountTypeFromString(s);
      AB_ImExporterAccountInfo_SetType(ai, t);
    }

    DBG_INFO(AQBANKING_LOGDOMAIN, "Adding investment account");
    AB_ImExporterContext_AddAccountInfo(AIO_OfxXmlCtx_GetIoContext(ctx), ai);
    xg->accountInfo=ai;
  }

  else if (strcasecmp(s, "INVTRANLIST") == 0) {
    /*Here when we finish an Investment transaction list. Uncommented and extended by SRB*/
    AB_TRANSACTION_LIST2 *tl;
    AB_TRANSACTION_LIST2_ITERATOR *it;

    tl=AIO_OfxGroup_INVTRANLIST_TakeTransactionList(sg);
    if (!tl) return 0;                                  /*No list - just return*/

    it=AB_Transaction_List2_First(tl);

    if (it) {
      AB_TRANSACTION *t;

      t=AB_Transaction_List2Iterator_Data(it);
/*      int transactionCount=0;
      char st[20]; */

      while(t) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Importing investment transaction");
        if (xg->currency) {                             /* set currency using xg->currency if missing from transaction */
          const AB_VALUE *v;
          v=AB_Transaction_GetValue(t);                 /*Returns pointer to actual data item*/
          if (v && AB_Value_GetCurrency(v)==NULL) {     /*If v is valid, check for currency valid*/
            AB_VALUE *v2;                               /*We're duplicating here (why??)*/
            v2=AB_Value_dup(v);                         /*Can do AB_Value_SetCurrency(v, xg->currency) for whole mess*/
            AB_Value_SetCurrency(v2, xg->currency);     /*Install the currency*/
            AB_Transaction_SetValue(t, v2);             /*This destroys v, duplicates v2 and uses duplicate*/
            AB_Value_free(v2);
          }
        }

/*        sprintf(st,"%d",transactionCount);
        AB_Transaction_SetCustomerReference(t,st);
        transactionCount++; */

        AB_ImExporterAccountInfo_AddTransaction(xg->accountInfo, t);
        t=AB_Transaction_List2Iterator_Next(it);
      }
      AB_Transaction_List2Iterator_free(it);
    }

  /* don't call AB_Transaction_List2_freeAll(), because the transactions
   from the list have been taken over by the AccountInfo object */

    AB_Transaction_List2_free(tl);
  }
  return 0;
}

