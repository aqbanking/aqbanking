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

#include "cbanking_p.h"
#include "globals.h"

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/text.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#ifdef HAVE_SIGNAL_H
# include <signal.h>
#endif
#ifdef HAVE_ICONV_H
# include <iconv.h>
#endif



GWEN_INHERIT(AB_BANKING, CBANKING)




AB_BANKING *CBanking_new(const char *appName,
                         const char *fname){
  AB_BANKING *ab;
  CBANKING *cb;

  ab=AB_Banking_new(appName, fname);
  assert(ab);
  GWEN_NEW_OBJECT(CBANKING, cb);
  GWEN_INHERIT_SETDATA(AB_BANKING, CBANKING, ab, cb, CBanking_FreeData);

  AB_Banking_SetMessageBoxFn(ab, CBanking_MessageBox);
  AB_Banking_SetInputBoxFn(ab, CBanking_InputBox);
  AB_Banking_SetShowBoxFn(ab, CBanking_ShowBox);
  AB_Banking_SetHideBoxFn(ab, CBanking_HideBox);
  AB_Banking_SetProgressStartFn(ab, CBanking_ProgressStart);
  AB_Banking_SetProgressAdvanceFn(ab, CBanking_ProgressAdvance);
  AB_Banking_SetProgressLogFn(ab, CBanking_ProgressLog);
  AB_Banking_SetProgressEndFn(ab, CBanking_ProgressEnd);
  AB_Banking_SetGetPinFn(ab, CBanking_GetPin);

  cb->progressList=CBankingProgress_List_new();

  return ab;
}



void CBanking_FreeData(void *bp, void *p) {
  CBANKING *cb;

  cb=(CBANKING*)p;
  assert(cb);

  CBankingProgress_List_free(cb->progressList);
  GWEN_DB_Group_free(cb->dbPins);

  GWEN_FREE_OBJECT(p);
}



const char *CBanking_GetCharSet(const AB_BANKING *ab){
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  return cb->charSet;
}



void CBanking_SetCharSet(AB_BANKING *ab, const char *s){
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  free(cb->charSet);
  if (s) cb->charSet=strdup(s);
  else cb->charSet=0;
}



int CBanking__ConvertFromUtf8(AB_BANKING *ab,
                              const char *text,
                              int len,
                              GWEN_BUFFER *tbuf){
  CBANKING *cb;

  assert(len);
  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  if (cb->charSet) {
    if (strcasecmp(cb->charSet, "utf-8")!=0) {
#ifndef HAVE_ICONV_H
      DBG_INFO(AQT_LOGDOMAIN, "iconv not available, can not convert to \"%s\"",
               cb->charSet);
#else
      iconv_t ic;

      ic=iconv_open(cb->charSet, "UTF-8");
      if (ic==((iconv_t)-1)) {
        DBG_ERROR(AQT_LOGDOMAIN, "Charset \"%s\" not available",
                  cb->charSet);
      }
      else {
        char *outbuf;
        char *pOutbuf;
        char *pInbuf;
        size_t inLeft;
        size_t outLeft;
        size_t done;
        size_t space;

        /* convert */
        pInbuf=(char*)text;

        outLeft=len*2;
        space=outLeft;
        outbuf=(char*)malloc(outLeft);
        assert(outbuf);

        inLeft=len;
        pInbuf=(char*)text;
        pOutbuf=outbuf;
        done=iconv(ic, &pInbuf, &inLeft, &pOutbuf, &outLeft);
        if (done==(size_t)-1) {
          DBG_ERROR(AQT_LOGDOMAIN, "Error in conversion: %s (%d)",
                    strerror(errno), errno);
          free(outbuf);
          iconv_close(ic);
          return -1;
        }

        GWEN_Buffer_AppendBytes(tbuf, outbuf, space-outLeft);
        free(outbuf);
        DBG_DEBUG(AQT_LOGDOMAIN, "Conversion done.");
        iconv_close(ic);
        return 0;
      }
#endif
    }
  }

  GWEN_Buffer_AppendString(tbuf, text);
  return 0;
}



void CBanking_GetRawText(AB_BANKING *ab,
                         const char *text,
                         GWEN_BUFFER *tbuf) {
  const char *p;
  int rv;

  assert(text);
  p=text;
  while ((p=strchr(p, '<'))) {
    const char *t;

    t=p;
    t++;
    if (toupper(*t)=='H') {
      t++;
      if (toupper(*t)=='T') {
        t++;
        if (toupper(*t)=='M') {
          t++;
          if (toupper(*t)=='L') {
            break;
          }
        }
      }
    }
    p++;
  } /* while */

  if (p)
    rv=CBanking__ConvertFromUtf8(ab, text, (p-text), tbuf);
  else
    rv=CBanking__ConvertFromUtf8(ab, text, strlen(text), tbuf);
  if (rv) {
    DBG_ERROR(AQT_LOGDOMAIN, "Error converting text");
    GWEN_Buffer_Reset(tbuf);
    if (p)
      GWEN_Buffer_AppendBytes(tbuf, text, (p-text));
    else
      GWEN_Buffer_AppendString(tbuf, text);
  }
}




