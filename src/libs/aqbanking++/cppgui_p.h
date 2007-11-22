/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking_p.h 562 2005-08-19 19:45:45Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQ_GUI_CPP_P_H
#define AQ_GUI_CPP_P_H


#include "cppgui.h"


class AB_GuiLinker {
  friend class AB_Gui;

  static int MessageBox(GWEN_GUI *gui,
			uint32_t flags,
			const char *title,
			const char *text,
			const char *b1,
			const char *b2,
			const char *b3,
			uint32_t guiid);
  static int InputBox(GWEN_GUI *gui,
		      uint32_t flags,
		      const char *title,
		      const char *text,
		      char *buffer,
		      int minLen,
		      int maxLen,
		      uint32_t guiid);
  static uint32_t ShowBox(GWEN_GUI *gui,
			  uint32_t flags,
			  const char *title,
			  const char *text,
			  uint32_t guiid);
  static void HideBox(GWEN_GUI *gui,uint32_t id);
  static uint32_t ProgressStart(GWEN_GUI *gui,
				uint32_t flags,
				const char *title,
				const char *text,
				uint64_t total,
				uint32_t guiid);
  static int ProgressAdvance(GWEN_GUI *gui,
			     uint32_t id,
			     uint64_t progress);
  static int ProgressLog(GWEN_GUI *gui,
			 uint32_t id,
			 GWEN_LOGGER_LEVEL level,
			 const char *text);
  static int ProgressEnd(GWEN_GUI *gui, uint32_t id);
  static int Print(GWEN_GUI *gui,
		   const char *docTitle,
		   const char *docType,
		   const char *descr,
		   const char *text,
		   uint32_t guiid);
  static void GWENHYWFAR_CB freeData(void *bp, void *p);
};




#endif /* AQ_BANKING_CPP_P_H */


