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


class JobListView;
class JobListViewItem;


class JobListViewItem: public KListViewItem {
private:
  AB_JOB *_job;

  void _populate();

public:
  JobListViewItem(JobListView *parent, AB_JOB *j);
  JobListViewItem(JobListView *parent,
		      KListViewItem *after,
		      AB_JOB *j);
  JobListViewItem(const JobListViewItem &item);

  virtual ~JobListViewItem();

  AB_JOB *getJob();
};



class JobListView: public KListView {
private:
public:
  JobListView(QWidget *parent=0, const char *name=0);
  virtual ~JobListView();

  void addJob(AB_JOB *j);
  void addJobs(const std::list<AB_JOB*> &js);

  AB_JOB *getCurrentJob();
  std::list<AB_JOB*> getSelectedJobs();

};




#endif /* AQHBCI_KDE_JOBLIST_H */



