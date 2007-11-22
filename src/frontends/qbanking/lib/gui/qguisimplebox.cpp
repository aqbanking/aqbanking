/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: qbsimplebox.cpp 816 2006-01-20 20:21:36Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include <qlabel.h>
#include <qtimer.h>

#include "qguisimplebox.h"



QGuiSimpleBox::QGuiSimpleBox(uint32_t id,
                         const QString& title,
                         const QString& text,
                         QWidget* parent, const char* name, WFlags fl)
:QGuiSimpleBoxUi(parent, name, fl), _id(id) {
  if (!title.isEmpty())
    setCaption(title);
  if (!text.isEmpty())
    textWidget->setText(text);
  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



QGuiSimpleBox::~QGuiSimpleBox(){
}



uint32_t QGuiSimpleBox::getId(){
  return _id;
}



#include "qguisimplebox.moc"


