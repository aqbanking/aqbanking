/***************************************************************************
 begin       : Thu Jun 18 2009
 copyright   : (C) 2009 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_GUI_P_H
#define AQBANKING_GUI_P_H


#include "abgui.h"


typedef struct AB_GUI AB_GUI;
struct AB_GUI {
  AB_BANKING *banking;
  GWEN_GUI_CHECKCERT_FN checkCertFn;

  GWEN_GUI_READ_DIALOG_PREFS_FN readDialogPrefsFn;
  GWEN_GUI_WRITE_DIALOG_PREFS_FN writeDialogPrefsFn;
};

static void GWENHYWFAR_CB AB_Gui_FreeData(void *bp, void *p);
static int AB_Gui__HashPair(const char *token,
			    const char *pin,
			    GWEN_BUFFER *buf);
static int AB_Gui_CheckCert(GWEN_GUI *gui,
			    const GWEN_SSLCERTDESCR *cd,
			    GWEN_SYNCIO *sio, uint32_t guiid);

static int AB_Gui_WriteDialogPrefs(GWEN_GUI *gui,
				   const char *groupName,
				   GWEN_DB_NODE *db);

static int AB_Gui_ReadDialogPrefs(GWEN_GUI *gui,
				  const char *groupName,
				  const char *altName,
				  GWEN_DB_NODE **pDb);



#endif


