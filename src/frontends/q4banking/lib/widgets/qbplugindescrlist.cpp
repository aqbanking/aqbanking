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


#include "qbplugindescrlist.h"
#include <assert.h>
#include <QtCore/qstring.h>



QBPluginDescrListViewItem::QBPluginDescrListViewItem(QBPluginDescrListView *parent,
                                                 GWEN_PLUGIN_DESCRIPTION *pd)
:Q3ListViewItem(parent)
,_descr(pd){
  assert(pd);
  _populate();
}



QBPluginDescrListViewItem::QBPluginDescrListViewItem(const QBPluginDescrListViewItem &item)
:Q3ListViewItem(item)
,_descr(0){
  if (item._descr) {
    _descr=item._descr;
  }
}


QBPluginDescrListViewItem::QBPluginDescrListViewItem(QBPluginDescrListView *parent,
						 Q3ListViewItem *after,
						 GWEN_PLUGIN_DESCRIPTION *pd)
:Q3ListViewItem(parent, after)
,_descr(pd){
  assert(pd);
  _populate();
}



QBPluginDescrListViewItem::~QBPluginDescrListViewItem(){
}



GWEN_PLUGIN_DESCRIPTION *QBPluginDescrListViewItem::getPluginDescr(){
  return _descr;
}


void QBPluginDescrListViewItem::_populate() {
  QString tmp;
  int i;

  assert(_descr);

  i=0;

  // name
  setText(i++, QString::fromUtf8(GWEN_PluginDescription_GetName(_descr)));

  // version
  tmp=QString::fromUtf8(GWEN_PluginDescription_GetVersion(_descr));
  if (tmp.isEmpty())
    tmp=QWidget::tr("(unknown)");
  setText(i++,tmp);

  // author
  setText(i++,QString::fromUtf8(GWEN_PluginDescription_GetAuthor(_descr)));

  // description
  tmp=QString::fromUtf8(GWEN_PluginDescription_GetShortDescr(_descr));
  if (tmp.isEmpty())
    tmp=QWidget::tr("(unknown)");
  setText(i++, tmp);
}









QBPluginDescrListView::QBPluginDescrListView(QWidget *parent, const char *name)
:Q3ListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Name"),-1);
  addColumn(QWidget::tr("Version"),-1);
  addColumn(QWidget::tr("Author"),-1);
  addColumn(QWidget::tr("Description"),-1);
}



QBPluginDescrListView::~QBPluginDescrListView(){
}



void QBPluginDescrListView::addPluginDescr(GWEN_PLUGIN_DESCRIPTION *pd){
  QBPluginDescrListViewItem *entry;

  entry=new QBPluginDescrListViewItem(this, pd);
}



void QBPluginDescrListView::addPluginDescrs(const std::list<GWEN_PLUGIN_DESCRIPTION*> &accs){
  std::list<GWEN_PLUGIN_DESCRIPTION*>::const_iterator it;

  for (it=accs.begin(); it!=accs.end(); it++) {
    QBPluginDescrListViewItem *entry;

    entry=new QBPluginDescrListViewItem(this, *it);
  } /* for */
}



void QBPluginDescrListView::addPluginDescrs(GWEN_PLUGIN_DESCRIPTION_LIST2 *l){
  GWEN_PLUGIN_DESCRIPTION_LIST2_ITERATOR *it;

  it=GWEN_PluginDescription_List2_First(l);
  if (it) {
    GWEN_PLUGIN_DESCRIPTION *pd;

    pd=GWEN_PluginDescription_List2Iterator_Data(it);
    while(pd) {
      QBPluginDescrListViewItem *entry;

      entry=new QBPluginDescrListViewItem(this, pd);
      pd=GWEN_PluginDescription_List2Iterator_Next(it);
    } // while
    GWEN_PluginDescription_List2Iterator_free(it);
  }
}



GWEN_PLUGIN_DESCRIPTION *QBPluginDescrListView::getCurrentPluginDescr() {
  QBPluginDescrListViewItem *entry;

  entry=dynamic_cast<QBPluginDescrListViewItem*>(currentItem());
  if (!entry) {
    return 0;
  }
  return entry->getPluginDescr();
}



std::list<GWEN_PLUGIN_DESCRIPTION*>
QBPluginDescrListView::getSelectedPluginDescrs(){
  std::list<GWEN_PLUGIN_DESCRIPTION*> accs;
  QBPluginDescrListViewItem *entry;

  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<QBPluginDescrListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getPluginDescr());
    }
  } // for

  return accs;
}



























