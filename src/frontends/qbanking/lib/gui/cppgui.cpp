/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.cpp 935 2006-02-14 02:11:55Z aquamaniac $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "cppgui_p.h"
#include "i18n_l.h"
#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>



GWEN_INHERIT(GWEN_GUI, CppGui);




int CppGuiLinker::MessageBox(GWEN_GUI *gui,
			     uint32_t flags,
			     const char *title,
			     const char *text,
			     const char *b1,
			     const char *b2,
			     const char *b3,
			     uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->messageBox(flags, title, text, b1, b2, b3, guiid);
}



int CppGuiLinker::InputBox(GWEN_GUI *gui,
			   uint32_t flags,
			   const char *title,
			   const char *text,
			   char *buffer,
			   int minLen,
			   int maxLen,
			   uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->inputBox(flags, title, text, buffer, minLen, maxLen, guiid);
}



uint32_t CppGuiLinker::ShowBox(GWEN_GUI *gui,
			       uint32_t flags,
			       const char *title,
			       const char *text,
			       uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->showBox(flags, title, text, guiid);
}



void CppGuiLinker::HideBox(GWEN_GUI *gui, uint32_t id){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->hideBox(id);
}



uint32_t CppGuiLinker::ProgressStart(GWEN_GUI *gui,
				     uint32_t flags,
				     const char *title,
				     const char *text,
				     uint64_t total,
				     uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->progressStart(flags, title, text, total, guiid);
}



int CppGuiLinker::ProgressAdvance(GWEN_GUI *gui,
				  uint32_t id,
				  uint64_t progress){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->progressAdvance(id, progress);
}



int CppGuiLinker::ProgressLog(GWEN_GUI *gui,
			      uint32_t id,
			      GWEN_LOGGER_LEVEL level,
			      const char *text){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->progressLog(id, level, text);
}



int CppGuiLinker::ProgressEnd(GWEN_GUI *gui, uint32_t id){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->progressEnd(id);
}



int CppGuiLinker::Print(GWEN_GUI *gui,
			const char *docTitle,
			const char *docType,
			const char *descr,
			const char *text,
			uint32_t guiid){
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->print(docTitle, docType, descr, text, guiid);
}



int CppGuiLinker::GetPassword(GWEN_GUI *gui,
			      uint32_t flags,
			      const char *token,
			      const char *title,
			      const char *text,
			      char *buffer,
			      int minLen,
			      int maxLen,
			      uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->getPassword(flags, token, title, text, buffer, minLen, maxLen, guiid);
}



int CppGuiLinker::SetPasswordStatus(GWEN_GUI *gui,
				    const char *token,
				    const char *pin,
				    GWEN_GUI_PASSWORD_STATUS status,
				    uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->setPasswordStatus(token, pin, status, guiid);
}



int CppGuiLinker::CheckCert(GWEN_GUI *gui,
			    const GWEN_SSLCERTDESCR *cert,
			    GWEN_IO_LAYER *io,
			    uint32_t guiid) {
  CppGui *xgui;

  assert(gui);
  xgui=GWEN_INHERIT_GETDATA(GWEN_GUI, CppGui, gui);
  assert(xgui);

  return xgui->checkCert(cert, io, guiid);
}



GWENHYWFAR_CB
void CppGuiLinker::freeData(void *bp, void *p) {
  CppGui *xgui;

  DBG_NOTICE(0, "CppGuiLinker: Freeing CppGui");
  xgui=(CppGui*)p;
  if (xgui->_gui) {
    xgui->_gui=0;
  }
  delete xgui;
}












