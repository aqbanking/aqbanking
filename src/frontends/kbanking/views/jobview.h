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


/* This file is just to make the transition easier for KMyMoney. */

#ifndef KBANKING_JOBVIEW_H
#define KBANKING_JOBVIEW_H

#include <kbanking/kbjobview.h>


class JobView: public KBJobView {
public:
  JobView(KBanking *kb,
          QWidget* parent=0, const char* name=0, WFlags fl=0);
  virtual ~JobView();

};


#endif /* KBANKING_JOBVIEW_H */



