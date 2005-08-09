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



#ifndef KBANKING_KBJOBVIEW_H
#define KBANKING_KBJOBVIEW_H

#include "kbjobview.ui.h"

#include <qwidget.h>


class JobView;


#include "kbjoblist.h"
#include "kbanking.h"


class KBJobView: public KBJobViewUi {
  Q_OBJECT
public:
  KBJobView(KBanking *kb,
            QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~KBJobView();

  bool init();
  bool fini();

protected:
  //void resizeEvent(QResizeEvent *e);

private:
  KBanking *_app;
  KBJobListView *_jobList;

public slots:
  void slotQueueUpdated();
  void slotExecute();
  void slotDequeue();
};







#endif /* KBANKING_KBJOBVIEW_H */



