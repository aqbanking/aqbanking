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


// QBanking includes
#include "qbselectbank.h"
#include "qbanking.h"

// AqBanking includes
#include <aqbanking/banking.h>

// Gwenhywfar includes
#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>

// QT includes
#include <qlistview.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>

// C++ includes
#include <string>


QBSelectBank::QBSelectBank(QBanking *kb,
                           QWidget* parent,
                           const char* name,
                           bool modal,
                           WFlags fl)
:QBSelectBankUi(parent, name, modal, fl)
,_app(kb)
,_bankInfo(0)
,_country("de")
,_changed(false) {

  bankListView->setAllColumnsShowFocus(TRUE);
  bankListView->setSelectionMode(QListView::Single);
  QObject::connect(bankCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotUpdate()));
  QObject::connect(swiftCodeEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotUpdate()));
  QObject::connect(bankNameEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotUpdate()));
  QObject::connect(locationEdit, SIGNAL(lostFocus()),
                   this, SLOT(slotUpdate()));
  QObject::connect(onlineCheckBox, SIGNAL(toggled(bool)),
                   this, SLOT(slotOnlineToggled(bool)));

  QObject::connect(bankCodeEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(slotChanged(const QString&)));
  QObject::connect(swiftCodeEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(slotChanged(const QString&)));
  QObject::connect(bankNameEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(slotChanged(const QString&)));
  QObject::connect(locationEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(slotChanged(const QString&)));
  QObject::connect(bankListView, SIGNAL(selectionChanged()),
                   this, SLOT(slotSelectionChanged()));
  QObject::connect(bankListView,
                   SIGNAL(doubleClicked(QListViewItem *,
                                        const QPoint &,
                                        int)),
                   this,
                   SLOT(slotDoubleClicked(QListViewItem *,
                                          const QPoint &,
                                          int)));
  QObject::connect(helpButton, SIGNAL(clicked()),
                   this, SLOT(slotHelpClicked()));
}



QBSelectBank::~QBSelectBank(){
  AB_BankInfo_free(_bankInfo);
}



const AB_BANKINFO *QBSelectBank::selectedBankInfo() const {
  return _bankInfo;
}



void QBSelectBank::slotChanged(const QString &) {
  _changed=true;
}



