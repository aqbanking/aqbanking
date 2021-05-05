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


#include "aqhbci/msglayer/hbci-updates_p.h"
#include "aqhbci/banking/user_l.h"
#include "aqhbci/banking/account_l.h"

#include "aqbanking/i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>



int AH_HBCI_UpdateDbUser(AH_HBCI *hbci, GWEN_DB_NODE *db)
{
  int rv;
  uint32_t oldVersion;
  uint32_t currentVersion;

  oldVersion=AH_HBCI_GetLastVersion(hbci);

  currentVersion=
    (AQBANKING_VERSION_MAJOR<<24) |
    (AQBANKING_VERSION_MINOR<<16) |
    (AQBANKING_VERSION_PATCHLEVEL<<8) |
    AQBANKING_VERSION_BUILD;

  if (currentVersion>oldVersion) {
    DBG_WARN(AQHBCI_LOGDOMAIN,
             "Updating user from %d.%d.%d.%d",
             (oldVersion>>24) & 0xff,
             (oldVersion>>16) & 0xff,
             (oldVersion>>8) & 0xff,
             oldVersion & 0xff);

    if (oldVersion<((5<<24) | (0<<16) | (3<<8) | 1)) {
      DBG_WARN(AQHBCI_LOGDOMAIN, "Updating user from pre 5.0.3.1");
      rv=AH_HBCI_UpdateUser_5_0_3_1(hbci, db);
      if (rv) {
        DBG_INFO(AQHBCI_LOGDOMAIN, "here (%d)", rv);
        return rv;
      }
    }

    /* insert more updates here */


    /* updated! */
    return 1;
  } /* if update */
  else
    /* not updated */
    return 0;
}



int AH_HBCI_UpdateUser_5_0_3_1(AH_HBCI *hbci, GWEN_DB_NODE *db)
{
  int tmn;

  /* update selectedTanMethod */
  tmn=GWEN_DB_GetIntValue(db, "selectedTanMethod", 0, 0);
  if (tmn>0 && tmn < 1000) {
    GWEN_DB_NODE *dbT;

    /* get first version group of "bpd/bpdjobs/HITANS" */
    dbT=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpd");
    if (dbT)
      dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "bpdjobs");
    if (dbT)
      dbT=GWEN_DB_GetGroup(dbT, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "HITANS");
    if (dbT)
      dbT=GWEN_DB_GetFirstGroup(dbT);

    if (dbT) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Starting with group %s", GWEN_DB_GroupName(dbT));
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "No group");
    }

    while (dbT) {
      int foundTm=0;
      int version;

      /* find tanMethod group in any of the tanMethod groups of every HITANS version */
      version=atoi(GWEN_DB_GroupName(dbT));
      if (version>0) {
        GWEN_DB_NODE *dbM;

        dbM=GWEN_DB_FindFirstGroup(dbT, "tanMethod");
        while (dbM) {
          int fn;

          fn=GWEN_DB_GetIntValue(dbM, "function", 0, 0);
          if (fn==tmn) {
            int newFn;

            newFn=(version*1000)+fn;
            DBG_WARN(AQHBCI_LOGDOMAIN, "Updating selectedTanMethod from %d to %d", tmn, newFn);
            GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                                "selectedTanMethod", newFn);
            foundTm=1;
            break;
          }
          dbM=GWEN_DB_FindNextGroup(dbM, "tanMethod");
        }
      }
      if (foundTm)
        break;
      dbT=GWEN_DB_GetNextGroup(dbT);
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "No selectedTanMethod");
  }

  return 0;
}









