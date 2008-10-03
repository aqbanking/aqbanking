/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQBANKING_BANKINFO_GENERIC_L_H
#define AQBANKING_BANKINFO_GENERIC_L_H

#include <aqbanking/bankinfoplugin_be.h>
#include <aqbanking/banking.h>


AB_BANKINFO_PLUGIN *AB_BankInfoPluginGENERIC_new(AB_BANKING *ab,
						 const char *country);



#endif

