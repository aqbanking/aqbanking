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

#ifndef QBANKING_SELECTFROMLIST_H
#define QBANKING_SELECTFROMLIST_H

class QBanking;


#include <qstringlist.h>

#include "qbselectfromlist.ui.h"




class QBSelectFromList : public QBSelectFromListUi {
  Q_OBJECT
public:
  QBSelectFromList(QBanking *kb,
                   const QString &title,
                   const QString &message,
                   const QString &listTypeName,
                   int minSelection,
                   int maxSelection,
                   QWidget* parent=0,
                   const char* name=0,
                   bool modal=FALSE,
                   Qt::WFlags fl=0);
  ~QBSelectFromList();

  void init();
  void fini();

  void addEntry(const QString &name, const QString &descr);

  void selectEntry(const QString &s);
  QStringList selectedEntries();

public slots:
  void slotSelectionChanged();

private:
  QBanking *_app;
  int _minSelection;
  int _maxSelection;
};





#endif
