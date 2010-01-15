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
#include <q4banking/qbanking.h>

#include "qbcfgtabpage.h"


class QBCfgTabUser;


class Q4BANKING_API QBCfgTabPageUser: public QBCfgTabPage {
  friend class QBCfgTabUser;
private:
  AB_USER *_user;

  QString _userIdLabel;
  QString _userIdToolTip;
  QString _customerIdLabel;
  QString _customerIdToolTip;

public:
  QBCfgTabPageUser(QBanking *qb,
                   const QString &title,
                   AB_USER *u,
                   QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~QBCfgTabPageUser();

  AB_USER *getUser();

  void setUserIdInfo(const QString &label,
		     const QString &toolTip);
  const QString &getUserIdLabel() const;
  const QString &getUserIdToolTip() const;

  void setCustomerIdInfo(const QString &label,
			 const QString &toolTip);
  const QString &getCustomerIdLabel() const;
  const QString &getCustomerIdToolTip() const;

};


#endif
