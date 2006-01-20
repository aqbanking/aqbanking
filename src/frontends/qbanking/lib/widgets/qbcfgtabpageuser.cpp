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

#include <qbanking/qbanking.h>



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



