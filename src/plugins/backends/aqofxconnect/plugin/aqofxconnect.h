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


#ifndef AQOFXCONNECT_AQOFXCONNECT_H
#define AQOFXCONNECT_AQOFXCONNECT_H


#ifdef __declspec
# if BUILDING_AQOFXCONNECT_DLL
#  define AQOFXCONNECT_API __declspec (dllexport)
# else /* Not BUILDING_AQOFXCONNECT_DLL */
#  define AQOFXCONNECT_API __declspec (dllimport)
# endif /* Not BUILDING_AQOFXCONNECT_DLL */
#else
# define AQOFXCONNECT_API
#endif


#define AQOFXCONNECT_LOGDOMAIN "aqofxconnect"


#endif /* AQOFXCONNECT_AQOFXCONNECT_H */

