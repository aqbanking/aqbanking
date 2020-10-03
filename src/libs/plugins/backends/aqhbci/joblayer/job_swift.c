/***************************************************************************
    begin       : Frin Oct 03 2020
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "job_swift.h"

#include "aqhbci/banking/user_l.h"

#include <aqbanking/i18n_l.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>





AB_SWIFT_DESCR_LIST *AH_Job_GetSwiftDescriptorsSupportedByJob(AH_JOB *j,
                                                              const char *paramDbGroupName,
                                                              const char *paramDbVarName,
                                                              const char *family,
                                                              int version1)
{
  GWEN_DB_NODE *dbParams;
  AB_BANKING *ab;
  AB_SWIFT_DESCR_LIST *descrList;
  AB_SWIFT_DESCR_LIST *returnDescrList;
  int i;

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  if (paramDbGroupName && *paramDbGroupName) {
    dbParams=GWEN_DB_GetGroup(dbParams, GWEN_PATH_FLAGS_NAMEMUSTEXIST, paramDbGroupName);
    if (dbParams==NULL) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Group \"%s\" not found in params for job \"%s\"",
                paramDbGroupName, AH_Job_GetName(j));
      return NULL;
    }
  }

  descrList=AB_Banking_GetSwiftDescriptorsForImExporter(ab, "xml");
  if (descrList==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SWIFT descriptors found for XML imexporter (job \"%s\")", AH_Job_GetName(j));
    return NULL;
  }

  returnDescrList=AB_SwiftDescr_List_new();
  for (i=0; i<100; i++) {
    const char *s;

    s=GWEN_DB_GetCharValue(dbParams, paramDbVarName, i, NULL);
    if (s==NULL) {
      if (i==0) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "No supported param stored in params (%s/%s)",
                  paramDbGroupName?paramDbGroupName:"<no group name>",
                  paramDbVarName?paramDbVarName:"<no var name>");
      }
      break;
    }
    else {
      AB_SWIFT_DESCR *tmpDescr;

      DBG_ERROR(AQHBCI_LOGDOMAIN, "Supported param %s/%s[%i]=%s)",
                paramDbGroupName?paramDbGroupName:"<no group name>",
                paramDbVarName?paramDbVarName:"<no var name>",
                i, s);

      tmpDescr=AB_SwiftDescr_FromString(s);
      if (tmpDescr) {
        if (AB_SwiftDescr_Matches(tmpDescr, family, version1, 0, 0)) {
          AB_SWIFT_DESCR *descrFromList;

          /* found a candidate */
          descrFromList=AB_SwiftDescr_List_FindFirst(descrList,
                                                     AB_SwiftDescr_GetFamily(tmpDescr),
                                                     AB_SwiftDescr_GetVersion1(tmpDescr),
                                                     AB_SwiftDescr_GetVersion2(tmpDescr),
                                                     AB_SwiftDescr_GetVersion3(tmpDescr));
          if (descrFromList) {
            AB_SWIFT_DESCR *descrCopy;

            /* store name of selected profile */
            AB_SwiftDescr_SetAlias2(descrFromList, s);
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Adding matching profile [%s] (%s)",
                      AB_SwiftDescr_GetAlias1(tmpDescr),
                      AB_SwiftDescr_GetAlias2(tmpDescr));
            /* copy to return list */
            descrCopy=AB_SwiftDescr_dup(descrFromList);
            AB_SwiftDescr_List_Add(descrCopy, returnDescrList);
          }
        }
        else {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Param [%s] does not match family %s.%d", s, family, version1);
        }
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Could not create SWIFT descriptor from string \"%s\" (job \"%s\"), ignoring",
                  s, AH_Job_GetName(j));
      }
    }
  } /* for */

  AB_SwiftDescr_List_free(descrList);
  if (AB_SwiftDescr_List_GetCount(returnDescrList)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No matching descriptors found for %s_%03d_* (job %s)",
              family?family:"<none>", version1,
              AH_Job_GetName(j));
    AB_SwiftDescr_List_free(returnDescrList);
    return NULL;
  }

  return returnDescrList;
}



