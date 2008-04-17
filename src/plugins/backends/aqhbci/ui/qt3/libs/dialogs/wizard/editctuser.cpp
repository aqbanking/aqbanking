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
#include <aqhbci/user.h>

#include <qbanking/qbanking.h>
#include <qbanking/qbselectbank.h>
//#include "selectcontext.h"
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

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
#include <gwenhywfar/gui.h>



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
,_dataIsOk(false)
,_idCount(0) {

  showSpecialCheck->setChecked(FALSE);
  specialGroup->hide();

  connect(bankCodeButton, SIGNAL(clicked()),
          this, SLOT(slotBankCodeClicked()));
  connect(bankCodeEdit, SIGNAL(lostFocus()),
          this, SLOT(slotBankCodeLostFocus()));
  connect(bankCodeEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(slotBankCodeChanged(const QString&)));
  connect(userCombo, SIGNAL(activated(int)),
          this, SLOT(slotContextActivated(int)));
  connect(showSpecialCheck, SIGNAL(toggled(bool)),
          this, SLOT(slotSpecialToggled(bool)));
}



EditCtUser::~EditCtUser() {
}



void EditCtUser::init() {
  int rv;
  uint32_t idx;
  int i;
  bool fromContextCalled=false;
  uint32_t userFlags;

  userCombo->clear();

  bankCodeEdit->setText(QString::fromUtf8(_wInfo->getBankId().c_str()));
  userIdEdit->setText(QString::fromUtf8(_wInfo->getUserId().c_str()));
  customerIdEdit->setText(QString::fromUtf8(_wInfo->getCustomerId().c_str()));
  nameEdit->setText(QString::fromUtf8(_wInfo->getUserName().c_str()));
  serverEdit->setText(QString::fromUtf8(_wInfo->getServer().c_str()));
  peerIdEdit->setText(QString::fromUtf8(_wInfo->getPeerId().c_str()));

  if (_wInfo->getCryptMode()==AH_CryptMode_Pintan) {
    userCombo->setEnabled(false);
    hbciVersionCombo->setCurrentItem(2);
    hbciVersionCombo->setEnabled(false);
    if (!(_wInfo->getHttpVersion().empty()))
      httpVersionCombo->setCurrentText(QString::fromUtf8(_wInfo->getHttpVersion().c_str()));
  }
  else {
    GWEN_CRYPT_TOKEN *ct;
    uint32_t idCount;

    httpVersionCombo->setEnabled(false);
    ct=_wInfo->getToken();
    assert(ct);

    if (!GWEN_Crypt_Token_IsOpen(ct)) {
      rv=GWEN_Crypt_Token_Open(ct, 0, 0);
      if (rv) {
	DBG_ERROR(0, "Error opening token (%d)", rv);
	QMessageBox::critical(this,
			      tr("Error"),
			      tr("Could not open crypt token"),
			      QMessageBox::Ok,QMessageBox::NoButton);
	return;
      }
    }

    idCount=32;
    rv=GWEN_Crypt_Token_GetContextIdList(ct, _idList, &idCount, 0);
    if (rv) {
      DBG_ERROR(0, "Error reading context list (%d)", rv);
      QMessageBox::critical(this,
			    tr("Error"),
			    tr("Could not read context list from token"),
			    QMessageBox::Ok,QMessageBox::NoButton);
      return;
    }
    _idCount=idCount;

    for (idx=0; idx<_idCount; idx++) {
      QString qs;
      const char *s;
      const GWEN_CRYPT_TOKEN_CONTEXT *cctx;

      cctx=GWEN_Crypt_Token_GetContext(ct, _idList[idx], 0);
      if (cctx) {
	qs=QString::number(_idList[idx]);
	qs+=":";
	s=GWEN_Crypt_Token_Context_GetUserId(cctx);
	if (s) {
	  qs+=" ";
	  qs+=QString::fromUtf8(s);
	}

	s=GWEN_Crypt_Token_Context_GetServiceId(cctx);
	if (s) {
	  qs+="/";
	  qs+=QString::fromUtf8(s);
	}
      }
      else {
	qs=tr("Unreadable Context");
      }
      userCombo->insertItem(qs);
      if (_idList[idx]==_wInfo->getContext()) {
	DBG_INFO(0, "Using Context %d", idx);
	userCombo->setCurrentItem(idx);
	_fromContext(idx, false);
	fromContextCalled=true;
      }
    }

    if (!fromContextCalled) {
      DBG_ERROR(0, "Reading context now");
      _fromContext(0, false);
    }

    if (_wInfo->getUser()) {
      switch(AH_User_GetHbciVersion(_wInfo->getUser())) {
      case 201: i=0; break;
      case 220: i=2; break;
      case 300: i=3; break;
      case 210:
      default:  i=1; break;
      }
    }
    else
      i=1;
    hbciVersionCombo->setCurrentItem(i);
  }

  userFlags=_wInfo->getUserFlags();
  bankSignCheck->setChecked(!(userFlags & AH_USER_FLAGS_BANK_DOESNT_SIGN));
  bankCounterCheck->setChecked(userFlags & AH_USER_FLAGS_BANK_USES_SIGNSEQ);
  forceSsl3Check->setChecked(userFlags & AH_USER_FLAGS_FORCE_SSL3);
  noBase64Check->setChecked(userFlags & AH_USER_FLAGS_NO_BASE64);

  /* validate server address in edit field */
  _getServerAddr();
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



bool EditCtUser::_checkStringSanity(const char *s) {
  assert(s);
  while(*s) {
    if (iscntrl(*s) || isspace(*s)) {
      return false;
    }
    s++;
  } /* while */
  return true;
}



bool EditCtUser::apply(){
  GWEN_INETADDRESS *addr;
  int err;
  QString qs;
  int i;
  AH_CRYPT_MODE cm;
  AB_USER *u;
  int idx;
  std::string bankId;
  std::string userId;
  std::string custId;
  std::string userName;
  std::string fullServerAddr;
  std::string serverAddr;
  std::string httpVersion;
  std::string peerId;
  int hbciVersion;
  uint32_t userFlags;

  /* do user data page */

  switch(hbciVersionCombo->currentItem()) {
  case 0:  hbciVersion=201; break;
  case 2:  hbciVersion=220; break;
  case 3:  hbciVersion=300; break;
  case 1: 
  default: hbciVersion=210; break;
  }

  /* read ids */
  bankId=QBanking::QStringToUtf8String(bankCodeEdit->text());
  userId=QBanking::QStringToUtf8String(userIdEdit->text());
  userName=QBanking::QStringToUtf8String(nameEdit->text());
  custId=QBanking::QStringToUtf8String(customerIdEdit->text());
  peerId=QBanking::QStringToUtf8String(peerIdEdit->text());
  qs=_getServerAddr();
  fullServerAddr=QBanking::QStringToUtf8String(qs);
  i=qs.find('/');
  if (i)
    qs.truncate(i);
  serverAddr=QBanking::QStringToUtf8String(qs);
  idx=userCombo->currentItem();

  httpVersion=QBanking::QStringToUtf8String(httpVersionCombo->currentText());

  /* some sanity checks */
  if (!_checkStringSanity(userId.c_str())) {
    QMessageBox::critical(this,
                          tr("Invalid Input"),
                          tr("<qt>"
                             "<p>"
                             "The user id contains invalid characters."
                             "</p>"
                             "<p>"
                             "Please correct your entry."
                             "</p>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    userIdEdit->setFocus();
    return false;
  }

  if (bankId.length()<8) {
    QMessageBox::critical(this,
                          tr("Invalid Input"),
                          tr("<qt>"
                             "<p>"
                             "The bank code needs at least 8 digits."
                             "</p>"
                             "<p>"
                             "Please correct your entry."
                             "</p>"
                             "</qt>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    bankCodeEdit->setFocus();
    return false;
  }

  if (userId.length()<1) {
    QMessageBox::critical(this,
                          tr("Invalid Input"),
                          tr("<qt>"
                             "<p>"
                             "The user id needs at least one character."
                             "</p>"
                             "<p>"
                             "Please correct your entry."
                             "</p>"),
                          QMessageBox::Ok,QMessageBox::NoButton);
    userIdEdit->setFocus();
    return false;
  }

  if (userName.empty()) {
    QMessageBox::critical(this,
			  tr("Invalid Input"),
			  tr("<qt>"
			     "<p>"
			     "The user name must not be empty."
			     "</p>"
			     "<p>"
			     "Please correct your entry."
                             "</p>"
                             "</qt>"),
			  QMessageBox::Ok,QMessageBox::NoButton);
    nameEdit->setFocus();
    return false;
  }

  if (custId.empty()) {
    int r=QMessageBox::warning(this,
			       tr("No Customer Id Given"),
			       tr("<qt>"
				  "<p>"
				  "You did not enter a customer id."
				  "</p>"
				  "<p>Many banks do not need an explicit "
				  "customer id. But some other "
				  "banks absolutey require a customer id. "
				  "Please double-check the "
				  "information provided to you by your bank."
				  "</p>"
				  "<p>"
				  "Are you sure you want to leave the "
				  "customer id empty?"
				  "</p>"
				  "</qt>"
				 ),
			       QMessageBox::Yes,QMessageBox::No);
    if (r != 0 && r != QMessageBox::Yes) {
      customerIdEdit->setFocus();
      return false;
    }
    custId=userId;
  }

  /* get crypt mode */
  cm=_wInfo->getCryptMode();
  if (cm==AH_CryptMode_Unknown) {
    DBG_ERROR(0, "Unknown crypt mode/mount error");
    return false;
  }

  /* get bank info */
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


  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, serverAddr.c_str());
  if (err) {
    uint32_t wid;

    wid=GWEN_Gui_ProgressLog(0,
			     GWEN_LoggerLevel_Notice,
			     QWidget::tr("Resolving host address...").utf8());
    // Note: The QCString::data() is the conversion that will
    // correctly convert the QCString to the char* pointer.
    DBG_INFO(0, "Resolving hostname \"%s\"",
	     qs.local8Bit().data());
    err=GWEN_InetAddr_SetName(addr, serverAddr.c_str());
    if (wid)
      GWEN_Gui_HideBox(wid);
    if (err) {
      DBG_ERROR(0,
		"Error resolving hostname \"%s\":",
		serverAddr.c_str());
      DBG_ERROR_ERR(0, err);
      QMessageBox::critical(this,
			    QWidget::tr("Network Error"),
			    QWidget::tr("Could not resolve the server address %1.\n"
					"Maybe there is a typo? Please correct the server address.")
			    .arg(QString::fromUtf8(serverAddr.c_str())),
			    QMessageBox::Ok,QMessageBox::NoButton);
      GWEN_InetAddr_free(addr);
      return false;
    }
  }
  GWEN_InetAddr_free(addr);

  /* handle user flags */
  userFlags=0;

  if (!(bankSignCheck->isChecked()))
    userFlags|=AH_USER_FLAGS_BANK_DOESNT_SIGN;

  if (bankCounterCheck->isChecked())
    userFlags|=AH_USER_FLAGS_BANK_USES_SIGNSEQ;

  if (forceSsl3Check->isChecked())
    userFlags|=AH_USER_FLAGS_FORCE_SSL3;

  if (noBase64Check->isChecked())
    userFlags|=AH_USER_FLAGS_NO_BASE64;
  _wInfo->setUserFlags(userFlags);

  /* select or create user */
  u=AB_Banking_FindUser(_app->getCInterface(),
			AH_PROVIDER_NAME,
			"de",
			bankId.c_str(),
			userId.c_str(),
			custId.c_str());
  if (!u) {
    GWEN_URL *url;

    DBG_INFO(0, "Creating user");
    u=AB_Banking_CreateUser(_app->getCInterface(), AH_PROVIDER_NAME);
    assert(u);
    AB_User_SetCountry(u, "de");
    AB_User_SetBankCode(u, bankId.c_str());
    AB_User_SetUserId(u, userId.c_str());
    AB_User_SetCustomerId(u, custId.c_str());
    AB_User_SetUserName(u, userName.c_str());
    if (!peerId.empty())
      AH_User_SetPeerId(u, peerId.c_str());

    AH_User_SetTokenType(u, _wInfo->getMediumType().c_str());
    AH_User_SetTokenName(u, _wInfo->getMediumName().c_str());
    _wInfo->setContext(_idList[idx]);
    AH_User_SetTokenContextId(u, _wInfo->getContext());

    AH_User_SetCryptMode(u, cm);
    AH_User_SetHbciVersion(u, hbciVersion);

    AH_User_AddFlags(u, userFlags);

    if (!(httpVersion.empty())) {
      int vmajor, vminor;

      if (sscanf(httpVersion.c_str(), "%d.%d", &vmajor, &vminor)==2) {
        DBG_ERROR(0, "Setting HTTP_Version to %d.%d", vmajor, vminor);
        AH_User_SetHttpVMajor(u, vmajor);
        AH_User_SetHttpVMinor(u, vminor);
      }
    }

    /* set address */
    url=GWEN_Url_fromString(fullServerAddr.c_str());
    assert(url);
    if (cm==AH_CryptMode_Pintan) {
      GWEN_Url_SetProtocol(url, "https");
      GWEN_Url_SetPort(url, 443);
    }
    else {
      GWEN_Url_SetProtocol(url, "hbci");
      GWEN_Url_SetPort(url, 3000);
    }
    AH_User_SetServerUrl(u, url);
    GWEN_Url_free(url);

    AB_Banking_AddUser(_app->getCInterface(), u);
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

  if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan) {
    /* PIN/TAN only works with HBCI version 2.20. It might work with
     * other versions as well but this is not defined in the specs. */
    AH_User_SetHbciVersion(u, 220);
  }

  return true;
}



bool EditCtUser::undo(){
  AB_USER *u;

  u=_wInfo->getUser();
  if (u) {
    if (_wInfo->getFlags() & WIZARDINFO_FLAGS_USER_CREATED) {
      DBG_INFO(0, "Removing user and all subordinate objects");
      AB_Banking_DeleteUser(_app->getCInterface(), u);
      _wInfo->setUser(0);
      _wInfo->subFlags(WIZARDINFO_FLAGS_USER_CREATED);
    } // if _userCreated
  } // if user

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
  std::string s;

  if (_bankInfo) {
    DBG_ERROR(0, "Deleting current bank info");
    AB_BankInfo_free(_bankInfo);
    _bankInfo=0;
  }

  s=QBanking::QStringToUtf8String(bankCodeEdit->text());
  if (!s.empty()) {
    if (strcasecmp(s.c_str(), "30060601")==0) {
      /* special handling of DT. Apotheker and Aerztebank: use
       * NO_BASE64 */
      noBase64Check->setChecked(true);
      _wInfo->addUserFlags(AH_USER_FLAGS_NO_BASE64);
    }
  }
}



void EditCtUser::slotBankCodeClicked() {
  AB_BANKINFO *bi;

  AB_BankInfo_free(_bankInfo);
  _bankInfo=0;
  bi=QBSelectBank::selectBank(_app,
                              0,
			      tr("Select a bank"),
			      "de",
			      bankCodeEdit->text());
  if (bi) {
    const char *s;
    AB_BANKINFO_SERVICE *sv;
    bool isPinTan;

    s=AB_BankInfo_GetBankId(bi);
    if (s)
      bankCodeEdit->setText(QString::fromUtf8(s));

    sv=AB_BankInfoService_List_First(AB_BankInfo_GetServices(bi));
    isPinTan=(_wInfo->getCryptMode()==AH_CryptMode_Pintan);

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



void EditCtUser::_fromContext(int i, bool overwrite) {
  if (i<(int)_idCount) {
    GWEN_CRYPT_TOKEN *ct;
    const GWEN_CRYPT_TOKEN_CONTEXT *cctx;
    int rv;

    ct=_wInfo->getToken();
    assert(ct);

    if (!GWEN_Crypt_Token_IsOpen(ct)) {
      rv=GWEN_Crypt_Token_Open(ct, 0, 0);
      if (rv) {
	DBG_ERROR(0, "Error opening token (%d)", rv);
	QMessageBox::critical(this,
			      tr("Error"),
			      tr("Could not open crypt token"),
			      QMessageBox::Ok,QMessageBox::NoButton);
	return;
      }
    }

    if (userIdEdit->text().isEmpty()) {
      DBG_ERROR(0, "User Id is empty");
    }
    else {
      DBG_ERROR(0, "User Id is not empty");
    }

    cctx=GWEN_Crypt_Token_GetContext(ct, _idList[i], 0);
    if (cctx) {
      const char *s;

      s=GWEN_Crypt_Token_Context_GetUserId(cctx);
      if (s) {
        DBG_ERROR(0, "User id available");
	if (overwrite || userIdEdit->text().isEmpty())
	  userIdEdit->setText(QString::fromUtf8(s));
	if (overwrite || customerIdEdit->text().isEmpty())
	  customerIdEdit->setText(QString::fromUtf8(s));
      }
      else {
	DBG_ERROR(0, "User id not available");
      }
      s=GWEN_Crypt_Token_Context_GetServiceId(cctx);
      if (s && (overwrite || bankCodeEdit->text().isEmpty()))
	bankCodeEdit->setText(QString::fromUtf8(s));
      s=GWEN_Crypt_Token_Context_GetAddress(cctx);
      if (s && (overwrite || serverEdit->text().isEmpty()))
	serverEdit->setText(QString::fromUtf8(s));
      s=GWEN_Crypt_Token_Context_GetUserName(cctx);
      if (s && (overwrite || nameEdit->text().isEmpty()))
	nameEdit->setText(QString::fromUtf8(s));
      s=GWEN_Crypt_Token_Context_GetPeerId(cctx);
      if (overwrite || peerIdEdit->text().isEmpty())
	peerIdEdit->setText(QString::fromUtf8(s));
    }
    _wInfo->setContext(_idList[i]);
    DBG_ERROR(0, "Using context %d", i);
  }
  else {
    DBG_ERROR(0, "Invalid context %d", i);
  }
}



void EditCtUser::slotContextActivated(int i) {
  _fromContext(i);
}



void EditCtUser::slotSpecialToggled(bool on) {
  if (on)
    specialGroup->show();
  else
    specialGroup->hide();
}



#include "editctuser.moc"




