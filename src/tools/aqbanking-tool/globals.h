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
#include <aqbanking/transaction.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>

#include <cbanking/cbanking.h>


#define AQT_LOGDOMAIN "aqbanking-tool"


AB_TRANSACTION *mkTransfer(AB_ACCOUNT *a, GWEN_DB_NODE *db);


int listAccs(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int request(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int qexec(AB_BANKING *ab,
          GWEN_DB_NODE *dbArgs,
          int argc,
          char **argv);

int listTrans(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int transfer(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int debitNote(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int chkAcc(AB_BANKING *ab,
           GWEN_DB_NODE *dbArgs,
           int argc,
           char **argv);

int listBal(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int import(AB_BANKING *ab,
           GWEN_DB_NODE *dbArgs,
           int argc,
           char **argv);

int jobLog(AB_BANKING *ab,
           GWEN_DB_NODE *dbArgs,
           int argc,
           char **argv);

int chkIban(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

AB_BANKINFO_CHECKRESULT accountcheck(AB_BANKING *ab, const char *country,
				     const char *bankcode, const char *accountid,
				     int forceCheck);

#endif




