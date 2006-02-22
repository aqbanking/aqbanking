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


#include "a_wait.h"
#include "wizard.h"
#include <qbanking/qbanking.h>

#include <gwenhywfar/debug.h>

#include <assert.h>

#include <qlabel.h>



ActionWait::ActionWait(Wizard *w)
:WizardAction(w, "Wait", QWidget::tr("Wait")) {
  QLabel *tl;

  tl=new QLabel(this, "WaitText");
  tl->setText("<qt>"
              "<p>"
              "The new user has now been setup partly."
              "</p>"
              "<p>"
              "You will now have to wait for the bank to acknowledge "
              "the registration and to complete your application."
              "</p>"
              "<p>"
              "In the next days your bank will inform you about the success "
              "of the application. You can then finish the setup of this "
              "user."
              "</p>"
              "</qt>");
  addWidget(tl);
}



ActionWait::~ActionWait() {
}



