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

#ifndef AQBANKING_KDE_SETTINGS_H
#define AQBANKING_KDE_SETTINGS_H



#include <kbanking/kbanking.h>
#include <qbanking/qbcfgtabsettings.h>



class KBankingSettings: public QBCfgTabSettings {
private:
public:
  KBankingSettings(KBanking *ab,
                   QWidget* parent = 0,
                   const char* name = 0, WFlags fl = 0);
  ~KBankingSettings();

  int init();
  int fini();
};










#endif /* AQBANKING_KDE_SETTINGS_H */
