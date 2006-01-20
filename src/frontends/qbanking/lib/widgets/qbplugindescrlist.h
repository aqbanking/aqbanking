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

#ifndef QBANKING_PLUGINDESCRLIST_H
#define QBANKING_PLUGINDESCRLIST_H


#include <qlistview.h>

#include <qbanking/qbanking.h>

#include <gwenhywfar/plugindescr.h>

#include <list>

class QBPluginDescrListView;
class QBPluginDescrListViewItem;


class QBANKING_API QBPluginDescrListViewItem: public QListViewItem {
private:
  GWEN_PLUGIN_DESCRIPTION *_descr;

  void _populate();

public:
  QBPluginDescrListViewItem(QBPluginDescrListView *parent,
                          GWEN_PLUGIN_DESCRIPTION *pd);
  QBPluginDescrListViewItem(QBPluginDescrListView *parent,
                          QListViewItem *after,
                          GWEN_PLUGIN_DESCRIPTION *pd);
  QBPluginDescrListViewItem(const QBPluginDescrListViewItem &item);

  virtual ~QBPluginDescrListViewItem();

  GWEN_PLUGIN_DESCRIPTION *getPluginDescr();
};



class QBPluginDescrListView: public QListView {
private:
public:
  QBPluginDescrListView(QWidget *parent=0, const char *name=0);
  virtual ~QBPluginDescrListView();

  void addPluginDescr(GWEN_PLUGIN_DESCRIPTION *pd);
  void addPluginDescrs(const std::list<GWEN_PLUGIN_DESCRIPTION*> &pds);
  void addPluginDescrs(GWEN_PLUGIN_DESCRIPTION_LIST2 *l);

  GWEN_PLUGIN_DESCRIPTION *getCurrentPluginDescr();
  std::list<GWEN_PLUGIN_DESCRIPTION*> getSelectedPluginDescrs();
};




#endif /* QBANKING_PLUGINDESCRLIST_H */



