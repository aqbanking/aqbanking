/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_PROVIDER_H
#define AH_PROVIDER_H


#include <aqbanking/banking.h>
#include <aqbanking/backendsupport/provider_be.h>
#include <aqbanking/backendsupport/user.h>

#include <gwenhywfar/ct.h>


/** @defgroup G_AB_BE_AQHBCI HBCI Backend (AqHBCI)
 *  @ingroup G_AB_BACKENDS
 *
 * AqHBCI supports the German HBCI (Homebanking Computer Interface) protocol
 * for online banking. It currently supports version 2.01, 2.10 and 2.20 of
 * the HBCI specification.
 */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * This enum can be used as argument to @ref AB_Provider_GetNewUserDialog
 */
enum AQHBCI_NEWUSER_DIALOG_CODE {
  AqHBCI_NewUserDialog_CodeGeneric=0,
  AqHBCI_NewUserDialog_CodeExistingPinTan,
  AqHBCI_NewUserDialog_CodeCreateKeyFile,
  AqHBCI_NewUserDialog_CodeExistingKeyFile,
  AqHBCI_NewUserDialog_CodeCreateChipcard,
  AqHBCI_NewUserDialog_CodeExistingChipcard
};


AB_PROVIDER *AH_Provider_new(AB_BANKING *ab, const char *name);


/** @name Informative Functions
 *
 */
/*@{*/
const char *AH_Provider_GetProductName(const AB_PROVIDER *pro);

const char *AH_Provider_GetProductVersion(const AB_PROVIDER *pro);

/*@}*/



/**
 * Creates user keys for RDH type users.
 *
 * @param pro provider
 * @param u user for which the keys are to be created
 * @param nounmount if !=0 the CryptToken will not be unmounted after use
 */
int AH_Provider_CreateKeys(AB_PROVIDER *pro, AB_USER *u, int nounmount);


int AH_Provider_Test(AB_PROVIDER *pro);


/*@}*/



#ifdef __cplusplus
}
#endif



/*@}*/ /* defgroup */



#endif /* AH_PROVIDER_H */