CppGui::CppGui()
:_gui(NULL)
,_checkCertFn(NULL)
,_dbPasswords(NULL)
,_dbCerts(NULL){
  _gui=GWEN_Gui_new();
  _dbPasswords=GWEN_DB_Group_new("passwords");
  _dbCerts=GWEN_DB_Group_new("certs");

  GWEN_INHERIT_SETDATA(GWEN_GUI, CppGui,
		       _gui, this,
		       CppGuiLinker::freeData);
  GWEN_Gui_SetMessageBoxFn(_gui, CppGuiLinker::MessageBox);
  GWEN_Gui_SetInputBoxFn(_gui, CppGuiLinker::InputBox);
  GWEN_Gui_SetShowBoxFn(_gui, CppGuiLinker::ShowBox);
  GWEN_Gui_SetHideBoxFn(_gui, CppGuiLinker::HideBox);
  GWEN_Gui_SetProgressStartFn(_gui, CppGuiLinker::ProgressStart);
  GWEN_Gui_SetProgressAdvanceFn(_gui, CppGuiLinker::ProgressAdvance);
  GWEN_Gui_SetProgressLogFn(_gui, CppGuiLinker::ProgressLog);
  GWEN_Gui_SetProgressEndFn(_gui, CppGuiLinker::ProgressEnd);
  GWEN_Gui_SetPrintFn(_gui, CppGuiLinker::Print);
  GWEN_Gui_SetGetPasswordFn(_gui, CppGuiLinker::GetPassword);
  GWEN_Gui_SetSetPasswordStatusFn(_gui, CppGuiLinker::SetPasswordStatus);
  _checkCertFn=GWEN_Gui_SetCheckCertFn(_gui, CppGuiLinker::CheckCert);
}



CppGui::~CppGui(){
  if (_gui) {
    GWEN_INHERIT_UNLINK(GWEN_GUI, CppGui, _gui)
    GWEN_Gui_free(_gui);
  }
  GWEN_DB_Group_free(_dbCerts);
  GWEN_DB_Group_free(_dbPasswords);
}



int CppGui::messageBox(uint32_t flags,
		       const char *title,
		       const char *text,
		       const char *b1,
		       const char *b2,
		       const char *b3,
		       uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::inputBox(uint32_t flags,
		     const char *title,
		     const char *text,
		     char *buffer,
		     int minLen,
		     int maxLen,
		     uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



uint32_t CppGui::showBox(uint32_t flags,
			 const char *title,
			 const char *text,
			 uint32_t guiid){
  return 0;
}



void CppGui::hideBox(uint32_t id){
}



uint32_t CppGui::progressStart(uint32_t flags,
			       const char *title,
			       const char *text,
			       uint64_t total,
			       uint32_t guiid){
  return 0;
}



int CppGui::progressAdvance(uint32_t id, uint64_t progress){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::progressLog(uint32_t id,
			GWEN_LOGGER_LEVEL level,
			const char *text){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::progressEnd(uint32_t id){
  return GWEN_ERROR_NOT_SUPPORTED;
}



int CppGui::print(const char *docTitle,
		  const char *docType,
		  const char *descr,
		  const char *text,
		  uint32_t guiid){
  return GWEN_ERROR_NOT_SUPPORTED;
}



std::string CppGui::_getPasswordHash(const char *token, const char *pin) {
  GWEN_MDIGEST *md;
  std::string s;
  GWEN_BUFFER *buf;
  int rv;

  /* hash token and pin */
  md=GWEN_MDigest_Md5_new();
  rv=GWEN_MDigest_Begin(md);
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)token, strlen(token));
  if (rv==0)
    rv=GWEN_MDigest_Update(md, (const uint8_t*)pin, strlen(pin));
  if (rv==0)
    rv=GWEN_MDigest_End(md);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Hash error (%d)", rv);
    GWEN_MDigest_free(md);
    return "";
  }

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Text_ToHexBuffer((const char*)GWEN_MDigest_GetDigestPtr(md),
			GWEN_MDigest_GetDigestSize(md),
			buf,
			0, 0, 0);
  s=std::string(GWEN_Buffer_GetStart(buf),
		GWEN_Buffer_GetUsedBytes(buf));
  GWEN_Buffer_free(buf);

  GWEN_MDigest_free(md);
  return s;
}



int CppGui::getPassword(uint32_t flags,
			const char *token,
			const char *title,
			const char *text,
			char *buffer,
			int minLen,
			int maxLen,
			uint32_t guiid) {
  if (flags & GWEN_GUI_INPUT_FLAGS_TAN) {
    return GWEN_Gui_InputBox(flags,
			     title,
			     text,
			     buffer,
			     minLen,
			     maxLen,
			     guiid);
  }
  else {
    GWEN_BUFFER *buf;
    int rv;
    const char *s;
  
    buf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Text_EscapeToBufferTolerant(token, buf);
  
    if (!(flags & GWEN_GUI_INPUT_FLAGS_CONFIRM)) {
      s=GWEN_DB_GetCharValue(_dbPasswords,
			     GWEN_Buffer_GetStart(buf),
			     0, NULL);
      if (s) {
	int i;

	i=strlen(s);
	if (i>=minLen && i<=maxLen) {
	  memmove(buffer, s, i+1);
	  GWEN_Buffer_free(buf);
	  return 0;
	}
      }
    }
  
    for (;;) {
      rv=GWEN_Gui_InputBox(flags,
			   title,
			   text,
			   buffer,
			   minLen,
			   maxLen,
			   guiid);
      if (rv) {
	GWEN_Buffer_free(buf);
	return rv;
      }
      else {
	std::string s;
	std::list<std::string>::iterator it;
	bool isBad=false;
  
	s=_getPasswordHash(token, buffer);
	for (it=_badPasswords.begin();
	     it!=_badPasswords.end();
	     it++) {
	  if (*it==s) {
	    /* password is bad */
	    isBad=true;
	    break;
	  }
	}
    
	if (!isBad)
	  break;
	rv=GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			       GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			       GWEN_GUI_MSG_FLAGS_SEVERITY_DANGEROUS,
			       I18N("Enforce PIN"),
			       I18N(
				   "You entered the same PIN twice.\n"
				   "The PIN is marked as bad, do you want\n"
				   "to use it anyway?"
				   "<html>"
				   "<p>"
				   "You entered the same PIN twice."
				   "</p>"
				   "<p>"
				   "The PIN is marked as <b>bad</b>, "
				   "do you want to use it anyway?"
				   "</p>"
				   "</html>"),
			       I18N("Use my input"),
			       I18N("Re-enter"),
			       0,
			       guiid);
	if (rv==1) {
	  /* accept this input */
	  _badPasswords.remove(s);
	  break;
	}
      }
    }
  
    GWEN_DB_SetCharValue(_dbPasswords, GWEN_DB_FLAGS_OVERWRITE_VARS,
			 GWEN_Buffer_GetStart(buf), buffer);
    GWEN_Buffer_free(buf);
    return 0;
  }
}



