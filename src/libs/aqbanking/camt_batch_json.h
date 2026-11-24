/* Shared compact JSON text encoding; never writes statement data to diagnostic logs. */
#ifndef AQB_CAMT_BATCH_JSON_H
#define AQB_CAMT_BATCH_JSON_H
#include <gwenhywfar/buffer.h>
#include <limits.h>
#include <string.h>

/** Count UTF-8 code points strictly per RFC 3629 (NUL-safe, rejects overlongs/surrogates/>U+10FFFF).
 * @param s UTF-8 string. @return Code point count, or -1 on NULL/invalid UTF-8/overflow.
 */
static int aqbCountUtf8Chars(const char *s) {
  const unsigned char *p=(const unsigned char *)s;
  int count=0;

  if (!p)
    return -1;
  while (*p) {
    unsigned char c=p[0];
    int seqLen=0;

    if (c<0x80) {
      seqLen=1;
    }
    else if (c>=0xc2 && c<=0xdf) {
      if ((p[1]&0xc0)!=0x80)
        return -1;
      seqLen=2;
    }
    else if (c>=0xe0 && c<=0xef) {
      if ((p[1]&0xc0)!=0x80 || (p[2]&0xc0)!=0x80)
        return -1;
      if ((c==0xe0 && p[1]<0xa0) || (c==0xed && p[1]>=0xa0))
        return -1;
      seqLen=3;
    }
    else if (c>=0xf0 && c<=0xf4) {
      if ((p[1]&0xc0)!=0x80 || (p[2]&0xc0)!=0x80 || (p[3]&0xc0)!=0x80)
        return -1;
      if ((c==0xf0 && p[1]<0x90) || (c==0xf4 && p[1]>0x8f))
        return -1;
      seqLen=4;
    }
    else {
      return -1;
    }
    if (count==INT_MAX)
      return -1;
    count++;
    p+=seqLen;
  }
  return count;
}

/** Append JSON string/null without changing valid UTF-8 text.
 * Never uses unchecked GWEN_Buffer_AppendArgs; fails closed on invalid UTF-8 or buffer limits.
 * @param b Output buffer. @param s Text, or NULL. @return Zero on success, negative error on failure.
 */
static int aqbJsonString(GWEN_BUFFER *b, const char *s) {
  static const char hex[]="0123456789abcdef";
  const unsigned char *p=(const unsigned char *)s;
  if (!s) return GWEN_Buffer_AppendString(b, "null")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
  if (*s && aqbCountUtf8Chars(s)<0) return GWEN_ERROR_BAD_DATA;
  if (GWEN_Buffer_AppendString(b, "\"")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  for (; *p; p++) {
    if (*p=='"' || *p=='\\') {
      if (GWEN_Buffer_AppendString(b, "\\")<0 || GWEN_Buffer_AppendByte(b, *p)<0)
        return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    else if (*p<32 || *p==0x7f) {
      char esc[6];
      esc[0]='\\'; esc[1]='u'; esc[2]='0'; esc[3]='0';
      esc[4]=hex[(*p>>4)&0x0f]; esc[5]=hex[*p&0x0f];
      if (GWEN_Buffer_AppendBytes(b, esc, 6)<0) return GWEN_ERROR_BUFFER_OVERFLOW;
    }
    else if (GWEN_Buffer_AppendByte(b, *p)<0) {
      return GWEN_ERROR_BUFFER_OVERFLOW;
    }
  }
  return GWEN_Buffer_AppendString(b, "\"")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
}

/** Append a named text/null field after an existing JSON property.
 * @param b Output. @param name Constant property name. @param value Text/null. @return Zero on success, negative on error.
 */
static int aqbJsonField(GWEN_BUFFER *b, const char *name, const char *value) {
  int rv;
  if (GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  rv=aqbJsonString(b, name);
  if (rv<0) return rv;
  if (GWEN_Buffer_AppendString(b, ":")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  return aqbJsonString(b, value);
}
#endif
