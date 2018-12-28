/***************************************************************************
 $RCSfile: adminjobs.h,v $
                             -------------------
    cvs         : $Id: adminjobs.h,v 1.3 2006/01/13 13:59:58 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "zip.h"

#include <gwenhywfar/debug.h>

#include <zlib.h>



int EB_Zip_Deflate(const char *ptr, unsigned int size, GWEN_BUFFER *buf) {
  z_stream z;
  char outbuf[512];
  int rv;
  int mode;

  z.next_in=(unsigned char*)ptr;
  z.avail_in=size;
  z.next_out=(unsigned char*)outbuf;
  z.avail_out=sizeof(outbuf);
  z.zalloc=Z_NULL;
  z.zfree=Z_NULL;

  rv=deflateInit(&z, (unsigned int) 5);
  if (rv!=Z_OK) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error on deflateInit (%d)", rv);
    return -1;
  }

  mode=Z_NO_FLUSH;
  for(;;) {
    rv=deflate(&z, mode);
    if (rv==Z_STREAM_END)
      break;
    if (rv!=Z_OK) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error on deflate (%d)", rv);
      deflateEnd(&z);
      return -1;
    }
    if (z.avail_in==0)
      mode=Z_FINISH;
    if (z.avail_out==0) {
      GWEN_Buffer_AppendBytes(buf, outbuf, (uint32_t) sizeof(outbuf));
      z.next_out=(unsigned char*)outbuf;
      z.avail_out=sizeof(outbuf);
    }
  }
  if (z.avail_out!=sizeof(outbuf)) {
    GWEN_Buffer_AppendBytes(buf, outbuf, (uint32_t)(sizeof(outbuf)-z.avail_out));
    z.next_out=(unsigned char*)outbuf;
    z.avail_out=sizeof(outbuf);
  }

  deflateEnd(&z);

  return 0;
}



int EB_Zip_Inflate(const char *ptr, unsigned int size, GWEN_BUFFER *buf) {
  z_stream z;
  char outbuf[512];
  int rv;
  int mode;

  z.next_in=(unsigned char*)ptr;
  z.avail_in=size;
  z.next_out=(unsigned char*)outbuf;
  z.avail_out=sizeof(outbuf);
  z.zalloc=Z_NULL;
  z.zfree=Z_NULL;

  rv=inflateInit(&z);
  if (rv!=Z_OK) {
    DBG_ERROR(AQEBICS_LOGDOMAIN, "Error on deflateInit (%d)", rv);
    return -1;
  }

  mode=Z_NO_FLUSH;
  for(;;) {
    rv=inflate(&z, mode);
    if (rv==Z_STREAM_END)
      break;
    if (rv!=Z_OK) {
      DBG_ERROR(AQEBICS_LOGDOMAIN, "Error on inflate (%d)", rv);
      deflateEnd(&z);
      return -1;
    }
    if (z.avail_in==0)
      mode=Z_FINISH;
    if (z.avail_out==0) {
      GWEN_Buffer_AppendBytes(buf, outbuf, sizeof(outbuf));
      z.next_out=(unsigned char*)outbuf;
      z.avail_out=sizeof(outbuf);
    }
  }
  if (z.avail_out!=sizeof(outbuf)) {
    GWEN_Buffer_AppendBytes(buf, outbuf, (uint32_t) (sizeof(outbuf)-z.avail_out));
    z.next_out=(unsigned char*)outbuf;
    z.avail_out=sizeof(outbuf);
  }

  inflateEnd(&z);

  return 0;
}