int CBanking_MessageBox(AB_BANKING *ab,
                        GWEN_TYPE_UINT32 flags,
                        const char *title,
                        const char *text,
                        const char *b1,
                        const char *b2,
                        const char *b3){
  CBANKING *cb;
  GWEN_BUFFER *tbuf;
  int c;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  CBanking_GetRawText(ab, text, tbuf);

  if (cb->nonInteractive) {
    if (AB_BANKING_MSG_FLAGS_SEVERITY_IS_DANGEROUS(flags)) {
      fprintf(stderr,
              "Got the following dangerous message:\n%s\n",
              GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return 0;
    }
    else {
      DBG_INFO(AQT_LOGDOMAIN,
               "Auto-answering the following message with %d:\n%s",
               AB_BANKING_MSG_FLAGS_CONFIRM_BUTTON(flags),
               GWEN_Buffer_GetStart(tbuf));
      GWEN_Buffer_free(tbuf);
      return AB_BANKING_MSG_FLAGS_CONFIRM_BUTTON(flags);
    }
  }

  fprintf(stderr, "===== %s =====\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (b1) {
    fprintf(stderr, "(1) %s", b1);
    if (b2) {
      fprintf(stderr, "  (2) %s", b2);
      if (b3) {
        fprintf(stderr, "  (3) %s", b3);
      }
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "Please enter your choice: ");
  for(;;) {
    c=CBanking__readCharFromStdin(0);
    if (c==EOF) {
      fprintf(stderr, "Aborted.\n");
      return AB_ERROR_USER_ABORT;
    }
    if (!b1 && c==13)
      return 0;
    if (c=='1' && b1) {
      fprintf(stderr, "1\n");
      return 1;
    }
    else if (c=='2' && b2) {
      fprintf(stderr, "2\n");
      return 2;
    }
    else if (c=='3' && b3) {
      fprintf(stderr, "3\n");
      return 3;
    }
    else {
      fprintf(stderr, "%c", 7);
    }
  } /* for */
}



char CBanking__readCharFromStdin(int waitFor) {
  int chr;
#ifdef HAVE_TERMIOS_H
  struct termios OldAttr, NewAttr;
  int AttrChanged = 0;
#endif
#if HAVE_DECL_SIGPROCMASK
  sigset_t snew, sold;
#endif

  // disable canonical mode to receive a single character
#if HAVE_DECL_SIGPROCMASK
  sigemptyset(&snew);
  sigaddset(&snew, SIGINT);
  sigaddset(&snew, SIGSTOP);
  sigprocmask(SIG_BLOCK, &snew, &sold);
#endif
#ifdef HAVE_TERMIOS_H
  if (0 == tcgetattr (fileno (stdin), &OldAttr)){
    NewAttr = OldAttr;
    NewAttr.c_lflag &= ~ICANON;
    NewAttr.c_lflag &= ~ECHO;
    tcsetattr (fileno (stdin), TCSAFLUSH, &NewAttr);
    AttrChanged = !0;
  }
#endif

  for (;;) {
    chr=getchar();
    if (waitFor) {
      if (chr==-1 ||
          chr==CBANKING_CHAR_ABORT ||
          chr==CBANKING_CHAR_ENTER ||
          chr==waitFor)
        break;
    }
    else
      break;
  }

#ifdef HAVE_TERMIOS_H
  /* re-enable canonical mode (if previously disabled) */
  if (AttrChanged)
    tcsetattr (fileno (stdin), TCSADRAIN, &OldAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif

  return chr;
}



int CBanking__input(AB_BANKING *ab,
                    GWEN_TYPE_UINT32 flags,
                    char *buffer,
                    int minLen,
                    int maxLen){
#ifdef HAVE_TERMIOS_H
  struct termios OldInAttr, NewInAttr;
  struct termios OldOutAttr, NewOutAttr;
  int AttrInChanged = 0;
  int AttrOutChanged = 0;
#endif
  int chr;
  unsigned int pos;
  int rv;
#if HAVE_DECL_SIGPROCMASK
  sigset_t snew, sold;
#endif

  /* if possible, disable echo from stdin to stderr during password
   * entry */
#if HAVE_DECL_SIGPROCMASK
  sigemptyset(&snew);
  sigaddset(&snew, SIGINT);
  sigaddset(&snew, SIGSTOP);
  sigprocmask(SIG_BLOCK, &snew, &sold);
#endif

#ifdef HAVE_TERMIOS_H
  if (0 == tcgetattr (fileno (stdin), &OldInAttr)){
    NewInAttr = OldInAttr;
    NewInAttr.c_lflag &= ~ECHO;
    NewInAttr.c_lflag &= ~ICANON;
    tcsetattr (fileno (stdin), TCSAFLUSH, &NewInAttr);
    AttrInChanged = !0;
  }
  if (0 == tcgetattr (fileno (stderr), &OldOutAttr)){
    NewOutAttr = OldOutAttr;
    NewOutAttr.c_lflag &= ~ICANON;
    tcsetattr (fileno (stderr), TCSAFLUSH, &NewOutAttr);
    AttrOutChanged = !0;
  }
#endif

  pos=0;
  rv=0;
  for (;;) {
    chr=getchar();
    if (chr==CBANKING_CHAR_DELETE) {
      if (pos) {
        pos--;
        fprintf(stderr, "%c %c", 8, 8);
      }
    }
    else if (chr==CBANKING_CHAR_ENTER) {
      if (minLen && pos<minLen) {
        if (pos==0 && (flags & AB_BANKING_INPUT_FLAGS_ALLOW_DEFAULT)) {
          rv=AB_Banking_MessageBox(ab,
                                   AB_BANKING_MSG_FLAGS_TYPE_INFO |
                                   AB_BANKING_MSG_FLAGS_CONFIRM_B1 |
                                   AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS,
                                   I18N("Empty Input"),
                                   I18N("Your input was empty.\n"
                                        "Do you want to use the default?"),
                                   I18N("Yes"),
                                   I18N("No"),
                                   I18N("Abort"));
          if (rv==1) {
            rv=AB_ERROR_DEFAULT_VALUE;
            break;
          }
          else {
            rv=AB_ERROR_USER_ABORT;
            break;
          }
        }
        else {
          /* too few characters */
          fprintf(stderr, "\007");
        }
      }
      else {
        fprintf(stderr, "\n");
        buffer[pos]=0;
        rv=0;
        break;
      }
    }
    else {
      if (pos<maxLen) {
        if (chr==CBANKING_CHAR_ABORT) {
          DBG_INFO(AQT_LOGDOMAIN, "User aborted");
          rv=AB_ERROR_USER_ABORT;
          break;
        }
        else {
          if ((flags & AB_BANKING_INPUT_FLAGS_NUMERIC) &&
              !isdigit(chr)) {
            /* bad character */
            fprintf(stderr, "\007");
          }
          else {
            if (flags & AB_BANKING_INPUT_FLAGS_SHOW)
              fprintf(stderr, "%c", chr);
            else
              fprintf(stderr, "*");
            buffer[pos++]=chr;
            buffer[pos]=0;
          }
        }
      }
      else {
        /* buffer full */
        fprintf(stderr, "\007");
      }
    }
  } /* for */

#ifdef HAVE_TERMIOS_H
  /* re-enable echo (if previously disabled) */
  if (AttrOutChanged)
    tcsetattr (fileno (stderr), TCSADRAIN, &OldOutAttr);
  if (AttrInChanged)
    tcsetattr (fileno (stdin), TCSADRAIN, &OldInAttr);
#endif

#if HAVE_DECL_SIGPROCMASK
  sigprocmask(SIG_BLOCK, &sold, 0);
#endif
  return rv;
}



int CBanking_InputBox(AB_BANKING *ab,
                      GWEN_TYPE_UINT32 flags,
                      const char *title,
                      const char *text,
                      char *buffer,
                      int minLen,
                      int maxLen){
  int rv;
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  CBanking_GetRawText(ab, text, tbuf);

  fprintf(stderr, "===== %s =====\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  if (flags & AB_BANKING_INPUT_FLAGS_CONFIRM) {
    for (;;) {
      char *lbuffer=0;

      lbuffer=(char*)malloc(maxLen);
      if (!lbuffer) {
        DBG_ERROR(AQT_LOGDOMAIN, "Not enough memory for %d bytes", maxLen);
        return AB_ERROR_INVALID;
      }
      fprintf(stderr, "Input: ");
      rv=CBanking__input(ab, flags, lbuffer, minLen, maxLen);
      if (rv) {
        free(lbuffer);
        return rv;
      }

      fprintf(stderr, "Again: ");
      rv=CBanking__input(ab, flags, buffer, minLen, maxLen);
      if (rv) {
        free(lbuffer);
        return rv;
      }
      if (strcmp(lbuffer, buffer)!=0) {
        fprintf(stderr,
                "ERROR: Entries do not match, please try (again or abort)\n");
      }
      else {
        rv=0;
        break;
      }

    } /* for */
  }
  else {
    fprintf(stderr, "Input: ");
    rv=CBanking__input(ab, flags, buffer, minLen, maxLen);
  }

  return rv;
}




GWEN_TYPE_UINT32 CBanking_ShowBox(AB_BANKING *ab,
                                  GWEN_TYPE_UINT32 flags,
                                  const char *title,
                                  const char *text){
  GWEN_BUFFER *tbuf;
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);
  CBanking_GetRawText(ab, text, tbuf);

  fprintf(stderr, "----- %s -----\n", title);
  fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
  tbuf=0;

  return ++(cb->nextBoxId);
}



void CBanking_HideBox(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  /* nothing do do */
}



GWEN_TYPE_UINT32 CBanking_ProgressStart(AB_BANKING *ab,
                                        const char *title,
                                        const char *text,
                                        GWEN_TYPE_UINT32 total){
  CBANKING *cb;
  CBANKING_PROGRESS *pr;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  pr=CBankingProgress_new(ab, ++(cb->nextProgressId), title, text, total);
  assert(pr);
  CBankingProgress_List_Insert(pr, cb->progressList);
  return CBankingProgress_GetId(pr);
}



CBANKING_PROGRESS *CBanking__findProgress(AB_BANKING *ab,
                                          GWEN_TYPE_UINT32 id) {
  CBANKING *cb;
  CBANKING_PROGRESS *pr;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  pr=CBankingProgress_List_First(cb->progressList);
  if (id==0)
    return pr;
  while(pr) {
    if (CBankingProgress_GetId(pr)==id)
      break;
    pr=CBankingProgress_List_Next(pr);
  } /* while */

  return pr;
}



int CBanking_ProgressAdvance(AB_BANKING *ab,
                             GWEN_TYPE_UINT32 id,
                             GWEN_TYPE_UINT32 progress){
  CBANKING *cb;
  CBANKING_PROGRESS *pr;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  pr=CBanking__findProgress(ab, id);
  if (!pr) {
    DBG_INFO(AQT_LOGDOMAIN, "Progress \"%d\" not found", id);
    return AB_ERROR_INVALID;
  }

  return CBankingProgress_Advance(pr, progress);
}



int CBanking_ProgressLog(AB_BANKING *ab,
                         GWEN_TYPE_UINT32 id,
                         AB_BANKING_LOGLEVEL level,
                         const char *text){
  CBANKING *cb;
  CBANKING_PROGRESS *pr;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  pr=CBanking__findProgress(ab, id);
  if (!pr) {
    DBG_INFO(AQT_LOGDOMAIN, "Progress \"%d\" not found", id);
    return AB_ERROR_INVALID;
  }

  return CBankingProgress_Log(pr, level, text);
}



int CBanking_ProgressEnd(AB_BANKING *ab, GWEN_TYPE_UINT32 id){
  CBANKING *cb;
  CBANKING_PROGRESS *pr;
  int rv;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  pr=CBanking__findProgress(ab, id);
  if (!pr) {
    DBG_ERROR(AQT_LOGDOMAIN, "Progress \"%d\" not found", id);
    return AB_ERROR_INVALID;
  }

  rv=CBankingProgress_End(pr);
  CBankingProgress_List_Del(pr);
  CBankingProgress_free(pr);
  return rv;
}



void CBanking_SetPinDb(AB_BANKING *ab, GWEN_DB_NODE *dbPins) {
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  GWEN_DB_Group_free(cb->dbPins);
  cb->dbPins=dbPins;
}



int CBanking_GetPin(AB_BANKING *ab,
                    GWEN_TYPE_UINT32 flags,
                    const char *token,
                    const char *title,
                    const char *text,
                    char *buffer,
                    int minLen,
                    int maxLen) {
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  if (cb->dbPins) {
    GWEN_BUFFER *nbuf;
    const char *s;

    nbuf=GWEN_Buffer_new(0, 256, 0, 1);
    if (GWEN_Text_EscapeToBuffer(token, nbuf)) {
      DBG_ERROR(AQT_LOGDOMAIN, "Error escaping token name");
      return -1;
    }
    s=GWEN_DB_GetCharValue(cb->dbPins, GWEN_Buffer_GetStart(nbuf), 0, 0);
    GWEN_Buffer_free(nbuf);
    if (s && *s) {
      int i;

      i=strlen(s);
      if (i>=minLen && i<=maxLen) {
        strncpy(buffer, s, maxLen);
        return 0;
      }
    }
  }
  return AB_Banking_InputBox(ab,
                             flags,
                             title,
                             text,
                             buffer,
                             minLen,
                             maxLen);
}



int CBanking_GetIsNonInteractive(const AB_BANKING *ab) {
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  return cb->nonInteractive;
}



void CBanking_SetIsNonInteractive(AB_BANKING *ab, int i) {
  CBANKING *cb;

  assert(ab);
  cb=GWEN_INHERIT_GETDATA(AB_BANKING, CBANKING, ab);
  assert(cb);

  cb->nonInteractive=i;
}







