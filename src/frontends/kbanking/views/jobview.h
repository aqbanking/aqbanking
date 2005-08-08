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



#ifndef KBANKING_JOBVIEW_H
#define KBANKING_JOBVIEW_H

#include "jobview.ui.h"

#include <qwidget.h>


class JobView;


#include "joblist.h"
#include "kbanking.h"


class JobView: public JobViewUi {
  Q_OBJECT
public:
  JobView(KBanking *kb,
          QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~JobView();

  bool init();
  bool fini();

protected:
  //void resizeEvent(QResizeEvent *e);

private:
  KBanking *_app;
  JobListView *_jobList;

public slots:
  void slotQueueUpdated();
  void slotExecute();
  void slotDequeue();
};







#endif /* KBANKING_JOBVIEW_H */



