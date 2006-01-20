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

#ifndef QBANKING_CFGTABPAGEUSER_H
#define QBANKING_CFGTABPAGEUSER_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include "qbcfgtabpage.h"



class QBCfgTabPageUser: public QBCfgTabPage {
private:
  AB_USER *_user;

public:
  QBCfgTabPageUser(QBanking *qb,
                   const QString &title,
                   AB_USER *u,
                   QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~QBCfgTabPageUser();

  AB_USER *getUser();

};


#endif
