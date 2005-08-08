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

#ifndef QBANKMANAGER_PICKSTARTDATE_H
#define QBANKMANAGER_PICKSTARTDATE_H

#include "pickstartdate.ui.h"
#include <qdatetime.h>


class PickStartDate: public PickStartDateUi {
  Q_OBJECT
private:
  const QDate &_firstPossible;
  const QDate &_lastUpdate;
public:
  PickStartDate(const QDate &firstPossible,
		const QDate &lastUpdate,
                int defaultChoice,
		QWidget* parent=0, const char* name=0,
		bool modal=FALSE, WFlags fl=0);
  ~PickStartDate();

  QDate getDate();

public slots:
  void slotNoDateToggled(bool on);
  void slotLastUpdateToggled(bool on);
  void slotFirstDateToggled(bool on);
  void slotPickDateToggled(bool on);
};




#endif
