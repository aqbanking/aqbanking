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


#include "jobview.h"

#include <gwenhywfar/debug.h>



JobView::JobView(KBanking *kb,
                 QWidget* parent,
                 const char* name,
                 WFlags fl)
:KBJobView(kb, parent, name, fl) {
  DBG_WARN(0, "Class JobView is deprecated, please use KBJobView instead");
}



JobView::~JobView(){
}



