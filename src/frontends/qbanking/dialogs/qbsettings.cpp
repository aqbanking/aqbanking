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


#include "qbsettings.h"
#include "qbprocesswatcher.h"

#include <qwidget.h>
#include <qgroupbox.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <qprocess.h>
#include <qpushbutton.h>


#include <aqbanking/provider.h>

#include <gwenhywfar/debug.h>


// DEBUG
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>




QBankingSettings::QBankingSettings(QBanking *ab,
                                   QWidget* parent,
                                   const char* name,
                                   WFlags fl)
:QBankingSettingsUi(parent, name, fl), _banking(ab) {
  GWEN_DB_NODE *dbSettings;
  int i, j;

  dbSettings=ab->getAppData();
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "bankingSettings");
  assert(dbSettings);

  /* setup account list view */

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  accountsBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *accountsBoxLayout = new QHBoxLayout( accountsBox->layout() );
  accountsBoxLayout->setAlignment( Qt::AlignTop );

  _accListView=new QBAccountListView(accountsBox, "AccountList");
  accountsBoxLayout->addWidget(_accListView);

  _accListView->setResizeMode(QListView::NoColumn);
  for (i=0; i<_accListView->columns(); i++) {
    _accListView->setColumnWidthMode(i, QListView::Manual);
    j=GWEN_DB_GetIntValue(dbSettings, "gui/accountList/columns", i, -1);
    if (j!=-1)
      _accListView->setColumnWidth(i, j);
  } /* for */
  _accListView->setSelectionMode(QListView::Single);
  //_accListView->resize(accountsBox->width(), accountsBox->height());
  _accListView->show();


  /* setup provider list view */

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  backendsBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *backendsBoxLayout = new QHBoxLayout( backendsBox->layout() );
  backendsBoxLayout->setAlignment( Qt::AlignTop );

  _providerListView=new QBPluginDescrListView(backendsBox,
                                              "BackendList");
  backendsBoxLayout->addWidget(_providerListView);

  _providerListView->setResizeMode(QListView::NoColumn);
  for (i=0; i<_providerListView->columns(); i++) {
    _providerListView->setColumnWidthMode(i, QListView::Manual);
    j=GWEN_DB_GetIntValue(dbSettings, "gui/providerList/columns", i, -1);
    if (j!=-1)
      _providerListView->setColumnWidth(i, j);
  } /* for */
  _providerListView->setSelectionMode(QListView::Single);
  //_providerListView->resize(backendsBox->width()-2, backendsBox->height()-2);
  _providerListView->show();

  /* connect buttons to actions */
  QObject::connect(closeButton, SIGNAL(clicked()),
                   this, SLOT(accept()));

  QObject::connect(backendEnableButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendEnable()));
  QObject::connect(backendDisableButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendDisable()));

  QObject::connect(backendSetupButton, SIGNAL(clicked()),
                   this, SLOT(slotBackendSetup()));

  QObject::connect(accountMapButton, SIGNAL(clicked()),
                   this, SLOT(slotAccountMap()));


}



QBankingSettings::~QBankingSettings(){
}



int QBankingSettings::init(){
  _accListView->addAccounts(_banking->getAccounts());
  _providerListView->addPluginDescrs(_banking->getProviderDescrs());

  return 0;
}



int QBankingSettings::fini(){
  GWEN_DB_NODE *dbSettings;
  int i, j;

  fprintf(stderr, "Saving settings (%d, %d).\n",
          _accListView->columns(),
          _providerListView->columns());
  dbSettings=_banking->getAppData();
  assert(dbSettings);
  dbSettings=GWEN_DB_GetGroup(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                              "bankingSettings");
  assert(dbSettings);

  /* save account list view settings */
  GWEN_DB_DeleteVar(dbSettings, "gui/accountList/columns");
  for (i=0; i<_accListView->columns(); i++) {
    j=_accListView->columnWidth(i);
    GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                        "gui/accountList/columns", j);
  } /* for */

  /* save provider list view settings */
  GWEN_DB_DeleteVar(dbSettings, "gui/providerList/columns");
  for (i=0; i<_providerListView->columns(); i++) {
    j=_providerListView->columnWidth(i);
    GWEN_DB_SetIntValue(dbSettings, GWEN_DB_FLAGS_DEFAULT,
                        "gui/providerList/columns", j);
  } /* for */

  return 0;
}



void QBankingSettings::_backendRescan(){
  _providerListView->clear();
  _providerListView->addPluginDescrs(_banking->getProviderDescrs());
}



void QBankingSettings::_accountRescan(){
  fprintf(stderr, "Rescanning accounts\n");
  _accListView->clear();
  _accListView->addAccounts(_banking->getAccounts());
}



void QBankingSettings::slotBackendEnable(){
  GWEN_PLUGIN_DESCRIPTION *pd;

  pd=_providerListView->getCurrentPluginDescr();
  if (!pd) {
    fprintf(stderr, "No provider selected.\n");
  }
  else {
    int rv;

    if (GWEN_PluginDescription_IsActive(pd)) {
      fprintf(stderr, "Provider already active.\n");
      return;
    }

    rv=_banking->activateProvider(GWEN_PluginDescription_GetName(pd));
    _accountRescan();
    _backendRescan();
    if (rv) {
      QMessageBox::critical(this,
                            tr("Backend Error"),
                            tr("Could not activate this backend.\n"),
			    QMessageBox::Ok,QMessageBox::NoButton);
    }
    else {
      QMessageBox::information(this,
                               tr("Backend Activated"),
                               tr("This backend has successfully been "
                                  "activated.\n"),
                               QMessageBox::Ok);
    }
  }
}



