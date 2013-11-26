/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQEBICS_AQEBICS_H
#define AQEBICS_AQEBICS_H



#include <aqbanking/system.h>
#include <gwenhywfar/types.h>


#ifdef BUILDING_AQEBICS
# /* building AqEBICS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQEBICS_API __declspec (dllexport)
#   else /* if __declspec */
#     define AQEBICS_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   ifdef GCC_WITH_VISIBILITY_ATTRIBUTE
#     define AQEBICS_API __attribute__((visibility("default")))
#   else
#     define AQEBICS_API
#   endif
# endif
#else
# /* not building AqEBICS */
# if AQBANKING_SYS_IS_WINDOWS
#   /* for windows */
#   ifdef __declspec
#     define AQEBICS_API __declspec (dllimport)
#   else /* if __declspec */
#     define AQEBICS_API
#   endif /* if NOT __declspec */
# else
#   /* for non-win32 */
#   define AQEBICS_API
# endif
#endif


#define EBC_PROVIDER_NAME "AQEBICS"

#define AQEBICS_LOGDOMAIN "aqebics"

#define EBICS_BUFFER_MAX_HARD_LIMIT (256*1024*1024)

typedef uint32_t EB_RC;


#define AQEBIC_RC_GET_CLASS(x)  ((x>>4) & 0xff)
#define AQEBIC_RC_GET_GRP(x)    ((x>>2) & 0xf)
#define AQEBIC_RC_GET_CODE(x)    (x & 0xff)

#define AQEBIC_RC_CLASS_INFO    0
#define AQEBIC_RC_CLASS_NOTICE  1
#define AQEBIC_RC_CLASS_WARNING 3
#define AQEBIC_RC_CLASS_ERROR   6
#define AQEBIC_RC_CLASS_FATAL   9

#define AQEBICS_RC_IS_INFO(x) (AQEBIC_RC_GET_CLASS(x)==AQEBIC_RC_CLASS_INFO)
#define AQEBICS_RC_IS_NOTICE(x) (AQEBIC_RC_GET_CLASS(x)==AQEBIC_RC_CLASS_NOTICE)
#define AQEBICS_RC_IS_WARNING(x) (AQEBIC_RC_GET_CLASS(x)==AQEBIC_RC_CLASS_WARNING)
#define AQEBICS_RC_IS_ERROR(x)  (AQEBIC_RC_GET_CLASS(x)==AQEBIC_RC_CLASS_ERROR)
#define AQEBICS_RC_IS_FATAL(x)  (AQEBIC_RC_GET_CLASS(x)==AQEBIC_RC_CLASS_FATAL)

#define AQEBICS_RC_GRP_NONE  0
#define AQEBICS_RC_GRP_TRANS 1
#define AQEBICS_RC_GRP_KEY   2
#define AQEBICS_RC_GRP_PRE   3

#define AQEBICS_RC_NEW(class, grp, code) \
  (((class & 0xff)<<4) |\
  0x1000 | \
  ((grp & 0xf)<<2) |\
  (code & 0xff))


#define EB_RC_OK                           0x000000
#define EB_RC_DOWNLOAD_POSTPROCESS_DONE    0x011000
#define EB_RC_DOWNLOAD_POSTPROCESS_SKIPPED 0x011001
#define EB_RC_TX_SEGMENT_NUMBER_UNDERRUN   0x011101
#define EB_RC_AUTHENTICATION_FAILED        0x061001
#define EB_RC_INVALID_REQUEST              0x061002
#define EB_RC_INTERNAL_ERROR               0x061099
#define EB_RC_TX_RECOVERY_SYNC             0x061101
#define EB_RC_INVALID_USER_OR_STATE        0x091002
#define EB_RC_USER_UNKNOWN                 0x091003
#define EB_RC_INVALID_USER_STATE           0x091004
#define EB_RC_INVALID_ORDER_TYPE           0x091005
#define EB_RC_UNSUPPORTED_ORDER_TYPE       0x091006
#define EB_RC_USER_AUTHENTICATION_REQUIRED 0x091007
#define EB_RC_BANK_PUBKEY_UPDATE_REQUIRED  0x091008
#define EB_RC_SEGMENT_SIZE_EXCEEDED        0x091009
#define EB_RC_TX_UNKNOWN_TXID              0x091101
#define EB_RC_TX_ABORT                     0x091102
#define EB_RC_TX_MESSAGE_REPLAY            0x091103
#define EB_RC_TX_SEGMENT_NUMBER_EXCEEDED   0x091104
#define EB_RC_AUTHORISATION_FAILED         0x090003
#define EB_RC_NO_DOWNLOAD_DATA_AVAILABLE   0x090005


/* fachliche codes (key management) */
#define EB_RC_KEYMGMT_UNSUPPORTED_VERSION_SIGNATURE      0x091201
#define EB_RC_KEYMGMT_UNSUPPORTED_VERSION_AUTHENTICATION 0x091202
#define EB_RC_KEYMGMT_UNSUPPORTED_VERSION_ENCRYPTION     0x091203
#define EB_RC_KEYMGMT_KEYLENGTH_ERROR_SIGNATURE          0x091204
#define EB_RC_KEYMGMT_KEYLENGTH_ERROR_AUTHENTICATION     0x091205
#define EB_RC_KEYMGMT_KEYLENGTH_ERROR_ENCRYPTION         0x091206
#define EB_RC_KEYMGMT_NO_X509_SUPPORT                    0x091207

#endif


