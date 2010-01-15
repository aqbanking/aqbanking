/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.h 764 2006-01-13 14:00:00Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef CPPGUI_H
#define CPPGUI_H

#include <gwenhywfar/gui_be.h>
#include <list>
#include <string>

class CppGui;

#include <q4banking/api.h> /* for Q4BANKING_API */

/**
 * @brief A C++ binding for the C module @ref GWEN_GUI
 *
 * This class simply is a C++ binding for the C module @ref GWEN_GUI.
 * It redirects C callbacks used by GWEN_GUI to virtual functions in
 * this class.
 *
 * @author Martin Preuss<martin@aquamaniac.de>
 */
class Q4BANKING_API CppGui {
  friend class CppGuiLinker;

private:
  GWEN_GUI *_gui;
  GWEN_GUI_CHECKCERT_FN _checkCertFn;

  GWEN_DB_NODE *_dbPasswords;
  GWEN_DB_NODE *_dbCerts;
  std::list<std::string> _badPasswords;

  std::string _getPasswordHash(const char *token, const char *pin);

public:
  CppGui();
  virtual ~CppGui();


  GWEN_GUI *getCInterface();

  DEPRECATED GWEN_DB_NODE *getDbCerts();

  /** Takes over ownership of the DB */
  DEPRECATED void setDbCerts(GWEN_DB_NODE *db);

protected:
  /** @name User Interaction
   *
   */
  /*@{*/
  /**
   * See @ref CppGui_MessageBox
   */
  virtual int messageBox(uint32_t flags,
                         const char *title,
                         const char *text,
                         const char *b1,
                         const char *b2,
			 const char *b3,
			 uint32_t guiid);

  /**
   * See @ref CppGui_InputBox
   */
  virtual int inputBox(uint32_t flags,
                       const char *title,
                       const char *text,
                       char *buffer,
                       int minLen,
		       int maxLen,
		       uint32_t guiid);

  /**
   * See @ref CppGui_ShowBox
   */
  virtual uint32_t showBox(uint32_t flags,
			   const char *title,
			   const char *text,
			   uint32_t guiid);

  /**
   * See @ref CppGui_HideBox
   */
  virtual void hideBox(uint32_t id);

  /**
   * See @ref CppGui_ProgressStart
   */
  virtual uint32_t progressStart(uint32_t flags,
				 const char *title,
				 const char *text,
				 uint64_t total,
				 uint32_t guiid);

  /**
   * See @ref CppGui_ProgressAdvance
   */
  virtual int progressAdvance(uint32_t id,
                              uint64_t progress);

  /**
   * See @ref CppGui_ProgressLog
   */
  virtual int progressLog(uint32_t id,
			  GWEN_LOGGER_LEVEL level,
			  const char *text);

  /**
   * See @ref CppGui_ProgressEnd
   */
  virtual int progressEnd(uint32_t id);


  /**
   * See @ref CppGui_Print
   */
  virtual int print(const char *docTitle,
                    const char *docType,
                    const char *descr,
		    const char *text,
		    uint32_t guiid);

  virtual int getPassword(uint32_t flags,
			  const char *token,
			  const char *title,
			  const char *text,
			  char *buffer,
			  int minLen,
			  int maxLen,
			  uint32_t guiid);

  virtual int setPasswordStatus(const char *token,
				const char *pin,
				GWEN_GUI_PASSWORD_STATUS status,
				uint32_t guiid);

  virtual int checkCert(const GWEN_SSLCERTDESCR *cert,
			GWEN_IO_LAYER *io,
			uint32_t guiid);

  int checkCertBuiltIn(const GWEN_SSLCERTDESCR *cert,
		       GWEN_IO_LAYER *io,
		       uint32_t guiid);

  };




#endif /* CPPGUI_H */


