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


#include "editctuser.h"
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



EditCtUser::EditCtUser(QBanking *qb,
                       WizardInfo *wi,
                       QWidget* parent,
                       const char* name,
                       bool modal,
                       WFlags fl)
:EditCtUserUi(parent, name, fl)
,_app(qb)
,_wInfo(wi)
,_bankInfo(0)
,_dataIsOk(false) {
  int i;

  assert(_wInfo->getMedium());

  connect(bankCodeButton, SIGNAL(clicked()),
          this, SLOT(slotBankCodeClicked()));
  connect(bankCodeEdit, SIGNAL(lostFocus()),
          this, SLOT(slotBankCodeLostFocus()));
  connect(bankCodeEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotBankCodeChanged(const QString&)));
  connect(userCombo, SIGNAL(activated(int)),
          this, SLOT(slotContextActivated(int)));

  for (i=0; i<5; i++) {
    GWEN_BUFFER *bufUserId;
    char numbuf[16];

    bufUserId=GWEN_Buffer_new(0, 64, 0, 1);
    snprintf(numbuf, sizeof(numbuf), "%d:", i+1);
    GWEN_Buffer_AppendString(bufUserId, numbuf);
    if (AH_Medium_ReadContext(_wInfo->getMedium(), i, 0, 0, bufUserId, 0, 0)){
      GWEN_Buffer_free(bufUserId);
      break;
    }
    userCombo->insertItem(QString::fromUtf8(GWEN_Buffer_GetStart(bufUserId)));
    GWEN_Buffer_free(bufUserId);
  }

  DBG_INFO(0, "Using Context %d", _wInfo->getContext());

  userCombo->setCurrentItem(_wInfo->getContext());
  _fromContext(_wInfo->getContext());
}



EditCtUser::~EditCtUser() {
}



QString EditCtUser::_getServerAddr() const {
  QString entered = serverEdit->text();
  const char * stripthis[] = { "http://", "https://", 0 };
  for (const char **a = stripthis ; *a != 0; a++) {
    if (entered.startsWith(QString::fromUtf8(*a)))
      return entered.mid(strlen(*a));
  }
  serverEdit->setText(entered);
  return entered;
}



AH_CRYPT_MODE EditCtUser::_getCryptMode(AH_MEDIUM *m, int idx) {
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



bool EditCtUser::apply(){
  GWEN_INETADDRESS *addr;
  GWEN_ERRORCODE err;
  QString qs;
  int i;
  AH_CRYPT_MODE cm;
  AH_MEDIUM *m;
  AH_BANK *b;
  AH_USER *u;
  AH_CUSTOMER *cu;
  int idx;
  QCString cuid;

  /* do user data page, we have to create the medium etc */
  m=_wInfo->getMedium();

  cuid=(customerIdEdit->text().isEmpty() ?
        userIdEdit->text().utf8() :
        cuid=customerIdEdit->text().utf8() );
  /* always create customer since we just created the user which starts with
   * an empty list of customers. So the customer we are about to create
   * *can* not exist. */
  if (AH_HBCI_CheckStringSanity(cuid)) {
    QMessageBox::critical(this,
                          tr("Invalid Input"),
                          tr("<qt>"
                             "<p>"
                             "The customer id contains invalid characters."
                             "</p>"
                             "<p>"
                             "Please correct your entry."
                             "</p>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }

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

  cm=_getCryptMode(_wInfo->getMedium(),
                   _wInfo->getContext());
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

  qs=_getServerAddr();
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
  b=AH_HBCI_FindBank(_wInfo->getHbci(), 280, bankCodeEdit->text().utf8());
  if (!b) {
    /* create bank */
    DBG_INFO(0, "Creating bank");
    b=AH_Bank_new(_wInfo->getHbci(), 280, bankCodeEdit->text().utf8());
    _wInfo->setBank(b);
    AH_HBCI_AddBank(_wInfo->getHbci(), b);
    _wInfo->addFlags(WIZARDINFO_FLAGS_BANK_CREATED);
  }
  else {
    _wInfo->setBank(b);
    _wInfo->subFlags(WIZARDINFO_FLAGS_BANK_CREATED);
  }

  /* select or create user */
  u=AH_Bank_FindUser(b, userIdEdit->text().utf8());
  if (!u) {
    AH_BPD_ADDR *ba;

    DBG_INFO(0, "Creating user");
    u=AH_User_new(b, userIdEdit->text().utf8(), cm, m);
    assert(u);
    ba=AH_BpdAddr_new();
    assert(ba);
    AH_BpdAddr_SetAddr(ba, _getServerAddr().utf8());
    if (cm==AH_CryptMode_Pintan) {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeSSL);
      AH_BpdAddr_SetSuffix(ba, "443");
    }
    else {
      AH_BpdAddr_SetType(ba, AH_BPD_AddrTypeTCP);
      AH_BpdAddr_SetSuffix(ba, "3000");
    }
    AH_User_SetAddress(u, ba);
    AH_BpdAddr_free(ba);

    AH_Bank_AddUser(b, u);
    _wInfo->setUser(u);
    _wInfo->addFlags(WIZARDINFO_FLAGS_USER_CREATED);
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
    _wInfo->setUser(0);
    _wInfo->subFlags(WIZARDINFO_FLAGS_USER_CREATED);
    return false;
  }

  idx=userCombo->currentItem();
  _wInfo->setContext(idx);
  AH_User_SetContextIdx(u, idx);

  /* select or create customer */
  cu=AH_Customer_new(u, cuid);
  AH_Customer_SetFullName(cu, nameEdit->text().utf8());

  /* Set a descriptiveName in the medium. */
  if (!descriptionEdit->text().isEmpty())
    AH_Medium_SetDescriptiveName(m, descriptionEdit->text().utf8());

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    /* PIN/TAN only works with HBCI version 2.20. It might work with
     * other versions as well but this is not defined in the specs. */
    AH_Customer_SetHbciVersion(cu, 220);
  }
  else {
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
                AH_Customer_SetHbciVersion(cu, 201);
              else if (strcmp(s, "2.10")==0)
                AH_Customer_SetHbciVersion(cu, 210);
              else if (strcmp(s, "2.20")==0)
                AH_Customer_SetHbciVersion(cu, 220);
              else
                set=0;
            }
          }

        }
        sv=AB_BankInfoService_List_Next(sv);
      }
    }
  }

  AH_User_AddCustomer(u, cu);
  _wInfo->setCustomer(cu);
  _wInfo->addFlags(WIZARDINFO_FLAGS_CUST_CREATED);

  return true;
}