void QBSelectBank::slotUpdate() {
  std::string sBankCode;
  std::string sBic;
  std::string sName;
  std::string sLoc;
  AB_BANKINFO_LIST2 *bl=0;
  AB_BANKINFO *tbi=0;
  int rv;

  if (!_changed) {
    DBG_ERROR(0, "Not changed, not updating...");
    return;
  }
  DBG_ERROR(0, "Updating...");
  sBankCode=QBanking::QStringToUtf8String(bankCodeEdit->text())+"*";
  sBic=QBanking::QStringToUtf8String(swiftCodeEdit->text())+"*";
  sName=QBanking::QStringToUtf8String(bankNameEdit->text())+"*";
  sLoc=QBanking::QStringToUtf8String(locationEdit->text())+"*";

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();

  if (sBankCode.length()>=3) {
    AB_BankInfo_SetBankId(tbi, sBankCode.c_str());
  }
  else if (sBic.length()>=3) {
    AB_BankInfo_SetBic(tbi, sBic.c_str());
  }
  else if (sName.length()>=3 || sLoc.length()>=3) {
    AB_BankInfo_SetBankName(tbi, sName.c_str());
    AB_BankInfo_SetLocation(tbi, sLoc.c_str());
  }
  else {
    AB_BankInfo_List2_freeAll(bl);
    AB_BankInfo_free(tbi);
    return;
  }

  rv=AB_Banking_GetBankInfoByTemplate(_app->getCInterface(),
                                      _country.c_str(),
                                      tbi,
                                      bl);
  if (rv==0) {
    AB_BANKINFO_LIST2_ITERATOR *it;

    bankListView->clear();
    _changed=false;
    it=AB_BankInfo_List2_First(bl);
    if (it) {
      AB_BANKINFO *bi;
  
      bi=AB_BankInfo_List2Iterator_Data(it);
      assert(bi);
      while(bi) {
        const char *blz;
        const char *bic;
        const char *name;
        const char *loc;
  
        blz=AB_BankInfo_GetBankId(bi);
        bic=AB_BankInfo_GetBic(bi);
        name=AB_BankInfo_GetBankName(bi);
        loc=AB_BankInfo_GetLocation(bi);
        if (loc==0)
          loc=AB_BankInfo_GetCity(bi);
        if (blz==0)
          blz="";
        if (bic==0)
          bic="";
        if (!name)
          name="";
        if (loc==0)
          loc="";

        if ((sBankCode.empty() ||
             GWEN_Text_ComparePattern(blz, sBankCode.c_str(), 0)!=-1) &&
            (sBic.empty() ||
             GWEN_Text_ComparePattern(bic, sBic.c_str(), 0)!=-1) &&
            (sName.empty() ||
             GWEN_Text_ComparePattern(name, sName.c_str(), 0)!=-1) &&
            (sLoc.empty() ||
             GWEN_Text_ComparePattern(loc, sLoc.c_str(), 0)!=-1)) {
          AB_BANKINFO_SERVICE *sv;
          QString onl;

          sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
          while(sv) {
            const char *s;

            s=AB_BankInfoService_GetType(sv);
            if (s) {
              QString qs;

              qs=QString::fromUtf8(s);
              if (onl.isEmpty())
                onl=qs;
              else if (-1==onl.find(qs)) {
                onl += "," + qs;
              }
            }
            sv=AB_BankInfoService_List_Next(sv);
          }
          if (!onlineCheckBox->isChecked() || !onl.isEmpty()) {
            QListViewItem *li;

            li=new QListViewItem(bankListView,
                                 QString::fromUtf8(blz),
                                 QString::fromUtf8(name),
                                 QString::fromUtf8(loc),
                                 QString::fromUtf8(bic),
                                 onl);
          }
        }
        bi=AB_BankInfo_List2Iterator_Next(it);
      } // while
      AB_BankInfo_List2Iterator_free(it);
    } // if list not empty
  } // if list received
  AB_BankInfo_List2_freeAll(bl);
  AB_BankInfo_free(tbi);
}



void QBSelectBank::slotSelectionChanged() {
  QListViewItemIterator it(bankListView);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      bankCodeEdit->setText(it.current()->text(0));
      bankNameEdit->setText(it.current()->text(1));
      locationEdit->setText(it.current()->text(2));
      swiftCodeEdit->setText(it.current()->text(3));
      break;
    }
  } // for
}



