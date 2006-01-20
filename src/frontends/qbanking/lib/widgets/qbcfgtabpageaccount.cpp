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


#include "qbcfgtabpageaccount.h"

#include <qbanking/qbanking.h>



QBCfgTabPageAccount::QBCfgTabPageAccount(QBanking *qb,
                                         const QString &title,
                                         AB_ACCOUNT *a,
                                         QWidget *parent,
                                         const char *name,
                                         Qt::WFlags f)
:QBCfgTabPage(qb, title, parent, name, f)
,_account(a) {
  assert(qb);
  assert(a);
}



QBCfgTabPageAccount::~QBCfgTabPageAccount() {
}



AB_ACCOUNT *QBCfgTabPageAccount::getAccount() {
  return _account;
}



