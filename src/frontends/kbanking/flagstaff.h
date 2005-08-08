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

#ifndef AQHBCI_KDE_FLAGSTAFF_H
#define AQHBCI_KDE_FLAGSTAFF_H

#include <qobject.h>


class FlagStaff: public QObject {
  Q_OBJECT
public:
  FlagStaff();
  virtual ~FlagStaff();

  void queueUpdated();
  void accountsUpdated();
  void outboxCountChanged(int count);
  void statusMessage(const QString &s);

signals:
  void signalQueueUpdated();
  void signalAccountsUpdated();
  void signalOutboxCountChanged(int count);
  void signalStatusMessage(const QString &s);
};




#endif /* AQHBCI_KDE_FLAGSTAFF_H */

