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

#include <gwenhywfar/md.h>
#include <gwenhywfar/crypt.h>
#include <gwenhywfar/text.h>

#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

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
#include <qlabel.h>
#include <qtimer.h>

#include "userlist.h"

#include <gwenhywfar/debug.h>

#ifdef WIN32
# define strcasecmp stricmp
#endif



Wizard::Wizard(AH_HBCI *hbci,
               QBanking *kb,
               QWidget* parent,
               const char* name,
               bool modal)
:WizardUi(parent, name)
,_hbci(hbci)
,_app(kb)
,_importMode(false)
,_firstInitMode(true)
,_hasAllKeys(false)
,_createFile(false)
,_mediumCreated(false)
,_bankCreated(false)
,_userCreated(false)
,_customerCreated(false)
,_index(0)
,_isFile(false)
,_medium(0)
,_bank(0)
,_user(0)
,_customer(0)
,_bankInfo(0)
,_enableServerTest(true)
,_plugins(0)
,_ResultMsg_Success(QString("<qt><font color=\"green\">")+
		   tr("Success")+QString("</font></qt>"))
,_ResultMsg_Failed(QString("<qt><font color=\"red\">")+
		   tr("Failed")+QString("</font></qt>"))
,_ResultMsg_Supported(QString("<qt><font color=\"green\">")+
		      tr("Supported")+QString("</font></qt>"))
