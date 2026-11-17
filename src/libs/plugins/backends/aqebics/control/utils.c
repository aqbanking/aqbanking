/***************************************************************************
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "globals.h"

#include <errno.h>


int readFile(const char *fname, GWEN_BUFFER *dbuf)
{
  FILE *f;

  f=fopen(fname, "rb");
  if (f) {
    while (!feof(f)) {
      uint32_t l;
      size_t s;
      char *p;

      GWEN_Buffer_AllocRoom(dbuf, 1024);
      l=GWEN_Buffer_GetMaxUnsegmentedWrite(dbuf);
      p=GWEN_Buffer_GetPosPointer(dbuf);
      s=fread(p, 1, l, f);
      if (ferror(f)) {
        DBG_INFO(AQEBICS_LOGDOMAIN,
                 "fread(%s): %s",
                 fname, strerror(errno));
        fclose(f);
        return GWEN_ERROR_IO;
      }
      if (s==0)
        break;

      GWEN_Buffer_IncrementPos(dbuf, s);
      GWEN_Buffer_AdjustUsedBytes(dbuf);
    }

    fclose(f);
    return 0;
  }
  else {
    DBG_INFO(AQEBICS_LOGDOMAIN,
             "fopen(%s): %s",
             fname, strerror(errno));
    return GWEN_ERROR_IO;
  }
}



int writeFile(FILE *f, const char *p, int len)
{
  while (len>0) {
    size_t l;
    size_t s;

    l=1024;
    if (l>(size_t)len)
      l=len;
    s=fwrite(p, 1, l, f);
    if (s==0) {
      DBG_INFO(AQEBICS_LOGDOMAIN,
               "fwrite: %s",
               strerror(errno));
      return GWEN_ERROR_IO;
    }
    p+=s;
    len-=s;
  }

  return 0;
}




