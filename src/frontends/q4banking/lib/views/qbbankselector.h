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

#include <q4banking/qbbankselector.ui.h>
#include <q4banking/qbanking.h>


class QBanking;


class Q4BANKING_API QBBankSelector : public QWidget, protected Ui_QBBankSelectorUi {
  Q_OBJECT

private:
  QBanking *_qbanking;
  QString _country;

public:
  QBBankSelector(QBanking *qb,
                 const QString &country=QString("de"),
                 QWidget* parent=0, const char* name=0, Qt::WFlags fl=0);
  ~QBBankSelector();

  QString getBankCode();

private slots:
  void slotButtonClicked();
};

#endif

