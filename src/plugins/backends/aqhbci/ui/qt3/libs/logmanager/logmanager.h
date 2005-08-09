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


#ifndef LOGMANAGER_H
#define LOGMANAGER_H


#include "logmanager.ui.h"
#include <gwenhywfar/msgengine.h>

#include <string>
#include <list>

using namespace std;

class QListViewItem;


class LogManager: public LogManagerUi {
  Q_OBJECT
private:
  string _baseDir;
  int _trustLevel;
  list<string> _banks;
  list<string> _logFiles;
  GWEN_MSGENGINE *_msgEngine;
  QString _currentFile;
  QString _lastDir;
  string _currentLog;

public:
  LogManager(const char *baseDir=0,
	     QWidget* parent=0,
	     const char* name=0,
	     bool modal=FALSE,
	     WFlags fl=0);
  ~LogManager();

public slots:
  void bankActivated(const QString &qs);
  void trustActivated(int idx);
  void fileSelected(QListViewItem *qv);
  void saveFile();

private:
  int _scanBanks();
  int _scanBank(const string &bankCode);
  string _anonymize(const string &bankCode, const string &fname,
                    int trustLevel);
  string _dump(const string &s);
};




#endif


