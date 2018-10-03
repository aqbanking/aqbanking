/***************************************************************************
 begin       : Sat Jun 30 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQBANKING_BANKING6_BE_H
#define AQBANKING_BANKING6_BE_H



#ifdef __cplusplus
extern "C" {
#endif


/** @name Account Spec Management Functions
 *
 */
/*@{*/

AQBANKING_API int AB_Banking_ReadAccountSpec(AB_BANKING *ab, uint32_t uniqueId, AB_ACCOUNT_SPEC **pAccountSpec);


AQBANKING_API int AB_Banking_WriteAccountSpec(AB_BANKING *ab, const AB_ACCOUNT_SPEC *accountSpec);

AQBANKING_API int AB_Banking_DeleteAccountSpec(AB_BANKING *ab, uint32_t uid);



/*@}*/



AQBANKING_API int AB_Banking_Read_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb);
AQBANKING_API int AB_Banking_Write_AccountConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db);
AQBANKING_API int AB_Banking_Delete_AccountConfig(AB_BANKING *ab, uint32_t uid);
AQBANKING_API int AB_Banking_Unlock_AccountConfig(AB_BANKING *ab, uint32_t uid);

AQBANKING_API int AB_Banking_Read_UserConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE **pDb);
AQBANKING_API int AB_Banking_Write_UserConfig(AB_BANKING *ab, uint32_t uid, int doLock, int doUnlock, GWEN_DB_NODE *db);
AQBANKING_API int AB_Banking_Delete_UserConfig(AB_BANKING *ab, uint32_t uid);
AQBANKING_API int AB_Banking_Unlock_UserConfig(AB_BANKING *ab, uint32_t uid);



AQBANKING_API int AB_Banking_ReadNamedConfigGroup(AB_BANKING *ab,
                                                  const char *groupName,
                                                  const char *subGroupName,
                                                  int doLock,
                                                  int doUnlock,
                                                  GWEN_DB_NODE **pDb);

AQBANKING_API int AB_Banking_WriteNamedConfigGroup(AB_BANKING *ab,
                                                   const char *groupName,
                                                   const char *subGroupName,
                                                   int doLock,
                                                   int doUnlock,
                                                   GWEN_DB_NODE *db);


AQBANKING_API int AB_Banking_ReadConfigGroup(AB_BANKING *ab,
                                             const char *groupName,
                                             uint32_t uniqueId,
                                             int doLock,
                                             int doUnlock,
                                             GWEN_DB_NODE **pDb);


AQBANKING_API int AB_Banking_WriteConfigGroup(AB_BANKING *ab,
                                              const char *groupName,
                                              uint32_t uniqueId,
                                              int doLock,
                                              int doUnlock,
                                              GWEN_DB_NODE *db);

AQBANKING_API int AB_Banking_DeleteConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId);

AQBANKING_API int AB_Banking_UnlockConfigGroup(AB_BANKING *ab, const char *groupName, uint32_t uniqueId);


/**
 * @return 0 if there are some groups, error code otherwise (especially GWEN_ERROR_PARTIAL if some groups couldn't be read
 *           and GWEN_ERROR_NOT_FOUND if there no groups found).
 * @param ab AQBANKING object
 * @param groupName name of the config group
 * @param uidField name of an integer variable in the config groups which MUST NOT be zero (NULL to skip this test)
 * @param matchVar name of a variable in the config groups whose value must match matchVal (NULL to skip this test)
 * @param matchVal value to match the matchVar variable (NULL for empty value)
 * @param pDb pointer to a variable to receive the newly created DB, each subgroup contains a config group
 */
AQBANKING_API int AB_Banking_ReadConfigGroups(AB_BANKING *ab,
                                              const char *groupName,
                                              const char *uidField,
                                              const char *matchVar,
                                              const char *matchVal,
                                              GWEN_DB_NODE **pDb);



#ifdef __cplusplus
}
#endif



#endif

