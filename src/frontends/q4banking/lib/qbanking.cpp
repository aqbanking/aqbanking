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


#include "qbanking.h"
#include "qbflagstaff.h"
#include "qbmapaccount.h"
#include "qbimporter.h"
#include "qbpickstartdate.h"
#include "qbprintdialog.h"
#include "qbselectbank.h"
#include "qbcfgmodule.h"
#include "qbcfgtabsettings.h"
//Added by qt3to4:
#include <Q3CString>

#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <assert.h>
#include <qstring.h>
#include <qmessagebox.h>
#include <qinputdialog.h>
#include <qapplication.h>
#include <qdatetime.h>
#include <qwidget.h>
#include <qtranslator.h>
#include <qtextcodec.h>
#include <q3process.h>
#include <qdir.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/pathmanager.h> // for GWEN_PathManager_GetPaths


/* Note: We use the key "AqBanking" because from the windows registry
 * point of view, these plugins all belong to the large AqBanking
 * package. */
#define Q4BANKING_REGKEY_PATHS        "Software\\AqBanking\\Paths"
#define Q4BANKING_REGKEY_CFGMODULEDIR "cfgmoduledir"
#define Q4BANKING_CFGMODULEDIR        "cfgmodules"


#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif

#define Q4BANKING_DESTLIB "q4banking"



QBanking::QBanking(const char *appname,
                   const char *fname)
:AB_Banking(appname, fname)
,_parentWidget(NULL)
,_logLevel(GWEN_LoggerLevel_Info)
,_translator(0)
,_gui(NULL)
,_pluginManagerCfgModules(0)
,_appCfgModule(0){
  _flagStaff=new QBFlagStaff();
}



QBanking::~QBanking(){
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
  }
  delete _flagStaff;
}



QBGui *QBanking::getGui() const {
  return _gui;
}



void QBanking::setGui(QBGui *g) {
  _gui=g;
}



void QBanking::setAppHelpPath(const QString &s) {
  _appHelpPath=s;
}



int QBanking::_extractHTML(const char *text, GWEN_BUFFER *tbuf) {
  GWEN_XMLNODE *xmlNode;

  xmlNode=GWEN_XMLNode_fromString(text, strlen(text),
				  GWEN_XML_FLAGS_DEFAULT |
				  GWEN_XML_FLAGS_HANDLE_OPEN_HTMLTAGS);
  if (xmlNode==NULL) {
    DBG_DEBUG(0, "here");
    return -1;
  }
  else {
    GWEN_XMLNODE *nn;

    nn=GWEN_XMLNode_FindFirstTag(xmlNode, "html", 0, 0);
    if (nn) {
      GWEN_XMLNODE *on, *onn;
      int rv;

      on=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "root");
      onn=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag, "qt");
      GWEN_XMLNode_AddChild(on, onn);
      GWEN_XMLNode_AddChildrenOnly(onn, nn, 1);

      /* text contains HTML tag, take it */
      rv=GWEN_XMLNode_toBuffer(on, tbuf, GWEN_XML_FLAGS_DEFAULT);
      GWEN_XMLNode_free(on);
      if (rv) {
	DBG_ERROR(AQBANKING_LOGDOMAIN, "Error writing data to stream");
	GWEN_XMLNode_free(xmlNode);
	return -1;
      }
    }
    else {
      GWEN_XMLNode_free(xmlNode);
      return 1;
    }
  }
  GWEN_XMLNode_free(xmlNode);
  return 0;
}



QBFlagStaff *QBanking::flagStaff(){
  return _flagStaff;
}



void QBanking::accountsUpdated(){
  flagStaff()->accountsUpdated();
}



