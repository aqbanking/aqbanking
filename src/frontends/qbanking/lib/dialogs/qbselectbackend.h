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


#ifndef QBANKING_SELBACKEND_H
#define QBANKING_SELBACKEND_H


class QBanking;

#include "qbselectbackend.ui.h"
#include <qbanking/banking.h> /* For QBANKING_API */
#include <string>


class QBSelectBackend: public QBSelectBackendUi {
  Q_OBJECT

private:
  QBanking *_app;
  QString _selectedBackend;
  std::list<GWEN_PLUGIN_DESCRIPTION*> _plugins;

public:
  QBSelectBackend(QBanking *kb,
		  const QString &backend=QString::null,
                  QWidget* parent = 0,
                  const char* name = 0,
                  bool modal = FALSE,
                  WFlags fl = 0);

  ~QBSelectBackend();

  const QString &getSelectedBackend() const;

  static QString selectBackend(QBanking *kb,
			       const QString &backend=QString::null,
			       QWidget* parent = 0);

public slots:
  void slotActivated(int idx);
  void slotHelp();
};


#endif // QBANKING_SELBACKEND_H

