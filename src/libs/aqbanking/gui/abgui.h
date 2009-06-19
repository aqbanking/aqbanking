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


/**
 * This function creates a GWEN_GUI object which uses AqBanking's shared certificate data
 * for certificate checking.
 * AB_Banking_Init() must be called before the certificate check callback of this GWEN_GUI
 * object is called.
 */
AQBANKING_API GWEN_GUI *AB_Gui_new(AB_BANKING *ab);

/**
 * This function can be used to add certificate handling using AqBanking's shared certificate
 * data to any GWEN_GUI object.
 * It sets the callback for certificate checking.
 * Use this function if you have your own GWEN_GUI implementation but still want to use AqBanking's
 * certificate handling.
 * AB_Banking_Init() must be called before the certificate check callback of this GWEN_GUI
 * object is called.
 */
AQBANKING_API void AB_Gui_Extend(GWEN_GUI *gui, AB_BANKING *ab);

/**
 * This function unlinks the given GWEN_GUI object from AqBanking.
 * It resets the callback for certificate checking to the value it had before
 * @ref AB_Gui_Extend was called.
 */
AQBANKING_API void AB_Gui_Unextend(GWEN_GUI *gui);


#ifdef __cplusplus
}
#endif


#endif


