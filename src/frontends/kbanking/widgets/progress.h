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

#ifndef AQBANKING_KDE_PROGRESS_H
#define AQBANKING_KDE_PROGRESS_H


#include "progress.ui.h"
#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include <time.h>


class QString;


class KBProgress: public KBProgressUI {
  Q_OBJECT
private:
  GWEN_TYPE_UINT32 _id;
  bool _aborted;
  bool _closed;
  QString _logtext;
  time_t _startTime;
  time_t _lastTime;

  void _handleTime();

protected:
  virtual void closeEvent(QCloseEvent *e);

public:
  KBProgress(GWEN_TYPE_UINT32 id,
             const QString& title,
             const QString& text,
             QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~KBProgress();

  int start(GWEN_TYPE_UINT32 total);
  int advance(GWEN_TYPE_UINT32 progress);
  int log(AB_BANKING_LOGLEVEL level,
          const QString& text);
  int end();

  GWEN_TYPE_UINT32 getId();
  bool isClosed();

protected slots:
  void abort();
};







#endif /* AQBANKING_KDE_PROGRESS_H */




