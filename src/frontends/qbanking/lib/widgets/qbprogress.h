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

#ifndef QBANKING_PROGRESS_H
#define QBANKING_PROGRESS_H


#include "qbprogress.ui.h"
#include <gwenhywfar/types.h>
#include <aqbanking/banking.h>

#include <time.h>


#define QBPROGRESS_SHOWTIMEOUT 5


class QString;


class QBProgress: public QBProgressUI {
  Q_OBJECT

public:
  typedef enum {
    ProgressTypeNormal=0,
    ProgressTypeSimple,
    ProgressTypeFast
  } ProgressType;

private:
  GWEN_TYPE_UINT32 _id;
  ProgressType _progressType;
  bool _aborted;
  bool _closed;
  bool _doShowText;
  bool _shouldStay;
  GWEN_TYPE_UINT32 _total;
  GWEN_TYPE_UINT32 _lastProgress;
  QString _logtext;
  QString _units;
  time_t _startTime;
  time_t _lastTime;

  static int _openCount;

  bool _handleTime();

protected:
  virtual void closeEvent(QCloseEvent *e);

public:
  QBProgress(GWEN_TYPE_UINT32 id,
             ProgressType pt,
             const QString& title,
             const QString& text,
             const QString& units,
             QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QBProgress();

  int setTotalPos(GWEN_TYPE_UINT32 total);

  int start(GWEN_TYPE_UINT32 total);
  int advance(GWEN_TYPE_UINT32 progress);
  int log(AB_BANKING_LOGLEVEL level,
          const QString& text);
  int end();

  GWEN_TYPE_UINT32 getId();
  bool isClosed();

  bool shouldStay();

  void setProgressText(const QString &s);
  void setProgressUnits(const QString &s);

  virtual void show();

protected slots:
  void abort();
};







#endif /* QBANKING_PROGRESS_H */




