/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef QBANKING_QBGUI_H
#define QBANKING_QBGUI_H


#include <q4banking/qbanking.h>
#include <gwen-gui-qt4/qt4_gui.hpp>

#include <gwenhywfar/gui.h>



class Q4BANKING_API QBGui: public QT4_Gui {
private:
  QBanking *_qbanking;

  static int _extractHTML(const char *text, GWEN_BUFFER *buf);

public:
  QBGui(QBanking *qb);
  virtual ~QBGui();

  /**
   * See @ref AB_Gui_Print
   */
  virtual int print(const char *docTitle,
                    const char *docType,
                    const char *descr,
		    const char *text,
		    uint32_t guiid);

};



#endif

