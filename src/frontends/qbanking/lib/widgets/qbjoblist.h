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

#ifndef QBANKING_JOBLIST_H
#define QBANKING_JOBLIST_H


#include <qlistview.h>
#include <aqbanking/job.h>

#include <qbanking/qbanking.h>

#include <list>


class QBJobListView;
class QBJobListViewItem;


class QBANKING_API QBJobListViewItem: public QListViewItem {
private:
  AB_JOB *_job;

  void _populate();

public:
  QBJobListViewItem(QBJobListView *parent, AB_JOB *j);
  QBJobListViewItem(QBJobListView *parent,
                    QListViewItem *after,
                    AB_JOB *j);
  QBJobListViewItem(const QBJobListViewItem &item);

  virtual ~QBJobListViewItem();

  AB_JOB *getJob();
};



class QBANKING_API QBJobListView: public QListView {
private:
public:
  QBJobListView(QWidget *parent=0, const char *name=0);
  virtual ~QBJobListView();

  void addJob(AB_JOB *j);
  void addJobs(const std::list<AB_JOB*> &js);

  AB_JOB *getCurrentJob();
  std::list<AB_JOB*> getSelectedJobs();

};




#endif /* QBANKING_JOBLIST_H */