bool EditCtUser::undo(){
  AH_BANK *b;

  // handle bank, user, customer
  b=_wInfo->getBank();
  if (b) {
    if (_wInfo->getFlags() & WIZARDINFO_FLAGS_BANK_CREATED) {
      /* bank created, so by removing the bank we also remove all other
       * objects below it */
      DBG_INFO(0, "Removing bank and all subordinate objects");
      AH_HBCI_RemoveBank(_wInfo->getHbci(), b);
      _wInfo->setBank(0);
      _wInfo->subFlags(WIZARDINFO_FLAGS_BANK_CREATED);
      AH_Bank_free(b);
    } // if bank created
    else {
      AH_USER *u;

      u=_wInfo->getUser();
      if (u) {
        if (_wInfo->getFlags() & WIZARDINFO_FLAGS_USER_CREATED) {
          /* user created, so by removing the user we also remove all other
           * objects below it */
          DBG_INFO(0, "Removing user and all subordinate objects");
          AH_Bank_RemoveUser(b, u);
          _wInfo->setUser(0);
          _wInfo->subFlags(WIZARDINFO_FLAGS_USER_CREATED);
          AH_User_free(u);
        } // if _userCreated
        else {
          AH_CUSTOMER *cu;

          cu=_wInfo->getCustomer();
          if (cu) {
            if (_wInfo->getFlags() & WIZARDINFO_FLAGS_CUST_CREATED) {
              DBG_INFO(0, "Removing customer");
              AH_User_RemoveCustomer(u, cu);
              _wInfo->setCustomer(0);
              _wInfo->subFlags(WIZARDINFO_FLAGS_CUST_CREATED);
              AH_Customer_free(cu);
            } // if customer created
          } // if customer
        } // if user not created
      } // if user
    } // if bank not created
  } // if bank

  return true;
}



void EditCtUser::slotBankCodeLostFocus() {
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



void EditCtUser::slotBankCodeChanged(const QString&) {
  if (_bankInfo) {
    DBG_ERROR(0, "Deleting current bank info");
    AB_BankInfo_free(_bankInfo);
    _bankInfo=0;
  }
}



void EditCtUser::slotBankCodeClicked() {
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
    isPinTan=_getCryptMode(_wInfo->getMedium(), _wInfo->getContext())==
      AH_CryptMode_Pintan;

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



void EditCtUser::_fromContext(int i) {
  int country;
  GWEN_BUFFER *bankId;
  GWEN_BUFFER *userId;
  GWEN_BUFFER *server;
  int port;
  int rv;

  bankId=GWEN_Buffer_new(0, 64, 0, 1);
  userId=GWEN_Buffer_new(0, 64, 0, 1);
  server=GWEN_Buffer_new(0, 64, 0, 1);
  rv=AH_Medium_ReadContext(_wInfo->getMedium(),
                           i, &country, bankId, userId, server, &port);
  if (rv) {
    DBG_ERROR(0, "Could not read context %d", i);
    GWEN_Buffer_free(server);
    GWEN_Buffer_free(userId);
    GWEN_Buffer_free(bankId);
    return;
  }

  bankCodeEdit->setText(QString::fromUtf8(GWEN_Buffer_GetStart(bankId)));
  userIdEdit->setText(QString::fromUtf8(GWEN_Buffer_GetStart(userId)));
  customerIdEdit->setText("");
  serverEdit->setText(QString::fromUtf8(GWEN_Buffer_GetStart(server)));
  GWEN_Buffer_free(server);
  GWEN_Buffer_free(userId);
  GWEN_Buffer_free(bankId);
}



void EditCtUser::slotContextActivated(int i) {
  _wInfo->setContext(i);
  _fromContext(i);
}