,_ResultMsg_NotSupported(QString("<qt><font color=\"red\">")+
			 tr("Not supported")+QString("</font></qt>"))
{
  setModal(modal);

  initImportPage();
  initMediumPage();
  initCheckCardPage();
  initSelectFilePage();
  initCheckFilePage();
  initUserDataPage();
  initInitModePage();
  initServerTestPage();
  initServerKeysPage();
  initVerifyKeysPage();
  initCreateKeysPage();
  initSendKeysPage();
  initIniLetterPage();
  initServerCertPage();
  initSystemIdPage();
  initAccListPage();

  // In qt-3.2, these texts do not seem to be translated. We do this
  // manually here. May be commented out at a later point in time.
  nextButton()->setText(tr("&Next >"));
  backButton()->setText(tr("< &Back"));
  finishButton()->setText(tr("&Finish"));
  cancelButton()->setText(tr("&Cancel"));
  helpButton()->setText(tr("&Help"));
  // FIXME: Help button is still not yet implemented. Better disable it.
  helpButton()->setEnabled(false);

  setAppropriate(initModePage, true);

  // QT_VERSION is (major << 16) + (minor << 8) + patch
#if QT_VERSION >= 0x040000
  // In qt4, the QLabel has the word wrapping switched off by default,
  // whereas in qt3 it had been switched on by default.
  pixmapLabel1->setWordWrap(true);
  TextLabel3_2->setWordWrap(true);
  pixmapLabel1_2->setWordWrap(true);
  TextLabel1_5_3_2->setWordWrap(true);
  pixmapLabel1_2_2->setWordWrap(true);
  TextLabel1_5_3->setWordWrap(true);
  TextLabel2_2->setWordWrap(true);
  pixmapLabel1_2_3->setWordWrap(true);
  TextLabel1_5_2_2->setWordWrap(true);
  TextLabel1->setWordWrap(true);
  checkCardLabel->setWordWrap(true);
  pixmapLabel1_2_4->setWordWrap(true);
  TextLabel1_3_2_2_2->setWordWrap(true);
  textLabel2->setWordWrap(true);
  pixmapLabel1_2_5->setWordWrap(true);
  TextLabel1_5_2_2_2->setWordWrap(true);
  TextLabel1_2->setWordWrap(true);
  checkFileLabel->setWordWrap(true);
  pixmapLabel1_2_6->setWordWrap(true);
  TextLabel1_5_2_3->setWordWrap(true);
  TextLabel1_4_2_3_2_2_2_2_2->setWordWrap(true);
  TextLabel1_2_2_2_2_2_2_2->setWordWrap(true);
  TextLabel1_4_2_3_2_2_2_2_2_2->setWordWrap(true);
  bankNameLabel->setWordWrap(true);
  TextLabel1_3_2_2_2_2_2->setWordWrap(true);
  TextLabel1_3_2_2_2_3->setWordWrap(true);
  TextLabel1_6_2_2_2->setWordWrap(true);
  TextLabelDescription->setWordWrap(true);
  pixmapLabel1_2_8_2->setWordWrap(true);
  textLabel1_2->setWordWrap(true);
  pixmapLabel1_2_7->setWordWrap(true);
  TextLabel1_5_2_2_2_2_2_2_2_2->setWordWrap(true);
  textLabel1->setWordWrap(true);
  serverTestLabel->setWordWrap(true);
  pixmapLabel1_2_8->setWordWrap(true);
  TextLabel1_5_2_2_3_3->setWordWrap(true);
  pixmapLabel1_2_9->setWordWrap(true);
  TextLabel1_5_2_2_3->setWordWrap(true);
  TextLabel1_2_2->setWordWrap(true);
  getKeysLabel->setWordWrap(true);
  pixmapLabel1_2_10->setWordWrap(true);
  TextLabel1_5_2_2_2_2->setWordWrap(true);
  TextLabel3->setWordWrap(true);
  pixmapLabel1_2_11->setWordWrap(true);
  TextLabel1_5_2_2_3_2->setWordWrap(true);
  TextLabel1_2_2_2->setWordWrap(true);
  createKeysLabel->setWordWrap(true);
  pixmapLabel1_2_12->setWordWrap(true);
  TextLabel1_5_2_2_3_2_2->setWordWrap(true);
  TextLabel1_2_2_2_2->setWordWrap(true);
  sendKeysLabel->setWordWrap(true);
  pixmapLabel1_2_13->setWordWrap(true);
  TextLabel1_5_2_2_2_2_2_2->setWordWrap(true);
  pixmapLabel1_2_14->setWordWrap(true);
  TextLabel1_5_2_2_2_2_2_2_2->setWordWrap(true);
  pixmapLabel1_2_15->setWordWrap(true);
  TextLabel1_3_2_2->setWordWrap(true);
  pixmapLabel1_2_16->setWordWrap(true);
  TextLabel1_5_2_2_3_2_2_2->setWordWrap(true);
  TextLabel1_2_2_2_2_2->setWordWrap(true);
  getSysIdLabel->setWordWrap(true);
  pixmapLabel1_2_17->setWordWrap(true);
  TextLabel1_5_2_2_3_2_2_2_2->setWordWrap(true);
  TextLabel1_2_2_2_2_2_2->setWordWrap(true);
  getAccListLabel->setWordWrap(true);
  pixmapLabel1_2_18->setWordWrap(true);
  TextLabel1_5_2_2_3_2_2_2_2_2->setWordWrap(true);
  pixmapLabel1_2_19->setWordWrap(true);
  TextLabel1_3_2->setWordWrap(true);
#endif

  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



Wizard::~Wizard(){
}




void Wizard::back(){
  QWidget *p;

  p=currentPage();
  if (p)
    leavePage(p,true);
  QWizard::back();
  p=currentPage();
  if (p)
    enterPage(p,true);
}



void Wizard::next(){
  QWidget *p;

  p=currentPage();
  if (p)
    if (!leavePage(p, false))
      return;
  QWizard::next();
  p=currentPage();
  if (p)
    enterPage(p, false);
}



void Wizard::selected(const QString &){
}



void Wizard::slotSettingsChanged(const QString &t){
  setNextEnabled(userDataPage, !(bankCodeEdit->text().isEmpty() ||
				 serverEdit->text().isEmpty() ||
				 nameEdit->text().isEmpty() ||
				 userIdEdit->text().isEmpty()));
}



bool Wizard::enterPage(QWidget *p, bool bk){
  if (p==mediumPage) {
    setNextEnabled(mediumPage, true);
  }

  else if (p==userDataPage) {
    /* can not read from medium if about to create it */
    fromMediumButton->setEnabled(!_createFile);
  }

  else if (p==serverTestPage) {
    serverTestButton->setEnabled(true);
    //setNextEnabled(serverTestPage, false);
  }

  else if (p==selectFilePage) {
    int i;
    int found;

    setNextEnabled(selectFilePage, false);
    /* reload plugin list */
    DBG_NOTICE(0, "Reloading plugin list");
    fileTypeCombo->clear();
    fileTypeCombo->insertItem(tr("Select File Type"));

    i=1;
    found=0;
    if (_plugins)
      GWEN_PluginDescription_List2_freeAll(_plugins);
    _plugins=AH_HBCI_GetMediumPluginDescrs(_hbci,
                                           GWEN_CryptToken_Device_File);
    if (_plugins) {
      GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

      it=GWEN_PluginDescription_List2_First(_plugins);
      if (it) {
	GWEN_PLUGIN_DESCRIPTION *pd;

	pd=GWEN_PluginDescription_List2Iterator_Data(it);
	while(pd) {
	  const char *p;

	  p=GWEN_PluginDescription_GetShortDescr(pd);
	  if (!p)
	    p=GWEN_PluginDescription_GetName(pd);
	  fileTypeCombo->insertItem(p);
	  if (!_mediumTypeName.empty()) {
	    p=GWEN_PluginDescription_GetName(pd);
	    assert(p);
	    if (strcasecmp(p, _mediumTypeName.c_str())==0)
	      found=i;
	  }
	  i++;
	  pd=GWEN_PluginDescription_List2Iterator_Next(it);
	} /* while */
	GWEN_PluginDescription_List2Iterator_free(it);
      } /* if it */
    }
    fileTypeCombo->setCurrentItem(found);
    slotFiletypeChanged(found);
  } // if selectFile

  else if (p==checkFilePage) {
    setNextEnabled(checkFilePage, false);
    checkFileButton->setEnabled(true);
  }

  else if (p==checkCardPage) {
    setNextEnabled(checkCardPage, false);
    checkCardButton->setEnabled(true);
  }

  else if (p==summary1Page) {
    enterSummary1Page(p);
  }

  else if (p==verifyKeysPage) {
    enterVerifyKeysPage(p);
  }

  else if (p==serverKeysPage) {
    setBackEnabled(serverKeysPage, false);
    setNextEnabled(serverKeysPage, false);
    getKeysButton->setEnabled(true);
  }

  else if (p==createKeysPage) {
    setNextEnabled(createKeysPage, false);
    createKeysButton->setEnabled(true);
    createKeysLabel->setText("");
  }

  else if (p==iniLetterPage) {
    enterIniLetterPage(p);
  }

  else if (p==finished1Page) {
    if (_user)
      AH_User_SetStatus(_user, AH_UserStatusPending);
    setBackEnabled(finished1Page, false);
    setNextEnabled(finished1Page, false);
    setFinishEnabled(finished1Page, true);
  }
  else if (p==finished2Page) {
    if (_user)
      AH_User_SetStatus(_user, AH_UserStatusEnabled);
    setBackEnabled(finished2Page, false);
    setNextEnabled(finished2Page, false);
    setFinishEnabled(finished2Page, true);
  }

  if (bk)
    return _undoPage(p);
  return true;
}



bool Wizard::leavePage(QWidget *p, bool bk){
  if (!bk)
    return _doPage(p);
  return true;
}



bool Wizard::_doPage(QWidget *p){
  bool rv;

  if (p==importPage)
    rv=doImportPage(p);
  else if (p==mediumPage)
    rv=doMediumPage(p);
  else if (p==checkCardPage)
    rv=doCheckCardPage(p);
  else if (p==selectFilePage)
    rv=doSelectFilePage(p);
  else if (p==checkFilePage)
    rv=doCheckFilePage(p);
  else if (p==userDataPage)
    rv=doUserDataPage(p);
  else if (p==serverTestPage)
    rv=doServerTestPage(p);
  else if (p==summary1Page)
    rv=doSummary1Page(p);
  else if (p==initModePage)
    rv=doInitModePage(p);
  else if (p==serverKeysPage)
    rv=doServerKeysPage(p);
  else if (p==verifyKeysPage)
    rv=doVerifyKeysPage(p);
  else if (p==createKeysPage)
    rv=doCreateKeysPage(p);
  else if (p==sendKeysPage)
    rv=doSendKeysPage(p);
  else if (p==iniLetterPage)
    rv=doIniLetterPage(p);
  else if (p==systemIdPage)
    rv=doSystemIdPage(p);
  else if (p==serverCertPage)
    rv=doServerCertPage(p);
  else if (p==accListPage)
    rv=doAccListPage(p);
  else {
    return true;
  }

  if (rv) {
    //DBG_NOTICE(0, "Pushing page %08x", (unsigned int)p);
    _pagesDone.push_front(p);
  }
  return rv;
}



bool Wizard::_undoPage(QWidget *p){
  bool rv;

  if (p==importPage)
    rv=undoImportPage(p);
  else if (p==mediumPage)
    rv=undoMediumPage(p);
  else if (p==checkCardPage)
    rv=undoCheckCardPage(p);
  else if (p==selectFilePage)
    rv=undoSelectFilePage(p);
  else if (p==checkFilePage)
    rv=undoCheckFilePage(p);
  else if (p==userDataPage)
    rv=undoUserDataPage(p);
  else if (p==serverTestPage)
    rv=undoServerTestPage(p);
  else if (p==summary1Page)
    rv=undoSummary1Page(p);
  else if (p==initModePage)
    rv=undoInitModePage(p);
  else if (p==serverKeysPage)
    rv=undoServerKeysPage(p);
  else if (p==verifyKeysPage)
    rv=undoVerifyKeysPage(p);
  else if (p==createKeysPage)
    rv=undoCreateKeysPage(p);
  else if (p==sendKeysPage)
    rv=undoSendKeysPage(p);
  else if (p==iniLetterPage)
    rv=undoIniLetterPage(p);
  else if (p==systemIdPage)
    rv=undoSystemIdPage(p);
  else if (p==serverCertPage)
    rv=undoServerCertPage(p);
  else if (p==accListPage)
    rv=undoAccListPage(p);
  else {
    return true;
  }

  if (rv) {
    DBG_NOTICE(0, "Popping page");
    _pagesDone.pop_front();
  }
  return rv;
}



void Wizard::reject() {
  QWidget *p;

  DBG_WARN(0, "Undoing all pages");
  while(_pagesDone.size()) {
    bool rv;

    p=_pagesDone.front();
    //DBG_NOTICE(0, "Undoing page %08x", (unsigned int)(p));
    rv=_undoPage(p);
    if (!rv)
      _pagesDone.pop_front();
  } // while

  QWizard::reject();
}



void Wizard::accept() {
  if (_medium) {
    if (AH_Medium_IsMounted(_medium)) {
      if (AH_Medium_Unmount(_medium, 1)) {
        QMessageBox::critical(0,
                              tr("Medium Error"),
                              tr("Could not unmount the medium.\n"
                                 "Please check the logs."
                                ),
                              tr("Dismiss"),0,0,0);
        return;
      }
    }
  }

  emit accepted();
  QWizard::accept();
}



bool Wizard::doSelectCheckFileCardPage(QWidget *p){
  if (!_medium) {
    DBG_ERROR(0, "No medium");
    return false;
  }

  return true;
}



bool Wizard::undoSelectCheckFileCardPage(QWidget *p){

  if (_medium) {
    if (AH_Medium_IsMounted(_medium))
      AH_Medium_Unmount(_medium, 1);
    if (_mediumCreated) {
      // remove medium
      DBG_INFO(0, "Removing medium");
      AH_HBCI_RemoveMedium(_hbci, _medium);
      AH_Medium_free(_medium);
    }
    if (_createFile)
      QFile::remove(fileNameEdit->text());
    _medium=0;
  }
  _mediumCreated=false;

  return true;
}











std::string Wizard::_dumpHexString(const std::string &s, int size) {
  std::string result;
  unsigned int pos;

  result+="   ";
  for (pos=0; pos<s.length(); pos++) {
    if ((pos%size)==0)
      result+="<br>";
    else if ((pos & 1)==0)
      result+=" ";
    result+=s.at(pos);
  } // for
  result+="<br>";
  return result;
}



std::string Wizard::_getModulusData(const GWEN_CRYPTKEY *key) const {
  GWEN_DB_NODE *n;
  const void *p;
  unsigned int l;
  std::string result;

  n=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_ToDb(key, n, 1)) {
    GWEN_DB_Group_free(n);
    return "";
  }

  p=GWEN_DB_GetBinValue(n,
                        "data/n",
                        0,
                        0,0,
                        &l);
  if (!p) {
    GWEN_DB_Group_free(n);
    return "";
  }
  result=std::string((const char*)p, l);
  GWEN_DB_Group_free(n);
  return result;
}



