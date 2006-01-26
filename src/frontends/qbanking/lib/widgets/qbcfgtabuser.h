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

#ifndef QBANKING_CFGTABUSER_H
#define QBANKING_CFGTABUSER_H


#include <qbanking/qbanking.h>
#include <qbanking/qbcfgtab.h>

#include <aqbanking/banking.h>

#include <gwenhywfar/types.h>

#include <qstring.h>


class QBanking;
class QBCfgTabPage;


class QBANKING_API QBCfgTabUser: protected QBCfgTab {
private:
  QString _userIdLabel;
  QString _userIdToolTip;
  QString _customerIdLabel;
  QString _customerIdToolTip;

public:
  QBCfgTabUser(QBanking *qb,
	       QWidget *parent=0, const char *name=0, WFlags f=0);
  virtual ~QBCfgTabUser();

  void setUserIdInfo(const QString &label,
		     const QString &toolTip);
  const QString &getUserIdLabel() const;
  const QString &getUserIdToolTip() const;

  void setCustomerIdInfo(const QString &label,
			 const QString &toolTip);
  const QString &getCustomerIdLabel() const;
  const QString &getCustomerIdToolTip() const;

  QBCfgTabUser *getCfgTabUser();

};


#endif
