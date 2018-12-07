/***************************************************************************
 $RCSfile: medium_p.h,v $
                             -------------------
    cvs         : $Id: medium_p.h,v 1.3 2006/01/23 05:16:27 aquamaniac Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef EBC_CLIENT_DIALOG_L_H
#define EBC_CLIENT_DIALOG_L_H

#include "msg/msg.h"

#include <aqbanking/provider.h>
#include <aqbanking/user.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/httpsession.h>



GWEN_HTTP_SESSION *EBC_Dialog_new(AB_PROVIDER *pro, AB_USER *u);


int EBC_Dialog_ExchangeMessages(GWEN_HTTP_SESSION *sess,
				EB_MSG *msg,
                                EB_MSG **pResponse);



#endif

