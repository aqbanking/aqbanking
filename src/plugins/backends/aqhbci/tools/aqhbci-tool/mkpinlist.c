/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "globals.h"
#include <aqhbci/user.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/syncio_file.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>


int mkPinList(AB_BANKING *ab,
              GWEN_DB_NODE *dbArgs,
              int argc,
              char **argv) {
  GWEN_DB_NODE *db;
  AB_PROVIDER *pro;
  GWEN_SYNCIO *sio;
  AB_USER_LIST2 *ul;
  int rv;
  const char *outFile;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsType_Char,           /* type */
    "outFile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "o",                          /* short option */
    "outfile",                    /* long option */
    "Specify the name of the output file", /* short description */
    "Specify the name of the output file"  /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsType_Int,            /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

  db=GWEN_DB_GetGroup(dbArgs, GWEN_DB_FLAGS_DEFAULT, "local");
  rv=GWEN_Args_Check(argc, argv, 1,
                     0 /*GWEN_ARGS_MODE_ALLOW_FREEPARAM*/,
                     args,
                     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments\n");
    return 1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutType_Txt)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }

  outFile=GWEN_DB_GetCharValue(db, "outfile", 0, 0);

  rv=AB_Banking_Init(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  rv=AB_Banking_OnlineInit(ab);
  if (rv) {
    DBG_ERROR(0, "Error on init (%d)", rv);
    return 2;
  }

  pro=AB_Banking_GetProvider(ab, "aqhbci");
  assert(pro);

  if (outFile==0) {
    sio=GWEN_SyncIo_File_fromStdout();
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FLAGS_DONTCLOSE);
  }
  else {
    sio=GWEN_SyncIo_File_new(outFile, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sio,
			 GWEN_SYNCIO_FILE_FLAGS_READ |
			 GWEN_SYNCIO_FILE_FLAGS_WRITE |
			 GWEN_SYNCIO_FILE_FLAGS_UREAD |
			 GWEN_SYNCIO_FILE_FLAGS_UWRITE |
			 GWEN_SYNCIO_FILE_FLAGS_GREAD |
			 GWEN_SYNCIO_FILE_FLAGS_GWRITE);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_ERROR(0, "Error opening output file: %s",
		strerror(errno));
      return 4;
    }
  }

  GWEN_SyncIo_WriteLine(sio,
			"# This is a PIN file to be used "
			"with AqBanking");
  GWEN_SyncIo_WriteLine(sio,
			"# Please insert the PINs/passwords "
			"for the users below");

  ul=AB_Banking_FindUsers(ab, AH_PROVIDER_NAME, "*", "*", "*", "*");
  if (ul) {
    AB_USER_LIST2_ITERATOR *uit;

    uit=AB_User_List2_First(ul);
    if (uit) {
      AB_USER *u;

      u=AB_User_List2Iterator_Data(uit);
      assert(u);

      while(u) {
	const char *s;
	GWEN_BUFFER *nbuf;
	int rv;

	GWEN_SyncIo_WriteLine(sio, "");
	GWEN_SyncIo_WriteString(sio, "# User \"");
	s=AB_User_GetUserId(u);
	assert(s);
	GWEN_SyncIo_WriteString(sio, s);
	GWEN_SyncIo_WriteString(sio, "\" at \"");
	s=AB_User_GetBankCode(u);
	GWEN_SyncIo_WriteString(sio, s);
	GWEN_SyncIo_WriteLine(sio, "\"");

	nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
	if (AH_User_GetCryptMode(u)==AH_CryptMode_Pintan)
	  rv=AH_User_MkPinName(u, nbuf);
	else
	  rv=AH_User_MkPasswdName(u, nbuf);

	if (rv==0) {
	  GWEN_BUFFER *obuf;

	  obuf=GWEN_Buffer_new(0, 256 ,0 ,1);
	  if (GWEN_Text_EscapeToBufferTolerant(GWEN_Buffer_GetStart(nbuf),
					       obuf)) {
	    DBG_ERROR(0, "Error escaping name to buffer");
	    return 3;
	  }
	  GWEN_SyncIo_WriteString(sio, GWEN_Buffer_GetStart(obuf));
	  GWEN_SyncIo_WriteLine(sio, " = \"\"");

	  GWEN_Buffer_free(obuf);
	}
	GWEN_Buffer_free(nbuf);

	u=AB_User_List2Iterator_Next(uit);
      }
      AB_User_List2Iterator_free(uit);
    }
    AB_User_List2_free(ul);
  }

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_ERROR_ERR(0, rv);
    GWEN_SyncIo_free(sio);
    return 4;
  }
  GWEN_SyncIo_free(sio);


  rv=AB_Banking_OnlineFini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  rv=AB_Banking_Fini(ab);
  if (rv) {
    fprintf(stderr, "ERROR: Error on deinit (%d)\n", rv);
    return 5;
  }

  return 0;
}