int CppGui::checkCert(const GWEN_SSLCERTDESCR *cd,
		      GWEN_IO_LAYER *io,
		      uint32_t guiid) {
  const char *hash;
  const char *status;
  std::string s;
  int i;

  DBG_ERROR(0, "Checking cert");

  hash=GWEN_SslCertDescr_GetFingerPrint(cd);
  status=GWEN_SslCertDescr_GetStatusText(cd);

  s=_getPasswordHash(hash, status);

  i=GWEN_DB_GetIntValue(_dbCerts, s.c_str(), 0, -1);
  if (i==0) {
    DBG_NOTICE(0, "Automatically accepting certificate [%s]", hash);
    return 0;
  }

  i=checkCertBuiltIn(cd, io, guiid);
  if (i==0) {
    GWEN_DB_SetIntValue(_dbCerts, GWEN_DB_FLAGS_OVERWRITE_VARS,
			s.c_str(), i);
  }

  return i;
}



int CppGui::checkCertBuiltIn(const GWEN_SSLCERTDESCR *cert,
			     GWEN_IO_LAYER *io,
			     uint32_t guiid) {
  if (_checkCertFn)
    return _checkCertFn(_gui, cert, io, guiid);
  else {
    DBG_ERROR(0, "No built-in checkcert function?");
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int CppGui::setPasswordStatus(const char *token,
			      const char *pin,
			      GWEN_GUI_PASSWORD_STATUS status,
			      uint32_t guiid) {
  if (token==NULL && pin==NULL && status==GWEN_Gui_PasswordStatus_Remove) {
    GWEN_DB_ClearGroup(_dbPasswords, NULL);
  }
  else {
    std::string s;

    s=_getPasswordHash(token, pin);
    if (status==GWEN_Gui_PasswordStatus_Bad) {
      std::list<std::string>::iterator it;

      s=_getPasswordHash(token, pin);
      for (it=_badPasswords.begin();
	   it!=_badPasswords.end();
	   it++) {
	if (*it==s) {
	  /* bad password already in list */
	  return 0;
	}
      }
      _badPasswords.push_back(s);
    }
  }

  return 0;
}



GWEN_GUI *CppGui::getCInterface(){
  return _gui;
}



GWEN_DB_NODE *CppGui::getDbCerts() {
  return _dbCerts;
}



void CppGui::setDbCerts(GWEN_DB_NODE *db) {
  GWEN_DB_Group_free(_dbCerts);
  _dbCerts=db;
}








