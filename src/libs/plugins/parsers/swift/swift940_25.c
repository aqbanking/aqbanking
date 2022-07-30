/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2022 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift940_25.h"

#include <aqbanking/error.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



int AHB_SWIFT940_Parse_25(const AHB_SWIFT_TAG *tg, uint32_t flags, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg)
{
  const char *p;
  const char *p2;

  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p && *p==32)
    p++;
  if (*p==0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Tag 25 is empty");
    return 0;
  }

  p2=strchr(p, '/');
  if (p2) {
    char *s;

    /* "BLZ/Konto" */
    s=(char *)GWEN_Memory_malloc(p2-p+1);
    memmove(s, p, p2-p+1);
    s[p2-p]=0;
    AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "localBankCode", s);
    GWEN_Memory_dealloc(s);
    p=p2+1;
  }

  /* Skip leading whitespaces */
  while (*p && *p==32)
    p++;

  if (*p) {
    char *s;
    int ll;

    /* Reaching this point, the remainder is at least 1 byte long. */
    p2 = p + strlen(p) - 1;

    /* Remove trailing whitespaces. */
    while ((*p2 == 32) && (p2>p))
      p2--;

    /* p2 now points to the last non-space character (or the beginning of the string),
     * so the total size without the trailing zero is (p2-p)+1
     */
    ll=(p2-p)+1;
    s=(char *)GWEN_Memory_malloc(ll+1); /* account for trailing zero */
    memmove(s, p, ll);                  /* copy string without trailing zero */
    s[ll]=0;                            /* ensure terminating zero */
    AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccountNumber", s);
    GWEN_Memory_dealloc(s);
  }
  else {
    DBG_INFO(AQBANKING_LOGDOMAIN, "LocalAccountNumber is empty (%s)", p);
    AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "localAccountNumber", p);
  }

  return 0;
}



