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

#ifndef Q4BANKING_BANKING_H
#define Q4BANKING_BANKING_H

#include <aqbanking/banking.h>

#include <q4banking/api.h>
//Added by qt3to4:
#include <QtCore/QTranslator>



#define Q4BANKING_PM_CFGMODULE        "q4banking_cfg_module"


#define Q4BANKING_IMPORTER_FLAGS_COMPLETE_DAYS  0x00000001
#define Q4BANKING_IMPORTER_FLAGS_OVERWRITE_DAYS 0x00000002
#define Q4BANKING_IMPORTER_FLAGS_ASK_ALL_DUPES  0x00000004
#define Q4BANKING_IMPORTER_FLAGS_FUZZY          0x00000008
#define Q4BANKING_IMPORTER_FLAGS_AS_ORDERS      0x00000010



#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <Qt/qobject.h>
#include <Qt/qdatetime.h>
#include <Qt/qstring.h>
#include <Qt/qstringlist.h>
#include <Qt/qwidget.h> // needed for QPointer<QWidget>
#include <Qt/qpointer.h>

#include <list>

class QTranslator;

class QBanking;
class QBGui;

#include <q4banking/banking.h>
#include <q4banking/qbflagstaff.h>
#include <q4banking/qbgui.h>

#include <gwenhywfar/plugin.h>


class QBCfgModule;


class Q4BANKING_API QBanking: public AB_Banking {
private:
  QPointer<QWidget> _parentWidget;
  GWEN_LOGGER_LEVEL _logLevel;
  QBFlagStaff *_flagStaff;
  QTranslator *_translator;

  QBGui *_gui;

  GWEN_PLUGIN_MANAGER *_pluginManagerCfgModules;
  QBCfgModule *_appCfgModule;

  std::list<QBCfgModule*> _cfgModules;

  QString _appHelpPath;

  AB_ACCOUNT *_getAccount(const char *accountId);
  static int _extractHTML(const char *text, GWEN_BUFFER *buf);

  QBCfgModule *_findCfgModule(const char *modname);
  QBCfgModule *_loadCfgModule(const char *modname);

public:
  QBanking(const char *appname,
           const char *fname=0);
  virtual ~QBanking();

  int init();
  int fini();

  QBGui *getGui() const;
  void setGui(QBGui *g);

  QWidget *getParentWidget() const { return _parentWidget;};
  void setParentWidget(QWidget *w) { _parentWidget=w;};

  void setAppHelpPath(const QString &s);

  virtual QBFlagStaff *flagStaff();

  void setAccountAlias(AB_ACCOUNT *a, const char *alias);

  void accountsUpdated();
  void outboxCountChanged(int count);
  void statusMessage(const QString &s);

  virtual void invokeHelp(const QString &context,
                          const QString &subject);

  virtual bool mapAccount(const AB_ACCOUNT *a);

  bool askMapAccount(const char *id,
                     const char *bankCode,
                     const char *accountId);


  virtual bool addTransaction(const AB_ACCOUNT *a, const AB_TRANSACTION *t);
  virtual bool setAccountStatus(const AB_ACCOUNT *a,
                                const AB_ACCOUNT_STATUS *ast);

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             uint32_t flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 uint32_t flags);

  virtual bool interactiveImport();


  /**
   * This function makes the application print something. This is
   * a wrapper around the print() with "const char*" which is the
   * implementation of AB_Banking_Print(); see there for
   * arguments.
   *
   * Note: Make sure all arguments passed here are really a
   * QString! If any argument is a "const char*" (e. g. simply a
   * string constant), then especially QT-4 will complain about an
   * ambiguity between the "const char*"-print() and this one.
   */
  virtual int print(const QString &docTitle,
                    const QString &docType,
                    const QString &descr,
                    const QString &text);

  /**
   * Opens a dialog which asks the user for a bank. This bank can be selected
   * using the bank code (also called <i>sort code</i> in some countries),
   * the SWIFT code (so called <i>BIC</i>), bank name and/or location of the
   * bank. This dialog also contains a list of matching banks which is
   * updated while the user enters data.
   * If any of the parameters contains a value those values will be used
   * to preset the bank list of the dialog widget.
   * @return a AB_BANKINFO object (owned by the caller, so the caller is
   * responsible for freeing this object via @ref AB_BankInfo_free)
   * @param parent parent widget (or NULL)
   * @param title caption of the dialog
   * @param country ISO 3166 country code (e.g. "de" for Germany, "at" for
   * Austria etc, like in TLDs)
   * @param bankCode bank code (also called <i>sort code</i> in some
   * countries, or <i>Bankleitzahl</i> in Germany/Austria)
   * @param swiftCode code assigned to the bank by SWIFT (so-called BIC)
   * @param bankName name of the bank
   * @param location location of the bank (typically the city the bank is
   * located in)
   */
  AB_BANKINFO *selectBank(QWidget* parent=0,
                          const QString &title=QString(""),
                          const QString &country=QString("de"),
                          const QString &bankCode=QString(""),
                          const QString &swiftCode=QString(""),
                          const QString &bankName=QString(""),
                          const QString &location=QString(""));

  void setupDialog(QWidget *parent=0);

  /**
   * This function makes sure that the configuration module with the given
   * name is available. It loads the appropriate plugin on demand.
   * You MUST NOT delete the pointer returned (if any).
   * @param modname name of the module to load. Normally this is the name
   * of the backend for which the configuration module is requested. If NULL
   * then the applications config module is returned (if one is set).
   */
  QBCfgModule *getConfigModule(const char *modname);

  /**
   * Sets the configuration module of the application.
   * The caller remains the owner of the module.
   */
  void setAppConfigModule(QBCfgModule *mod);

  /** Convenience function to convert a QString into a std::string
   * that will contain UTF-8 encoded characters, as necessary when
   * passing strings into aqbanking.
   *
   * Watch out with the correct encodings! String passed into and
   * out of aqbanking are expected in UTF-8, but NOT in Latin-1 or
   * similar! */
  static Q4BANKING_API std::string QStringToUtf8String(const QString &qs);

  /** Convenience function for extracting the GUI part of the
   * HTML/cleartext-combi-strings from aqbanking, adapted to
   * strings for QT gui widgets.
   * 
   * If the given string contains a
   * &lt;html>html-part...&lt;/html> section, then the "html-part"
   * section will be returned with enclosing &lt;qt>...&lt;/qt>
   * tags plus potentially some whitespace around those. If the
   * given string does not contain a section like this, then the
   * string will be returned unchanged. */
  static Q4BANKING_API std::string guiString(const char *s);

  /** Convenience function that returns true if the given string
   * consists of pure 7-bit ASCII characters, and false
   * otherwise. 
   *
   * In particular, if the given string contains Umlauts, accents,
   * or similar, then this will return false. */
  static Q4BANKING_API bool isPure7BitAscii(const QString &s);

  static QString sanitizedNumber(const QString &qs);
  static QString sanitizedAlphaNum(const QString &qs);

};




#endif /* Q4BANKING_BANKING_H */


