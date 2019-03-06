/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CBANKING_GLOBALS_H
#define CBANKING_GLOBALS_H

#include "aqebics/client/provider_l.h"
#include "aqebics/client/provider.h"
#include "aqebics/client/user.h"

#include <aqbanking/banking_be.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/i18n.h>



#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg



int readFile(const char *fname, GWEN_BUFFER *dbuf);
int writeFile(FILE *f, const char *p, int len);


int EBC_Control(AB_PROVIDER *pro, int argc, char **argv);


int addAccount(AB_PROVIDER *pro, GWEN_DB_NODE *dbArgs, int argc, char **argv);

int addUser(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int createKeys(AB_PROVIDER *pro,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv);

int createTempKey(AB_PROVIDER *pro,
                  GWEN_DB_NODE *dbArgs,
                  int argc,
                  char **argv);

int sendKeys(AB_PROVIDER *pro,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int sendSignKey(AB_PROVIDER *pro,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv);

int getKeys(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);


int iniLetter(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);


int hiaLetter(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int sendHPD(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int sendHKD(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int download(AB_PROVIDER *pro,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int upload(AB_PROVIDER *pro,
           GWEN_DB_NODE *dbArgs,
           int argc,
           char **argv);

int getAccounts(AB_PROVIDER *pro,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv);

int mkPinList(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int resetUser(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int addUserFlags(AB_PROVIDER *pro,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv);

int subUserFlags(AB_PROVIDER *pro,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv);

int getCert(AB_PROVIDER *pro,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int listUsers(AB_PROVIDER *pro,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int listAccounts(AB_PROVIDER *pro,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv);

int setEbicsVersion(AB_PROVIDER *pro,
                    GWEN_DB_NODE *dbArgs,
                    int argc,
                    char **argv);


#endif