std::string Wizard::_getExpData(const GWEN_CRYPTKEY *key) const {
  GWEN_DB_NODE *n;
  const void *p;
  unsigned int l;
  std::string result;

  n=GWEN_DB_Group_new("keydata");
  if (GWEN_CryptKey_ToDb(key, n, 1)) {
    GWEN_DB_Group_free(n);
    return "";
  }

  p=GWEN_DB_GetBinValue(n,
                        "data/e",
                        0,
                        0,0,
                        &l);
  if (!p) {
    GWEN_DB_Group_free(n);
    return "";
  }
  result=std::string((const char*)p, l);
  GWEN_DB_Group_free(n);
  return result;
}



std::string Wizard::_getIniLetterModulus(const GWEN_CRYPTKEY *key) const {
  char buffer[256];
  std::string modulus;

  modulus=_getModulusData(key);
  if (modulus.length()<96)
    modulus=std::string(96 - modulus.length(), 0x0) + modulus;
  if (GWEN_Text_ToHex((const char*)modulus.data(), modulus.length(),
		      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}



std::string Wizard::_getIniLetterExponent(const GWEN_CRYPTKEY *key) const {
  char buffer[256];
  std::string expo;

  expo=_getExpData(key);
  if (expo.length()<96)
    expo=std::string(96 - expo.length(), 0x0) + expo;
  if (GWEN_Text_ToHex((const char*)expo.data(), expo.length(),
                      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}



std::string Wizard::_getIniLetterHash(const GWEN_CRYPTKEY *key) const {
  std::string expo;
  std::string modulus;
  std::string result;
  char buffer[64];

  expo=_getExpData(key);
  modulus=_getModulusData(key);
  result = std::string(128 - expo.length(), 0x0) + expo;
  result += std::string(128 - modulus.length(), 0x0) + modulus;
  result = _ripe(result);
  if (GWEN_Text_ToHex((const char*)result.data(), result.length(),
		      buffer, sizeof(buffer))==0) {
    return "";
  }
  else
    return buffer;
}




std::string Wizard::_ripe(const std::string &src) const {
  std::string result;
  char buffer[32];
  unsigned int bsize;

  /* hash data */
  DBG_DEBUG(0, "Hash data");
  bsize=sizeof(buffer);
  if (GWEN_MD_Hash("RMD160",
                   (const char*)src.data(),
                   src.length(),
                   buffer,
                   &bsize)) {
    DBG_ERROR(0, "Could not hash");
    return "";
  }

  result=std::string(buffer, bsize);
  return result;
}



bool Wizard::_adjustToUser(AH_USER *u) {
  switch(AH_User_GetCryptMode(u)) {
  case AH_CryptMode_Ddv:
    DBG_NOTICE(0, "DDV medium");
    setAppropriate(serverTestPage, _enableServerTest);
    setAppropriate(serverKeysPage, false);
    setAppropriate(verifyKeysPage, false);
    setAppropriate(createKeysPage, false);
    setAppropriate(sendKeysPage, false);
    setAppropriate(iniLetterPage, false);
    setAppropriate(serverCertPage, false);
    setAppropriate(finished1Page, false);
    setAppropriate(systemIdPage, false);
    setAppropriate(accListPage, true);
    setAppropriate(addAccPage, true);
    setAppropriate(finished2Page, true);
    break;
  case AH_CryptMode_Rdh:
    DBG_NOTICE(0, "RDH medium");
    setAppropriate(serverTestPage, _enableServerTest);
    setAppropriate(serverKeysPage, _firstInitMode);
    setAppropriate(verifyKeysPage, _firstInitMode);
    setAppropriate(createKeysPage, _firstInitMode);
    setAppropriate(sendKeysPage, _firstInitMode && !_hasAllKeys);
    setAppropriate(iniLetterPage, _firstInitMode);
    setAppropriate(serverCertPage, false);
    setAppropriate(systemIdPage, !_firstInitMode);
    setAppropriate(finished1Page, _firstInitMode);
    setAppropriate(accListPage, !_firstInitMode);
    setAppropriate(addAccPage, !_firstInitMode);
    setAppropriate(finished2Page, !_firstInitMode);
    break;
  case AH_CryptMode_Pintan:
    DBG_NOTICE(0, "PIN/TAN medium");
    setAppropriate(serverTestPage, false);
    setAppropriate(serverKeysPage, false);
    setAppropriate(verifyKeysPage, false);
    setAppropriate(createKeysPage, false);
    setAppropriate(sendKeysPage, false);
    setAppropriate(iniLetterPage, false);
    setAppropriate(serverCertPage, true);
    setAppropriate(systemIdPage, true);
    setAppropriate(finished1Page, false);
    setAppropriate(accListPage, true);
    setAppropriate(addAccPage, true);
    setAppropriate(finished2Page, true);
    break;
  default:
    DBG_ERROR(0, "Unsupported medium type");
    QMessageBox::critical(0,
			  tr("Medium Error"),
			  tr("Unsupported medium type."),
                          tr("Dismiss"),0,0,0);
    return false;
  } // switch

  return true;
}



bool Wizard::completeUser(AH_USER *u) {
  _bank=AH_User_GetBank(u);
  _medium=AH_User_GetMedium(u);
  _user=u;
  _firstInitMode=false;

  if (!_adjustToUser(u))
    return false;

  _customer=AH_User_FindCustomer(u, "*");
  if (!_customer) {
    DBG_ERROR(0, "No customer found");
    QMessageBox::critical(0,
                          tr("Invalid Setup"),
                          tr("<qt>"
                             "<p>"
                             "No customer found. You should remove this "
                             "user."
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"),0,0,0);
    return false;
  }

  userIdEdit->setText(AH_User_GetUserId(_user));
  customerIdEdit->setText(AH_Customer_GetCustomerId(_customer));
  bankCodeEdit->setText(AH_Bank_GetBankId(_bank));

  showPage(systemIdPage);
  if (exec()!=QDialog::Accepted)
    return false;

  AH_User_SetStatus(u, AH_UserStatusEnabled);
  return true;
}

bool Wizard::showIniLetter(AH_USER *u) {
  _bank=AH_User_GetBank(u);
  _medium=AH_User_GetMedium(u);
  _user=u;
  _firstInitMode=false;

  if (!_adjustToUser(u))
    return false;

  _customer=AH_User_FindCustomer(u, "*");
  if (!_customer) {
    DBG_ERROR(0, "No customer found");
    QMessageBox::critical(0,
                          tr("Invalid Setup"),
                          tr("<qt>"
                             "<p>"
                             "No customer found. You should remove this "
                             "user."
                             "</p>"
                             "</qt>"
                            ),
                          tr("Dismiss"),0,0,0);
    return false;
  }

  initIniLetterPage();
  showPage(iniLetterPage);
	
  enterIniLetterPage(iniLetterPage);
  setBackEnabled(iniLetterPage, false);
  setNextEnabled(iniLetterPage, false);

  if (exec()!=QDialog::Accepted)
    return false;

  return true;
}
