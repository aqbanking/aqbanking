/***************************************************************************
 begin       : Sat Nov 29 2025
 copyright   : (C) 2025 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqhbci/banking/user.h"
#include "aqhbci/banking/provider_l.h"
#include "aqhbci/banking/provider_tan.h"
#include "aqhbci/dialogs/dlg_vop.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/gui.h>


#include <stdlib.h>
#include <stdio.h>
#include <errno.h>



/* ------------------------------------------------------------------------------------------------
 * defs
 * ------------------------------------------------------------------------------------------------
 */

#define A_ARG GWEN_ARGS_FLAGS_HAS_ARGUMENT
#define A_END (GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST)
#define A_CHAR GWEN_ArgsType_Char
#define A_INT GWEN_ArgsType_Int



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static AH_VOP_RESULT_LIST *_createResultList(void);



/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */



int AH_Control_Test2(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv)
{
  GWEN_DIALOG *dlg;
  AH_VOP_RESULT_LIST *resultList;
  const char *sMsg=
    "This is a reminder from your bank just to tell you that you are "
    "screwed if you really allow me to send your money to THAT person "
    "and that we won't help you get even a cent back.<br>"
    "Oh, and we will send this message every single time you try to transfer money "
    "to anyone.";

  resultList=_createResultList();

  dlg=AH_VopDialog_new("Transfer", "Any bank", "Any user", sMsg, resultList);
  if (dlg) {
    int rv;

    rv=GWEN_Gui_ExecDialog(dlg, 0);
    if (rv==0) {
      /* rejected */
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Rejected");
    }
    else {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Accepted");
    }
  }
  else {
    DBG_ERROR(AQHBCI_LOGDOMAIN, "Error creating dialog");
  }

  GWEN_Dialog_free(dlg);

  AH_VopResult_List_free(resultList);

  return 0;
}



AH_VOP_RESULT_LIST *_createResultList()
{
  AH_VOP_RESULT_LIST *resultList;
  AH_VOP_RESULT *r;

  resultList=AH_VopResult_List_new();

  r=AH_VopResult_new();
  AH_VopResult_SetLocalBic(r, "ANYBICXXXXX");
  AH_VopResult_SetRemoteIban(r, "DE99111111112222222222");
  AH_VopResult_SetRemoteName(r, "Mickey Mouse");
  AH_VopResult_SetAltRemoteName(r, "Mickey H. Mouse");
  AH_VopResult_List_Add(r, resultList);

  r=AH_VopResult_new();
  AH_VopResult_SetLocalBic(r, "ANYBICXXXXX");
  AH_VopResult_SetRemoteIban(r, "DE88333333334444444444");
  AH_VopResult_SetRemoteName(r, "Minnie Mouse");
  AH_VopResult_SetAltRemoteName(r, "Minnie K. Mouse");
  AH_VopResult_List_Add(r, resultList);

  return resultList;
}







