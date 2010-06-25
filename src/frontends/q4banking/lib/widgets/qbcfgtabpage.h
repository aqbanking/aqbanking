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

#ifndef QBANKING_CFGTABPAGE_H
#define QBANKING_CFGTABPAGE_H


#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>
#include <q4banking/qbanking.h>

#include <QtGui/qwidget.h>
#include <QtCore/qstring.h>
//Added by qt3to4:
#include <Q3VBoxLayout>



class QBanking;
class Q3VBoxLayout;
class QComboBox;
class QBCfgTab;


class Q4BANKING_API QBCfgTabPage: public QWidget {
  friend class QBCfgTab;
private:
  QBanking *_qbanking;
  QString _title;
  QString _description;
  Q3VBoxLayout *_pageLayout;
  QBCfgTab *_cfgTab;
  QString _helpSubject;

  void _setCfgTab(QBCfgTab *w);

public:
  QBCfgTabPage(QBanking *qb,
               const QString &title,
	       QWidget *parent=0, const char *name=0, Qt::WFlags f=0);
  virtual ~QBCfgTabPage();

  void addWidget(QWidget *w);

  QBanking *getBanking();
  const QString &getTitle();

  void setDescription(const QString &s);
  const QString &getDescription();

  void setHelpSubject(const QString &s);
  const QString &getHelpSubject();

  QBCfgTab *getCfgTab();

  virtual bool fromGui();
  virtual bool toGui();
  virtual bool checkGui();

  virtual void updateView();

  void fillCountryCombo(QComboBox *qb);
  void selectCountryInCombo(QComboBox *qcb, const char *s);

};


#endif