AB_BANKINFO *QBSelectBank::_getBankInfo() {
  std::string sBankCode;
  std::string sBic;
  std::string sName;
  std::string sLoc;
  AB_BANKINFO_LIST2 *bl;
  AB_BANKINFO *tbi;
  int rv;
  AB_BANKINFO *nbi=0;

  sBankCode=QBanking::QStringToUtf8String(bankCodeEdit->text())+"*";
  sBic=QBanking::QStringToUtf8String(swiftCodeEdit->text())+"*";
  sName=QBanking::QStringToUtf8String(bankNameEdit->text())+"*";
  sLoc=QBanking::QStringToUtf8String(locationEdit->text())+"*";

  bl=AB_BankInfo_List2_new();
  tbi=AB_BankInfo_new();

  DBG_ERROR(0, "Bankname: %s Location: %s BIC: %s BankCode: %s",
            sName.c_str(), sLoc.c_str(),
            sBic.c_str(), sBankCode.c_str());

  if (sBankCode.length()>=3) {
    DBG_ERROR(0, "Setting bank id");
    AB_BankInfo_SetBankId(tbi, sBankCode.c_str());
  }
  else if (sBic.length()>=3) {
    DBG_ERROR(0, "Setting bic");
    AB_BankInfo_SetBic(tbi, sBic.c_str());
  }
  else if (sName.length()>=3 || sLoc.length()>=3) {
    DBG_ERROR(0,
              "Searching for %s/%s",
              sName.c_str(), sLoc.c_str());
    AB_BankInfo_SetBankName(tbi, sName.c_str());
    AB_BankInfo_SetLocation(tbi, sLoc.c_str());
  }
  else {
    AB_BankInfo_List2_freeAll(bl);
    AB_BankInfo_free(tbi);
    return 0;
  }

  rv=AB_Banking_GetBankInfoByTemplate(_app->getCInterface(),
                                      _country.c_str(),
                                      tbi,
                                      bl);
  if (rv==0) {
    AB_BANKINFO_LIST2_ITERATOR *it;

    it=AB_BankInfo_List2_First(bl);
    if (it) {
      AB_BANKINFO *bi;
  
      bi=AB_BankInfo_List2Iterator_Data(it);
      assert(bi);
      while(bi) {
        const char *blz;
        const char *bic;
        const char *name;
        const char *loc;
  
        blz=AB_BankInfo_GetBankId(bi);
        bic=AB_BankInfo_GetBic(bi);
        name=AB_BankInfo_GetBankName(bi);
        loc=AB_BankInfo_GetLocation(bi);
        if (loc==0)
          loc=AB_BankInfo_GetCity(bi);
        if (blz==0)
          blz="";
        if (bic==0)
          bic="";
        if (!name)
          name="";
        if (loc==0)
          loc="";

        if ((sBankCode.empty() ||
             GWEN_Text_ComparePattern(blz, sBankCode.c_str(), 0)!=-1) &&
            (sBic.empty() ||
             GWEN_Text_ComparePattern(bic, sBic.c_str(), 0)!=-1) &&
            (sName.empty() ||
             GWEN_Text_ComparePattern(name, sName.c_str(), 0)!=-1) &&
            (sLoc.empty() ||
             GWEN_Text_ComparePattern(loc, sLoc.c_str(), 0)!=-1)) {
          if (nbi) {
            AB_BankInfo_free(nbi);
            nbi=0;
            break;
          }
          else {
            nbi=AB_BankInfo_dup(bi);
          }
        }
        bi=AB_BankInfo_List2Iterator_Next(it);
      } // while
      AB_BankInfo_List2Iterator_free(it);
    } // if list not empty
  } // if list received

  AB_BankInfo_List2_freeAll(bl);
  AB_BankInfo_free(tbi);
  return nbi;
}



void QBSelectBank::accept() {
  AB_BankInfo_free(_bankInfo);
  _bankInfo=_getBankInfo();
  if (_bankInfo)
    QBSelectBankUi::accept();
  else {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No bank selected."),
			  QMessageBox::Retry,QMessageBox::NoButton);
  }
}



void QBSelectBank::slotDoubleClicked(QListViewItem *lv,
                                   const QPoint &,
                                   int) {
  if (lv) {
    bankCodeEdit->setText(lv->text(0));
    bankNameEdit->setText(lv->text(1));
    locationEdit->setText(lv->text(2));
    swiftCodeEdit->setText(lv->text(3));
    accept();
  }
}



AB_BANKINFO *QBSelectBank::selectBank(QBanking *kb,
				      QWidget *parent,
				      const QString &title,
				      const QString &country,
				      const QString &bankCode,
				      const QString &swiftCode,
				      const QString &bankName,
				      const QString &location) {

  QBSelectBank sb(kb, parent, 0, false);

  if (!title.isEmpty())
    sb.setCaption(title);
  if (!country.isEmpty()) {
    sb._country=QBanking::QStringToUtf8String(country);
  }
  if (!bankCode.isEmpty())
    sb.bankCodeEdit->setText(bankCode);
  if (!swiftCode.isEmpty())
    sb.swiftCodeEdit->setText(swiftCode);
  if (!bankName.isEmpty())
    sb.bankNameEdit->setText(bankName);
  if (!location.isEmpty())
    sb.locationEdit->setText(location);
  sb._changed=true;
  sb.slotUpdate();

  if (sb.exec()==QDialog::Accepted) {
    const AB_BANKINFO *bi;

    bi=sb.selectedBankInfo();
    if (bi) {
      AB_BANKINFO *nbi;

      nbi=AB_BankInfo_dup(bi);
      return nbi;
    }
  }

  return 0;
}



void QBSelectBank::slotOnlineToggled(bool on) {
  _changed=true;
  slotUpdate();
}



void QBSelectBank::slotHelpClicked() {
  _app->invokeHelp("QBSelectBank", "none");
}



#include "qbselectbank.moc"



