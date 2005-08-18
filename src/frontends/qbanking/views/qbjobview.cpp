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


#include "qbjobview.h"
#include "qbanking.h"
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <qevent.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <gwenhywfar/debug.h>



#define BUTTON_WIDTH 110


QBJobView::QBJobView(QBanking *kb,
                 QWidget* parent,
                 const char* name,
                 WFlags fl)
:QBJobViewUi(parent, name, fl), _app(kb) {
  assert(kb);

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  jobBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *jobBoxLayout = new QHBoxLayout( jobBox->layout() );
  jobBoxLayout->setAlignment( Qt::AlignTop );

  _jobList=new QBJobListView((QWidget*)jobBox, name);
  jobBoxLayout->addWidget(_jobList);

  QObject::connect((QObject*)_app->flagStaff(), SIGNAL(signalQueueUpdated()),
                   this, SLOT(slotQueueUpdated()));
  QObject::connect((QObject*)executeButton, SIGNAL(clicked()),
                   this, SLOT(slotExecute()));
  QObject::connect((QObject*)dequeueButton, SIGNAL(clicked()),
                   this, SLOT(slotDequeue()));

}



QBJobView::~QBJobView(){
}


bool QBJobView::init(){
  GWEN_DB_NODE *db;

  db=_app->getAppData();
  assert(db);
  db=GWEN_DB_GetGroup(db, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                      "gui/views/jobview");
  if (db) {
    int i, j;

    /* found settings */
    for (i=0; i<_jobList->columns(); i++) {
      _jobList->setColumnWidthMode(i, QListView::Manual);
      j=GWEN_DB_GetIntValue(db, "columns", i, -1);
      if (j!=-1)
        _jobList->setColumnWidth(i, j);
    } /* for */
  } /* if settings */

  _jobList->addJobs(_app->getEnqueuedJobs());
  _app->outboxCountChanged(_jobList->childCount());

  return true;
}



bool QBJobView::fini(){
  GWEN_DB_NODE *db;
  int i, j;

  db=_app->getAppData();
  assert(db);
  assert(db);
  GWEN_DB_ClearGroup(db, "gui/views/jobview");
  for (i=0; i<_jobList->columns(); i++) {
    j=_jobList->columnWidth(i);
    GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_DEFAULT,
                        "gui/views/jobview/columns", j);
  } /* for */

  return true;
}


void QBJobView::slotQueueUpdated(){
  DBG_DEBUG(0, "Job queue updated");
  _jobList->clear();
  _jobList->addJobs(_app->getEnqueuedJobs());
  _app->outboxCountChanged(_jobList->childCount());
}



void QBJobView::slotExecute(){
  std::list<AB_JOB*> jl;
  std::list<AB_JOB*>::iterator jit;
  int rv;
  bool updated;
  AB_IMEXPORTER_CONTEXT *ctx;

  updated=false;
  jl=_app->getEnqueuedJobs();
  if (jl.size()==0) {
    QMessageBox::warning(0,
                         tr("No Jobs"),
                         tr("There are no jobs in the queue."),
                         tr("Dismiss"), 0, 0, 0);
    return;
  }

  DBG_INFO(0, "Executing queue");
  rv=_app->executeQueue();
  if (rv) {
    DBG_NOTICE(0, "Error %d", rv);
  }

  /* handle results of all jobs */
  ctx=AB_ImExporterContext_new();
  rv=AB_Banking_GatherResponses(_app->getCInterface(),
                                ctx);
  if (!rv) {
    _app->importContext(ctx,
                        QBANKING_IMPORTER_FLAGS_COMPLETE_DAYS /*|
                        QBANKING_IMPORTER_FLAGS_OVERWRITE_DAYS */);
  }
  AB_ImExporterContext_free(ctx);
}



void QBJobView::slotDequeue(){
  std::list<AB_JOB*> jl;
  std::list<AB_JOB*>::iterator jit;

  jl=_jobList->getSelectedJobs();
  if (jl.empty()) {
    DBG_DEBUG(0, "No job selected");
    QMessageBox::warning(0,
			 tr("No Selection"),
                         tr("Please select a job first."),
			 tr("Dismiss"), 0, 0, 0);
    return;
  }

  if (QMessageBox::warning(0,
			   tr("Delete job"),
			   tr("Do you really want to delete the "
			      "selected job(s)?"),
			   tr("Yes"), tr("No"), 0, 0)!=0)
    return;

  for (jit=jl.begin(); jit!=jl.end(); jit++) {
    int rv;

    rv=AB_Banking_DequeueJob(_app->getCInterface(), *jit);
    if (rv) {
      DBG_ERROR(0, "Error dequeing job (%d)", rv);
    }
  } // for
  slotQueueUpdated();
}









