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


#ifndef AQYELLOWNET_AQYELLOWNET_H
#define AQYELLOWNET_AQYELLOWNET_H


#include <aqbanking/system.h>

#ifdef BUILDING_AQYELLOWNET
# /* building AqYELLOWNET */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQYELLOWNET_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQYELLOWNET_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQYELLOWNET_API __attribute__((visibility("default")))
#   else
#     define AQYELLOWNET_API
#   endif
# endif
#else
# /* not building AqYELLOWNET */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQYELLOWNET_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQYELLOWNET_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQYELLOWNET_API
# endif
#endif


#define AQYELLOWNET_LOGDOMAIN "aqyellownet"

#define AQYELLOWNET_FILEFLAGS_ESR_ASR        0x00000001
#define AQYELLOWNET_FILEFLAGS_DEBIT_DIRECT   0x00000002
#define AQYELLOWNET_FILEFLAGS_STATEMENTS     0x00000004
#define AQYELLOWNET_FILEFLAGS_AVIS_CREDITOR  0x00000008
#define AQYELLOWNET_FILEFLAGS_AVIS_DEBITOR   0x00000010
#define AQYELLOWNET_FILEFLAGS_AVIS_DEPOT     0x00000020

#define AY_URL_EINST_A \
  "https://www.yellownet.ch/neu/onl_kdl_sess.sw_einst_a"
#define AY_URL_EINST_B \
  "https://www.yellownet.ch/neu/onl_kdl_sess.sw_einst_b"
#define AY_URL_UPLOAD \
  "https://www.yellownet.ch/cgi/upldsoftw_2"
#define AY_URL_DOWNLOAD \
  "https://www.yellownet.ch/cgi/sw_file_dwld"
#define AY_URL_GETFILELIST \
  "https://www.yellownet.ch/neu/onl_kdl_file.sw_file_list_get"


#endif /* AQYELLOWNET_AQYELLOWNET_H */

