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

#include <qbanking/qbflagstaff.h>


class KBFlagStaff: public QBFlagStaff {
  Q_OBJECT
public:
  KBFlagStaff();
  virtual ~KBFlagStaff();
};




#endif /* AQHBCI_KDE_FLAGSTAFF_H */

