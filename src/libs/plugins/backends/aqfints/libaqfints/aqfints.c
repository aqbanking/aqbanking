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



static void _openLogger(const char *sLogdomainId, const char *sLogdomainLabel, const char *sEnvName);





int AQFINTS_Init()
{
  int rv;

  rv=GWEN_Init();
  if (rv) {
    DBG_ERROR_ERR(AQFINTS_LOGDOMAIN, rv);
    return rv;
  }

  _openLogger(AQFINTS_LOGDOMAIN, "aqfints", "AQFINTS_LOGLEVEL");
  _openLogger(AQFINTS_PARSER_LOGDOMAIN, "aqfints-parser", "AQFINTS_PARSER_LOGLEVEL");

  DBG_INFO(AQFINTS_LOGDOMAIN, "AqFinTS initialising");

  return 0;
}



int AQFINTS_Fini()
{
  GWEN_Logger_Close(AQFINTS_LOGDOMAIN);
  GWEN_Fini();
  return 0;
}



void _openLogger(const char *sLogdomainId, const char *sLogdomainLabel, const char *sEnvName)
{
  const char *s;

  if (!GWEN_Logger_IsOpen(sLogdomainId)) {
    GWEN_Logger_Open(sLogdomainId,
                     sLogdomainLabel, 0,
                     GWEN_LoggerType_Console,
                     GWEN_LoggerFacility_User);
  }

  s=getenv(sEnvName);
  if (s && *s) {
    GWEN_LOGGER_LEVEL ll;
  
    ll=GWEN_Logger_Name2Level(s);
    GWEN_Logger_SetLevel(sLogdomainId, ll);
  }
  else
    GWEN_Logger_SetLevel(sLogdomainId, GWEN_LoggerLevel_Notice);

}

