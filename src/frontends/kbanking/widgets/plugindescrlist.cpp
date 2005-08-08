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


#include "plugindescrlist.h"
#include <assert.h>
#include <qstring.h>



PluginDescrListViewItem::PluginDescrListViewItem(PluginDescrListView *parent,
                                                 GWEN_PLUGIN_DESCRIPTION *pd)
:KListViewItem(parent)
,_descr(pd){
  assert(pd);
  _populate();
}



PluginDescrListViewItem::PluginDescrListViewItem(const PluginDescrListViewItem &item)
:KListViewItem(item)
,_descr(0){
  if (item._descr) {
    _descr=item._descr;
  }
}


PluginDescrListViewItem::PluginDescrListViewItem(PluginDescrListView *parent,
						 KListViewItem *after,
						 GWEN_PLUGIN_DESCRIPTION *pd)
:KListViewItem(parent, after)
,_descr(pd){
  assert(pd);
  _populate();
}



PluginDescrListViewItem::~PluginDescrListViewItem(){
}



GWEN_PLUGIN_DESCRIPTION *PluginDescrListViewItem::getPluginDescr(){
  return _descr;
}


void PluginDescrListViewItem::_populate() {
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









PluginDescrListView::PluginDescrListView(QWidget *parent, const char *name)
:KListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Name"),-1);
  addColumn(QWidget::tr("Version"),-1);
  addColumn(QWidget::tr("Active"),-1);
  addColumn(QWidget::tr("Author"),-1);
  addColumn(QWidget::tr("Description"),-1);
}



PluginDescrListView::~PluginDescrListView(){
}



void PluginDescrListView::addPluginDescr(GWEN_PLUGIN_DESCRIPTION *pd){
  PluginDescrListViewItem *entry;

  entry=new PluginDescrListViewItem(this, pd);
}



void PluginDescrListView::addPluginDescrs(const std::list<GWEN_PLUGIN_DESCRIPTION*> &accs){
  std::list<GWEN_PLUGIN_DESCRIPTION*>::const_iterator it;

  for (it=accs.begin(); it!=accs.end(); it++) {
    PluginDescrListViewItem *entry;

    entry=new PluginDescrListViewItem(this, *it);
  } /* for */
}



GWEN_PLUGIN_DESCRIPTION *PluginDescrListView::getCurrentPluginDescr() {
  PluginDescrListViewItem *entry;

  entry=dynamic_cast<PluginDescrListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getPluginDescr();
}



std::list<GWEN_PLUGIN_DESCRIPTION*>
PluginDescrListView::getSelectedPluginDescrs(){
  std::list<GWEN_PLUGIN_DESCRIPTION*> accs;
  PluginDescrListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<PluginDescrListViewItem*>(it.current());
      if (entry)
        accs.push_back(entry->getPluginDescr());
    }
  } // for

  return accs;
}



























