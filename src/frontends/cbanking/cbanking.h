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


#ifndef CBANKING_CBANKING_H
#define CBANKING_CBANKING_H


#include <aqbanking/banking.h>

/** @addtogroup G_AB_CBANKING
 *
 * This is a console backend to be used by command line tools (like
 * AqBanking-Tool and AqHBCI-Tool).
 */
/*@{*/

/**
 * Constructor (see @ref AB_Banking_new).
 */
AB_BANKING *CBanking_new(const char *appName,
                         const char *fname);


/** @name Character Set
 *
 * <p>
 * AqBanking internally uses UTF8 in all functions. Not all systems use this
 * for user interaction (e.g. not all linux consoles show UTF-8 data, some
 * systems might still use ISO-8859-1 or 15.
 * </p>
 * <p>
 * Therefore this frontend transforms interactive data (i.e. data that is
 * output to the console or input from it) between UTF-8 and a given character
 * set.
 * </p>
 *
 */
/*@{*/

/**
 * Returns the currently selected character set (or NULL if none is selected).
 */
const char *CBanking_GetCharSet(const AB_BANKING *ab);

/**
 * Set the character set to transform from/to.
 */
void CBanking_SetCharSet(AB_BANKING *ab, const char *s);
/*@}*/


/** @name Non-Interaction
 *
 * AqBanking is prepared to be used by non-interactive tools as well.
 * However, sometimes PINs must be entered or message boxes are to be
 * answered, so this frontend needs to know whether it is used interactively.
 *
 * In non-interactive mode this frontend ignores all message box which are not
 * important. It returns an ABORT indicator on interactive functions (e.g.
 * AB_ERROR_USER_ABORT) when it encounters a message flagged as dangerous
 * (see @ref AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS).
 */
/*@{*/
/**
 * This function sets the PIN db. For every pin there is a variable (with the
 * name of the pin) with a single value: The pin.
 * The caller is free to decide how this DB is to be read (AqBanking-Tool
 * reads it from a file).
 * Takes over the DB.
 */
void CBanking_SetPinDb(AB_BANKING *ab, GWEN_DB_NODE *dbPins);


/**
 * @returns 0 if in interactive mode, !=0 otherwise
 */
int CBanking_GetIsNonInteractive(const AB_BANKING *ab);

/**
 * Tell this frontend whether it is to be used interactively.
 */
void CBanking_SetIsNonInteractive(AB_BANKING *ab, int i);
/*@}*/

/*@}*/ /* addtogroup */

#endif

