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


#ifndef QBANKING_WAIT_CALLBACK_H
#define QBANKING_WAIT_CALLBACK_H

#include <gwenhywfar/gwenhywfarapi.h>
#include <gwenhywfar/waitcallback.h>
#include <gwenhywfar/misc.h>


/**
 * This class allows to easily use the QBWaitCallback framework from
 * within C++.
 */
class QBWaitCallback {
private:
  GWEN_WAITCALLBACK *_ctx;

  static GWEN_WAITCALLBACK *_instantiate(GWEN_WAITCALLBACK *ctx);
  static GWEN_WAITCALLBACK_RESULT _checkAbort(GWEN_WAITCALLBACK *ctx,
                                              unsigned int level);
  static void _log(GWEN_WAITCALLBACK *ctx,
                   unsigned int level,
                   GWEN_LOGGER_LEVEL logLevel,
                   const char *s);
  static void GWENHYWFAR_CB _freeData(void *bp, void *p);

protected:
  GWEN_WAITCALLBACK *cCallback();

public:
  /**
   * Returns the current progress position (as set by
   * @ref GWEN_WaitCallback_SetProgressPos).
   * This can be used by the callback context to correctly display a
   * progress bar.
   */
  GWEN_TYPE_UINT64 getProgressPos() const;

  /**
   * Returns the progress total (as set by
   * @ref GWEN_WaitCallback_SetProgressTotal).
   * This can be used by the callback context to correctly display a
   * progress bar.
   */
  GWEN_TYPE_UINT64 getProgressTotal() const;

  /**
   * Returns the time when the callback function was last called
   * (or 0 if it has never been called)
   */
  time_t lastCalled() const;

  /**
   * Returns the time when the callback context was last entered (or 0 if
   * it never has been).
   */
  time_t lastEntered() const;

  /**
   * Sets the proposed distance in milliseconds between two
   * calls to the callback. This value is never enforced by the callback
   * mechanism itself.
   */
  void setDistance(int d);

  int nestingLevel() const;

  GWEN_TYPE_UINT32 flags() const;

  /** @name Functions Called by Gwenhywfar
   *
   * These are functions internally called by Gwenhywfar.
   * You should never call one of these functions from within your own
   * code !
   *
   */
  /*@{*/

  /**
   * Uses this callback as a template to instantiate a new one.
   * For GUI callbacks this function can be used to open a window (such as
   * progress dialogs etc).
   */
  virtual QBWaitCallback *instantiate();


  /**
   * Checks whether the user wants to abort the current action.
   * This function is optional.
   * @param level If the context given to @ref GWEN_WaitCallback_Enter did
   * not exist then a new default context has been created which uses the
   * functions (like this one) of the at that time active context. For such
   * an artificially derived context the level represents the current level
   * below the context given as parameter <i>ctx</i>. So if the level is 0
   * then the given context actually <b>is</b> the currently active one.
   */
  virtual GWEN_WAITCALLBACK_RESULT checkAbort(unsigned int level);

  /**
   * Logs a message to this callback.
   * A GUI program could use this function to write the given string to an
   * open window.
   * This function is optional.
   * @param level see @ref GWEN_WAITCALLBACK_CHECKABORTFN
   * @param loglevel a higher level results in a more detailed output.
   * Loglevels are defined from 0 (the most important level) and 10 (the
   * least important level).
   * @param s log string
   */
  virtual void log(unsigned int level,
                   GWEN_LOGGER_LEVEL loglevel,
                   const char *s);

  /**
   * Constructor.
   * Please note that this callback is freed by Gwenhywfar, you should never
   * try to free this callback yourself once it has been registered.
   */
  QBWaitCallback(const char *id);
  virtual ~QBWaitCallback();

public:

  /**
   * Registers this callback with Gwenhywfar.
   * After having registered this callback its method @ref instantiate()
   * will be called by Gwenhywfar whenever a callback of this type
   * is entered via @ref GWEN_WaitCallback_Enter.
   * You must unregister this callback before exiting from the calling
   * application/library.
   */
  int registerCallback();

  /**
   * You must call this member in order to make a clean shutdown of your
   * program/library.
   */
  int unregisterCallback();

  /**
   * Returns the Id of this callback.
   */
  const char *getId() const;

  const char *getText() const;

  const char *getUnits() const;

  /*@}*/

  int getDistance() const;


};


#endif /* QBANKING_WAIT_CALLBACK_H */


