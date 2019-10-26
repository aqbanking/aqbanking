/***************************************************************************
 begin       : Sat Oct 26 2019
 copyright   : (C) 2019 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQFINTS_BANKING_LOGFILE_H
#define AQFINTS_BANKING_LOGFILE_H


#include <aqbanking/backendsupport/provider.h>


int AF_Control_LogFile(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv);



#endif

