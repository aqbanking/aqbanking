/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCI_GLOBALS_H
#define AQHBCI_GLOBALS_H


#ifdef HAVE_I18N
# ifdef HAVE_LOCALE_H
#  include <locale.h>
# endif
# ifdef HAVE_LIBINTL_H
#  include <libintl.h>
# endif
# define I18N(msg) dgettext(PACKAGE, msg)
#else
# define I18N(msg) msg
#endif

#define I18N_NOOP(msg) msg

#include "aqhbci/aqhbci.h"
#include "aqhbci/banking/provider.h"

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>


#include "control_l.h"


int AH_Control_MkPinList(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv);

int AH_Control_AddUser(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv);

int AH_Control_DelUser(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv);

int AH_Control_GetAccounts(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv);

int AH_Control_AddAccount(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv);

int AH_Control_DelAccount(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv);

int AH_Control_GetSysId(AB_PROVIDER *pro,
                        GWEN_DB_NODE *dbArgs,
                        int argc,
                        char **argv);

int AH_Control_GetKeys(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv);

int AH_Control_CreateKeys(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv);

int AH_Control_SendKeys(AB_PROVIDER *pro,
                        GWEN_DB_NODE *dbArgs,
                        int argc,
                        char **argv);

int AH_Control_ListUsers(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv);

int AH_Control_ListAccounts(AB_PROVIDER *pro,
                            GWEN_DB_NODE *dbArgs,
                            int argc,
                            char **argv);

int AH_Control_IniLetter(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv);

int AH_Control_GetItanModes(AB_PROVIDER *pro,
                            GWEN_DB_NODE *dbArgs,
                            int argc,
                            char **argv);

int AH_Control_ListItanModes(AB_PROVIDER *pro,
                             GWEN_DB_NODE *dbArgs,
                             int argc,
                             char **argv);

int AH_Control_SetItanMode(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv);

int AH_Control_ChangePin(AB_PROVIDER *pro,
                         GWEN_DB_NODE *dbArgs,
                         int argc,
                         char **argv);

int AH_Control_GetCert(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv);

int AH_Control_SetHbciVersion(AB_PROVIDER *pro,
                              GWEN_DB_NODE *dbArgs,
                              int argc,
                              char **argv);

int AH_Control_AddsubUserFlags(AB_PROVIDER *pro,
                               GWEN_DB_NODE *dbArgs,
                               int argc,
                               char **argv,
                               int is_add);

int AH_Control_AddsubAccountFlags(AB_PROVIDER *pro,
                                  GWEN_DB_NODE *dbArgs,
                                  int argc,
                                  char **argv,
                                  int is_add);



int AH_Control_SetMaxTransfers(AB_PROVIDER *pro,
                               GWEN_DB_NODE *dbArgs,
                               int argc,
                               char **argv);

int AH_Control_SetSepaProfile(AB_PROVIDER *pro,
                              GWEN_DB_NODE *dbArgs,
                              int argc,
                              char **argv);

int AH_Control_SetTanMediumId(AB_PROVIDER *pro,
                              GWEN_DB_NODE *dbArgs,
                              int argc,
                              char **argv);

int AH_Control_GetAccSepa(AB_PROVIDER *pro,
                          GWEN_DB_NODE *dbArgs,
                          int argc,
                          char **argv);


int AH_Control_LogFile(AB_PROVIDER *pro,
                       GWEN_DB_NODE *dbArgs,
                       int argc,
                       char **argv);


int AH_Control_Test1(AB_PROVIDER *pro,
                     GWEN_DB_NODE *dbArgs,
                     int argc,
                     char **argv);


int AH_Control_GetBankInfo(AB_PROVIDER *pro,
                           GWEN_DB_NODE *dbArgs,
                           int argc,
                           char **argv);


#endif




