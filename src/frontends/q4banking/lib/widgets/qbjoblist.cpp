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


#include "qbjoblist.h"
#include <assert.h>
#include <Qt/qstring.h>
#include <Qt/qwidget.h>
#include <gwenhywfar/debug.h>



QBJobListViewItem::QBJobListViewItem(QBJobListView *parent,
                                 AB_JOB *j)
:Q3ListViewItem(parent)
,_job(j){
  assert(j);
  _populate();
}



QBJobListViewItem::QBJobListViewItem(const QBJobListViewItem &item)
:Q3ListViewItem(item)
,_job(0){

  if (item._job) {
    _job=item._job;
  }
}


QBJobListViewItem::QBJobListViewItem(QBJobListView *parent,
                                 Q3ListViewItem *after,
                                 AB_JOB *j)
:Q3ListViewItem(parent, after)
,_job(j){
  assert(j);
  _populate();
}



QBJobListViewItem::~QBJobListViewItem(){
}



AB_JOB *QBJobListViewItem::getJob(){
  return _job;
}


void QBJobListViewItem::_populate() {
  QString tmp;
  int i;
  AB_ACCOUNT *a;
  const char *p;

  assert(_job);

  i=0;

  a=AB_Job_GetAccount(_job);
  assert(a);

  // job id
  setText(i++, QString::number(AB_Job_GetJobId(_job)));

  // job type
  tmp=QString::fromUtf8(AB_Job_Type2LocalChar(AB_Job_GetType(_job)));
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









QBJobListView::QBJobListView(QWidget *parent, const char *name)
:Q3ListView(parent, name){
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



QBJobListView::~QBJobListView(){
}



void QBJobListView::addJob(AB_JOB *j){
  QBJobListViewItem *entry;

  entry=new QBJobListViewItem(this, j);
}



void QBJobListView::addJobs(const std::list<AB_JOB*> &js){
  std::list<AB_JOB*>::const_iterator it;

  for (it=js.begin(); it!=js.end(); it++) {
    QBJobListViewItem *entry;

    entry=new QBJobListViewItem(this, *it);
  } /* for */
}



AB_JOB *QBJobListView::getCurrentJob() {
  QBJobListViewItem *entry;

  entry=dynamic_cast<QBJobListViewItem*>(currentItem());
  if (!entry) {
    DBG_DEBUG(0,"No item selected in list.");
    return 0;
  }
  return entry->getJob();
}



std::list<AB_JOB*> QBJobListView::getSelectedJobs(){
  std::list<AB_JOB*> js;
  QBJobListViewItem *entry;

  // Create an iterator and give the listview as argument
  Q3ListViewItemIterator it(this);
  // iterate through all items of the listview
  for (;it.current();++it) {
    if (it.current()->isSelected()) {
      entry=dynamic_cast<QBJobListViewItem*>(it.current());
      if (entry)
        js.push_back(entry->getJob());
    }
  } // for

  return js;
}



























