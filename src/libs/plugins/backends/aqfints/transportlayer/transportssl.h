/***************************************************************************
 begin       : Wed Jul 31 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQFINTS_TRANSPORTSSL_H
#define AQFINTS_TRANSPORTSSL_H


#include "transportlayer/transport.h"



AQFINTS_TRANSPORT *AQFINTS_TransportSsl_new(const char *url);


int AQFINTS_TransportSsl_TestConnection(AQFINTS_TRANSPORT *trans);


const char *AQFINTS_TransportSsl_GetContentType(const AQFINTS_TRANSPORT *trans);
void AQFINTS_TransportSsl_SetContentType(AQFINTS_TRANSPORT *trans, const char *s);

const char *AQFINTS_TransportSsl_GetUserAgent(const AQFINTS_TRANSPORT *trans);
void AQFINTS_TransportSsl_SetUserAgent(AQFINTS_TRANSPORT *trans, const char *s);

int AQFINTS_TransportSsl_GetVersionMajor(const AQFINTS_TRANSPORT *trans);
void AQFINTS_TransportSsl_SetVersionMajor(AQFINTS_TRANSPORT *trans, int v);

int AQFINTS_TransportSsl_GetVersionMinor(const AQFINTS_TRANSPORT *trans);
void AQFINTS_TransportSsl_SetVersionMinor(AQFINTS_TRANSPORT *trans, int v);





#endif

