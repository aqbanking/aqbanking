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

#ifndef QBANKING_BANKSELECTOR_H
#define QBANKING_BANKSELECTOR_H

#include "qbbankselector.ui.h"


class QBanking;


class QBBankSelector : protected QBBankSelectorUi {
  Q_OBJECT

private:
  QBanking *_qbanking;
  QString _country;

public:
  QBBankSelector(QBanking *qb,
                 const QString &country=QString("de"),
                 QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QBBankSelector();

  QString getBankCode();

private slots:
  void slotButtonClicked();
};

#endif

