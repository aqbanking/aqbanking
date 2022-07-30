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

#include "swift940_60.h"


#include <aqbanking/error.h>
#include <aqbanking/backendsupport/imexporter_be.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/gui.h>

#include <ctype.h>



/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */




/* ------------------------------------------------------------------------------------------------
 * implementations
 * ------------------------------------------------------------------------------------------------
 */

int AHB_SWIFT940_Parse_60_62(const AHB_SWIFT_TAG *tg,
                             uint32_t flags,
                             GWEN_DB_NODE *data,
                             GWEN_DB_NODE *cfg)
{
  const char *p;
  const char *p2;
  char *s;
  char buffer[32];
  unsigned int bleft;
  int d1, d2, d3;
  int neg;
  GWEN_DATE *dt;

  p=AHB_SWIFT_Tag_GetData(tg);
  assert(p);
  bleft=strlen(p);

  /* credit/debit mark (M) */
  if (bleft<2) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value string (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "SWIFT: Bad value string");
    return -1;
  }
  neg=0;
  if (*p=='D' || *p=='d')
    neg=1;
  p++;
  bleft--;

  /* date (M) */
  if (bleft<6) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing date (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "SWIFT: Missing date");
    return -1;
  }
  d1=((p[0]-'0')*10) + (p[1]-'0');
  if (d1>AHB_SWIFT_CENTURY_CUTOFF_YEAR)
    d1+=1900;
  else
    d1+=2000;
  d2=((p[2]-'0')*10) + (p[3]-'0');
  d3=((p[4]-'0')*10) + (p[5]-'0');

  dt=GWEN_Date_fromGregorian(d1, d2, d3);
  assert(dt);
  GWEN_DB_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "date", GWEN_Date_GetString(dt));
  GWEN_Date_free(dt);

  p+=6;
  bleft-=6;

  /* currency (M) */
  if (!isdigit(*p)) {
    /* only read currency if this is not part of the value (like in some
     * swiss MT940) */
    if (bleft<3) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing currency (%s)", p);
      GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                           "SWIFT: Missing currency");
      return -1;
    }
    memmove(buffer, p, 3);
    buffer[3]=0;
    AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "value/currency", buffer);
    p+=3;
    bleft-=3;
  }

  /* value (M) */
  if (bleft<1) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Missing value (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "SWIFT: Missing value");
    return -1;
  }

  p2=p;
  while (*p2 && (isdigit(*p2) || *p2==','))
    p2++;
  if (p2==p) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Bad value (%s)", p);
    GWEN_Gui_ProgressLog(0, GWEN_LoggerLevel_Error,
                         "SWIFT: Bad value");
    return -1;
  }
  s=(char *)GWEN_Memory_malloc(p2-p+1+(neg?1:0));
  if (neg) {
    s[0]='-';
    memmove(s+1, p, p2-p+1);
    s[p2-p+1]=0;
  }
  else {
    memmove(s, p, p2-p+1);
    s[p2-p]=0;
  }
  AHB_SWIFT_SetCharValue(data, GWEN_DB_FLAGS_OVERWRITE_VARS, "value/value", s);
  GWEN_Memory_dealloc(s);
  /*bleft-=p2-p;*/
  /*p=p2;*/

  return 0;
}