void QBanking::invokeHelp(const QString &context,
                          const QString &subject){
  QString url;
  Q3Process *p;

  DBG_ERROR(0, "Help wanted for \"%s\"/\"%s\"",
            context.latin1(), subject.latin1());

  url=context.lower()+".html";
  if (!subject.isEmpty())
    url+=+"#"+subject;

  p=new Q3Process();
  p->addArgument(Q4BHELP_BINARY_NAME);
  p->addArgument(url);
  if (!_appHelpPath.isEmpty())
    p->addArgument(_appHelpPath);
  if (!p->launch(QString::null)) {
    DBG_ERROR(0, "Could not start process");
  }
  delete p;
}



bool QBanking::mapAccount(const AB_ACCOUNT *a){
  return false;
}



bool QBanking::addTransaction(const AB_ACCOUNT *a, const AB_TRANSACTION *t){
  return false;
}



bool QBanking::setAccountStatus(const AB_ACCOUNT *a,
                                const AB_ACCOUNT_STATUS *ast){
  return false;
}



AB_ACCOUNT *QBanking::_getAccount(const char *accountId){
  AB_ACCOUNT *a;

  a=AB_Banking_GetAccountByAlias(getCInterface(), accountId, 0);
  if (!a) {
    // should not happen anyway
    QMessageBox::critical(_parentWidget,
			  QWidget::tr("Account Not Mapped"),
			  QWidget::tr("<qt>"
				      "<p>"
				      "The given application account has not "
				      "been mapped to banking accounts."
				      "</p>"
				      "</qt>"
				     ),
			  QMessageBox::Ok,Qt::NoButton);
    return 0;
  }

  return a;
}



void QBanking::setAccountAlias(AB_ACCOUNT *a, const char *alias){
  assert(a);
  assert(alias);

  AB_Banking_SetAccountAlias(getCInterface(), a, alias, 0);
}



bool QBanking::askMapAccount(const char *id,
                             const char *bankCode,
                             const char *accountId){
  QBMapAccount *w;

  w=new QBMapAccount(this, bankCode, accountId,
                     _parentWidget, "MapAccount", true);
  if (w->exec()==QDialog::Accepted) {
    AB_ACCOUNT *a;

    a=w->getAccount();
    assert(a);
    DBG_NOTICE(0,
               "Mapping application account \"%s\" to "
               "online account \"%s/%s\"",
               id,
               AB_Account_GetBankCode(a),
               AB_Account_GetAccountNumber(a));
    setAccountAlias(a, id);
    delete w;
    return true;
  }

  delete w;
  return false;
}



bool QBanking::importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             uint32_t flags){
  AB_IMEXPORTER_ACCOUNTINFO *ai;

  ai=AB_ImExporterContext_GetFirstAccountInfo(ctx);
  while(ai) {
    if (!importAccountInfo(ai, flags))
      return false;
    ai=AB_ImExporterContext_GetNextAccountInfo(ctx);
  }
  return true;
}


bool QBanking::importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 uint32_t flags){
  DBG_NOTICE(0, "Import account info function not overloaded");
  return false;
}



bool QBanking::interactiveImport(){
  QBImporter *w;
  bool res;

  w=new QBImporter(this, _parentWidget, "Importer", true);
  if (!w->init()) {
    delete w;
    return false;
  }
  res=(w->exec()==QDialog::Accepted);
  res&=w->fini();
  delete w;
  return res;
}


// ////////////////////////////////////////
// This part is needed only for qt4, see the explanation at the class.
#if QT_VERSION >= 0x040000

// These lines copied from aqbanking/i18n_l.h
#ifdef HAVE_I18N
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
# endif
# define I18N(msg) dgettext(PACKAGE, msg)
#else
# define I18N(msg) msg
#endif

/** An override implementation of the default QTranslation behaviour.
 *
 * In qt4, the QTranslator will refuse to translate our messages which
 * have a NULL contextString in the po file. The po file does not have
 * a notion of "contextString", so there's no way we could provide the
 * proper context anyway. In qt3, this was accepted by the
 * QTranslator, but not so in qt4.
 *
 * Hence for qt4 we have to override the QTranslator::translate()
 * method by our own method that additionally looks up strings by
 * dgettext(). We do this by providing our own derived class of
 * QTranslator and install that one as translator instead of the
 * default class.
 */
