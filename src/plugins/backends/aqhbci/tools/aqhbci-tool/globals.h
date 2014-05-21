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

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>



int mkPinList(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int addUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int delUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int getAccounts(AB_BANKING *ab,
                GWEN_DB_NODE *dbArgs,
                int argc,
                char **argv);

int addAccount(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int delAccount(AB_BANKING *ab,
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

int sendKeys(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int listUsers(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int listAccounts(AB_BANKING *ab,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv);

int iniLetter(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int getItanModes(AB_BANKING *ab,
                 GWEN_DB_NODE *dbArgs,
                 int argc,
                 char **argv);

int listItanModes(AB_BANKING *ab,
		  GWEN_DB_NODE *dbArgs,
		  int argc,
		  char **argv);

int setItanMode(AB_BANKING *ab,
		GWEN_DB_NODE *dbArgs,
		int argc,
		char **argv);

int changePin(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv);

int getCert(AB_BANKING *ab,
	    GWEN_DB_NODE *dbArgs,
	    int argc,
	    char **argv);

int setHbciVersion(AB_BANKING *ab,
		   GWEN_DB_NODE *dbArgs,
		   int argc,
		   char **argv);

int addUserFlags(AB_BANKING *ab,
		 GWEN_DB_NODE *dbArgs,
		 int argc,
		 char **argv);

int subUserFlags(AB_BANKING *ab,
		 GWEN_DB_NODE *dbArgs,
		 int argc,
		 char **argv);

int addsubAccountFlags(AB_BANKING *ab,
		       GWEN_DB_NODE *dbArgs,
		       int argc,
		       char **argv,
		       int is_add );



int setMaxTransfers(AB_BANKING *ab,
		    GWEN_DB_NODE *dbArgs,
		    int argc,
		    char **argv);

int setSepaProfile(AB_BANKING *ab,
		   GWEN_DB_NODE *dbArgs,
		   int argc,
		   char **argv);

int setTanMediumId(AB_BANKING *ab,
		   GWEN_DB_NODE *dbArgs,
		   int argc,
		   char **argv);

int getAccSepa(AB_BANKING *ab,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv);


int logFile(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);


int test1(AB_BANKING *ab,
          GWEN_DB_NODE *dbArgs,
          int argc,
          char **argv);


#endif




