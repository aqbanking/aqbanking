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


#include "kbjobview.h"
#include "kbanking.h"
#include <aqbanking/jobgetbalance.h>
#include <aqbanking/jobgettransactions.h>

#include <qevent.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qmessagebox.h>
#include <qlayout.h>

#include <gwenhywfar/debug.h>



#define BUTTON_WIDTH 110


KBJobView::KBJobView(KBanking *kb,
                     QWidget* parent,
                     const char* name,
                     WFlags fl)
:KBJobViewUi(parent, name, fl), _app(kb) {
  assert(kb);

  // Manually create and add layout here because the .ui-generated
  // QGroupBox doesn't have one.
  jobBox->setColumnLayout(0, Qt::Vertical );
  QBoxLayout *jobBoxLayout = new QHBoxLayout( jobBox->layout() );
  jobBoxLayout->setAlignment( Qt::AlignTop );

  _jobList=new KBJobListView(jobBox, name);
  jobBoxLayout->addWidget(_jobList);

  QObject::connect(_app->flagStaff(), SIGNAL(signalQueueUpdated()),
                   this, SLOT(slotQueueUpdated()));
  QObject::connect(executeButton, SIGNAL(clicked()),
                   this, SLOT(slotExecute()));
  QObject::connect(dequeueButton, SIGNAL(clicked()),
                   this, SLOT(slotDequeue()));

}



KBJobView::~KBJobView(){
}



bool KBJobView::init(){
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

  return true;
}



bool KBJobView::fini(){
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


void KBJobView::slotQueueUpdated(){
  DBG_NOTICE(0, "Job queue updated");
  _jobList->clear();
  _jobList->addJobs(_app->getEnqueuedJobs());
}



void KBJobView::slotExecute(){
  std::list<AB_JOB*> jl;
  int rv;
  bool updated;
  AB_IMEXPORTER_CONTEXT *ctx;

  updated=false;
  jl=_app->getEnqueuedJobs();
  if (jl.size()==0) {
    QMessageBox::warning(this,
                         tr("No Jobs"),
                         tr("There are no jobs in the queue."),
                         QMessageBox::Ok,QMessageBox::NoButton);
    return;
  }

  DBG_NOTICE(0, "Executing queue");
  ctx=AB_ImExporterContext_new();
  rv=_app->executeQueue(ctx);
  if (!rv)
    _app->importContext(ctx);
  else {
    DBG_ERROR(0, "Error: %d", rv);
  }
  AB_ImExporterContext_free(ctx);

  // let App emit signals to inform account views
  _app->accountsUpdated();
}



void KBJobView::slotDequeue(){
}









