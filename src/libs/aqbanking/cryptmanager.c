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


#include "cryptmanager_p.h"
#include "i18n_l.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>

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
  GWEN_CryptManager_SetBeginEnterPinFn(cm, AB_CryptManager_BeginEnterPin);
  GWEN_CryptManager_SetEndEnterPinFn(cm, AB_CryptManager_EndEnterPin);
  GWEN_CryptManager_SetInsertTokenFn(cm, AB_CryptManager_InsertToken);
  GWEN_CryptManager_SetInsertCorrectTokenFn(cm,
					    AB_CryptManager_InsertCorrectToken);
  GWEN_CryptManager_SetShowMessageFn(cm, AB_CryptManager_ShowMessage);

  return cm;
}



void AB_CryptManager_FreeData(void *bp, void *p) {
  AB_CRYPTMANAGER *bcm;

  bcm=(AB_CRYPTMANAGER *)p;
  GWEN_FREE_OBJECT(bcm);
}



int AB_CryptManager_GetPin(GWEN_PLUGIN_MANAGER *cm,
                           GWEN_CRYPTTOKEN *token,
                           GWEN_CRYPTTOKEN_PINTYPE pt,
			   GWEN_CRYPTTOKEN_PINENCODING pe,
			   GWEN_TYPE_UINT32 flags,
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
  GWEN_TYPE_UINT32 bflags=0;

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
    bflags|=AB_BANKING_INPUT_FLAGS_NUMERIC;
  }

  if (flags & GWEN_CRYPTTOKEN_GETPIN_FLAGS_ALLOW_DEFAULT)
    bflags|=AB_BANKING_INPUT_FLAGS_ALLOW_DEFAULT;

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
    bflags|=AB_BANKING_INPUT_FLAGS_CONFIRM;
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
  }
  else {
    rv=AB_Banking_InputBox(bcm->banking,
                           bflags,
			   I18N("Enter Password"),
                           buffer,
			   notunsigned_pwbuffer,
			   minLength,
			   maxLength);
  }

  if (rv) {
    free(notunsigned_pwbuffer);
    if (rv==AB_ERROR_USER_ABORT)
      return GWEN_ERROR_USER_ABORTED;
    else if (rv==AB_ERROR_DEFAULT_VALUE)
      return GWEN_ERROR_CT_DEFAULT_PIN;
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
    memset(notunsigned_pwbuffer, '\0', maxLength+1);
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




int AB_CryptManager_BeginEnterPin(GWEN_PLUGIN_MANAGER *cm,
				  GWEN_CRYPTTOKEN *token,
				  GWEN_CRYPTTOKEN_PINTYPE pt) {
  AB_CRYPTMANAGER *bcm;
  char buffer[512];

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  snprintf(buffer, sizeof(buffer)-1,
	   I18N("Please enter your PIN into the card reader."
		"<html>"
		"Please enter your PIN into the card reader."
		"</html>"));
  bcm->showBoxId=AB_Banking_ShowBox(bcm->banking,
				    AB_BANKING_SHOWBOX_FLAGS_BEEP,
				    I18N("Secure PIN Input"),
				    buffer);
  return 0;
}



int AB_CryptManager_EndEnterPin(GWEN_PLUGIN_MANAGER *cm,
                                GWEN_CRYPTTOKEN *token,
                                GWEN_CRYPTTOKEN_PINTYPE pt,
				int ok) {
  AB_CRYPTMANAGER *bcm;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  AB_Banking_HideBox(bcm->banking, bcm->showBoxId);
  bcm->showBoxId=0;

  return 0;
}



int AB_CryptManager_InsertToken(GWEN_PLUGIN_MANAGER *cm,
				GWEN_CRYPTTOKEN *token) {
  AB_CRYPTMANAGER *bcm;
  int rv;
  char buffer[512];

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  if (GWEN_CryptToken_GetDeviceType(token)==GWEN_CryptToken_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the security disc\nfor %s"
		  "<html>"
		  "Please insert the security disc for <i>%s</i>"
		  "</html>"),
	     GWEN_CryptToken_GetDescriptiveName(token),
	     GWEN_CryptToken_GetDescriptiveName(token));
  else
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the chip card\nfor %s"
		  "<html>"
		  "Please insert the chip card for <i>%s</i>"
		  "</html>"),
	     GWEN_CryptToken_GetDescriptiveName(token),
	     GWEN_CryptToken_GetDescriptiveName(token));

  rv=AB_Banking_MessageBox(bcm->banking,
			   AB_BANKING_MSG_FLAGS_TYPE_WARN |
			   AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			   AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			   I18N("Insert Medium"),
			   buffer,
			   I18N("OK"), I18N("Abort"), 0);
  if (rv==2) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "User aborted");
    AB_Banking_MessageBox(bcm->banking,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			  AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			  I18N("User Abort"),
			  I18N("Action aborted by user."),
			  I18N("Dismiss"), 0, 0);
    return GWEN_ERROR_USER_ABORTED;
  }
  else if (rv!=1) {
    AB_Banking_MessageBox(bcm->banking,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			  AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			  I18N("Error"),
			  I18N("An internal error occurred."),
			  I18N("Dismiss"), 0, 0);
    return -1;
  }

  return 0;
}



