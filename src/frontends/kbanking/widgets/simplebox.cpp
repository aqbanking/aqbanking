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


#include <qlabel.h>
#include <qtimer.h>

#include "simplebox.h"



KBSimpleBox::KBSimpleBox(GWEN_TYPE_UINT32 id,
                         const QString& title,
                         const QString& text,
                         QWidget* parent, const char* name, WFlags fl)
:KBSimpleBoxUi(parent, name, fl), _id(id) {
  if (!title.isEmpty())
    setCaption(title);
  if (!text.isEmpty())
    textWidget->setText(text);
  QTimer::singleShot(0, this, SLOT(adjustSize()));
}



KBSimpleBox::~KBSimpleBox(){
}



GWEN_TYPE_UINT32 KBSimpleBox::getId(){
  return _id;
}






