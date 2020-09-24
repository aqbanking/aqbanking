/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2020 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_INILETTER_H
#define AH_PROVIDER_INILETTER_H

#include <aqbanking/backendsupport/provider.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/buffer.h>




/** @name Generating Ini-Letters
 *
 * INI letters are used in RDH mode only. They are used to verify the public
 * server keys and to create a letter to be sent to the bank for verification
 * of the public user keys.
 */
/*@{*/
/**
 * Creates a text version of the INI letter. This function needs to mount
 * the medium so the user will be ask for the pin.
 * @param pro provider
 * @param u user for which the iniletter is to be created
 * @param useBankKey if !=0 create an iniletter for the bank key
 * @param variant use 1 for RDH1, 2 for RDH2-10 (or 0 for autoselection)
 * @param lbuf buffer to write the iniletter to
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
int AH_Provider_GetIniLetterTxt(AB_PROVIDER *pro,
                                AB_USER *u,
                                int useBankKey,
                                int variant,
                                GWEN_BUFFER *lbuf,
                                int nounmount);

/**
 * Creates a HTML version of the INI letter. This function needs to mount
 * the medium so the user will be ask for the pin.
 * @param pro provider
 * @param u user for which the iniletter is to be created
 * @param useBankKey if !=0 create an iniletter for the bank key
 * @param variant use 1 for RDH1, 2 for RDH2-10 (or 0 for autoselection)
 * @param lbuf buffer to write the iniletter to
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
int AH_Provider_GetIniLetterHtml(AB_PROVIDER *pro,
                                 AB_USER *u,
                                 int useBankKey,
                                 int variant,
                                 GWEN_BUFFER *lbuf,
                                 int nounmount);
/*@}*/




#endif

