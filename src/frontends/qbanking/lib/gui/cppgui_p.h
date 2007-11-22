/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking_p.h 562 2005-08-19 19:45:45Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPGUI_P_H
#define CPPGUI_P_H


#include "cppgui.h"


class CppGuiLinker {
  friend class CppGui;

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

  static int GetPassword(GWEN_GUI *gui,
			 uint32_t flags,
			 const char *token,
			 const char *title,
			 const char *text,
			 char *buffer,
			 int minLen,
			 int maxLen,
			 uint32_t guiid);

  static int SetPasswordStatus(GWEN_GUI *gui,
			       const char *token,
			       const char *pin,
			       GWEN_GUI_PASSWORD_STATUS status,
			       uint32_t guiid);

  static int CheckCert(GWEN_GUI *gui,
		       const GWEN_SSLCERTDESCR *cert,
		       GWEN_IO_LAYER *io,
		       uint32_t guiid);

  static GWENHYWFAR_CB void freeData(void *bp, void *p);
};




#endif /* CPPGUI_P_H */


