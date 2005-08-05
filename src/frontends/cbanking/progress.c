/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "progress_p.h"
#include "cbanking_l.h"
#include "i18n_l.h"

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>



GWEN_LIST_FUNCTIONS(CBANKING_PROGRESS, CBankingProgress)


CBANKING_PROGRESS *CBankingProgress_new(AB_BANKING *ab,
                                        GWEN_TYPE_UINT32 id,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total){
  CBANKING_PROGRESS *pr;
  GWEN_BUFFER *tbuf;

  assert(title);
  assert(text);

  GWEN_NEW_OBJECT(CBANKING_PROGRESS, pr);
  GWEN_LIST_INIT(CBANKING_PROGRESS, pr);

  pr->banking=ab;
  pr->title=strdup(title);
  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  CBanking_GetRawText(ab, text, tbuf);
  pr->text=strdup(GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;
  pr->total=total;
  pr->logBuf=GWEN_Buffer_new(0, 256, 0, 1);

  return pr;
}



void CBankingProgress_free(CBANKING_PROGRESS *pr){
  if (pr) {
    GWEN_Buffer_free(pr->logBuf);
    free(pr->text);
    free(pr->title);
    GWEN_LIST_FINI(CBANKING_PROGRESS, pr);
    GWEN_FREE_OBJECT(pr);
  }
}



GWEN_TYPE_UINT32 CBankingProgress_GetId(const CBANKING_PROGRESS *pr){
  assert(pr);
  return pr->id;
}



const char *CBankingProgress_GetTitle(const CBANKING_PROGRESS *pr){
  assert(pr);
  return pr->title;
}



const char *CBankingProgress_GetText(const CBANKING_PROGRESS *pr){
  assert(pr);
  return pr->text;
}



GWEN_TYPE_UINT32 CBankingProgress_GetTotal(const CBANKING_PROGRESS *pr){
  assert(pr);
  return pr->total;
}



GWEN_TYPE_UINT32 CBankingProgress_GetCurrent(const CBANKING_PROGRESS *pr){
  assert(pr);
  return pr->current;
}



const char *CBankingProgress_GetLog(const CBANKING_PROGRESS *pr){
  assert(pr);
  return GWEN_Buffer_GetStart(pr->logBuf);
}



int CBankingProgress_Advance(CBANKING_PROGRESS *pr,
                             GWEN_TYPE_UINT32 progress){
#ifndef OS_WIN32
  int fl;
#endif

  assert(pr);
  if (progress!=AB_BANKING_PROGRESS_NONE) {
    if (progress!=pr->current) {
      fprintf(stderr,
              "%s: "GWEN_TYPE_TMPL_UINT32" of "GWEN_TYPE_TMPL_UINT32"\n",
              pr->title, progress, pr->total);
      pr->current=progress;
    }
  }
  if (pr->aborted)
    return AB_ERROR_USER_ABORT;

#ifndef OS_WIN32
  /* check for abort */
  fl=fcntl(fileno(stdin), F_GETFL);
  if (fl!=-1) {
    int chr;

    /* set stdin to nonblocking */
    if (fcntl(fileno(stdin), F_SETFL, fl | O_NONBLOCK)) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "fcntl(stdin): %s", strerror(errno));
      return 0;
    }
    /* check whether there is a character */
    chr=getchar();
    /* set blocking mode to what we found before modification */
    fcntl(fileno(stdin), F_SETFL, fl);
    if (chr==CBANKING_PROGRESS_CHAR_ABORT) {
      fprintf(stderr, "------> ABORTED BY USER\n");
      pr->aborted=1;
      return AB_ERROR_USER_ABORT;
    }
  }
#endif

  return 0;
}



int CBankingProgress_Log(CBANKING_PROGRESS *pr,
                         AB_BANKING_LOGLEVEL level,
                         const char *text){
  GWEN_BUFFER *tbuf;
  const char *t;

  assert(pr);
  assert(text);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  CBanking_GetRawText(pr->banking, text, tbuf);
  t=GWEN_Buffer_GetStart(tbuf);
  if (t[strlen(t)-1]!='\n')
    GWEN_Buffer_AppendByte(tbuf, '\n');
  fprintf(stderr, "%s", t);

  GWEN_Buffer_AppendString(pr->logBuf, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;
  if (pr->aborted)
    return AB_ERROR_USER_ABORT;

  return 0;
}



int CBankingProgress_End(CBANKING_PROGRESS *pr){
  assert(pr);

  fprintf(stderr, "%s: Finished.\n", pr->title);
  if (pr->aborted)
    return AB_ERROR_USER_ABORT;

  return 0;
}








