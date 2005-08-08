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



#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "flagstaff.h"




FlagStaff::FlagStaff():QObject() {
}



FlagStaff::~FlagStaff(){
}



void FlagStaff::queueUpdated(){
  emit (signalQueueUpdated());
}



void FlagStaff::accountsUpdated(){
  emit(signalAccountsUpdated());
}



void FlagStaff::outboxCountChanged(int count){
  emit (signalOutboxCountChanged(count));
}



void FlagStaff::statusMessage(const QString &s){
  emit (signalStatusMessage(s));
}







