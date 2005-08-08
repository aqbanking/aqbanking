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


#include "kbplugindescrlist.h"
#include <assert.h>
#include <qstring.h>



KBPluginDescrListViewItem::KBPluginDescrListViewItem(KBPluginDescrListView *parent,
                                                 GWEN_PLUGIN_DESCRIPTION *pd)
:KListViewItem(parent)
,_descr(pd){
  assert(pd);
  _populate();
}



KBPluginDescrListViewItem::KBPluginDescrListViewItem(const KBPluginDescrListViewItem &item)
:KListViewItem(item)
,_descr(0){
  if (item._descr) {
    _descr=item._descr;
  }
}


KBPluginDescrListViewItem::KBPluginDescrListViewItem(KBPluginDescrListView *parent,
						 KListViewItem *after,
						 GWEN_PLUGIN_DESCRIPTION *pd)
:KListViewItem(parent, after)
,_descr(pd){
  assert(pd);
  _populate();
}



KBPluginDescrListViewItem::~KBPluginDescrListViewItem(){
}



GWEN_PLUGIN_DESCRIPTION *KBPluginDescrListViewItem::getPluginDescr(){
  return _descr;
}


void KBPluginDescrListViewItem::_populate() {
  QString tmp;
  int i;

  assert(_descr);

  i=0;

  fprintf(stderr, "Populating...\n");

  // name
  setText(i++, GWEN_PluginDescription_GetName(_descr));

  // version
  tmp=GWEN_PluginDescription_GetVersion(_descr);
  if (tmp.isEmpty())
    tmp="(unknown)";
  setText(i++,tmp);

  if (GWEN_PluginDescription_IsActive(_descr))
    setText(i++, "yes");
  else
    setText(i++, "no");

  // author
  setText(i++,GWEN_PluginDescription_GetAuthor(_descr));

  // description
  tmp=GWEN_PluginDescription_GetShortDescr(_descr);
  if (tmp.isEmpty())
    tmp="(unknown)";
  setText(i++, tmp);
}









KBPluginDescrListView::KBPluginDescrListView(QWidget *parent, const char *name)
:KListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Name"),-1);
  addColumn(QWidget::tr("Version"),-1);
  addColumn(QWidget::tr("Active"),-1);
  addColumn(QWidget::tr("Author"),-1);
  addColumn(QWidget::tr("Description"),-1);
}



KBPluginDescrListView::~KBPluginDescrListView(){
}



void KBPluginDescrListView::addPluginDescr(GWEN_PLUGIN_DESCRIPTION *pd){
  KBPluginDescrListViewItem *entry;

  entry=new KBPluginDescrListViewItem(this, pd);
}



void KBPluginDescrListView::addPluginDescrs(const std::list<GWEN_PLUGIN_DESCRIPTION*> &accs){
  std::list<GWEN_PLUGIN_DESCRIPTION*>::const_iterator it;

  for (it=accs.begin(); it!=accs.end(); it++) {
    KBPluginDescrListViewItem *entry;

    entry=new KBPluginDescrListViewItem(this, *it);
  } /* for */
}



GWEN_PLUGIN_DESCRIPTION *KBPluginDescrListView::getCurrentPluginDescr() {
  KBPluginDescrListViewItem *entry;

  entry=dynamic_cast<KBPluginDescrListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getPluginDescr();
}



std::list<GWEN_PLUGIN_DESCRIPTION*>
KBPluginDescrListView::getSelectedPluginDescrs(){
  std::list<GWEN_PLUGIN_DESCRIPTION*> accs;
  KBPluginDescrListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<KBPluginDescrListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getPluginDescr());
    }
  } // for

  return accs;
}



























