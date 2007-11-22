/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbprogress.h 809 2006-01-20 14:15:15Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QBANKING_PROGRESSWIDGET_H
#define QBANKING_PROGRESSWIDGET_H


#include "qguiprogresswidget.ui.h"
#include <gwenhywfar/types.h>
#include <gwenhywfar/gui.h>

#include <time.h>


#define QGUI_PROGRESS_SHOWTIMEOUT 5

#define QGUI_PROGRESS_FLAGS_SHOW_TEXT 0x00000001


class QGuiProgress;
class QString;


class QGuiProgressWidget: public QGuiProgressWidgetUI {
  Q_OBJECT
private:
  bool _aborted;
  bool _shouldStay;
  bool _doShowText;
  QString _logtext;
  time_t _startTime;
  time_t _lastTime;
  QGuiProgress *_currentSubProgress;

  std::list<QGuiProgress*> _progressPtrList;

  static int _openCount;

  bool _handleTime();
  void _selectSublevel();

protected:
  virtual void closeEvent(QCloseEvent *e);

public:
  QGuiProgressWidget(QGuiProgress *firstProgress,
		     const QString &title,
		     const QString &text,
		     QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QGuiProgressWidget();

  void addProgress(QGuiProgress *pr);
  void delProgress(QGuiProgress *pr);

  void check();

  int log(GWEN_LOGGER_LEVEL level,
	  const QString& text);

  int checkAbort();

  bool aborted() const { return _aborted;};
  bool shouldStay() const { return _shouldStay;};
  bool hasProgresses() const;

protected slots:
  void abort();
};







#endif /* QBANKING_PROGRESS_H */




