/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Nov 18 2001
    copyright   : (C) 2001 by Martin Preuss
    email       : openhbci@aquamaniac.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef HBCIERROR_H
#define HBCIERROR_H

/** @file error.h
 *
 * @short Definitions of HBCI::Error and its C wrapper @ref HBCI_Error. */


/** Severity Level of this error */
typedef enum ErrorLevel {
    /** no error */
    ERROR_LEVEL_NONE     = 0,
    /** unimportant error, warning */
    ERROR_LEVEL_EASY,
    /** normal error */
    ERROR_LEVEL_NORMAL,
    /** critical error */
    ERROR_LEVEL_CRITICAL,
    /** very, very critical error */
    ERROR_LEVEL_PANIC,
    /** internal error */
    ERROR_LEVEL_INTERNAL
} ErrorLevel;

/** Proposed reaction on this error */
typedef enum ErrorAdvise {
    /** Unknown, unspecified */
    ERROR_ADVISE_DONTKNOW = 0,
    /** Ignore this error */
    ERROR_ADVISE_IGNORE   = 1,
    /** Retry the last operation */
    ERROR_ADVISE_RETRY    = 2,
    /** It does not make sense to continue or retry */
    ERROR_ADVISE_ABORT    = 100,
    /** Stop the program */
    ERROR_ADVISE_SHUTDOWN = 101
} ErrorAdvise;

/** Some error codes for very specific errors */
enum ErrorCodes {
    /** Unknown, unspecified */
    HBCI_ERROR_CODE_UNKNOWN = 0,
    /** PIN was wrong (no info about retry count) */
    HBCI_ERROR_CODE_PIN_WRONG = 100,
    /** PIN wrong, no errorneous pin input left */
    HBCI_ERROR_CODE_PIN_WRONG_0,
    /** PIN wrong, one errorneous pin input left */
    HBCI_ERROR_CODE_PIN_WRONG_1,
    /** PIN wrong, two errorneous pin input left */
    HBCI_ERROR_CODE_PIN_WRONG_2,
    /** PIN dialog was aborted by user */
    HBCI_ERROR_CODE_PIN_ABORTED,
    /** The PIN entered was too short */
    HBCI_ERROR_CODE_PIN_TOO_SHORT,
    /** Wrong pin entered too often, card is unusable */
    HBCI_ERROR_CODE_CARD_DESTROYED,
    /** Medium file (keyfile) was not found */
    HBCI_ERROR_CODE_FILE_NOT_FOUND,
    /** No chip card inserted */
    HBCI_ERROR_CODE_NO_CARD,
    /** A specific HBCI job is not supported by the institute,
     * i.e. they won't do this kind of job. Either the BPD said that
     * they don't support this job, or they signalled an error when
     * the textkey was sent to them. */
    HBCI_ERROR_CODE_JOB_NOT_SUPPORTED,
    /** The socket connection failed -- this probably means that the
     * bank server currently refuses connections. */
    HBCI_ERROR_CODE_SOCKET_NO_CONNECT,
    /** Timeout during socket connect */
    HBCI_ERROR_CODE_SOCKET_ERROR_TIMEOUT,
    /** System call interrupted -- please try again */
    HBCI_ERROR_CODE_SOCKET_ERROR_INTERRUPT,
    /** general socket error */
    HBCI_ERROR_CODE_SOCKET_ERROR_UNKNOWN,
    /** wrong medium inserted */
    HBCI_ERROR_CODE_WRONG_MEDIUM,
    /** error with library loader */
    HBCI_ERROR_CODE_LIBRARY,
    /** whenever there is a double entry in any kind of list */
    HBCI_ERROR_CODE_EXISTS,
    /** whenever something does not exist */
    HBCI_ERROR_CODE_INEXISTENT,
    /** invalid argument */
    HBCI_ERROR_CODE_INVALID,
    /** general medium error */
    HBCI_ERROR_CODE_MEDIUM,
    /** method/function not supported */
    HBCI_ERROR_CODE_UNSUPPORTED,
    /**
     * this is used when MediumPlugin::mediumCheck finds an unsupported
     * medium
     */
    HBCI_ERROR_CODE_BAD_MEDIUM,
    /** user aborted an operation */
    HBCI_ERROR_CODE_USER_ABORT,
    /** a given property is not supported by the medium */
    HBCI_ERROR_CODE_UNKNOWN_PROPERTY,
    /** Medium::setProperty called with an illegal value */
    HBCI_ERROR_CODE_INVALID_VALUE,
    /** Medium::setProperty called for a readonly property */
    HBCI_ERROR_CODE_PROPERTY_READONLY,

    HBCI_ERROR_CODE_JOB_NOT_SUPPORTED_BY_LIB,
    HBCI_ERROR_CODE_JOB_NOT_SUPPORTED_BY_BANK,
    HBCI_ERROR_CODE_JOB_NOT_SUPPORTED_FOR_CUSTOMER,
    HBCI_ERROR_CODE_BAD_MESSAGE
};

#include <string>

