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


#ifndef AQHBCI_WIZARD_WIZARD_H
#define AQHBCI_WIZARD_WIZARD_H


#include "wizard.ui.h"

#include <qguardedptr.h>
#include <aqhbci/hbci.h>
#include <string>
#include <list>

class QBanking;


class Wizard : public WizardUi {
  Q_OBJECT
private:
  AH_HBCI *_hbci;
  QBanking *_app;
  bool _importMode;
  bool _firstInitMode;
  bool _hasAllKeys;
  bool _createFile;
  bool _mediumCreated;
  bool _bankCreated;
  bool _userCreated;
  bool _customerCreated;
  int _index;
  std::string _mediumName;
  std::string _mediumTypeName;
  std::string _mediumSubTypeName;
  std::string _bankCode;
  std::string _userId;
  std::string _customerId;
  bool _isFile;
  std::list<QWidget*> _pagesDone;
  QGuardedPtr<QWidget> _prevParentWidget;

  AH_MEDIUM *_medium;
  AH_BANK *_bank;
  AH_USER *_user;
  AH_CUSTOMER *_customer;
  AB_BANKINFO *_bankInfo;
  bool _enableServerTest;

  GWEN_PLUGIN_DESCRIPTION_LIST2 *_plugins;

  bool _doPage(QWidget *p);
  bool _undoPage(QWidget *p);

  std::string _getModulusData(const GWEN_CRYPTKEY *key) const;
  std::string _getExpData(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterModulus(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterExponent(const GWEN_CRYPTKEY *key) const;
  std::string _getIniLetterHash(const GWEN_CRYPTKEY *key) const;
  std::string _ripe(const std::string &src) const;
  std::string _dumpHexString(const std::string &s, int size=32);

  AH_CRYPT_MODE _getCryptMode(AH_MEDIUM *m, int idx);

  bool _adjustToUser(AH_USER *u);

  // Do not access the serverEdit widget directly; instead, get its
  // content through this function!
  QString Wizard::getServerAddr() const;

public:
  Wizard(AH_HBCI *hbci,
         QBanking *kb,
	 QWidget* parent=0,
	 const char* name=0,
	 bool modal=FALSE);
  virtual ~Wizard();

  bool completeUser(AH_USER *u);
  bool showIniLetter(AH_USER *u);


  bool enterPage(QWidget *p, bool back);
  bool leavePage(QWidget *p, bool back);

  bool initImportPage();
  bool doImportPage(QWidget *p);
  bool undoImportPage(QWidget *p);

  bool initMediumPage();
  bool doMediumPage(QWidget *p);
  bool undoMediumPage(QWidget *p);

  bool doSelectCheckFileCardPage(QWidget *p);
  bool undoSelectCheckFileCardPage(QWidget *p);

  bool initSelectFilePage();
  bool doSelectFilePage(QWidget *p);
  bool undoSelectFilePage(QWidget *p);

  bool initCheckFilePage();
  bool doCheckFilePage(QWidget *p);
  bool undoCheckFilePage(QWidget *p);

  bool initCheckCardPage();
  bool doCheckCardPage(QWidget *p);
  bool undoCheckCardPage(QWidget *p);

  bool initUserDataPage();
  bool doUserDataPage(QWidget *p);
  bool undoUserDataPage(QWidget *p);

  bool initServerTestPage();
  bool doServerTestPage(QWidget *p);
  bool undoServerTestPage(QWidget *p);

  bool initSummary1Page();
  bool enterSummary1Page(QWidget *p);
  bool doSummary1Page(QWidget *p);
  bool undoSummary1Page(QWidget *p);

  bool initInitModePage();
  bool doInitModePage(QWidget *p);
  bool undoInitModePage(QWidget *p);

  bool initServerKeysPage();
  bool doServerKeysPage(QWidget *p);
  bool undoServerKeysPage(QWidget *p);

  bool initVerifyKeysPage();
  bool enterVerifyKeysPage(QWidget *p);
  bool doVerifyKeysPage(QWidget *p);
  bool undoVerifyKeysPage(QWidget *p);

  bool initCreateKeysPage();
  bool doCreateKeysPage(QWidget *p);
  bool undoCreateKeysPage(QWidget *p);

  bool initSendKeysPage();
  bool doSendKeysPage(QWidget *p);
  bool undoSendKeysPage(QWidget *p);


  bool initIniLetterPage();
  bool enterIniLetterPage(QWidget *p);
  bool doIniLetterPage(QWidget *p);
  bool undoIniLetterPage(QWidget *p);


  bool initServerCertPage();
  bool doServerCertPage(QWidget *p);
  bool undoServerCertPage(QWidget *p);


  bool initSystemIdPage();
  bool doSystemIdPage(QWidget *p);
  bool undoSystemIdPage(QWidget *p);


  bool initAccListPage();
  bool doAccListPage(QWidget *p);
  bool undoAccListPage(QWidget *p);

  const QString _ResultMsg_Success;
  const QString _ResultMsg_Failed;
  const QString _ResultMsg_Supported;
  const QString _ResultMsg_NotSupported;

signals:
  void accepted();

public slots:

  void back();
  void next();
  void reject();
  void accept();
  void selected(const QString &);

  void slotSettingsChanged(const QString &t);

  void slotImportToggled(bool on);
  void slotNewToggled(bool on);
  void slotPinTanToggled(bool on);

  void slotRDHToggled(bool on);
  void slotDDVToggled(bool on);
  void slotPathClicked();
  void slotPathChanged(const QString &t);

  void slotCheckCard();

  void slotFiletypeChanged(int i);

  void slotCheckFile();

  void slotUserDataBankCodeChanged(const QString&);
  void slotUserDataBankCodeLostFocus();
  void slotUserDataBankCodeClicked();
  void slotFromMedium();

  void slotFirstInitToggled(bool on);

  void slotServerTest();

  void slotGetKeys();

  void slotKeysOk();
  void slotKeysNotOk();

  void slotCreateKeys();

  void slotSendKeys();

  void slotPrintIniLetter();

  void slotGetCert();

  void slotGetSysId();

  void slotGetAccList();


};







#endif /* AQHBCI_WIZARD_WIZARD_H */






