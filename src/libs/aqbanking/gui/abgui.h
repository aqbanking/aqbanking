/***************************************************************************
 begin       : Thu Jun 18 2009
 copyright   : (C) 2009 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_GUI_H
#define AQBANKING_GUI_H


#include <gwenhywfar/gui_be.h>
#include <aqbanking/banking.h>

#ifdef __cplusplus
extern "C" {
#endif


AQBANKING_API GWEN_GUI *AB_Gui_new(AB_BANKING *ab);
AQBANKING_API void AB_Gui_Extend(GWEN_GUI *gui, AB_BANKING *ab);


#ifdef __cplusplus
}
#endif


#endif


