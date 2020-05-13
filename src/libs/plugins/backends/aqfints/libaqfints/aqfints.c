/***************************************************************************
 begin       : Sun Jul 28 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "libaqfints/aqfints.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/debug.h>

#include <stdlib.h>





int AQFINTS_Init()
{
  int rv;
  const char *s;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(AQFINTS_LOGDOMAIN, rv);
    return rv;
  }
  if (!GWEN_Logger_IsOpen(AQFINTS_LOGDOMAIN)) {
    GWEN_Logger_Open(AQFINTS_LOGDOMAIN,
                     "aqbankservers", 0,
                     GWEN_LoggerType_Console,
                     GWEN_LoggerFacility_User);
  }

  s=getenv("AQFINTS_LOGLEVEL");
  if (s && *s) {
    GWEN_LOGGER_LEVEL ll;

    ll=GWEN_Logger_Name2Level(s);
    GWEN_Logger_SetLevel(AQFINTS_LOGDOMAIN, ll);
  }
  else
    GWEN_Logger_SetLevel(AQFINTS_LOGDOMAIN, GWEN_LoggerLevel_Notice);

  DBG_INFO(AQFINTS_LOGDOMAIN, "AqFinTS initialising");

  return 0;
}



int AQFINTS_Fini()
{
  GWEN_Logger_Close(AQFINTS_LOGDOMAIN);
  GWEN_Fini();
  return 0;
}


