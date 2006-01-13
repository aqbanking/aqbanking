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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "settings.h"

#include <gwenhywfar/debug.h>


KBankingSettings::KBankingSettings(KBanking *ab,
                                   QWidget* parent,
                                   const char* name,
                                   WFlags fl)
:QBCfgTabSettings(ab, parent, name, fl) {
  addUsersPage();
  addAccountsPage();
  addBackendsPage();
}


KBankingSettings::~KBankingSettings() {
}



int KBankingSettings::init() {
  if (!toGui()) {
    DBG_ERROR(0, "Could not init dialog");
    return -1;
  }
  return 0;
}



int KBankingSettings::fini() {
  if (!fromGui())
    return -1;
  return 0;
}














