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

#ifndef QBANKING_BANKING_H
#define QBANKING_BANKING_H


#include <aqbanking/banking.h>
#include <aqbanking/accstatus.h>

#include <gwenhywfar/nettransportssl.h>

#include <qobject.h>
#include <qdatetime.h>
#include <qstring.h>

#include <list>

class QTranslator;

class QBanking;

#include "banking.h"
#include "qbflagstaff.h"


class QBProgress;
class QBSimpleBox;


class QBanking: public Banking {
  friend class QBanking_Linker;
private:
  QWidget *_parentWidget;
  GWEN_TYPE_UINT32 _lastWidgetId;
  AB_BANKING_LOGLEVEL _logLevel;
  std::list<QBProgress*> _progressWidgets;
  std::list<QBSimpleBox*> _simpleBoxWidgets;
  QBFlagStaff *_flagStaff;
  QTranslator *_translator;

  QBProgress *_findProgressWidget(GWEN_TYPE_UINT32 id);
  void _cleanupProgressWidgets();

  AB_ACCOUNT *_getAccount(const char *accountId);
  int _extractHTML(const char *text, GWEN_BUFFER *buf);

protected:
  virtual int messageBox(GWEN_TYPE_UINT32 flags,
			 const char *title,
			 const char *text,
			 const char *b1,
			 const char *b2,
			 const char *b3);

  virtual int inputBox(GWEN_TYPE_UINT32 flags,
		       const char *title,
                       const char *text,
                       char *buffer,
                       int minLen,
                       int maxLen);

  virtual GWEN_TYPE_UINT32 showBox(GWEN_TYPE_UINT32 flags,
                                   const char *title,
                                   const char *text);
  virtual void hideBox(GWEN_TYPE_UINT32 id);

  virtual GWEN_TYPE_UINT32 progressStart(const char *title,
                                         const char *text,
                                         GWEN_TYPE_UINT32 total);

  /**
   * Advances the progress bar an application might present to the user.
   * @param id id assigned by @ref AB_Banking_Progress_Start
   * @param progress new value for progress. A special value is
   *  AB_BANKING_PROGRESS_NONE which means that the progress is unchanged.
   * This might be used as a keepalive call to a GUI.
   */
  virtual int progressAdvance(GWEN_TYPE_UINT32 id,
                              GWEN_TYPE_UINT32 progress);
  virtual int progressLog(GWEN_TYPE_UINT32 id,
                          AB_BANKING_LOGLEVEL level,
                          const char *text);
  virtual int progressEnd(GWEN_TYPE_UINT32 id);

  virtual int print(const char *docTitle,
                    const char *docType,
                    const char *descr,
                    const char *text);

public:
  QBanking(const char *appname,
           const char *fname=0);
  virtual ~QBanking();

  int init();
  int fini();


  QBFlagStaff *flagStaff();

  void setParentWidget(QWidget *w);

  int enqueueJob(AB_JOB *j);
  int dequeueJob(AB_JOB *j);
  int executeQueue();

  void setAccountAlias(AB_ACCOUNT *a, const char *alias);

  void accountsUpdated();
  void outboxCountChanged(int count);
  void statusMessage(const QString &s);

  virtual void invokeHelp(const char *subject);

  virtual bool mapAccount(const AB_ACCOUNT *a);

  bool askMapAccount(const char *id,
                     const char *bankCode,
                     const char *accountId);


  virtual bool addTransaction(const AB_ACCOUNT *a, const AB_TRANSACTION *t);
  virtual bool setAccountStatus(const AB_ACCOUNT *a,
                                const AB_ACCOUNT_STATUS *ast);

  bool requestBalance(const char *accountId);
  bool requestTransactions(const char *accountId,
                           const QDate &fromDate,
                           const QDate &toDate);

  virtual bool importContext(AB_IMEXPORTER_CONTEXT *ctx,
                             GWEN_TYPE_UINT32 flags);

  virtual bool importAccountInfo(AB_IMEXPORTER_ACCOUNTINFO *ai,
                                 GWEN_TYPE_UINT32 flags);

  virtual bool interactiveImport();

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
                          const QString &title="",
                          const QString &country="de",
                          const QString &bankCode="",
                          const QString &swiftCode="",
                          const QString &bankName="",
                          const QString &location="");

  static std::string QStringToUtf8String(const QString &qs);

};




#endif /* QBANKING_BANKING_H */


