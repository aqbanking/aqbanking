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


#include "qbcfgtabpageuser.h"
// #include "qbcfgtabuser.h" -- unused, isn't it?


#include <gwenhywfar/debug.h>



QBCfgTabPageUser::QBCfgTabPageUser(QBanking *qb,
                                   const QString &title,
                                   AB_USER *u,
                                   QWidget *parent,
                                   const char *name,
                                   Qt::WFlags f)
:QBCfgTabPage(qb, title, parent, name, f)
,_user(u) {
  assert(qb);
  assert(u);
}



QBCfgTabPageUser::~QBCfgTabPageUser() {
}



AB_USER *QBCfgTabPageUser::getUser() {
  return _user;
}



void QBCfgTabPageUser::setUserIdInfo(const QString &label,
                                     const QString &toolTip) {
  _userIdLabel=label;
  _userIdToolTip=toolTip;
}



const QString &QBCfgTabPageUser::getUserIdLabel() const {
  return _userIdLabel;
}



const QString &QBCfgTabPageUser::getUserIdToolTip() const {
  return _userIdToolTip;
}



void QBCfgTabPageUser::setCustomerIdInfo(const QString &label,
                                         const QString &toolTip) {
  _customerIdLabel=label;
  _customerIdToolTip=toolTip;
}



const QString &QBCfgTabPageUser::getCustomerIdLabel() const {
  return _customerIdLabel;
}



const QString &QBCfgTabPageUser::getCustomerIdToolTip() const {
  return _customerIdToolTip;
}
