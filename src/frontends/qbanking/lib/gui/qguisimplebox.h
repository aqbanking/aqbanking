/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbsimplebox.h 809 2006-01-20 14:15:15Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QGUI_SIMPLEBOX_H
#define QGUI_SIMPLEBOX_H


#include "qguisimplebox.ui.h"
#include <gwenhywfar/types.h>



class QGuiSimpleBox: public QGuiSimpleBoxUi {
  Q_OBJECT
private:
  uint32_t _id;

public:
  QGuiSimpleBox(uint32_t id,
              const QString& title,
              const QString& text,
              QWidget* parent=0, const char* name=0, WFlags fl=0);
  ~QGuiSimpleBox();

  uint32_t getId();
};







#endif /* QGUI_SIMPLEBOX_H */

