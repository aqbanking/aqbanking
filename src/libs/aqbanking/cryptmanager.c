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


#include "cryptmanager_p.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/gui.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>



GWEN_INHERIT(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER)



GWEN_PLUGIN_MANAGER *AB_CryptManager_new(AB_BANKING *ab) {
  GWEN_PLUGIN_MANAGER *cm;
  AB_CRYPTMANAGER *bcm;

  cm=GWEN_CryptManager_new();
  GWEN_NEW_OBJECT(AB_CRYPTMANAGER, bcm);
  GWEN_INHERIT_SETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm, bcm,
		       AB_CryptManager_FreeData);
  bcm->banking=ab;

  /* set virtual functions */
  GWEN_CryptManager_SetGetPinFn(cm, AB_CryptManager_GetPin);
  GWEN_CryptManager_SetSetPinStatusFn(cm, AB_CryptManager_SetPinStatus);

  return cm;
}



void GWENHYWFAR_CB AB_CryptManager_FreeData(void *bp, void *p) {
  AB_CRYPTMANAGER *bcm;

  bcm=(AB_CRYPTMANAGER *)p;
  GWEN_FREE_OBJECT(bcm);
}



int AB_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                           GWEN_CRYPTTOKEN *token,
                           GWEN_CRYPTTOKEN_PINTYPE pt,
			   GWEN_CRYPTTOKEN_PINENCODING pe,
			   uint32_t flags,
                           unsigned char *pwbuffer,
                           unsigned int minLength,
                           unsigned int maxLength,
			   unsigned int *pinLength) {
  AB_CRYPTMANAGER *bcm;
  int rv;
  const char *name;
  const char *dname;
  const char *mode;
  const char *numeric_warning = "";
  char buffer[512];
  char *notunsigned_pwbuffer;
  uint32_t bflags=0;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  dname=GWEN_CryptToken_GetDescriptiveName(token);
  if (!dname || !*dname)
    dname=GWEN_CryptToken_GetTokenName(token);

  if (pt==GWEN_CryptToken_PinType_Access)
    mode=I18N("access password");
  else if (pt==GWEN_CryptToken_PinType_Manage)
    mode=I18N("manager password");
  else
    mode=I18N("password");

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;
  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_NUMERIC) {
    numeric_warning = I18N("\nYou must only enter numbers, not letters.");
    bflags|=GWEN_GUI_INPUT_FLAGS_NUMERIC;
  }

  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_ALLOW_DEFAULT)
    bflags|=GWEN_GUI_INPUT_FLAGS_ALLOW_DEFAULT;

  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_CONFIRM) {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter a new %s for \n"
		  "%s\n"
		  "The password must be at least %d characters long.%s"
		  "<html>"
		  "Please enter a new %s for <i>%s</i>. "
		  "The password must be at least %d characters long.%s"
                  "</html>"),
             mode,
             dname,
	     minLength,
             numeric_warning,
             mode,
             dname,
	     minLength,
	     numeric_warning);
    bflags|=GWEN_GUI_INPUT_FLAGS_CONFIRM;
  }
  else {
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please enter the %s for \n"
		  "%s\n"
		  "%s<html>"
		  "Please enter the %s for <i>%s</i>.%s"
                  "</html>"),
             mode,
             dname,
             numeric_warning,
             mode,
             dname,
	     numeric_warning);
  }

  /* Allocate new buffer that is not marked as unsigned in order
     to pass the correct pointer type into AB_Banking_GetPin. */
  notunsigned_pwbuffer = (char*)malloc((maxLength+1)*sizeof(char));

  name=GWEN_CryptToken_GetTokenName(token);
  if (name) {
    GWEN_BUFFER *nbuf;

    nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
    GWEN_Buffer_AppendString(nbuf, "PASSWORD::");
    GWEN_Buffer_AppendString(nbuf, name);
    rv=AB_Banking_GetPin(bcm->banking,
                         bflags,
                         GWEN_Buffer_GetStart(nbuf),
			 I18N("Enter Password"),
			 buffer,
			 notunsigned_pwbuffer,
                         minLength,
			 maxLength);
    GWEN_Buffer_free(nbuf);
    if (rv)
      rv=GWEN_ERROR_GENERIC;
  }
  else {
    rv=GWEN_Gui_InputBox(bflags,
			 I18N("Enter Password"),
			 buffer,
			 notunsigned_pwbuffer,
			 minLength,
			 maxLength);
  }

  if (rv) {
    free(notunsigned_pwbuffer);
    if (rv==GWEN_ERROR_USER_ABORTED ||
	rv==GWEN_ERROR_DEFAULT_VALUE)
      return rv;
    return -1;
  }

  *pinLength=strlen(notunsigned_pwbuffer);
    {
      /* Copy the resulting password into the original buffer. Copy
       this byte-wise and not by strcpy() because strcpy() does
       not accept an unsigned char pointer but only a char
       pointer. (would give a "pointer differ in signedness"
       warning in gcc4.x) */
      int k;
      for (k=0; k < *pinLength; ++k)
	pwbuffer[k] = notunsigned_pwbuffer[k];
      /* The returned length of strlen() does not include the
       trailing \0, so append it extra. */
      pwbuffer[k] = '\0';
      /* Clear the temporary buffer. */
      memset(notunsigned_pwbuffer, '\0', maxLength);
    }
  free(notunsigned_pwbuffer);

  if (pe!=GWEN_CryptToken_PinEncoding_ASCII) {
    rv=GWEN_CryptToken_TransformPin(GWEN_CryptToken_PinEncoding_ASCII,
				    pe,
				    pwbuffer,
				    maxLength,
				    pinLength);
    if (rv) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }
  }

  return 0;
}



