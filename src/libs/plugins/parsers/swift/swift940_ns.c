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

#include "swift940_ns.h"

#include <aqbanking/error.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



int AHB_SWIFT940_Parse_NS(const AHB_SWIFT_TAG *tg, uint32_t flags, GWEN_DB_NODE *data, GWEN_DB_NODE *cfg)
{
  const char *p;
  const char *p2;

  /* TODO: Use AHB_SWIFT_ParseSubTags */
  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);

  while (*p) {
    int code;

    code=0;
    /* read code */
    if (strlen(p)>2) {
      if (isdigit(p[0]) && isdigit(p[1])) {
        /* starts with a two digit number */
        code=(((p[0]-'0')*10) + (p[1]-'0'));
        p+=2;
      }
    }

    /* search for end of line */
    p2=p;
    while (*p2 && *p2!=10 && *p2!=13)
      p2++;

    if (code==0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "No code in line");
      p=p2;
    }
    else {
      int len;

      len=p2-p;
      if (len<1 || (len==1 && *p=='/')) {
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Empty field %02d", code);
      }
      else {
        char *s;

        s=(char *)GWEN_Memory_malloc(len+1);
        memmove(s, p, len);
        s[len]=0;
        DBG_DEBUG(AQBANKING_LOGDOMAIN, "Got his field: %02d: %s", code, s);

        switch (code) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
          AHB_SWIFT_SetCharValue(data, flags, "purpose", s);
          break;

        case 15: /* Auftraggeber1 */
        case 16: /* Auftraggeber2 */
          AHB_SWIFT_SetCharValue(data, flags, "localName", s);
          break;

        case 17: /* Buchungstext */
          AHB_SWIFT_SetCharValue(data, flags, "transactionText", s);
          break;

        case 18: /* Primanota */
          AHB_SWIFT_SetCharValue(data, flags, "primanota", s);
          break;

        case 19: /* Uhrzeit der Buchung */
        case 20: /* Anzahl der Sammlerposten */
        case 33: /* BLZ Auftraggeber */
        case 34: /* Konto Auftraggeber */
          break;

        default: /* ignore all other fields (if any) */
          DBG_WARN(AQBANKING_LOGDOMAIN,
                   "Unknown :NS: field \"%02d\" (%s) (%s)",
                   code, s,
                   AHB_SWIFT_Tag_GetData(tg));
          break;
        }
        GWEN_Memory_dealloc(s);
      }
      p=p2;
    }

    if (*p==10)
      p++;
    if (*p==13)
      p++;
    if (*p==10)
      p++;
  } /* while */

  return 0;
}



