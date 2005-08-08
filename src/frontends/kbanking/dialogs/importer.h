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

#ifndef AQBANKING_KDE_EDITTRANS_H
#define AQBANKING_KDE_EDITTRANS_H

#define KBANKING_IMPORTER_CB_ID "KBANKING_IMPORTER_CB_ID"

class KBanking;
class Importer;


#include "importer.ui.h"
#include "kbanking.h"
#include "waitcallback.h"

#include <list>
#include <string>

#include <qstring.h>


class Importer: public ImporterUi {
  Q_OBJECT

private:
  class WorkCallback: public WaitCallback {
  public:
    WorkCallback(Importer *ie, const char *id);
    virtual ~WorkCallback();
    virtual WaitCallback *instantiate();
    virtual GWEN_WAITCALLBACK_RESULT checkAbort(unsigned int level);
    virtual void log(unsigned int level,
                     unsigned int loglevel,
                     const char *s);
  private:
    Importer *_importer;
  };

public:
  Importer(KBanking *kb,
           QWidget* parent=0,
           const char* name=0,
           bool modal=FALSE,
           WFlags fl=0);
  ~Importer();

  bool init();
  bool fini();

  void log(unsigned int level,
           unsigned int loglevel,
           const char *s);
  GWEN_WAITCALLBACK_RESULT checkAbort(Importer::WorkCallback *wcb,
                                      unsigned int level);

public slots:
  void back();
  void next();
  void reject();
  void accept();

  void slotSelectFile();
  void slotFileNameChanged(const QString &s);

  void slotProfileSelected();

  void slotProfileDetails();
  void slotProfileEdit();

private:
  KBanking *_app;
  AB_IMEXPORTER_CONTEXT *_context;
  bool _aborted;
  GWEN_PLUGIN_DESCRIPTION_LIST2 *_importerList;
  QString _importerName;
  AB_IMEXPORTER *_importer;
  GWEN_DB_NODE *_profiles;
  GWEN_DB_NODE *_profile;
  std::list<QWidget*> _pagesDone;
  QString _logText;
  GWEN_DB_NODE *_dbData;
  WorkCallback *_waitCallback;

  void _wcbLog(GWEN_LOGGER_LEVEL loglevel, const QString &s);
  bool _updateImporterList();
  bool _readFile(const QString &fname);
  bool _checkFileType(const QString &fname);
  bool _importData(AB_IMEXPORTER_CONTEXT *ctx);

  bool _doPage(QWidget *p);
  bool _undoPage(QWidget *p);

  bool enterPage(QWidget *p, bool back);
  bool leavePage(QWidget *p, bool back);

  bool initSelectSourcePage();
  bool doSelectSourcePage(QWidget *p);
  bool undoSelectSourcePage(QWidget *p);

  bool initSelectImporterPage();
  bool enterSelectImporterPage(QWidget *p);
  bool doSelectImporterPage(QWidget *p);
  bool undoSelectImporterPage(QWidget *p);

  bool initSelectProfilePage();
  bool doSelectProfilePage(QWidget *p);
  bool undoSelectProfilePage(QWidget *p);

  void enterWorkingPage(QWidget *p);
  void enterImportingPage(QWidget *p);

  void save();


};



#endif // AQBANKING_KDE_EDITTRANS_H




