/* Included by listtrans.c; optional JSON-lines output leaves default CLI formatting unchanged. */
#include "../../libs/aqbanking/camt_batch_json.h"

/** Check whether preserved CAMT metadata is a single-line valid UTF-8 JSON object without JSONL framing control chars.
 * @param s Preserved CAMT JSON string. @return Nonzero if valid inline JSON object.
 */
static int aqbValidInlineJsonObject(const char *s) {
  const unsigned char *p=(const unsigned char *)s;
  int depth=0, inStr=0, esc=0;
  if (!p || *p!='{') return 0;
  if (aqbCountUtf8Chars(s)<0) return 0;
  for (; *p; p++) {
    if (*p<0x20 || *p==0x7f) return 0;
    if (depth==0 && p!=(const unsigned char *)s) return 0;
    if (inStr) {
      if (esc) esc=0;
      else if (*p=='\\') esc=1;
      else if (*p=='"') inStr=0;
    } else {
      if (*p=='"') inStr=1;
      else if (*p=='{') depth++;
      else if (*p=='}') {
        depth--;
        if (depth<0) return 0;
      }
    }
  }
  return !inStr && !esc && depth==0;
}

/** Serialize a booking and preserved CAMT metadata as a single JSON line.
 * @param t Native booking. @param b Output buffer. @return Zero on success.
 */
static int aqbTransactionJson(const AB_TRANSACTION *t, GWEN_BUFFER *b) {
  const char *s;
  const char *localBankCode;
  const char *localAccountNumber;
  GWEN_BUFFER *value=GWEN_Buffer_new(0, 256, 0, 1);
  const GWEN_DATE *date=AB_Transaction_GetDate(t);
  const GWEN_DATE *vdate=AB_Transaction_GetValutaDate(t);
  int rv;
  if (!value) return GWEN_ERROR_MEMORY_FULL;
  s=AB_Transaction_GetCamtDetailsJson(t);
  if (s && *s && !aqbValidInlineJsonObject(s)) {
    GWEN_Buffer_free(value);
    return GWEN_ERROR_BAD_DATA;
  }
  if (!date) date=AB_Transaction_GetValutaDate(t);
  if (date && GWEN_Date_toStringWithTemplate(date, "DD.MM.YYYY", value)<0) {
    GWEN_Buffer_free(value);
    return GWEN_ERROR_INVALID;
  }
  if (GWEN_Buffer_AppendString(b, "{\"date\":")<0 ||
      aqbJsonString(b, GWEN_Buffer_GetStart(value))<0) {
    GWEN_Buffer_free(value);
    return GWEN_ERROR_BUFFER_OVERFLOW;
  }
  GWEN_Buffer_Reset(value);
  if (vdate && GWEN_Date_toStringWithTemplate(vdate, "DD.MM.YYYY", value)<0) {
    GWEN_Buffer_free(value);
    return GWEN_ERROR_INVALID;
  }
  rv=aqbJsonField(b, "valutaDate", GWEN_Buffer_GetStart(value));
  if (rv<0) {
    GWEN_Buffer_free(value);
    return rv;
  }
  GWEN_Buffer_Reset(value);
  if (AB_Transaction_GetValue(t)) AB_Value_toHumanReadableString(AB_Transaction_GetValue(t), value, 2, 0);
  localBankCode=AB_Transaction_GetLocalBankCode(t);
  localAccountNumber=AB_Transaction_GetLocalAccountNumber(t);
  rv=aqbJsonField(b, "amount", GWEN_Buffer_GetStart(value));
  if (rv==0) rv=aqbJsonField(b, "localBankCode", localBankCode?localBankCode:"");
  if (rv==0) rv=aqbJsonField(b, "localAccountNumber", localAccountNumber?localAccountNumber:"");
  if (rv==0) rv=aqbJsonField(b, "localIban", AB_Transaction_GetLocalIban(t)?AB_Transaction_GetLocalIban(t):"");
  if (rv==0) rv=aqbJsonField(b, "remoteName", AB_Transaction_GetRemoteName(t)?AB_Transaction_GetRemoteName(t):"");
  if (rv==0) rv=aqbJsonField(b, "remoteIban", AB_Transaction_GetRemoteIban(t)?AB_Transaction_GetRemoteIban(t):"");
  if (rv==0) rv=aqbJsonField(b, "purpose", AB_Transaction_GetPurpose(t)?AB_Transaction_GetPurpose(t):"");
  if (rv==0 && (GWEN_Buffer_AppendString(b, ",\"camt\":")<0 ||
                GWEN_Buffer_AppendString(b, (s && *s)?s:"null")<0 ||
                GWEN_Buffer_AppendString(b, "}")<0)) {
    rv=GWEN_ERROR_BUFFER_OVERFLOW;
  }
  GWEN_Buffer_free(value);
  return rv;
}