void QBankingSettings::slotBackendDisable(){
  GWEN_PLUGIN_DESCRIPTION *pd;

  pd=_providerListView->getCurrentPluginDescr();
  if (!pd) {
    fprintf(stderr, "No provider selected.\n");
  }
  else {
    int rv;

    if (!GWEN_PluginDescription_IsActive(pd)) {
      fprintf(stderr, "Provider already inactive.\n");
      return;
    }

    int r = QMessageBox::warning(this,
                             tr("Disable Backend"),
                             tr("This would remove all accounts currently "
                                "supported by that backend.\n"
                                "\n"
                                "Do you still want me to disable it?"),
			     QMessageBox::Yes,QMessageBox::No);
    if (r !=0 && r != QMessageBox::Yes)
      return;

    rv=_banking->deactivateProvider(GWEN_PluginDescription_GetName(pd));
    _accountRescan();
    _backendRescan();
    if (rv) {
      QMessageBox::critical(this,
                            tr("Backend Error"),
                            tr("Could not deactivate this backend.\n"),
			    QMessageBox::Ok,QMessageBox::NoButton);
    }
  }
}



void QBankingSettings::slotBackendSetup(){
  GWEN_PLUGIN_DESCRIPTION *pd;

  pd=_providerListView->getCurrentPluginDescr();
  if (!pd) {
    fprintf(stderr, "No provider selected.\n");
  }
  else {
    GWEN_BUFFER *pbuf;
    QString qs;
    int rv;
    GWEN_DB_NODE *dbAppData;
    bool wasActive;

    dbAppData=_banking->getAppData();
    wasActive=AB_Banking_IsProviderActive(_banking->getCInterface(),
                                          GWEN_PluginDescription_GetName(pd));

    pbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (AB_Banking_FindWizard(_banking->getCInterface(),
			      GWEN_PluginDescription_GetName(pd),
                              "kde;qt;gtk;gnome",
			      pbuf)) {
      DBG_ERROR(0, "Could not get wizard path");
      QMessageBox::critical(this,
			    tr("Wizard Not Installed"),
                            tr("<qt>"
                               "<p>"
                               "The KDE wizard for this backend is "
                               "not installed."
                               "</p>"
                               "</qt>"),
			    QMessageBox::Ok,QMessageBox::NoButton);
      GWEN_Buffer_free(pbuf);
      return;
    }

    qs=GWEN_Buffer_GetStart(pbuf);
    GWEN_Buffer_free(pbuf);

    if (wasActive) {
      rv=AB_Banking_SuspendProvider(_banking->getCInterface(),
                                    GWEN_PluginDescription_GetName(pd));
      if (rv) {
        DBG_ERROR(0, "Error suspending backend \"%s\" (%d)",
                  GWEN_PluginDescription_GetName(pd), rv);
        QMessageBox::critical(this,
                              tr("Backend Error"),
                              tr("<qt>"
                                 "<p>"
                                 "Error suspending the backend."
                                 "</p>"
                                 "<p>"
                                 "To edit the settings of a backend we need "
                                 "to temporarily suspend it, otherwise "
                                 "your new settings would be silently "
                                 "overwritten upon shutdown of the program."
                                 "</p>"
                                 "</qt>"),
                              QMessageBox::Abort,QMessageBox::NoButton);
        return;
      }
    }

    QProcess wp(qs);

    if (!wp.launch(QString::null)) {
      QMessageBox::critical(this,
                            tr("Wizard Not Started"),
                            tr("<qt>"
                               "<p>"
                               "The KDE wizard for this backend could "
                               "not be started."
                               "</p>"
                               "</qt>"),
			    QMessageBox::Ok,QMessageBox::NoButton);
      if (wasActive)
        // resume provider if we suspended it
        AB_Banking_ResumeProvider(_banking->getCInterface(),
                                  GWEN_PluginDescription_GetName(pd));
      return;
    }

    QBProcessWatcher pwatcher(&wp,
                              tr("<qt>"
                                 "<p>"
                                 "Running KDE wizard, this window will close "
                                 "automatically when the wizard finishes."
                                 "</p>"
                                 "<p>"
                                 "Please wait..."
                                 "</p>"
                                 "</qt"
                                ),
                              this,
                              "ProcessWatcher",
                              true);
    pwatcher.setCaption(caption());
    pwatcher.exec();

    if (wasActive) {
      rv=AB_Banking_ResumeProvider(_banking->getCInterface(),
                                   GWEN_PluginDescription_GetName(pd));
      if (rv) {
        QMessageBox::critical(this,
                              tr("Backend Error"),
                              tr("<qt>"
                                 "<p>"
                                 "Could not resume the backend."
                                 "</p>"
                                 "</qt>"),
                              QMessageBox::Ok,QMessageBox::NoButton);
        return;
      }
    }
  } // if provider selected
}



void QBankingSettings::slotAccountMap(){
  std::list<AB_ACCOUNT*> al;
  AB_ACCOUNT *a;

  al=_accListView->getSelectedAccounts();
  if (al.empty()) {
    QMessageBox::critical(this,
                          tr("Selection Error"),
                          tr("No account selected.\n"),
                          QMessageBox::Retry,QMessageBox::NoButton);
  }
  a=al.front();
  _banking->mapAccount(a);
}













