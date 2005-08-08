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

#ifndef AQHBCI_KDE_JOBLIST_H
#define AQHBCI_KDE_JOBLIST_H


#include <klistview.h>
#include <aqbanking/job.h>

#include <list>


class KBJobListView;
class KBJobListViewItem;


class KBJobListViewItem: public KListViewItem {
private:
  AB_JOB *_job;

  void _populate();

public:
  KBJobListViewItem(KBJobListView *parent, AB_JOB *j);
  KBJobListViewItem(KBJobListView *parent,
		      KListViewItem *after,
		      AB_JOB *j);
  KBJobListViewItem(const KBJobListViewItem &item);

  virtual ~KBJobListViewItem();

  AB_JOB *getJob();
};



class KBJobListView: public KListView {
private:
public:
  KBJobListView(QWidget *parent=0, const char *name=0);
  virtual ~KBJobListView();

  void addJob(AB_JOB *j);
  void addJobs(const std::list<AB_JOB*> &js);

  AB_JOB *getCurrentJob();
  std::list<AB_JOB*> getSelectedJobs();

};




#endif /* AQHBCI_KDE_JOBLIST_H */



