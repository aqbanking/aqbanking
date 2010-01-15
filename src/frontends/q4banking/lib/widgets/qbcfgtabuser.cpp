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


#include "qbcfgtabuser.h"

#include <gwenhywfar/debug.h>




QBCfgTabUser::QBCfgTabUser(QBanking *qb,
                           QWidget *parent, const char *name, WFlags f)
:QBCfgTab(qb, parent, name, f){

}



QBCfgTabUser::~QBCfgTabUser() {
}



void QBCfgTabUser::setUserIdInfo(const QString &label,
                                 const QString &toolTip) {
  _userIdLabel=label;
  _userIdToolTip=toolTip;
}



const QString &QBCfgTabUser::getUserIdLabel() const {
  return _userIdLabel;
}



const QString &QBCfgTabUser::getUserIdToolTip() const {
  return _userIdToolTip;
}



void QBCfgTabUser::setCustomerIdInfo(const QString &label,
                                     const QString &toolTip) {
  _customerIdLabel=label;
  _customerIdToolTip=toolTip;
}



const QString &QBCfgTabUser::getCustomerIdLabel() const {
  return _customerIdLabel;
}



const QString &QBCfgTabUser::getCustomerIdToolTip() const {
  return _customerIdToolTip;
}








