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


#include "qbflagstaff.h"




QBFlagStaff::QBFlagStaff():QObject() {
}



QBFlagStaff::~QBFlagStaff(){
}



void QBFlagStaff::queueUpdated(){
  emit (signalQueueUpdated());
}



void QBFlagStaff::accountsUpdated(){
  emit(signalAccountsUpdated());
}



void QBFlagStaff::outboxCountChanged(int count){
  emit (signalOutboxCountChanged(count));
}



void QBFlagStaff::statusMessage(const QString &s){
  emit (signalStatusMessage(s));
}