int AB_CryptManager_SetPinStatus(GWEN_PLUGIN_MANAGER *cm,
				 GWEN_CRYPTTOKEN *token,
				 GWEN_CRYPTTOKEN_PINTYPE pt,
				 GWEN_CRYPTTOKEN_PINENCODING pe,
				 uint32_t flags,
				 unsigned char *buffer,
				 unsigned int pinLength,
				 int isOk){
  AB_CRYPTMANAGER *bcm;
  const char *name;
  AB_BANKING_PINSTATUS pst;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  pst=isOk?AB_Banking_PinStatusOk:AB_Banking_PinStatusBad;

  name=GWEN_CryptToken_GetTokenName(token);
  if (name) {
    GWEN_BUFFER *nbuf;
    char pinBuffer[64];
    unsigned int newPinLength;

    nbuf=GWEN_Buffer_new(0, 256 ,0 ,1);
    GWEN_Buffer_AppendString(nbuf, "PASSWORD::");
    GWEN_Buffer_AppendString(nbuf, name);

    assert(pinLength<sizeof(pinBuffer));
    memset(pinBuffer, 0, sizeof(pinBuffer));
    memmove(pinBuffer, buffer, pinLength);
    newPinLength=pinLength;
    if (pe!=GWEN_CryptToken_PinEncoding_ASCII) {
      int rv;

      /* transfor back to ASCII */
      rv=GWEN_CryptToken_TransformPin(pe,
				      GWEN_CryptToken_PinEncoding_ASCII,
				      (unsigned char*)pinBuffer,
				      sizeof(pinBuffer),
                                      &newPinLength);
      if (rv) {
	DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
        GWEN_Buffer_free(nbuf);
	memset(pinBuffer, 0, sizeof(pinBuffer));
	return rv;
      }
    }
    AB_Banking_SetPinStatus(bcm->banking,
			    GWEN_Buffer_GetStart(nbuf),
			    pinBuffer, pst);
    GWEN_Buffer_free(nbuf);
    memset(pinBuffer, 0, sizeof(pinBuffer));
  }
  else {
    DBG_WARN(AQBANKING_LOGDOMAIN, "CryptToken has no name");
    return GWEN_ERROR_INVALID;
  }

  return 0;
}


