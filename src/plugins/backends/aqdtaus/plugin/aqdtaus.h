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


#ifndef AQDTAUS_AQDTAUS_H
#define AQDTAUS_AQDTAUS_H


#ifdef __declspec
# if BUILDING_AQDTAUS_DLL
#  define AQDTAUS_API __declspec (dllexport)
# else /* Not BUILDING_AQDTAUS_DLL */
#  define AQDTAUS_API __declspec (dllimport)
# endif /* Not BUILDING_AQDTAUS_DLL */
#else
# define AQDTAUS_API
#endif


#define AQDTAUS_LOGDOMAIN "aqdtaus"


#endif /* AQDTAUS_AQDTAUS_H */

