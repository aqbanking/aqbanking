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

// QBanking includes
#include "qbselectfromlist.h"
#include "qbanking.h"

// Gwenhywfar includes
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

// QT includes
#include <qlabel.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlistview.h>



QBSelectFromList::QBSelectFromList(QBanking *kb,
                                   const QString &title,
                                   const QString &message,
                                   const QString &listTypeName,
                                   int minSelection,
                                   int maxSelection,
                                   QWidget* parent,
                                   const char* name,
                                   bool modal,
                                   WFlags fl)
:QBSelectFromListUi(parent, name, modal, fl)
,_app(kb)
,_minSelection(minSelection)
,_maxSelection(maxSelection){
  setCaption(title);
  msgLabel->setText(message);
  choiceBox->setCaption(listTypeName);

  if (maxSelection>1)
    listView->setSelectionMode(QListView::Multi);
  else
    listView->setSelectionMode(QListView::Single);
  listView->setAllColumnsShowFocus(true);

  QObject::connect(listView,
                   SIGNAL(selectionChanged()),
                   this, SLOT(slotSelectionChanged()));

}



QBSelectFromList::~QBSelectFromList(){
}



void QBSelectFromList::init(){
  GWEN_DB_NODE *dbConfig=NULL;
  const char *s;
  int rv;

  s=name();
  rv=_app->loadSharedSubConfig("qbanking",
			       "gui/dlgs/QBSelectFromList",
			       &dbConfig);
  if (rv==0) {
    GWEN_DB_NODE *db;

    assert(dbConfig);
    if (s)
      db=GWEN_DB_GetGroup(dbConfig, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
    else
      db=GWEN_DB_GetGroup(dbConfig, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "generic");
    if (db) {
      int i, j;
      const char *p;
      int x, y;

      x=GWEN_DB_GetIntValue(db, "width", 0, -1);
      y=GWEN_DB_GetIntValue(db, "height", 0, -1);
      if (x!=-1 && y!=-1) {
	DBG_ERROR(0, "Resizing to %d/%d", x, y);
	resize(x, y);
      }
      x=GWEN_DB_GetIntValue(db, "x", 0, -1);
      y=GWEN_DB_GetIntValue(db, "y", 0, -1);
      if (x!=-1 && y!=-1)
	move(x, y);

      p=GWEN_DB_GetCharValue(db, "sortOrder", 0, "ascending");
      if (p) {
	if (strcasecmp(p, "ascending")==0)
	  listView->setSortOrder(Qt::Ascending);
	else
	  if (strcasecmp(p, "descending")==0)
	    listView->setSortOrder(Qt::Descending);
      }
      i=GWEN_DB_GetIntValue(db, "sortColumn", 0, -1);
      if (i!=-1)
	listView->setSortColumn(i);

      /* found settings */
      for (i=0; i<listView->columns(); i++) {
	listView->setColumnWidthMode(i, QListView::Manual);
	j=GWEN_DB_GetIntValue(db, "columns", i, -1);
	if (j!=-1)
	  listView->setColumnWidth(i, j);
      } /* for */
    }
    GWEN_DB_Group_free(dbConfig);
  }
}



void QBSelectFromList::fini(){
  GWEN_DB_NODE *dbConfig;
  GWEN_DB_NODE *db;
  int i, j;
  const char *s;
  int rv;

  dbConfig=GWEN_DB_Group_new("config");
  assert(dbConfig);
  s=name();
  if (s)
    db=GWEN_DB_GetGroup(dbConfig, GWEN_DB_FLAGS_OVERWRITE_GROUPS, s);
  else
    db=GWEN_DB_GetGroup(dbConfig, GWEN_DB_FLAGS_OVERWRITE_GROUPS, "generic");
  assert(db);

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "x", x());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "y", y());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "width", width());
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS,
                      "height", height());

  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                      "sortColumn", listView->sortColumn());
  switch(listView->sortOrder()) {
  case Qt::Ascending:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "sortOrder", "ascending");
    break;
  case Qt::Descending:
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_DEFAULT,
                         "sortOrder", "descending");
    break;
  default:
    break;
  }

  for (i=0; i<listView->columns(); i++) {
    j=listView->columnWidth(i);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "columns", j);
  } /* for */

  rv=_app->saveSharedSubConfig("qbanking",
			       "gui/dlgs/QBSelectFromList",
			       dbConfig);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
  }
  GWEN_DB_Group_free(dbConfig);
}



void QBSelectFromList::addEntry(const QString &name, const QString &descr){
  QListViewItem *lv;

  lv=new QListViewItem(listView, name, descr);
}



void QBSelectFromList::selectEntry(const QString &s){
  // Create an iterator and give the listview as argument
  QListViewItemIterator it(listView);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (s.lower()==it.current()->text(0).lower()) {
      listView->ensureItemVisible(it.current());
      listView->setSelected(it.current(), true);
      it.current()->repaint();
    }
    else {
      if (it.current()->isSelected()==true) {
        listView->setSelected(it.current(), false);
        it.current()->repaint();
      }
    }
  } // for

}



QStringList QBSelectFromList::selectedEntries(){
  QStringList sl;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(listView);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      sl.append(it.current()->text(0));
    }
  } // for

  return sl;
}



void QBSelectFromList::slotSelectionChanged(){
  if (_minSelection<1) {
    int sel=0;

    QListViewItemIterator it(listView);
    // iterate through all items of the listview
    for (;it.current();++it) {
      if (it.current()->isSelected()) {
        sel++;
      }
    } // for
    if (sel<_minSelection ||
        (_maxSelection && sel>_maxSelection)) {
      buttonOk->setEnabled(false);
    }
    else
      buttonOk->setEnabled(true);
  }
  else {
    buttonOk->setEnabled(true);
  }
}




#include "qbselectfromlist.moc"


