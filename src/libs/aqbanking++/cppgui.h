/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id: banking.h 764 2006-01-13 14:00:00Z cstim $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/** @file 
 * @short A C++ wrapper of the main aqbanking interface
 */

#ifndef AQ_GUI_CPP_H
#define AQ_GUI_CPP_H


#include <aqbanking++/bprogress.h>
#include <gwenhywfar/gui.h>
#include <list>
#include <string>


#define QBANKING_IMPORTER_FLAGS_COMPLETE_DAYS  0x00000001
#define QBANKING_IMPORTER_FLAGS_OVERWRITE_DAYS 0x00000002
#define QBANKING_IMPORTER_FLAGS_ASK_ALL_DUPES  0x00000004
#define QBANKING_IMPORTER_FLAGS_FUZZY          0x00000008


/**
 * @brief A C++ binding for the C module @ref AB_BANKING
 *
 * This class simply is a C++ binding for the C module @ref AB_BANKING.
 * It redirects C callbacks used by AB_BANKING to virtual functions in
 * this class. It als transforms some return values inconveniant for
 * C++ into STL objects (such as "list<T>").
 *
 * @ingroup G_AB_CPP_INTERFACE
 *
 * @author Martin Preuss<martin@aquamaniac.de>
 */
class AB_Gui {
  friend class AB_GuiLinker;

private:
  GWEN_GUI *_gui;

public:
  AB_Gui();
  virtual ~AB_Gui();


  GWEN_GUI *getCInterface();


  /** @name User Interaction
   *
   */
  /*@{*/
  /**
   * See @ref AB_Gui_MessageBox
   */
  virtual int messageBox(uint32_t flags,
                         const char *title,
                         const char *text,
                         const char *b1,
                         const char *b2,
			 const char *b3,
			 uint32_t guiid);

  /**
   * See @ref AB_Gui_InputBox
   */
  virtual int inputBox(uint32_t flags,
                       const char *title,
                       const char *text,
                       char *buffer,
                       int minLen,
		       int maxLen,
		       uint32_t guiid);

  /**
   * See @ref AB_Gui_ShowBox
   */
  virtual uint32_t showBox(uint32_t flags,
			   const char *title,
			   const char *text,
			   uint32_t guiid);

  /**
   * See @ref AB_Gui_HideBox
   */
  virtual void hideBox(uint32_t id);

  /**
   * See @ref AB_Gui_ProgressStart
   */
  virtual uint32_t progressStart(uint32_t flags,
				 const char *title,
				 const char *text,
				 uint64_t total,
				 uint32_t guiid);

  /**
   * See @ref AB_Gui_ProgressAdvance
   */
  virtual int progressAdvance(uint32_t id,
                              uint64_t progress);

  /**
   * See @ref AB_Gui_ProgressLog
   */
  virtual int progressLog(uint32_t id,
			  GWEN_LOGGER_LEVEL level,
			  const char *text);

  /**
   * See @ref AB_Gui_ProgressEnd
   */
  virtual int progressEnd(uint32_t id);


  /**
   * See @ref AB_Gui_Print
   */
  virtual int print(const char *docTitle,
                    const char *docType,
                    const char *descr,
		    const char *text,
		    uint32_t guiid);

};




#endif /* AQ_BANKING_CPP_H */


