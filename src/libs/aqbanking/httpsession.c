/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004-2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "httpsession_p.h"
#include "i18n_l.h"

#include <gwenhywfar/misc.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif



GWEN_INHERIT(GWEN_HTTP_SESSION, AB_HTTP_SESSION)



GWEN_HTTP_SESSION *AB_HttpSession_new(AB_PROVIDER *pro,
				      AB_USER *u,
				      const char *url,
				      const char *defaultProto,
				      int defaultPort) {
  GWEN_HTTP_SESSION *sess;
  AB_HTTP_SESSION *xsess;

  assert(pro);
  assert(u);

  sess=GWEN_HttpSession_new(url, defaultProto, defaultPort);
  assert(sess);
  GWEN_NEW_OBJECT(AB_HTTP_SESSION, xsess);
  GWEN_INHERIT_SETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess, xsess,
                       AB_HttpSession_FreeData);

  xsess->provider=pro;
  xsess->user=u;
  xsess->logs=GWEN_Buffer_new(0, 256, 0, 1);

  return sess;
}



void GWENHYWFAR_CB AB_HttpSession_FreeData(void *bp, void *p) {
  AB_HTTP_SESSION *xsess;

  xsess=(AB_HTTP_SESSION*)p;
  GWEN_Buffer_free(xsess->logs);
  GWEN_FREE_OBJECT(xsess);
}



AB_PROVIDER *AB_HttpSession_GetProvider(const GWEN_HTTP_SESSION *sess) {
  AB_HTTP_SESSION *xsess;

  assert(sess);
  xsess=GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess);
  assert(xsess);

  return xsess->provider;
}



AB_USER *AB_HttpSession_GetUser(const GWEN_HTTP_SESSION *sess) {
  AB_HTTP_SESSION *xsess;

  assert(sess);
  xsess=GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess);
  assert(xsess);

  return xsess->user;
}



void Ab_HttpSession_AddLog(GWEN_HTTP_SESSION *sess,
			   const char *s) {
  AB_HTTP_SESSION *xsess;

  assert(sess);
  xsess=GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess);
  assert(xsess);

  if (s) {
    size_t l=strlen(s);
    if (s) {
      GWEN_Buffer_AppendString(xsess->logs, s);
      if (s[l-1]!='\n')
	GWEN_Buffer_AppendByte(xsess->logs, '\n');
    }
  }
}



const char *AB_HttpSession_GetLog(const GWEN_HTTP_SESSION *sess) {
  AB_HTTP_SESSION *xsess;

  assert(sess);
  xsess=GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess);
  assert(xsess);

  if (GWEN_Buffer_GetUsedBytes(xsess->logs))
    return GWEN_Buffer_GetStart(xsess->logs);
  else
    return NULL;
}



void AB_HttpSession_ClearLog(GWEN_HTTP_SESSION *sess) {
  AB_HTTP_SESSION *xsess;

  assert(sess);
  xsess=GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess);
  assert(xsess);

  GWEN_Buffer_Reset(xsess->logs);
}