class MyTranslator : public QTranslator {
public:
  // Standard constructor
  MyTranslator(QObject *parent)
    : QTranslator(parent)
  {
  }
  // Standard destructor
  ~MyTranslator() { }
  // This overrides the default translate() method of QTranslator.
  QString translate ( const char * context, const char * sourceText,
		      const char * comment = 0 ) const
  {
    // First lookup the normal qt translation method.
    QString result = QTranslator::translate(context, sourceText, comment);
    //qDebug("Qt Translation: context='%s', sourceText='%s', result='%s'", context, sourceText, result.ascii());

    // Did we get a translation from qt's QTranslator?
    if (result.isEmpty()) {
      // No, the normal qt method didn't find anything. Therefore try
      // dgettext().
      const char *gtext = I18N(sourceText);
      // Did gettext find a translation?
      if (gtext && *gtext && gtext != sourceText) {
	// Yes, so convert the utf8 gettext string properly to a
	// QString.
	result = QString::fromUtf8(gtext);
	// qDebug("Gettext got translation: sourceText='%s', result='%s'", sourceText, result.toLocal8Bit().data());
      }
    }
    // Returning the resulting translation.
    return result;
  }
};
#endif // QT_VERSION >= 0x040000
// ////////////////////////////////////////


int QBanking::init(){
  int rv;
  GWEN_PLUGIN_MANAGER *pm;

  rv=AB_Banking::init();
  if (rv)
    return rv;

  _translator=new 
#if QT_VERSION >= 0x040000
    MyTranslator(0) // In qt4, use our own translation implementation.
#else
    QTranslator(0) // In qt3, QTranslator works fine for us.
#endif
    ;
  QString languageCode = QTextCodec::locale();
  languageCode.truncate(2);

  GWEN_STRINGLIST *sl =
    GWEN_PathManager_GetPaths(AB_PM_LIBNAME, AB_PM_DATADIR);
  assert(sl);
  QString datadir(GWEN_StringList_FirstString(sl));
  GWEN_StringList_free(sl);
  datadir+=DIRSEP;
  datadir+="aqbanking";
  QDir i18ndir = datadir;
  if (!i18ndir.exists())
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "Datadir %s does not exist.", i18ndir.path().ascii());
  i18ndir.cd("i18n");
  if (!i18ndir.exists())
    DBG_INFO(AQBANKING_LOGDOMAIN,
	     "I18ndir %s does not exist.", i18ndir.path().ascii());

  // no need to specify ".qm" suffix; QTranslator tries that itself
  if (_translator->load(languageCode,
			i18ndir.path())) {
    DBG_DEBUG(AQBANKING_LOGDOMAIN,
	      "Qt I18N available for your language");
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "No Qt translation found for your language %s",
	      languageCode.ascii());
  }
  qApp->installTranslator(_translator);

  /* create cfg module plugin manager */
  DBG_DEBUG(AQBANKING_LOGDOMAIN,
	    "Registering cfg module plugin manager");
  pm=GWEN_PluginManager_new(Q4BANKING_PM_CFGMODULE, Q4BANKING_DESTLIB);
  if (GWEN_PluginManager_Register(pm)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not register cfg module plugin manager");
    return -1;
  }
  GWEN_PluginManager_AddPathFromWinReg(pm, Q4BANKING_DESTLIB,
                                       Q4BANKING_REGKEY_PATHS,
				       Q4BANKING_REGKEY_CFGMODULEDIR);
  GWEN_PluginManager_AddPath(pm, Q4BANKING_DESTLIB,
                             Q4BANKING_PLUGINS
                             DIRSEP
                             Q4BANKING_CFGMODULEDIR);
  _pluginManagerCfgModules=pm;

  return 0;
}



