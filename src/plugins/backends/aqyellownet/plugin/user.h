/***************************************************************************
 $RCSfile: provider_p.h,v $
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AY_USER_H
#define AY_USER_H


#include <aqyellownet/aqyellownet.h>
#include <aqbanking/user.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  AY_User_LanguageUnknown=-1,
  AY_User_LanguageGerman=1,
  AY_User_LanguageFrench,
  AY_User_LanguageItalian,
  AY_User_LanguageEnglish
} AY_USER_LANGUAGE;
AQYELLOWNET_API AY_USER_LANGUAGE AY_User_Language_fromString(const char *s);
AQYELLOWNET_API const char *AY_User_Language_toString(AY_USER_LANGUAGE v);

AQYELLOWNET_API AY_USER_LANGUAGE AY_User_GetLanguage(const AB_USER *u);
AQYELLOWNET_API void AY_User_SetLanguage(AB_USER *u, AY_USER_LANGUAGE d);

#ifdef __cplusplus
} /* __cplusplus */
#endif


#endif /* AB_USER_H */
