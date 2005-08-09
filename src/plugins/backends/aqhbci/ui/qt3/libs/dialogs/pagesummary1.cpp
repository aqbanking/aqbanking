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


#include "wizard.h"
#include "userlist.h"
#include "selectcontext.h"
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>

#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>


#include <gwenhywfar/debug.h>


bool Wizard::initSummary1Page() {
  return true;
}



bool Wizard::doSummary1Page(QWidget *p){
  _pagesDone.clear();
  setBackEnabled(serverKeysPage, false);
  setBackEnabled(createKeysPage, false);
  setBackEnabled(iniLetterPage, false);
  setBackEnabled(systemIdPage, false);
  setBackEnabled(finished1Page, false);
  setBackEnabled(finished2Page, false);

  return true;
}



bool Wizard::undoSummary1Page(QWidget *p){
  return false;
}



bool Wizard::enterSummary1Page(QWidget *p){
  QString s;

  setNextEnabled(summary1Page, true);
  s="<qt><table>";
  s+="<tr><td>";
  s+=tr("Bank Code ");
  s+="</td><td>";
  s+=bankCodeEdit->text();
  s+="</td></tr>";

  s+="<tr><td>";
  s+=tr("Server ");
  s+="</td><td>";
  s+=getServerAddr();
  s+="</td></tr>";

  s+="<tr><td>"+tr("HBCI Version")+"</td><td>";
  s+=UserListView::hbciVersionToString(AH_Customer_GetHbciVersion(_customer));
  s+="</td></tr>";

  s+="<tr><td>";
  s+=tr("User Id");
  s+="</td><td>";
  s+=userIdEdit->text();
  s+="</td></tr>";

  s+="<tr><td>";
  s+=tr("Customer Id");
  s+="</td><td>";
  s+=(customerIdEdit->text().isEmpty() ? 
      userIdEdit->text() :
      customerIdEdit->text());
  s+="</td></tr>";

  s+="<tr><td>";
  s+=tr("Description");
  s+="</td><td>";
  s+=descriptionEdit->text();
  s+="</td></tr>";

  s+="</table></qt>";
  summaryBrowser->setText(s);

  _adjustToUser(_user);

  return true;
}