int QBanking::fini(){
  int rv;
  std::list<QBCfgModule*>::iterator it;

  /* unload and free all config modules */
  for (it=_cfgModules.begin(); it!=_cfgModules.end(); it++) {
    GWEN_PLUGIN *pl;
    pl=(*it)->getPlugin();
    (*it)->setPlugin(0);
    delete *it;
    GWEN_Plugin_free(pl);
  }
  _cfgModules.clear();

  if (_pluginManagerCfgModules) {
    if (GWEN_PluginManager_Unregister(_pluginManagerCfgModules)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Could not unregister cfg module plugin manager");
    }
    GWEN_PluginManager_free(_pluginManagerCfgModules);
    _pluginManagerCfgModules=0;
  }

  /* deinit base class */
  rv=AB_Banking::fini();
  if (_translator) {
    qApp->removeTranslator(_translator);
    delete _translator;
    _translator=0;
  }

  return rv;
}



void QBanking::outboxCountChanged(int count){
  flagStaff()->outboxCountChanged(count);
}



void QBanking::statusMessage(const QString &s){
  flagStaff()->statusMessage(s);
}



std::string QBanking::QStringToUtf8String(const QString &qs) {
  if (qs.isEmpty())
    return "";
  else {
    Q3CString utfData=qs.utf8();
  
    // Note: This commented-out code below introduced an extra
    // char-by-char copying that I don't consider necessary. The
    // std::string contains "char", not "unsigned char", so the
    // right side below ("unsigned char") will be converted back to
    // "char" when it is appended to the std::string on the left
    // side. This is unnecessary. The only necessary thing is that
    // the "const char*" buffer of the QCString is copied into a
    // std::string, and exactly that happens when the std::string
    // return value is created, so there is really nothing more to
    // do.  -- cstim, 2005-09-19
  
     //std::string result;
     //int len;
     //int i;
     //len=utfData.length();
     //for (i=0; i<len; i++)
     //  result+=(unsigned char)utfData[i];
     //return result;
  
    return utfData.data();
  }
}



std::string QBanking::guiString(const char *s) {
  GWEN_BUFFER *tbuf;
  std::string res;

  assert(s);
  tbuf=GWEN_Buffer_new(0, strlen(s), 0, 1);
  if (_extractHTML(s, tbuf)) {
    GWEN_Buffer_free(tbuf);
    return s;
  }
  res=std::string(GWEN_Buffer_GetStart(tbuf),
                  GWEN_Buffer_GetUsedBytes(tbuf));
  GWEN_Buffer_free(tbuf);
  return res;
}



AB_BANKINFO *QBanking::selectBank(QWidget* parent,
                                  const QString &title,
                                  const QString &country,
                                  const QString &bankCode,
                                  const QString &swiftCode,
                                  const QString &bankName,
                                  const QString &location) {
  return QBSelectBank::selectBank(this, parent, title, country,
                                  bankCode, swiftCode,
                                  bankName, location);
}


bool QBanking::isPure7BitAscii(const QString &input){
  unsigned stringlength = input.length();

  for (unsigned k = 0; k < stringlength; ++k) {
    if (input[k].unicode() > 0x7f) {
      DBG_DEBUG(0, "String \"%s\" is not pure-7bit-ascii at character %d.\n",
		input.local8Bit().data(), k);
      return false;
    }
  }
  return true;
}



