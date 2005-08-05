/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CBANKING_GLOBALS_H
#define CBANKING_GLOBALS_H


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

#include <aqbanking/banking.h>
#include <aqbanking/banking_be.h>
#include <aqhbci/provider.h>
#include <aqhbci/hbci.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include "cbanking.h"



int mkPinList(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int addUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int getAccounts(AB_BANKING *ab,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv);

int addMedium(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int listMedia(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int getSysId(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int getKeys(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int createKeys(AB_BANKING *ab,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv);

int resetKeys(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int sendKeys(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);


#endif




