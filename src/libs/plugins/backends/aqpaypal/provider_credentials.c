/***************************************************************************
    begin       : Sat Dec 01 2018
    copyright   : (C) 2022 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "aqpaypal/provider_credentials.h"
#include "aqpaypal/aqpaypal.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/i18n.h>
#include <gwenhywfar/gui.h>
#include <gwenhywfar/smalltresor.h>
#include <gwenhywfar/directory.h>

#include <stdio.h>
#include <ctype.h>
#include <errno.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)

#define AQPAYPAL_PASSWORD_ITERATIONS 1467
#define AQPAYPAL_CRYPT_ITERATIONS    648


static int _readFile(const char *fname, GWEN_BUFFER *dbuf);
static int _writeToFile(FILE *f, const char *p, int len);
static int _writeFile(const char *fname, const char *p, int len);





int APY_Provider_ReadUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, GWEN_BUFFER *secbuf)
{
  int rv;
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *sbuf;
  GWEN_BUFFER *tbuf;
  const char *uid;
  char text[512];
  char pw[129];

  uid=AB_User_GetUserId(u);
  if (!(uid && *uid)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No user id");
    return GWEN_ERROR_INVALID;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Provider_GetUserDataDir(pro, pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_UnescapeToBufferTolerant(uid, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".sec");

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=_readFile(GWEN_Buffer_GetStart(pbuf), sbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  snprintf(text, sizeof(text)-1,
           I18N("Please enter the password for \n"
                "Paypal user %s\n"
                "<html>"
                "Please enter the password for Paypal user <i>%s</i></br>"
                "</html>"),
           uid, uid);
  text[sizeof(text)-1]=0;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "PASSWORD_");
  GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(pbuf), tbuf);

  rv=GWEN_Gui_GetPassword(0,
                          GWEN_Buffer_GetStart(tbuf),
                          I18N("Enter Password"),
                          text,
                          pw,
                          4,
                          sizeof(pw)-1,
                          GWEN_Gui_PasswordMethod_Text, NULL,
                          0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  rv=GWEN_SmallTresor_Decrypt((const uint8_t *) GWEN_Buffer_GetStart(sbuf),
                              GWEN_Buffer_GetUsedBytes(sbuf),
                              pw,
                              secbuf,
                              AQPAYPAL_PASSWORD_ITERATIONS,
                              AQPAYPAL_CRYPT_ITERATIONS);
  /* overwrite password ASAP */
  memset(pw, 0, sizeof(pw));
  GWEN_Buffer_free(tbuf);
  GWEN_Buffer_free(sbuf);
  GWEN_Buffer_free(pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int APY_Provider_WriteUserApiSecrets(AB_PROVIDER *pro, const AB_USER *u, const char *sec)
{
  int rv;
  GWEN_BUFFER *pbuf;
  GWEN_BUFFER *sbuf;
  GWEN_BUFFER *tbuf;
  const char *uid;
  char text[512];
  char pw[129];

  uid=AB_User_GetUserId(u);
  if (!(uid && *uid)) {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "No user id");
    return GWEN_ERROR_INVALID;
  }

  pbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=AB_Provider_GetUserDataDir(pro, pbuf);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  /* make sure the data dir exists */
  DBG_INFO(0, "Looking for [%s]", GWEN_Buffer_GetStart(pbuf));
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(pbuf), 0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_AppendString(pbuf, GWEN_DIR_SEPARATOR_S);
  GWEN_Text_UnescapeToBufferTolerant(uid, pbuf);
  GWEN_Buffer_AppendString(pbuf, ".sec");

  snprintf(text, sizeof(text)-1,
           I18N("Please enter the password for \n"
                "Paypal user %s\n"
                "<html>"
                "Please enter the password for Paypal user <i>%s</i></br>"
                "</html>"),
           uid, uid);
  text[sizeof(text)-1]=0;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(tbuf, "PASSWORD_");
  GWEN_Text_UnescapeToBufferTolerant(GWEN_Buffer_GetStart(pbuf), tbuf);

  rv=GWEN_Gui_GetPassword(GWEN_GUI_INPUT_FLAGS_CONFIRM,
                          GWEN_Buffer_GetStart(tbuf),
                          I18N("Enter Password"),
                          text,
                          pw,
                          4,
                          sizeof(pw)-1,
                          GWEN_Gui_PasswordMethod_Text, NULL,
                          0);
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(tbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }
  GWEN_Buffer_free(tbuf);

  sbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_SmallTresor_Encrypt((const uint8_t *) sec,
                              strlen(sec),
                              pw,
                              sbuf,
                              AQPAYPAL_PASSWORD_ITERATIONS,
                              AQPAYPAL_CRYPT_ITERATIONS);
  /* overwrite password ASAP */
  memset(pw, 0, sizeof(pw));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  /* write file */
  rv=_writeFile(GWEN_Buffer_GetStart(pbuf), GWEN_Buffer_GetStart(sbuf), GWEN_Buffer_GetUsedBytes(sbuf));
  if (rv<0) {
    DBG_INFO(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(sbuf);
    GWEN_Buffer_free(pbuf);
    return rv;
  }

  GWEN_Buffer_free(sbuf);
  GWEN_Buffer_free(pbuf);

  return 0;
}



int _readFile(const char *fname, GWEN_BUFFER *dbuf)
{
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while (!feof(f)) {
      uint32_t l;
      ssize_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (s==0)
        break;
      if (s==(ssize_t)-1) {
        DBG_ERROR(AQPAYPAL_LOGDOMAIN,
                  "fread(%s): %s",
                  fname, strerror(errno));
        fclose(f);
        return GWEN_ERROR_IO;
      }

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN,
              "fopen(%s): %s",
              fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}



int _writeToFile(FILE *f, const char *p, int len)
{
  while (len>0) {
    ssize_t l;
    ssize_t s;

    l=1024;
    if (l>len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==(ssize_t)-1 || s==0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "fwrite: %s", strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}



int _writeFile(const char *fname, const char *p, int len)
{
  FILE *f;

  f=fopen(fname, "wb");
  if (f) {
    int rv;

    rv=_writeToFile(f, p, len);
    if (rv<0) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      fclose(f);
      return rv;
    }
    if (fclose(f)) {
      DBG_ERROR(AQPAYPAL_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }
  else {
    DBG_ERROR(AQPAYPAL_LOGDOMAIN, "fopen(%s): %s",
              fname, strerror(errno));
    return GWEN_ERROR_IO;
  }

  return 0;
}





