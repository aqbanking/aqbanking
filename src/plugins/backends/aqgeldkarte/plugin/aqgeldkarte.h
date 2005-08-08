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


#ifndef AQGELDKARTE_AQGELDKARTE_H
#define AQGELDKARTE_AQGELDKARTE_H


#ifdef __declspec
# if BUILDING_AQGELDKARTE_DLL
#  define AQGELDKARTE_API __declspec (dllexport)
# else /* Not BUILDING_AQGELDKARTE_DLL */
#  define AQGELDKARTE_API __declspec (dllimport)
# endif /* Not BUILDING_AQGELDKARTE_DLL */
#else
# define AQGELDKARTE_API
#endif


#define AQGELDKARTE_LOGDOMAIN "aqgeldkarte"


#endif /* AQGELDKARTE_AQGELDKARTE_H */

