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

#ifndef AQBANKING_KDE_PLUGINDESCRLIST_H
#define AQBANKING_KDE_PLUGINDESCRLIST_H


#include <klistview.h>
#include <gwenhywfar/plugindescr.h>

#include <list>

class KBPluginDescrListView;
class KBPluginDescrListViewItem;


class KBPluginDescrListViewItem: public KListViewItem {
private:
  GWEN_PLUGIN_DESCRIPTION *_descr;

  void _populate();

public:
  KBPluginDescrListViewItem(KBPluginDescrListView *parent,
                            GWEN_PLUGIN_DESCRIPTION *pd);
  KBPluginDescrListViewItem(KBPluginDescrListView *parent,
                            KListViewItem *after,
                            GWEN_PLUGIN_DESCRIPTION *pd);
  KBPluginDescrListViewItem(const KBPluginDescrListViewItem &item);

  virtual ~KBPluginDescrListViewItem();

  GWEN_PLUGIN_DESCRIPTION *getPluginDescr();
};



class KBPluginDescrListView: public KListView {
private:
public:
  KBPluginDescrListView(QWidget *parent=0, const char *name=0);
  virtual ~KBPluginDescrListView();

  void addPluginDescr(GWEN_PLUGIN_DESCRIPTION *pd);
  void addPluginDescrs(const std::list<GWEN_PLUGIN_DESCRIPTION*> &pds);

  GWEN_PLUGIN_DESCRIPTION *getCurrentPluginDescr();
  std::list<GWEN_PLUGIN_DESCRIPTION*> getSelectedPluginDescrs();
};




#endif /* AQBANKING_KDE_PLUGINDESCRLIST_H */



