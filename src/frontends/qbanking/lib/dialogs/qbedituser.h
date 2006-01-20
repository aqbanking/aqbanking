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

#ifndef QBANKING_EDITUSER_H
#define QBANKING_EDITUSER_H


#include <qbanking/qbcfgtab.h>

class QBanking;


class QBEditUser: public QBCfgTab {
private:
  AB_USER *_user;

public:
  QBEditUser(QBanking *kb,
             AB_USER *u,
             QWidget* parent=0,
             const char* name=0,
             Qt::WFlags fl=0);
  ~QBEditUser();

  static bool editUser(QBanking *kb, AB_USER *u, QWidget* parent=0);

};



#endif // QBANKING_EDITUSER_H




