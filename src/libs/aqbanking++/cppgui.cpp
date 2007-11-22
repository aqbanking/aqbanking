/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.cpp 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* don't warn about our own deprecated functions */
#define AQBANKING_NOWARN_DEPRECATED 


#include "cppgui_p.h"
#include <assert.h>

#include <aqbanking/banking.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>



GWEN_INHERIT(GWEN_GUI, AB_Gui);




int AB_GuiLinker::MessageBox(GWEN_GUI *gui,
			     uint32_t flags,
			     const char *title,
			     const char *text,
			     const char *b1,
			     const char *b2,
			     const char *b3,
			     uint32_t guiid){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->messageBox(flags, title, text, b1, b2, b3, guiid);
}



int AB_GuiLinker::InputBox(GWEN_GUI *gui,
			   uint32_t flags,
			   const char *title,
			   const char *text,
			   char *buffer,
			   int minLen,
			   int maxLen,
			   uint32_t guiid){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->inputBox(flags, title, text, buffer, minLen, maxLen, guiid);
}



uint32_t AB_GuiLinker::ShowBox(GWEN_GUI *gui,
			       uint32_t flags,
			       const char *title,
			       const char *text,
			       uint32_t guiid){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->showBox(flags, title, text, guiid);
}



void AB_GuiLinker::HideBox(GWEN_GUI *gui, uint32_t id){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->hideBox(id);
}



uint32_t AB_GuiLinker::ProgressStart(GWEN_GUI *gui,
				     uint32_t flags,
				     const char *title,
				     const char *text,
				     uint64_t total,
				     uint32_t guiid){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->progressStart(flags, title, text, total, guiid);
}



int AB_GuiLinker::ProgressAdvance(GWEN_GUI *gui,
				  uint32_t id,
				  uint64_t progress){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->progressAdvance(id, progress);
}



int AB_GuiLinker::ProgressLog(GWEN_GUI *gui,
			      uint32_t id,
			      GWEN_LOGGER_LEVEL level,
			      const char *text){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->progressLog(id, level, text);
}



int AB_GuiLinker::ProgressEnd(GWEN_GUI *gui, uint32_t id){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->progressEnd(id);
}



int AB_GuiLinker::Print(GWEN_GUI *gui,
			const char *docTitle,
			const char *docType,
			const char *descr,
			const char *text,
			uint32_t guiid){
  AB_Gui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, AB_Gui, gui);
  assert(xgui);

  return xgui->print(docTitle, docType, descr, text, guiid);
}



void AB_GuiLinker::freeData(void *bp, void *p) {
  AB_Gui *xgui;

  DBG_NOTICE(AQBANKING_LOGDOMAIN, "AB_GuiLinker: Freeing AB_Gui");
  xgui=(AB_Gui*)p;
  if (xgui->_gui) {
    xgui->_gui=0;
  }
  delete xgui;
}












AB_Gui::AB_Gui()
:_gui(NULL) {
  _gui=GWEN_Gui_new();
  GWEN_INHERIT_SETDATA(GWEN_GUI, AB_Gui,
		       _gui, this,
		       AB_GuiLinker::freeData);
  GWEN_Gui_SetMessageBoxFn(_gui, AB_GuiLinker::MessageBox);
  GWEN_Gui_SetInputBoxFn(_gui, AB_GuiLinker::InputBox);
  GWEN_Gui_SetShowBoxFn(_gui, AB_GuiLinker::ShowBox);
  GWEN_Gui_SetHideBoxFn(_gui, AB_GuiLinker::HideBox);
  GWEN_Gui_SetProgressStartFn(_gui, AB_GuiLinker::ProgressStart);
  GWEN_Gui_SetProgressAdvanceFn(_gui, AB_GuiLinker::ProgressAdvance);
  GWEN_Gui_SetProgressLogFn(_gui, AB_GuiLinker::ProgressLog);
  GWEN_Gui_SetProgressEndFn(_gui, AB_GuiLinker::ProgressEnd);
  GWEN_Gui_SetPrintFn(_gui, AB_GuiLinker::Print);
}



AB_Gui::~AB_Gui(){
  DBG_NOTICE(AQBANKING_LOGDOMAIN, "~AB_Gui: Freeing AB_Gui");
  if (_gui) {
    GWEN_INHERIT_UNLINK(GWEN_GUI, AB_Gui, _gui)
    GWEN_Gui_free(_gui);
  }
}



int AB_Gui::messageBox(uint32_t flags,
		       const char *title,
		       const char *text,
		       const char *b1,
		       const char *b2,
		       const char *b3,
		       uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_Gui::inputBox(uint32_t flags,
		     const char *title,
		     const char *text,
		     char *buffer,
		     int minLen,
		     int maxLen,
		     uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



uint32_t AB_Gui::showBox(uint32_t flags,
			 const char *title,
			 const char *text,
			 uint32_t guiid){
  return 0;
}



void AB_Gui::hideBox(uint32_t id){
}



uint32_t AB_Gui::progressStart(uint32_t flags,
			       const char *title,
			       const char *text,
			       uint64_t total,
			       uint32_t guiid){
  return 0;
}



int AB_Gui::progressAdvance(uint32_t id,
			    uint64_t progress){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_Gui::progressLog(uint32_t id,
			GWEN_LOGGER_LEVEL level,
			const char *text){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_Gui::progressEnd(uint32_t id){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int AB_Gui::print(const char *docTitle,
		  const char *docType,
		  const char *descr,
		  const char *text,
		  uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



GWEN_GUI *AB_Gui::getCInterface(){
  return _gui;
}





