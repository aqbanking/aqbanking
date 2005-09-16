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


#include "selectcontext.h"
#include <qlistview.h>
#include <qmessagebox.h>

#include <gwenhywfar/debug.h>



SelectContext::SelectContext(AH_HBCI *hbci, AH_MEDIUM *medium)
:SelectContextUi(0,0, true)
,_hbci(hbci)
,_medium(medium)
,_index(-1)
,_country(280)
,_instCode("")
,_userId("")
,_server(""){

  QObject::connect((QObject*)okButton, SIGNAL(clicked()),
                   this,SLOT(setValues()));
  QObject::connect((QObject*)abortButton,SIGNAL(clicked()),
                   this, SLOT(reject()));
  contextList->setAllColumnsShowFocus(true);
}



SelectContext::~SelectContext(){
}



bool SelectContext::selectContext(std::string &instcode,
                                  std::string &userid,
                                  std::string &server,
                                  int &ctx) {
  int i;
  int country;
  GWEN_BUFFER *bankId;
  GWEN_BUFFER *userId;
  GWEN_BUFFER *serverAddr;
  int port;
  QListViewItem *item = 0;
  std::string mtype;
  int rv;

  // mount medium
  if (!AH_Medium_IsMounted(_medium)) {
    rv=AH_Medium_Mount(_medium);
    if (rv) {
      DBG_ERROR(0, "Error mounting (%d)", rv);
      QMessageBox::critical(this,
                            tr("Select Context"),
                            tr("Could not mount medium"),
                            tr("Dismiss"),0,0,0);
    }
  }

  bankId=GWEN_Buffer_new(0, 64, 0, 1);
  userId=GWEN_Buffer_new(0, 32, 0, 1);
  serverAddr=GWEN_Buffer_new(0, 64, 0, 1);

  for (i=0; ; i++) {
    GWEN_Buffer_Reset(bankId);
    GWEN_Buffer_Reset(userId);
    GWEN_Buffer_Reset(serverAddr);
    rv=AH_Medium_ReadContext(_medium, i,
                             &country,
                             bankId,
                             userId,
                             serverAddr,
                             &port);
    if (rv) {
      if (i==1 && item) {
	contextList->setSelected(item, true);
      }
      break;
    }
    else {
      DBG_INFO(0, "Found %d context", i);
      item=new QListViewItem(contextList,
                             QString::number(i),
                             QString::fromUtf8(GWEN_Buffer_GetStart(bankId)),
                             QString::fromUtf8(GWEN_Buffer_GetStart(userId)),
                             QString::fromUtf8(GWEN_Buffer_GetStart(serverAddr)));
    }
  } // for
  GWEN_Buffer_free(bankId);
  GWEN_Buffer_free(userId);
  GWEN_Buffer_free(serverAddr);

  if (!i) {
    DBG_ERROR(0, "No context");
    QMessageBox::critical(this,
                          tr("Select Context"),
                          tr("No user found on this medium"),
                          tr("Dismiss"),0,0,0);
    return false;
  }

  if (exec()!=Accepted)
    return false;
  instcode=_instCode;
  userid=_userId;
  server=_server;
  ctx=_index;

  return true;
}



void SelectContext::setValues(){
  QListViewItem *item = contextList->selectedItem();
  if (!item) {
    // nothing selected
    return;
  }
  _index=item->text(0).toInt();
  _instCode=std::string(item->text(1).utf8());
  _userId=std::string(item->text(2).utf8());
  _server=std::string(item->text(3).utf8());
  emit accept();
}




