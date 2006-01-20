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

#include "qbbankselector.h"
#include "qbselectbank.h"
#include <qlineedit.h>




QBBankSelector::QBBankSelector(QBanking *qb,
                               const QString &country,
                               QWidget* parent,
                               const char* name,
                               WFlags fl)
:QBBankSelectorUi(parent, name, fl)
,_qbanking(qb)
,_country(country) {
}



QBBankSelector::~QBBankSelector() {
}



QString QBBankSelector::getBankCode() {
  return lineEdit->text();
}



void QBBankSelector::slotButtonClicked() {
  AB_BANKINFO *bi;

  bi=QBSelectBank::selectBank(_qbanking, this,
                              tr("Select the bank"),
                              _country);
  if (bi) {
    const char *s;

    s=AB_BankInfo_GetBankId(bi);
    assert(s);
    lineEdit->setText(QString::fromUtf8(s));
    AB_BankInfo_free(bi);
  }
}


#include "qbbankselector.moc"