AB_SWIFT_DESCR_LIST *AH_Job_GetSwiftDescriptorsSupportedByUser(AH_JOB *j, const char *family, int version1)
{
  GWEN_DB_NODE *dbParams;
  AB_BANKING *ab;
  const GWEN_STRINGLIST *userDescriptors;
  GWEN_STRINGLISTENTRY *se;
  AB_SWIFT_DESCR_LIST *descrList;
  AB_SWIFT_DESCR_LIST *returnDescrList;
  AB_USER *user;

  dbParams=AH_Job_GetParams(j);
  assert(dbParams);

  ab=AH_Job_GetBankingApi(j);
  assert(ab);

  user=AH_Job_GetUser(j);
  assert(user);

  userDescriptors=AH_User_GetSepaDescriptors(user);
  if (GWEN_StringList_Count(userDescriptors)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SEPA descriptor found, please update your account information");
    return NULL;
  }

  descrList=AB_Banking_GetSwiftDescriptorsForImExporter(ab, "xml");
  if (descrList==NULL) {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No SWIFT descriptors found for XML imexporter (job \"%s\")", AH_Job_GetName(j));
    return NULL;
  }

  returnDescrList=AB_SwiftDescr_List_new();
  se=GWEN_StringList_FirstEntry(userDescriptors);
  while(se) {
    const char *s;

    s=GWEN_StringListEntry_Data(se);
    if (s && *s) {
      AB_SWIFT_DESCR *tmpDescr;
  
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Handling user supported param [%s] (job \"%s\")", s, AH_Job_GetName(j));
      tmpDescr=AB_SwiftDescr_FromString(s);
      if (tmpDescr) {
        if (AB_SwiftDescr_Matches(tmpDescr, family, version1, 0, 0)) {
          AB_SWIFT_DESCR *descrFromList;
  
          /* found a candidate */
          descrFromList=AB_SwiftDescr_List_FindFirst(descrList,
                                                     AB_SwiftDescr_GetFamily(tmpDescr),
                                                     AB_SwiftDescr_GetVersion1(tmpDescr),
                                                     AB_SwiftDescr_GetVersion2(tmpDescr),
                                                     AB_SwiftDescr_GetVersion3(tmpDescr));
          if (descrFromList) {
            AB_SWIFT_DESCR *descrCopy;
  
            /* store name of selected profile */
            AB_SwiftDescr_SetAlias2(descrFromList, s);
            DBG_ERROR(AQHBCI_LOGDOMAIN,
                      "Adding matching profile [%s] (%s)",
                      AB_SwiftDescr_GetAlias1(tmpDescr),
                      AB_SwiftDescr_GetAlias2(tmpDescr));
            /* copy to return list */
            descrCopy=AB_SwiftDescr_dup(descrFromList);
            AB_SwiftDescr_List_Add(descrCopy, returnDescrList);
          }
        }
        else {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Param [%s] does not match family %s.%d", s, family, version1);
        }
      }
      else {
        DBG_ERROR(AQHBCI_LOGDOMAIN,
                  "Could not create SWIFT descriptor from string \"%s\" (job \"%s\"), ignoring",
                  s, AH_Job_GetName(j));
      }
    }

    se=GWEN_StringListEntry_Next(se);
  }


  AB_SwiftDescr_List_free(descrList);
  if (AB_SwiftDescr_List_GetCount(returnDescrList)==0) {
    DBG_ERROR(AQHBCI_LOGDOMAIN,
              "No matching descriptors found for %s_%03d_* (job %s)",
              family?family:"<none>", version1, AH_Job_GetName(j));
    AB_SwiftDescr_List_free(returnDescrList);
    return NULL;
  }

  return returnDescrList;
}
