/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
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



GWEN_HTTP_SESSION *AB_HttpSession_new(AB_PROVIDER *pro, AB_USER *u,
				      const char *url,
				      uint32_t guiid) {
  GWEN_HTTP_SESSION *sess;
  AB_HTTP_SESSION *xsess;

  assert(pro);
  assert(u);

  sess=GWEN_HttpSession_new(url, guiid);
  assert(sess);
  GWEN_NEW_OBJECT(AB_HTTP_SESSION, xsess);
  GWEN_INHERIT_SETDATA(GWEN_HTTP_SESSION, AB_HTTP_SESSION, sess, xsess,
                       AB_HttpSession_FreeData);

  xsess->provider=pro;
  xsess->user=u;

  return sess;
}



void AB_HttpSession_FreeData(void *bp, void *p) {
  AB_HTTP_SESSION *xsess;

  xsess=(AB_HTTP_SESSION*)p;
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




