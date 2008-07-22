/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AB_HTTPSESS_P_H
#define AB_HTTPSESS_P_H

#include "httpsession.h"

#include <aqbanking/user.h>



typedef struct AB_HTTP_SESSION AB_HTTP_SESSION;
struct AB_HTTP_SESSION {
  AB_PROVIDER *provider;
  AB_USER *user;
  GWEN_BUFFER *logs;
};


static void GWENHYWFAR_CB AB_HttpSession_FreeData(void *bp, void *p);




#endif

