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



#ifndef QBANKING_JOBVIEW_H
#define QBANKING_JOBVIEW_H

#include "qbjobview.ui.h"

#include <qwidget.h>


class QBJobView;


#include "qbjoblist.h"
#include "qbanking.h"


class QBANKING_API QBJobView: public QBJobViewUi {
  Q_OBJECT
public:
  QBJobView(QBanking *kb,
            QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QBJobView();

  bool init();
  bool fini();

protected:
  //void resizeEvent(QResizeEvent *e);

private:
  QBanking *_app;
  QBJobListView *_jobList;

public slots:
  void slotQueueUpdated();
  void slotExecute();
  void slotDequeue();
};







#endif /* QBANKING_JOBVIEW_H */



