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


#ifndef AQBANKING_ERROR_H
#define AQBANKING_ERROR_H

#ifdef __declspec
# if BUILDING_AQBANKING_DLL
#  define AQBANKING_API __declspec (dllexport)
# else /* Not BUILDING_AQBANKING_DLL */
#  define AQBANKING_API __declspec (dllimport)
# endif /* Not BUILDING_AQBANKING_DLL */
#else
# define AQBANKING_API
#endif

/** @defgroup AB_ERROR (Error Codes)
 * @ingroup AB_C_INTERFACE
 */
/*@{*/
#define AB_ERROR_SUCCESS           0
#define AB_ERROR_GENERIC         (-1)
#define AB_ERROR_NOT_SUPPORTED   (-2)
#define AB_ERROR_NOT_AVAILABLE   (-3)
#define AB_ERROR_BAD_CONFIG_FILE (-4)
#define AB_ERROR_INVALID         (-5)
#define AB_ERROR_NETWORK         (-6)
#define AB_ERROR_NOT_FOUND       (-7)
#define AB_ERROR_EMPTY           (-8)
#define AB_ERROR_USER_ABORT      (-9)
#define AB_ERROR_FOUND           (-10)
#define AB_ERROR_NO_DATA         (-11)
#define AB_ERROR_NOFN            (-12)
/*@}*/




#endif /* AQBANKING_ERROR_H */


