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

#ifndef QBANKING_CFGTAB_H
#define QBANKING_CFGTAB_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include <q4banking/qbanking.h>

#include "qbcfgtab.ui.h"
#include <qstring.h>


class QBanking;
class QBCfgTabPage;


class Q4BANKING_API QBCfgTab: public QDialog, public Ui_QBCfgTabUi {
  Q_OBJECT
private:
  QBanking *_qbanking;
  QString _description;
  QString _fullDescription;
  QString _helpContext;
  bool _allowApply;

protected:
  void accept();

public:
  QBCfgTab(QBanking *qb, QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~QBCfgTab();

  QBanking *getBanking();

  void setDescription(const QString &s);
  const QString &getDescription();

  void setHelpContext(const QString &s);
  const QString &getHelpContext();

  void addPage(QBCfgTabPage *p);
  QBCfgTabPage *getPage(int idx);
  QBCfgTabPage *getCurrentPage();
  void setCurrentPage(int idx);

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

  virtual void updateViews();

  int exec();

  void setAllowApply(bool b);

public slots:
  virtual void slotHelp();
  virtual void slotApply();

  virtual void languageChange();

};


#endif