int AB_CryptManager_InsertCorrectToken(GWEN_PLUGIN_MANAGER *cm,
				       GWEN_CRYPTTOKEN *token) {
  AB_CRYPTMANAGER *bcm;
  int rv;
  char buffer[512];

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  buffer[0]=0;
  buffer[sizeof(buffer)-1]=0;

  if (GWEN_CryptToken_GetDeviceType(token)==GWEN_CryptToken_Device_File)
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the correct security disc\nfor %s"
		  "<html>"
		  "Please insert the <b>correct</b> security disc for "
		  "<i>%s</i>"
		  "</html>"),
	     GWEN_CryptToken_GetDescriptiveName(token),
	     GWEN_CryptToken_GetDescriptiveName(token));
  else
    snprintf(buffer, sizeof(buffer)-1,
	     I18N("Please insert the correct chip card\nfor %s"
		  "<html>"
		  "Please insert the <b>correct</b> chip card for <i>%s</i>"
		  "</html>"),
	     GWEN_CryptToken_GetDescriptiveName(token),
	     GWEN_CryptToken_GetDescriptiveName(token));

  rv=AB_Banking_MessageBox(bcm->banking,
			   AB_BANKING_MSG_FLAGS_TYPE_WARN |
			   AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			   AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			   I18N("Insert Medium"),
			   buffer,
			   I18N("OK"), I18N("Abort"), 0);
  if (rv==2) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "User aborted");
    AB_Banking_MessageBox(bcm->banking,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			  AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			  I18N("User Abort"),
			  I18N("Action aborted by user."),
			  I18N("Dismiss"), 0, 0);
    return GWEN_ERROR_USER_ABORTED;
  }
  else if (rv!=1) {
    AB_Banking_MessageBox(bcm->banking,
			  AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			  AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			  AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			  I18N("Error"),
			  I18N("An internal error occurred."),
			  I18N("Dismiss"), 0, 0);
    return -1;
  }

  return 0;
}



int AB_CryptManager_ShowMessage(GWEN_PLUGIN_MANAGER *cm,
				GWEN_CRYPTTOKEN *token,
				const char *title,
				const char *msg) {
  AB_CRYPTMANAGER *bcm;

  assert(cm);
  bcm=GWEN_INHERIT_GETDATA(GWEN_PLUGIN_MANAGER, AB_CRYPTMANAGER, cm);
  assert(bcm);

  AB_Banking_MessageBox(bcm->banking,
			AB_BANKING_MSG_FLAGS_TYPE_ERROR |
			AB_BANKING_MSG_FLAGS_SEVERITY_DANGEROUS |
			AB_BANKING_MSG_FLAGS_CONFIRM_B1,
			title,
			msg,
			I18N("Dismiss"), 0, 0);
  return 0;
}















