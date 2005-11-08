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


#include "wizard.h"
#include <qbanking/qbanking.h>
#include <qbanking/qbselectbank.h>
#include "selectcontext.h"
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>
#include <qlabel.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>


#include <gwenhywfar/debug.h>



bool Wizard::initUserDataPage() {
  slotSettingsChanged(QString::null);
  QObject::connect((userIdEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((customerIdEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((bankCodeEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((serverEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((nameEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((descriptionEdit),
                   SIGNAL(textChanged(const QString &)),
                   this,
                   SLOT(slotSettingsChanged(const QString &)));
  QObject::connect((fromMediumButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotFromMedium()));
  QObject::connect((bankCodeButton),
		   SIGNAL(clicked()),
                   this,
		   SLOT(slotUserDataBankCodeClicked()));
  QObject::connect((bankCodeEdit),
		   SIGNAL(lostFocus()),
		   this,
		   SLOT(slotUserDataBankCodeLostFocus()));
  QObject::connect((bankCodeEdit),
                   SIGNAL(textChanged(const QString&)),
                   this,
		   SLOT(slotUserDataBankCodeChanged(const QString&)));

  return true;
}



void Wizard::slotFromMedium(){
  SelectContext *sc;
  std::string instcode;
  std::string userid;
  std::string server;
  bool rv;
  int idx;

  sc=new SelectContext(_hbci, _medium);
  rv=sc->selectContext(instcode,
                       userid,
                       server,
                       idx);
  delete sc;
  if (!rv)
    return;

  // otherwise set the text
  bankCodeEdit->setText(QString::fromUtf8(instcode.c_str()));
  userIdEdit->setText(QString::fromUtf8(userid.c_str()));
  serverEdit->setText(QString::fromUtf8(server.c_str()));
  _index=idx;
  slotSettingsChanged(QString::null);
}



QString Wizard::getServerAddr() const {
  QString entered = serverEdit->text();
  const char * stripthis[] = { "http://", "https://", 0 };
  for (const char **a = stripthis ; *a != 0; a++) {
    if (entered.startsWith(QString::fromUtf8(*a)))
      return entered.mid(strlen(*a));
  }
  serverEdit->setText(entered);
  return entered;
}



AH_CRYPT_MODE Wizard::_getCryptMode(AH_MEDIUM *m, int idx) {
  const GWEN_CRYPTTOKEN_CONTEXT *ctx;
  const GWEN_CRYPTTOKEN_CRYPTINFO *ci;
  GWEN_CRYPTTOKEN_CRYPTALGO ca;
  AH_MEDIUM_CTX *mctx;
  AH_CRYPT_MODE cm;
  int rv;

  if (!AH_Medium_IsMounted(m)) {
    rv=AH_Medium_Mount(m);
    if (rv) {
      DBG_ERROR(0, "Error mounting (%d)", rv);
      QMessageBox::critical(this,
                            tr("Mount Medium"),
                            tr("Could not mount medium"),
                            QMessageBox::Ok,QMessageBox::NoButton);
    }
    return AH_CryptMode_Unknown;
  }

  rv=AH_Medium_SelectContext(m, idx);
  if (rv) {
    DBG_ERROR(0, "Could not select context %d (%d)", idx, rv);
    return AH_CryptMode_Unknown;
  }

  mctx=AH_Medium_GetCurrentContext(m);
  assert(mctx);

  ctx=AH_MediumCtx_GetTokenContext(mctx);
  assert(ctx);
  ci=GWEN_CryptToken_Context_GetCryptInfo(ctx);
  assert(ci);
  ca=GWEN_CryptToken_CryptInfo_GetCryptAlgo(ci);
  if (ca==GWEN_CryptToken_CryptAlgo_RSA)
    cm=AH_CryptMode_Rdh;
  else if (ca==GWEN_CryptToken_CryptAlgo_DES_3K)
    cm=AH_CryptMode_Ddv;
  else if (ca==GWEN_CryptToken_CryptAlgo_None)
    cm=AH_CryptMode_Pintan;
  else {
    DBG_ERROR(0, "Invalid crypt algo (%s), unable to detect crypt mode",
              GWEN_CryptToken_CryptAlgo_toString(ca));
    return AH_CryptMode_Unknown;
  }

  return cm;
}



bool Wizard::doUserDataPage(QWidget *p){
  GWEN_INETADDRESS *addr;
  GWEN_ERRORCODE err;
  QString qs;
  int i;
  AH_CRYPT_MODE cm;

  /* do user data page, we have to create the medium etc */
  if (bankCodeEdit->text().length()<8) {
    QMessageBox::critical(this,
                          tr("Invalid Input"),
                          tr("<qt>"
                             "<p>"
                             "The bank code needs at least 8 digits."
                             "</p>"
                             "<p>"
                             "Please correct your entry."
                             "</p>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

  cm=_getCryptMode(_medium, _index);
  if (cm==AH_CryptMode_Unknown) {
    DBG_ERROR(0, "Unknown crypt mode/mount error");
    return false;
  }

  if (!_bankInfo) {
    std::string s;

    s=QBanking::QStringToUtf8String(bankCodeEdit->text());
    if (!s.empty()) {
      AB_BANKINFO *bi;

      bi=AB_Banking_GetBankInfo(_app->getCInterface(),
                                "de", 0, s.c_str());
      AB_BankInfo_free(_bankInfo);
      _bankInfo=bi;
    }
    else {
      AB_BankInfo_free(_bankInfo);
      _bankInfo=0;
    }
  }

  if (!_bankInfo) {
    if (QMessageBox::critical(this,
                              tr("Unknown Bank"),
                              tr("<qt>"
                                 "<p>"
                                 "The bank you selected is unknown."
                                 "</p>"
                                 "<p>"
                                 "Do you want to use it anyway?"
                                 "</p>"),
                              tr("Yes"),tr("No, let me edit"))!=0)
      return false;
  }


  _hasAllKeys=false;
  if (customerIdEdit->text().isEmpty()) {
    int r = QMessageBox::warning(this,
                             tr("No customer id found"),
                             tr("<qt>"
                                "<p>"
                                "You did not enter a customer id."
                                "</p>"
                                "<p>Many banks do not need an explicit customer id. But some other "
				"banks absolutey require a customer id. Please double-check the "
				"information provided to you by your bank.</p>"
                                "<p>"
                                "Are you sure you want to leave the customer "
                                "id empty?"
                                "</p>"
                                "</qt>"
                               ),
                             QMessageBox::Yes,QMessageBox::No);
    if (r != 0 && r != QMessageBox::Yes) {
      customerIdEdit->setFocus();
      return false;
    }
  }

  // TODO: Select context !!


  qs=getServerAddr();
  i=qs.find('/');
  if (i)
    qs.truncate(i);
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, qs.local8Bit());
  if (!GWEN_Error_IsOk(err)) {
    GWEN_TYPE_UINT32 wid;

    wid=AB_Banking_ShowBox(_app->getCInterface(),
                           0,
                           QWidget::tr("Please wait").utf8(),
                           QWidget::tr("Resolving host address...").utf8());
    // Note: The QCString::data() is the conversion that will
    // correctly convert the QCString to the char* pointer.
    DBG_INFO(0, "Resolving hostname \"%s\"",
             qs.local8Bit().data());
    err=GWEN_InetAddr_SetName(addr, qs.local8Bit());
    if (wid)
      AB_Banking_HideBox(_app->getCInterface(), wid);
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR(0,
                "Error resolving hostname \"%s\":",
                qs.local8Bit().data());
      DBG_ERROR_ERR(0, err);
      QMessageBox::critical(this,
			    QWidget::tr("Network Error"),
			    QWidget::tr("Could not resolve the address %1.\n"
                                        "Maybe there is a typo?")
                            .arg(qs),
			    QMessageBox::Ok,QMessageBox::NoButton);
      GWEN_InetAddr_free(addr);
      return false;
    }
  }
  GWEN_InetAddr_free(addr);

  /* select or create bank */
  _bank=AH_HBCI_FindBank(_hbci, 280, bankCodeEdit->text().utf8());
  if (!_bank) {
    /* create bank */
    DBG_INFO(0, "Creating bank");
    _bank=AH_Bank_new(_hbci, 280, bankCodeEdit->text().utf8());
    AH_HBCI_AddBank(_hbci, _bank);
    _bankCreated=true;
  }
  else {
    _bankCreated=false;
  }

  /* select or create user */
  _user=AH_Bank_FindUser(_bank, userIdEdit->text().utf8());
  if (!_user) {
    AH_BPD_ADDR *ba;

    DBG_INFO(0, "Creating user");
    _user=AH_User_new(_bank, userIdEdit->text().utf8(),
                      cm,
                      _medium);
    assert(_user);
    ba=AH_BpdAddr_new();
    assert(ba);
    AH_BpdAddr_SetAddr(ba, getServerAddr().utf8());
    if (cm==AH_CryptMode_Pintan) {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeSSL);
      AH_BpdAddr_SetSuffix(ba, "443");
    }
    else {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeTCP);
      AH_BpdAddr_SetSuffix(ba, "3000");
    }
    AH_User_SetAddress(_user, ba);
    AH_BpdAddr_free(ba);

    AH_Bank_AddUser(_bank, _user);
    _userCreated=true;
  }
  else {
    QMessageBox::critical(this,
                          tr("User Exists"),
                          tr("<qt>"
                             "<p>"
                             "The user already exists."
                             "</p>"
                             "<p>"
                             "This setup is not necessary for users "
                             "which have already been set up."
                             "</p>"
                             "<p>"
                             "Please press <i>cancel</i> after dismissing "
                             "this message."
                             "</p>"
                             "</qt>"
                            ),
                          QMessageBox::Ok,QMessageBox::NoButton);
    _user=0;
    _userCreated=false;
    return false;
  }
  AH_User_SetContextIdx(_user, _index);

  /* now the correct user is selected */
  _hasAllKeys=false;
  if (AH_User_GetCryptMode(_user)==AH_CryptMode_Rdh) {
    GWEN_CRYPTKEY *key1, *key2;

    key1=AH_Medium_GetLocalPubSignKey(_medium);
    key2=AH_Medium_GetLocalPubCryptKey(_medium);
    GWEN_CryptKey_free(key2);
    GWEN_CryptKey_free(key1);
    if (key1 && key2) {
      _hasAllKeys=true;
      DBG_NOTICE(0, "All user keys exist");
    }
  }

  /* select or create customer */
  QCString cuid = (customerIdEdit->text().isEmpty() ? 
		   userIdEdit->text().utf8() :
		   cuid=customerIdEdit->text().utf8() );
  /* always create customer since we just created the user which starts with
   * an empty list of customers. So the customer we are about to create
   * *can* not exist. */
  _customer=AH_Customer_new(_user, cuid);
  AH_Customer_SetFullName(_customer, nameEdit->text().utf8());

  /* Set a descriptiveName in the medium. */
  if (!descriptionEdit->text().isEmpty())
    AH_Medium_SetDescriptiveName(_medium, descriptionEdit->text().utf8());

  if (AH_User_GetCryptMode(_user)==AH_CryptMode_Pintan) {
    /* PIN/TAN only works with HBCI version 2.20. It might work with
     * other versions as well but this is not defined in the specs. */
    AH_Customer_SetHbciVersion(_customer, 220);
    _enableServerTest=false;
    setAppropriate(serverTestPage, false);
  }
  else {
    _enableServerTest=true;
    if (_bankInfo) {
      AB_BANKINFO_SERVICE *sv;

      sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(_bankInfo));
      while (sv) {
        const char *s;

        s=AB_BankInfoService_GetType(sv);
        if (s && strcasecmp(s, "HBCI")==0) {
          s=AB_BankInfoService_GetMode(sv);
          if (s && strcasecmp(s, "PINTAN")!=0) {
            s=AB_BankInfoService_GetPversion(sv);
            if (s) {
              int set=1;

              if (strcmp(s, "2.01")==0)
                AH_Customer_SetHbciVersion(_customer, 201);
              else if (strcmp(s, "2.10")==0)
                AH_Customer_SetHbciVersion(_customer, 210);
              else if (strcmp(s, "2.20")==0)
                AH_Customer_SetHbciVersion(_customer, 220);
              else
                set=0;
              if (set) {
                _enableServerTest=false;
                setAppropriate(serverTestPage, false);
              }
            }
          }

        }
        sv=AB_BankInfoService_List_Next(sv);
      }
    }
  }

  AH_User_AddCustomer(_user, _customer);
  _customerCreated=true;

  setAppropriate(initModePage, false);
  _firstInitMode=true;

  if (_importMode) {
    if (AH_User_GetCryptMode(_user)==AH_CryptMode_Rdh) {
      if (_hasAllKeys) {
        DBG_NOTICE(0, "All user keys exist, asking for first init");
      }
      setAppropriate(initModePage, _hasAllKeys);
    } // if RDH mode
  } /* if not import mode */
  else {
    // not in import mode
    setAppropriate(initModePage, false);
  }

  return true;
}



bool Wizard::undoUserDataPage(QWidget *p){
  // handle bank, user, customer
  if (_bank) {
    if (_bankCreated) {
      /* bank created, so by removing the bank we also remove all other
       * objects below it */
      DBG_INFO(0, "Removing bank and all subordinate objects");
      AH_HBCI_RemoveBank(_hbci, _bank);
      AH_Bank_free(_bank);
    } // if bank created
    else {
      if (_user) {
        if (_userCreated) {
          /* user created, so by removing the user we also remove all other
           * objects below it */
          DBG_INFO(0, "Removing user and all subordinate objects");
          AH_Bank_RemoveUser(_bank, _user);
          AH_User_free(_user);
        } // if _userCreated
        else {
          if (_customer) {
            if (_customerCreated) {
              DBG_INFO(0, "Removing customer");
              AH_User_RemoveCustomer(_user, _customer);
              AH_Customer_free(_customer);
            } // if customer created
          } // if customer
        } // if user not created
      } // if user
    } // if bank not created
  } // if bank
  _customer=0;
  _user=0;
  _bank=0;
  _bankCreated=_userCreated=_customerCreated=false;
  _enableServerTest=true;

  return true;
}



void Wizard::slotUserDataBankCodeLostFocus() {
  std::string s;

  s=QBanking::QStringToUtf8String(bankCodeEdit->text());
  AB_BankInfo_free(_bankInfo);
  _bankInfo=0;
  if (!s.empty()) {
    AB_BANKINFO *bi;

    bi=AB_Banking_GetBankInfo(_app->getCInterface(),
                              "de", 0, s.c_str());
    if (bi) {
      const char *p;

      p=AB_BankInfo_GetBankName(bi);
      if (p)
        bankNameLabel->setText(QString::fromUtf8(p));
    }
    AB_BankInfo_free(_bankInfo);
    _bankInfo=bi;
  }
}



void Wizard::slotUserDataBankCodeChanged(const QString&) {
  if (_bankInfo) {
    DBG_ERROR(0, "Deleting current bank info");
    AB_BankInfo_free(_bankInfo);
    _bankInfo=0;
  }
}



void Wizard::slotUserDataBankCodeClicked() {
  AB_BANKINFO *bi;

  AB_BankInfo_free(_bankInfo);
  _bankInfo=0;
  bi=QBSelectBank::selectBank(_app,
                              0,
                              tr("Select a bank"),
                              bankCodeEdit->text());
  if (bi) {
    const char *s;
    AB_BANKINFO_SERVICE *sv;
    bool isPinTan;

    s=AB_BankInfo_GetBankId(bi);
    if (s)
      bankCodeEdit->setText(QString::fromUtf8(s));

    sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
    isPinTan=pinTanRadio->isOn();

    while(sv) {
      s=AB_BankInfoService_GetType(sv);
      if (s && strcasecmp(s, "HBCI")==0) {
        s=AB_BankInfoService_GetMode(sv);
        if (s) {
          if (!((strcasecmp(s, "PINTAN")==0) ^ isPinTan)) {
            s=AB_BankInfoService_GetAddress(sv);
            serverEdit->setText(QString::fromUtf8(s));
            _bankInfo=bi;
            break;
          }
        }
      }
      sv=AB_BankInfoService_List_Next(sv);
    }
  }
}


