QBCfgModule *QBanking::_loadCfgModule(const char *modname){
  GWEN_LIBLOADER *ll;
  QBCfgModule *mod;
  QBCFGMODULE_FACTORY_FN fn;
  void *p;
  const char *s;
  int err;
  GWEN_BUFFER *mbuf;
  GWEN_PLUGIN *pl;
  GWEN_PLUGIN_MANAGER *pm;

  pm=GWEN_PluginManager_FindPluginManager(Q4BANKING_PM_CFGMODULE);
  if (!pm) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not find plugin manager for \"%s\"",
              Q4BANKING_PM_CFGMODULE);
    return 0;
  }
  pl=GWEN_PluginManager_LoadPlugin(pm, modname);
  if (!pl) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
	      "Could not load %s plugin for \"%s\"",
              Q4BANKING_PM_CFGMODULE, modname);
    return 0;
  }
  ll=GWEN_Plugin_GetLibLoader(pl);

  mbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(mbuf, Q4BANKING_PM_CFGMODULE "_");
  s=modname;
  while(*s) GWEN_Buffer_AppendByte(mbuf, tolower(*(s++)));

  /* create name of init function */
  GWEN_Buffer_AppendString(mbuf, "_modfactory");

  /* resolve name of factory function */
  err=GWEN_LibLoader_Resolve(ll, GWEN_Buffer_GetStart(mbuf), &p);
  if (err) {
    DBG_ERROR_ERR(0, err);
    GWEN_Buffer_free(mbuf);
    GWEN_Plugin_free(pl);
    return 0;
  }
  GWEN_Buffer_free(mbuf);

  fn=(QBCFGMODULE_FACTORY_FN)p;
  assert(fn);
  mod=fn(this);
  if (!mod) {
    DBG_ERROR(0, "Error in plugin: No module created");
    GWEN_Plugin_free(pl);
    return 0;
  }

  /* store libloader */
  mod->setPlugin(pl);

  return mod;
}



QBCfgModule *QBanking::_findCfgModule(const char *modname) {
  std::list<QBCfgModule*>::iterator it;
  QString qs;

  qs=QString::fromUtf8(modname).lower();
  for (it=_cfgModules.begin(); it!=_cfgModules.end(); it++) {
    if (qs.compare((*it)->getName().lower())==0)
      return *it;
  }
  return 0;
}



QBCfgModule *QBanking::getConfigModule(const char *modname) {
  QBCfgModule *mod;

  if (modname==0)
    return _appCfgModule;
  mod=_findCfgModule(modname);
  if (mod)
    return mod;
  mod=_loadCfgModule(modname);
  if (!mod)
    return 0;
  _cfgModules.push_back(mod);
  return mod;
}



void QBanking::setAppConfigModule(QBCfgModule *mod) {
  _appCfgModule=mod;
}



void QBanking::setupDialog(QWidget *parent) {
  QBCfgTabSettings dlg(this, parent, "QBankingSettings");

  dlg.addUsersPage();
  dlg.addAccountsPage();
  dlg.addBackendsPage();
  if (!dlg.toGui()) {
    DBG_ERROR(0, "Could not init dialog");
    return;
  }
  if (dlg.exec()==QDialog::Accepted) {
    dlg.fromGui();
    flagStaff()->accountsUpdated();
  }
}



int QBanking::print(const QString &docTitle,
                    const QString &docType,
                    const QString &descr,
                    const QString &text) {
  return GWEN_Gui_Print(QStringToUtf8String(docTitle).c_str(),
			QStringToUtf8String(docType).c_str(),
			QStringToUtf8String(descr).c_str(),
			QStringToUtf8String(text).c_str(),
			0);
}



QString QBanking::sanitizedNumber(const QString &qs) {
  if (!qs.isEmpty()) {
    QString qs2;
    int i;

    for (i=0; i<qs.length(); i++) {
      if (qs[i].isDigit())
	qs2+=qs[i];
    }
    return qs2;
  }
  else
    return qs;
}



QString QBanking::sanitizedAlphaNum(const QString &qs) {
  if (!qs.isEmpty()) {
    QString qs2;
    int i;

    for (i=0; i<qs.length(); i++) {
      if (qs[i].isLetterOrNumber())
	qs2+=qs[i];
    }
    return qs2;
  }
  else
    return qs;
}