namespace HBCI {
using namespace std;
class Error;

/**
 * @short This class is thrown when an error occurs.
 *
 * An object of this class is thrown as an exception if an error
 * occurs in OpenHBCI.
 *
 * @author Martin Preuss<openhbci@aquamaniac.de> */
class Error {
private:
    string _where;
    ErrorLevel _level;
    int _code;
    ErrorAdvise _advise;
    string _message;
    string _info;
    string _reportedFrom;

public:
    /**
     * Empty constructor, representing "no error".
     * 
     * Use this to return an error code that says "All Ok."
     * You can check if there was an error by calling @ref isOk().
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    Error();

    /** DEPRECATED. This is the old, deprecated constructor. Please
     *  use the new, more precise constructor. This one here might
     *  be removed soon.
     *
     * @author Martin Preuss<openhbci@aquamaniac.de> */
    Error(string where,
	  string message,
	  int code);

    /**
     * New constructor, taken from my other projects as they proofed to be
     * valuable.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     *
     * @param where String that describes the location of the error
     * (like "MyClass::myMethod()")
     *
     * @param level Severity level, can be one of
     * <ul>
     * <li>@ref ERROR_LEVEL_NONE: no error</li>
     * <li>ERROR_LEVEL_EASY: unimportant error, warning</li>
     * <li>ERROR_LEVEL_NORMAL: normal error</li>
     * <li>ERROR_LEVEL_CRITICAL: critical error</li>
     * <li>ERROR_LEVEL_PANIC: very, very critical error</li>
     * <li>ERROR_LEVEL_INTERNAL: internal error</li>
     * </ul>
     *
     * @param code any integer code you want to assign. Some of these
     * codes (starting from integer number 100) have been specified
     * for very specific conditions in OpenHBCI now -- see @ref
     * ErrorCodes .
     *
     * @param advise Proposed reaction to this error, can be one of
     * <ul>
     * <li>@ref ERROR_ADVISE_DONTKNOW: unknown, unspecified</li>
     * <li>ERROR_ADVISE_IGNORE: ignore this error</li>
     * <li>ERROR_ADVISE_RETRY: retry the last operation</li>
     * <li>ERROR_ADVISE_ABORT: it does not make sense to continue or retry</li>
     * <li>ERROR_ADVISE_SHUTDOWN: stop the program</li>
     * </ul>
     * @param message A short string describing the error.
     * @param info Additional information, for debugging purposes. */
    Error(string where,
	  ErrorLevel level,
	  int code,
	  ErrorAdvise advise,
          string message,
          string info="");

    /**
     * This constructor can be used to return an error which occurred
     * in a called method. If this one is used, the given string will
     * added to the reporter list of the error.
     * Example:
     * <ul>
     * <li>Method A calls method B </li>
     * <li>Method B is maybe called by multiple other Methods </li>
     * <li>Method B returns an error
     * </ul>
     * You now would have two possibilities:
     * <ul>
     * <li>return the error returned by Method B</li>
     * <li>return a new error, telling the caller of Method A that there was
     * an error in Method B</li>
     * </ul>
     * With this constructor Method A is able to return an error telling that
     * <ul>
     * <li>an error occurred</li>
     * <li>the error occurred in Method B (returning all information Method
     * B provided)</li>
     * <li>this error occurred Method B was called by Method A</li>
     * </ul>
     * So this constructor allows to build up an error chain, so that the
     * debugging person is able to follow the steps that lead to this error
     * by inspecting the "reporter" list.
     * If the error referred to is already an error, that has been reported
     * using this constructor then the new reporter is simply added to the
     * list of reporters (to form the chain).
     * @author Martin Preuss<martin@libchipcard.de>
     * @param where the location which encountered the error reported
     * @param err the error that occurred first place
     */
    Error(const string &where,
	  const Error &err);

    ~Error();

    /**
     * Tells you if this object shows an error or if all was ok.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     * @return true if there was no error, false otherwise
     */
    bool isOk() const { return _level==ERROR_LEVEL_NONE; };

    /**
     * Return the location of the error, e.g. "c_error::c_error()".
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    const string& where() const {return _where; };

    /**
     * Returns the error code. Some of these codes (starting from
     * integer number 100) have been specified for very specific
     * conditions in OpenHBCI now -- see @ref ErrorCodes.
     *
     * @see @ref ErrorCodes
     */
    int code() const { return _code; };

    /**
     * Returns the error message itself (e.g. "Bad filename").
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    const string& message() const { return _message;};

    /**
     * Returns the severity of the error.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    ErrorLevel level() const { return _level; };

    /**
     * Returns an advise upon this error (0 if no advise)
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    ErrorAdvise advise() const { return _advise; };

    /**
     * Returns additional info about the error.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    const string& info() const { return _info; };

    /**
     * Returns a short error string containing all important information.
     * @author Martin Preuss<openhbci@aquamaniac.de>
     */
    string errorString() const;


};

} /* namespace HBCI */

typedef class HBCI::Error HBCI_Error;

/* Now the typedefs and function prototypes for C. */


#endif
