/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AQBANKING_BANKING_INFO_H
#define AQBANKING_BANKING_INFO_H

#include <aqbanking/types/bankinfo.h>


typedef enum {
  AB_BankInfoCheckResult_Ok=0,
  AB_BankInfoCheckResult_NotOk,
  AB_BankInfoCheckResult_UnknownBank,
  AB_BankInfoCheckResult_UnknownResult
} AB_BANKINFO_CHECKRESULT;



#ifdef __cplusplus
extern "C" {
#endif


/** @addtogroup G_AB_INFO
 */
/*@{*/


/** @name Getting Bank/Account Information
 *
 * Functions in this group retrieve information about credit institutes and
 * allow checking of bank code/account id combinations.
 * These functions load the appropriate checker plugins for selected
 * countries.
 */
/*@{*/


/**
 * This functions retrieves information about a given bank. It loads the
 * appropriate bank checker module and asks it for information about the given
 * bank. The caller is responsible for freeing the object returned (if any)
 * by calling @ref AB_BankInfo_free.
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param branchId optional branch id (not needed for "de")
 * @param bankId bank id ("Bankleitzahl" for "de")
 */
AQBANKING_API AB_BANKINFO *AB_Banking_GetBankInfo(AB_BANKING *ab,
                                                  const char *country,
                                                  const char *branchId,
                                                  const char *bankId);


/**
 * This function retrieves information about banks. It loads the
 * appropriate bank checker module and asks it for a list of AB_BANKINFO
 * objects which match the given template. Empty fields in this template
 * always match. Service entries (AB_BANKINFO_SERVICE) are not compared.
 * Matching entries are added to the given list.
 * The caller is responsible for freeing the objects returned (if any)
 * by calling @ref AB_BankInfo_free (or by calling
 *  @ref AB_BankInfo_List_freeAll).
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param tbi template to compare against
 * @param bl list to which matching banks are added
 */
AQBANKING_API int AB_Banking_GetBankInfoByTemplate(AB_BANKING *ab,
                                                   const char *country,
                                                   AB_BANKINFO *tbi,
                                                   AB_BANKINFO_LIST2 *bl);


/**
 * This function checks whether the given combination represents a valid
 * account. It loads the appropriate bank checker module and lets it check
 * the information.
 * @param ab AqBanking main object
 * @param country ISO country code ("de" for Germany, "at" for Austria etc)
 * @param branchId optional branch id (not needed for "de")
 * @param bankId bank id ("Bankleitzahl" for "de")
 * @param accountId account id
 */
AQBANKING_API AB_BANKINFO_CHECKRESULT AB_Banking_CheckAccount(AB_BANKING *ab,
                                                              const char *country,
                                                              const char *branchId,
                                                              const char *bankId,
                                                              const char *accountId);

/**
 * Checks whether a given international bank account number (IBAN) is
 * valid or not.
 * @return 0 if valid, 1 if not and -1 on error
 * @param iban IBAN (e.g. "DE88 2008 0000 09703 7570 0")
 */
AQBANKING_API int AB_Banking_CheckIban(const char *iban);


/**
 * Create an IBAN from German bank code and account number.
 */
AQBANKING_API int AB_Banking_MakeGermanIban(const char *bankCode, const char *accountNumber, GWEN_BUFFER *ibanBuf);

/*@}*/


/*@}*/ /* addtogroup */


#ifdef __cplusplus
}
#endif

#endif


