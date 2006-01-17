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


#include "qbcfgtabpage.h"
#include "qbanking.h"

#include <qlayout.h>
#include <qcombobox.h>
#if (QT_VERSION >= 0x040100)
// qt-4.1.0 errorneously forgets to add this include
#  include <QComboBox>
#endif


QBCfgTabPage::QBCfgTabPage(QBanking *qb,
                           const QString &title,
			   QWidget *parent,
			   const char *name,
			   WFlags f)
:QWidget(parent, name, f), _qbanking(qb), _title(title), _cfgTab(0) {
  _pageLayout=new QVBoxLayout(this, 11, 6, "pageLayout");

}



QBCfgTabPage::~QBCfgTabPage() {
}



void QBCfgTabPage::addWidget(QWidget *w) {
  _pageLayout->addWidget(w);
}



QBanking *QBCfgTabPage::getBanking() {
  return _qbanking;
}



const QString &QBCfgTabPage::getTitle() {
  return _title;
}



bool QBCfgTabPage::fromGui() {
  return true;
}



bool QBCfgTabPage::toGui() {
  return true;
}



bool QBCfgTabPage::checkGui() {
  return true;
}



void QBCfgTabPage::fillCountryCombo(QComboBox *qcb) {
  AB_COUNTRY_CONSTLIST2 *cl;

  qcb->clear();
  qcb->insertItem(tr("- select country -"));
  cl=AB_Banking_ListCountriesByName(getBanking()->getCInterface(), "*");
  if (cl) {
    AB_COUNTRY_CONSTLIST2_ITERATOR *it;

    it=AB_Country_ConstList2_First(cl);
    if (it) {
      const AB_COUNTRY *c;
      GWEN_STRINGLIST *sl;
      GWEN_STRINGLISTENTRY *se;
      const char *s;

      sl=GWEN_StringList_new();
      c=AB_Country_ConstList2Iterator_Data(it);
      while(c) {
        s=AB_Country_GetLocalName(c);
        assert(s);
        GWEN_StringList_AppendString(sl, s, 0, 1);
        c=AB_Country_ConstList2Iterator_Next(it);
      }
      AB_Country_ConstList2Iterator_free(it);
      GWEN_StringList_Sort(sl, 0, GWEN_StringList_SortModeNoCase);
      se=GWEN_StringList_FirstEntry(sl);
      while(se) {
	s=GWEN_StringListEntry_Data(se);
        assert(s);
        qcb->insertItem(QString::fromUtf8(s));
        se=GWEN_StringListEntry_Next(se);
      }
      GWEN_StringList_free(sl);
    }

    AB_Country_ConstList2_free(cl);
  }
}



void QBCfgTabPage::selectCountryInCombo(QComboBox *qcb, const char *s) {
  const AB_COUNTRY *ci;

  if (!s)
    s="de";

  ci=AB_Banking_FindCountryByCode(getBanking()->getCInterface(), s);
  if (ci) {
    s=AB_Country_GetLocalName(ci);
    assert(s);
    qcb->setCurrentText(QString::fromUtf8(s));
  }
}



void QBCfgTabPage::_setCfgTab(QBCfgTab *w) {
  _cfgTab=w;
}



QBCfgTab *QBCfgTabPage::getCfgTab() {
  return _cfgTab;
}



void QBCfgTabPage::setDescription(const QString &s) {
  _description=s;
}



const QString &QBCfgTabPage::getDescription() {
  return _description;
}



void QBCfgTabPage::setHelpSubject(const QString &s) {
  _helpSubject=s;
}



const QString &QBCfgTabPage::getHelpSubject() {
  return _helpSubject;
}



void QBCfgTabPage::updateView() {
}













