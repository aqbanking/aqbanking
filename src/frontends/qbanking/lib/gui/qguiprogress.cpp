/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbprogress.h 809 2006-01-20 14:15:15Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "qguiprogress.h"



QGuiProgress::QGuiProgress(uint32_t id, const char *title,
			   uint32_t flags, uint64_t total)
:_id(id)
,_flags(flags)
,_finished(false)
,_isVisible(false)
,_total(total)
,_current(0)
,_startTime(0)
,_lastTime(0)
,_widget(NULL)
,_lastPos(0)
{
  _startTime=time(0);
  if (title)
    _title=QString::fromUtf8(title);
}



QGuiProgress::~QGuiProgress() {
}



