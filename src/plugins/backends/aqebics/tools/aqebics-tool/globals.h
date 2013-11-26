/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef CBANKING_GLOBALS_H
#define CBANKING_GLOBALS_H


#include <aqbanking/banking_be.h>
#include <aqebics/provider.h>
#include <aqebics/user.h>

#include <gwenhywfar/args.h>
#include <gwenhywfar/buffer.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/cgui.h>
#include <gwenhywfar/i18n.h>

#include <stdio.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)
#define I18S(msg) msg



int readFile(const char *fname, GWEN_BUFFER *dbuf);
int writeFile(FILE *f, const char *p, int len);



int addUser(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);

int createKeys(AB_BANKING *ab,
               GWEN_DB_NODE *dbArgs,
               int argc,
               char **argv);

int createTempKey(AB_BANKING *ab,
		  GWEN_DB_NODE *dbArgs,
		  int argc,
		  char **argv);

int sendKeys(AB_BANKING *ab,
             GWEN_DB_NODE *dbArgs,
             int argc,
             char **argv);

int sendSignKey(AB_BANKING *ab,
		GWEN_DB_NODE *dbArgs,
		int argc,
		char **argv);

int getKeys(AB_BANKING *ab,
            GWEN_DB_NODE *dbArgs,
            int argc,
            char **argv);


int iniLetter(AB_BANKING *ab,
	      GWEN_DB_NODE *dbArgs,
	      int argc,
	      char **argv);


int hiaLetter(AB_BANKING *ab,
	      GWEN_DB_NODE *dbArgs,
	      int argc,
	      char **argv);

int sendHPD(AB_BANKING *ab,
	    GWEN_DB_NODE *dbArgs,
	    int argc,
	    char **argv);

int sendHKD(AB_BANKING *ab,
	    GWEN_DB_NODE *dbArgs,
	    int argc,
	    char **argv);

int download(AB_BANKING *ab,
	     GWEN_DB_NODE *dbArgs,
	     int argc,
	     char **argv);

int upload(AB_BANKING *ab,
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

int mkPinList(AB_BANKING *ab,
	      GWEN_DB_NODE *dbArgs,
	      int argc,
	      char **argv);

int resetUser(AB_BANKING *ab,
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

int getCert(AB_BANKING *ab,
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

int setEbicsVersion(AB_BANKING *ab,
		    GWEN_DB_NODE *dbArgs,
		    int argc,
		    char **argv);


#endif




