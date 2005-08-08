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


#include "kbjoblist.h"
#include <assert.h>
#include <qstring.h>
#include <qwidget.h>



KBJobListViewItem::KBJobListViewItem(KBJobListView *parent,
                                 AB_JOB *j)
:KListViewItem(parent)
,_job(j){
  assert(j);
  _populate();
}



KBJobListViewItem::KBJobListViewItem(const KBJobListViewItem &item)
:KListViewItem(item)
,_job(0){

  if (item._job) {
    _job=item._job;
  }
}


KBJobListViewItem::KBJobListViewItem(KBJobListView *parent,
                                 KListViewItem *after,
                                 AB_JOB *j)
:KListViewItem(parent, after)
,_job(j){
  assert(j);
  _populate();
}



KBJobListViewItem::~KBJobListViewItem(){
}



AB_JOB *KBJobListViewItem::getJob(){
  return _job;
}


void KBJobListViewItem::_populate() {
  QString tmp;
  int i;
  AB_ACCOUNT *a;
  const char *p;

  assert(_job);

  i=0;

  fprintf(stderr, "Populating...\n");

  a=AB_Job_GetAccount(_job);
  assert(a);

  // job id
  setText(i++, QString::number(AB_Job_GetJobId(_job)));

  // job type
  switch(AB_Job_GetType(_job)) {
  case AB_Job_TypeGetBalance:
    tmp=QWidget::tr("Get Balance");
    break;
  case AB_Job_TypeGetTransactions:
    tmp=QWidget::tr("Get Transactions");
    break;
  case AB_Job_TypeTransfer:
    tmp=QWidget::tr("Transfer");
    break;
  case AB_Job_TypeDebitNote:
    tmp=QWidget::tr("Debit Note");
    break;
  default:
    tmp=QWidget::tr("(unknown)");
    break;
  }
  setText(i++, tmp);

  // bank name
  tmp=AB_Account_GetBankName(a);
  if (tmp.isEmpty())
    tmp=AB_Account_GetBankCode(a);
  if (tmp.isEmpty())
    tmp=QWidget::tr("(unknown)");
  setText(i++,tmp);

  // account name
  tmp=AB_Account_GetAccountName(a);
  if (tmp.isEmpty())
    tmp=AB_Account_GetAccountNumber(a);
  if (tmp.isEmpty())
    tmp=QWidget::tr("(unknown)");
  setText(i++,tmp);

  // status
  switch(AB_Job_GetStatus(_job)) {
  case AB_Job_StatusNew:
    tmp=QWidget::tr("new");
    break;
  case AB_Job_StatusUpdated:
    tmp=QWidget::tr("updated");
    break;
  case AB_Job_StatusEnqueued:
    tmp=QWidget::tr("enqueued");
    break;
  case AB_Job_StatusSent:
    tmp=QWidget::tr("sent");
    break;
  case AB_Job_StatusPending:
    tmp=QWidget::tr("pending");
    break;
  case AB_Job_StatusFinished:
    tmp=QWidget::tr("finished");
    break;
  case AB_Job_StatusError:
    tmp=QWidget::tr("error");
    break;
  default:
    tmp=QWidget::tr("(unknown)");
    break;
  }
  setText(i++, tmp);

  p=AB_Provider_GetName(AB_Account_GetProvider(a));
  if (!p)
    tmp="(unknown)";
  else
    tmp=p;
  setText(i++, tmp);

  p=AB_Job_GetCreatedBy(_job);
  if (!p)
    tmp="(unknown)";
  else
    tmp=p;
  setText(i++, tmp);
}









KBJobListView::KBJobListView(QWidget *parent, const char *name)
:KListView(parent, name){
  setAllColumnsShowFocus(true);
  setShowSortIndicator(true);
  addColumn(QWidget::tr("Job Id"),-1);
  addColumn(QWidget::tr("Job Type"),-1);
  addColumn(QWidget::tr("Institute"),-1);
  addColumn(QWidget::tr("Account"),-1);
  addColumn(QWidget::tr("Status"),-1);
  addColumn(QWidget::tr("Backend"),-1);
  addColumn(QWidget::tr("Application"),-1);
}



KBJobListView::~KBJobListView(){
}



void KBJobListView::addJob(AB_JOB *j){
  KBJobListViewItem *entry;

  entry=new KBJobListViewItem(this, j);
}



void KBJobListView::addJobs(const std::list<AB_JOB*> &js){
  std::list<AB_JOB*>::const_iterator it;

  fprintf(stderr, "Adding jobs...\n");
  for (it=js.begin(); it!=js.end(); it++) {
    KBJobListViewItem *entry;

    fprintf(stderr, "Adding job...\n");
    entry=new KBJobListViewItem(this, *it);
  } /* for */
}



AB_JOB *KBJobListView::getCurrentJob() {
  KBJobListViewItem *entry;

  entry=dynamic_cast<KBJobListViewItem*>(currentItem());
  if (!entry) {
    fprintf(stderr,"No item selected in list.\n");
    return 0;
  }
  return entry->getJob();
}



std::list<AB_JOB*> KBJobListView::getSelectedJobs(){
  std::list<AB_JOB*> js;
  KBJobListViewItem *entry;

  // Create an iterator and give the listview as argument
  QListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<KBJobListViewItem*>(it.current());
      if (entry)
        js.push_back(entry->getJob());
    }
  } // for

  return js;
}



























