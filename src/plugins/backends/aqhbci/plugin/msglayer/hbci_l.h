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


#ifndef GWHBCI_HBCI_L_H
#define GWHBCI_HBCI_L_H

#include <aqhbci/hbci.h>

/** @name Virtual Functions
 *
 */
/*@{*/
/**
 * This function mounts the medium for the given user if it not already is
 * mounted and unmount the previously mounted medium
 * (if any, and if it was other than the now returned one).
 * This makes sure that only one medium is mounted at any time
 * and that a medium is only mounted and unmounted at this point in a program.
 * @param cu customer whose medium is needed
 */
AH_MEDIUM *AH_HBCI_GetMedium(AH_HBCI *hbci, AH_USER *u);
/*@}*/


int AH_HBCI_AddObjectPath(const AH_HBCI *hbci,
                          int country,
                          const char *bankId,
                          const char *accountId,
                          const char *userId,
                          const char *customerId,
                          GWEN_BUFFER *nbuf);

GWEN_DB_NODE *AH_HBCI_LoadSettings(const char *path);
int AH_HBCI_SaveSettings(const char *path, GWEN_DB_NODE *db);


int AH_HBCI_GetAccountPath(const AH_HBCI *hbci,
                           const AH_ACCOUNT *acc,
                           GWEN_BUFFER *buf);
int AH_HBCI_GetCustomerPath(const AH_HBCI *hbci,
                            const AH_CUSTOMER *cu,
                            GWEN_BUFFER *buf);


int AH_HBCI_AddBankPath(const AH_HBCI *hbci,
                        const AH_BANK *b,
                        GWEN_BUFFER *nbuf);
int AH_HBCI_AddUserPath(const AH_HBCI *hbci,
                        const AH_USER *u,
                        GWEN_BUFFER *nbuf);
int AH_HBCI_AddCustomerPath(const AH_HBCI *hbci,
                            const AH_CUSTOMER *cu,
                            GWEN_BUFFER *nbuf);
int AH_HBCI_AddAccountPath(const AH_HBCI *hbci,
                           const AH_ACCOUNT *a,
                           GWEN_BUFFER *nbuf);

void AH_HBCI_AppendUniqueName(AH_HBCI *hbci, GWEN_BUFFER *nbuf);


GWEN_XMLNODE *AH_HBCI_GetDefinitions(const AH_HBCI *hbci);


int AH_HBCI_CheckStringSanity(const char *s);


#endif /* GWHBCI_HBCI_L_H */



