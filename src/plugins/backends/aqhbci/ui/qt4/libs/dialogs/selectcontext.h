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


#ifndef AQHBCI_WIZARD_SELCTX_H
#define AQHBCI_WIZARD_SELCTX_H


#include "selectcontext.ui.h"

#include <aqhbci/hbci.h>
#include <aqhbci/medium.h>

#include <string>



/**
 *
 */
class SelectContext: public SelectContextUi {
    Q_OBJECT
private:
  AH_HBCI *_hbci;
  AH_MEDIUM *_medium;

  int _index;
  int _country;
  std::string _instCode;
  std::string _userId;
  std::string _server;

public:
  SelectContext(AH_HBCI *hbci, AH_MEDIUM *medium);
  virtual ~SelectContext();

  bool selectContext(std::string &instcode,
                     std::string &userid,
                     std::string &server,
                     int &ctx);

  public slots:
    void setValues();
};



#endif



