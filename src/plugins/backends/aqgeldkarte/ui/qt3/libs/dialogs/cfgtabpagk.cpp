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


#include "cfgtabpagk.h"
#include "cfgtabpagk.ui.h"

#include <aqgeldkarte/account.h>

#include <qbanking/qbanking.h>
#include <qbanking/qbcfgtab.h>

#include <chipcard2-client/cards/geldkarte.h>

#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qlistview.h>
#include <qtimer.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qfiledialog.h>




CfgTabPageAccountGeldKarte::CfgTabPageAccountGeldKarte(QBanking *qb,
                                                       AB_ACCOUNT *a,
                                                       QWidget *parent,
                                                       const char *name,
                                                       WFlags f)
:QBCfgTabPageAccount(qb, "GeldKarte", a, parent, name, f) {

  _realPage=new CfgTabPageAccountGeldKarteUi(this);
  setHelpSubject("CfgTabPageAccountGeldKarte");
  setDescription(tr("<p>This page contains GeldKarte specific "
                    "account settings.</p>"));

  addWidget(_realPage);
  _realPage->show();

  connect(_realPage->fromCardButton, SIGNAL(clicked()),
          this, SLOT(slotReadFromCard()));

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



CfgTabPageAccountGeldKarte::~CfgTabPageAccountGeldKarte() {
}



bool CfgTabPageAccountGeldKarte::fromGui() {
  AB_ACCOUNT *a;
  std::string s;

  a=getAccount();
  assert(a);

  return true;
}



bool CfgTabPageAccountGeldKarte::toGui() {
  AB_ACCOUNT *a;

  a=getAccount();
  assert(a);

  return true;
}



bool CfgTabPageAccountGeldKarte::checkGui() {
  return true;
}



void CfgTabPageAccountGeldKarte::slotReadFromCard() {
  LC_CARD *card;
  AB_ACCOUNT *a;
  AB_PROVIDER *pro;
  std::string cardId;
  const char *s;
  GWEN_TYPE_UINT32 bid;

  a=getAccount();
  assert(a);
  pro=getBanking()->getProvider(AG_PROVIDER_NAME);
  assert(pro);

  s=AG_Account_GetCardId(a);
  if (s)
    cardId=std::string(s);
  // this is needed to make the module read *any* card
  AG_Account_SetCardId(a, 0);
  bid=AB_Banking_ShowBox
    (AB_Provider_GetBanking(pro),
     0,
     QBanking::QStringToUtf8String(tr("Accessing Card")).c_str(),
     QBanking::QStringToUtf8String(tr("Reading card, "
                                      "please wait...")).c_str()
    );
  card=AG_Provider_MountCard(pro, a);
  AB_Banking_HideBox(AB_Provider_GetBanking(pro), bid);
  if (card) {
    GWEN_DB_NODE *dbAccount;

    dbAccount=LC_GeldKarte_GetAccountDataAsDb(card);
    assert(dbAccount);
    s=GWEN_DB_GetCharValue(dbAccount, "bankCode", 0, 0);
    if (s) {
      AB_Account_SetBankCode(a, s);
    }
    s=GWEN_DB_GetCharValue(dbAccount, "accountId", 0, 0);
    if (s) {
      AB_Account_SetAccountNumber(a, s);
    }

    /* update views */
    getCfgTab()->updateViews();

    LC_Card_Close(card);
    LC_Card_free(card);
  }
  else {
    if (!cardId.empty())
      AG_Account_SetCardId(a, cardId.c_str());
  }
}








