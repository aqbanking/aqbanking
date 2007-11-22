/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.cpp 1106 2007-01-09 21:14:59Z martin $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "bprogress.h"



AB_Progress::AB_Progress(uint32_t id,  const char *title,
			 uint32_t flags, uint64_t total)
:_id(id)
,_flags(flags)
,_finished(false)
,_total(total)
,_startTime(0)
,_lastTime(0) {
  _startTime=time(0);
  if (title)
    _title=std::string(title);
}



AB_Progress::~AB_Progress() {
}







