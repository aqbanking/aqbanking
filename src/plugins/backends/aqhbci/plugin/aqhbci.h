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


#ifndef AQHBCI_AQHBCI_H
#define AQHBCI_AQHBCI_H


#ifdef __declspec
# if BUILDING_AQHBCI_DLL
#  define AQHBCI_API __declspec (dllexport)
# else /* Not BUILDING_AQHBCI_DLL */
#  define AQHBCI_API __declspec (dllimport)
# endif /* Not BUILDING_AQHBCI_DLL */
#else
# define AQHBCI_API
#endif


#define AQHBCI_LOGDOMAIN "aqhbci"


#endif /* AQHBCI_AQHBCI_H */

