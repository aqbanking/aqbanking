/* Included by xml.c. Preserve all CAMT entry details before schema flattening loses them. */
#include "../../../aqbanking/camt_batch_json.h"
#include <gwenhywfar/gwendate.h>
#include <ctype.h>
#include <stdio.h>

/** Read optional XML text. @param n XML parent. @param p Relative path. @return Borrowed text/null. */
static const char *aqbCamtText(GWEN_XMLNODE *n, const char *p) {
  GWEN_XMLNODE *value=GWEN_XMLNode_GetNodeByXPath(n, p, GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  return value?GWEN_XMLNode_GetCharValue(value, NULL, NULL):NULL;
}

/** Identify CAMT documents before schema processing can create empty lookup nodes.
 * @param xml Original parsed tree. @return Nonzero for CAMT, zero otherwise.
 */
static int aqbCamtDocument(GWEN_XMLNODE *xml) {
  GWEN_XMLNODE *doc=GWEN_XMLNode_FindFirstTag(xml, "Document", NULL, NULL);
  const char *source=doc?GWEN_XMLNode_GetProperty(doc, "xmlns", NULL):NULL;
  return source && strstr(source, ":camt.");
}

/** Read either legacy or PartyIdentificationChoice name.
 * @param n Detail node. @param party Dbtr or Cdtr. @return Borrowed name/null.
 */
static const char *aqbCamtName(GWEN_XMLNODE *n, const char *party) {
  char p[96]; const char *s;
  snprintf(p, sizeof(p), "RltdPties/%s/Nm", party); s=aqbCamtText(n, p);
  if (!s) { snprintf(p, sizeof(p), "RltdPties/%s/Pty/Nm", party); s=aqbCamtText(n, p); }
  return s;
}

/** Append a named amount/currency object without guessing missing values.
 * @param b JSON output. @param name Field name. @param n Parent. @param p Amount path. @return Zero on success, negative on error.
 */
static int aqbCamtMoney(GWEN_BUFFER *b, const char *name, GWEN_XMLNODE *n, const char *p) {
  GWEN_XMLNODE *amount=GWEN_XMLNode_GetNodeByXPath(n, p, GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  int rv;
  if (GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  rv=aqbJsonString(b, name);
  if (rv<0) return rv;
  if (GWEN_Buffer_AppendString(b, ":")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  if (!amount) return GWEN_Buffer_AppendString(b, "null")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
  if (GWEN_Buffer_AppendString(b, "{\"value\":")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  rv=aqbJsonString(b, GWEN_XMLNode_GetCharValue(amount, NULL, NULL));
  if (rv<0) return rv;
  rv=aqbJsonField(b, "currency", GWEN_XMLNode_GetProperty(amount, "Ccy", NULL));
  if (rv<0) return rv;
  return GWEN_Buffer_AppendString(b, "}")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
}

/** Validate an unsigned decimal CAMT amount string without whitespace or separators.
 * ISO 20022 ActiveOrHistoricCurrencyAndAmount is non-negative; sign comes strictly from CdtDbtInd.
 * @param s Amount text. @return Nonzero when syntactically valid decimal.
 */
static int aqbCamtValidAmount(const char *s) {
  int digits=0;
  int frac=0;
  if (!s || !*s) return 0;
  while (*s>='0' && *s<='9') {
    if (++digits>32) return 0;
    s++;
  }
  if (!digits) return 0;
  if (*s=='.') {
    s++;
    while (*s>='0' && *s<='9') {
      if (++frac>12) return 0;
      s++;
    }
    if (!frac) return 0;
  }
  return *s=='\0';
}

/** Populate the native transaction DB "value" from entry-level Ntry/Amt + Ccy + CdtDbtInd.
 * Ensures formats without child TxDtls/Amt (e.g. camt.052.001.08) retain the entry amount
 * and batch bookings never inherit a child detail amount at top level. Fails closed on missing/unknown CdtDbtInd.
 * @param entry Ntry XML. @param db Transaction DB. @param direction Entry CdtDbtInd.
 * @return Zero on success, negative on invalid amount/direction/currency.
 */
static int aqbCamtSetEntryValue(GWEN_XMLNODE *entry, GWEN_DB_NODE *db, const char *direction) {
  GWEN_XMLNODE *amtNode=GWEN_XMLNode_GetNodeByXPath(entry, "Amt", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  const char *valStr=amtNode?GWEN_XMLNode_GetCharValue(amtNode, NULL, NULL):NULL;
  const char *ccy=amtNode?GWEN_XMLNode_GetProperty(amtNode, "Ccy", NULL):NULL;
  const char *sign;
  char vbuf[96];
  int n;
  if (!aqbCamtValidAmount(valStr)) return GWEN_ERROR_INVALID;
  if (!direction || (strcasecmp(direction, "CRDT")!=0 && strcasecmp(direction, "DBIT")!=0)) return GWEN_ERROR_INVALID;
  sign=(strcasecmp(direction, "DBIT")==0)?"-":"";
  if (ccy && *ccy) {
    const char *cp=ccy;
    int clen=0;
    while (*cp) {
      unsigned char ch=(unsigned char)*cp;
      if (!((ch>='A' && ch<='Z') || (ch>='a' && ch<='z') || (ch>='0' && ch<='9'))) return GWEN_ERROR_INVALID;
      if (++clen>12) return GWEN_ERROR_INVALID;
      cp++;
    }
    n=snprintf(vbuf, sizeof(vbuf), "%s%s:%s", sign, valStr, ccy);
  } else {
    n=snprintf(vbuf, sizeof(vbuf), "%s%s", sign, valStr);
  }
  if (n<0 || (size_t)n>=sizeof(vbuf)) return GWEN_ERROR_INVALID;
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "value", vbuf);
  return 0;
}

/** Append every unstructured/structured remittance item, preserving separate strings.
 * @param b JSON output. @param detail TxDtls. @return Zero on success, negative on limit/buffer/UTF-8 error.
 */
static int aqbCamtRemittance(GWEN_BUFFER *b, GWEN_XMLNODE *detail) {
  GWEN_XMLNODE *r=GWEN_XMLNode_GetNodeByXPath(detail, "RmtInf", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  GWEN_XMLNODE *n; int first=1, items=0;
  if (GWEN_Buffer_AppendString(b, ",\"purposes\":[")<0) return GWEN_ERROR_INVALID;
  n=r?GWEN_XMLNode_FindFirstTag(r, "Ustrd", NULL, NULL):NULL;
  while (n) {
    if (++items>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) return GWEN_ERROR_INVALID;
    if (!first && GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_INVALID;
    first=0;
    if (aqbJsonString(b, GWEN_XMLNode_GetCharValue(n, NULL, ""))<0) return GWEN_ERROR_INVALID;
    n=GWEN_XMLNode_FindNextTag(n, "Ustrd", NULL, NULL);
  }
  if (GWEN_Buffer_AppendString(b, "],\"structuredRemittance\":[")<0) return GWEN_ERROR_INVALID;
  first=1;
  n=r?GWEN_XMLNode_FindFirstTag(r, "Strd", NULL, NULL):NULL;
  while (n) {
    GWEN_XMLNODE *a; int firstInfo=1;
    if (++items>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) return GWEN_ERROR_INVALID;
    if (!first && GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_INVALID;
    first=0;
    if (GWEN_Buffer_AppendString(b, "{\"creditorReference\":")<0) return GWEN_ERROR_INVALID;
    if (aqbJsonString(b, aqbCamtText(n, "CdtrRefInf/Ref"))<0) return GWEN_ERROR_INVALID;
    if (GWEN_Buffer_AppendString(b, ",\"additionalInformation\":[")<0) return GWEN_ERROR_INVALID;
    a=GWEN_XMLNode_FindFirstTag(n, "AddtlRmtInf", NULL, NULL);
    while (a) {
      if (++items>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) return GWEN_ERROR_INVALID;
      if (!firstInfo && GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_INVALID;
      firstInfo=0;
      if (aqbJsonString(b, GWEN_XMLNode_GetCharValue(a, NULL, ""))<0) return GWEN_ERROR_INVALID;
      a=GWEN_XMLNode_FindNextTag(a, "AddtlRmtInf", NULL, NULL);
    }
    if (GWEN_Buffer_AppendString(b, "]}")<0) return GWEN_ERROR_INVALID;
    n=GWEN_XMLNode_FindNextTag(n, "Strd", NULL, NULL);
  }
  if (GWEN_Buffer_AppendString(b, "]")<0) return GWEN_ERROR_INVALID;
  return GWEN_Buffer_GetUsedBytes(b)>1048576?GWEN_ERROR_INVALID:0;
}

/** Apply ISO 8601 DtTm YYYY-MM-DD prefix fallback to date/valutaDate DB field when Dt was absent.
 * @param entry Ntry XML. @param db Native transaction DB. @param dtTmPath XML path. @param dbVarName DB field.
 */
static void aqbCamtApplyDateTimeFallback(GWEN_XMLNODE *entry, GWEN_DB_NODE *db, const char *dtTmPath, const char *dbVarName) {
  const char *existing=GWEN_DB_GetCharValue(db, dbVarName, 0, NULL);
  const char *s;
  int year=0, month=0, day=0;
  GWEN_DATE *gd;
  if (existing && *existing) return;
  s=aqbCamtText(entry, dtTmPath);
  if (!s || strlen(s)<10) return;
  if (!isdigit((unsigned char)s[0]) || !isdigit((unsigned char)s[1]) ||
      !isdigit((unsigned char)s[2]) || !isdigit((unsigned char)s[3]) || s[4]!='-' ||
      !isdigit((unsigned char)s[5]) || !isdigit((unsigned char)s[6]) || s[7]!='-' ||
      !isdigit((unsigned char)s[8]) || !isdigit((unsigned char)s[9])) return;
  if (s[10]!='\0' && s[10]!='T' && s[10]!='t' && s[10]!=' ') return;
  year=(s[0]-'0')*1000+(s[1]-'0')*100+(s[2]-'0')*10+(s[3]-'0');
  month=(s[5]-'0')*10+(s[6]-'0');
  day=(s[8]-'0')*10+(s[9]-'0');
  gd=GWEN_Date_fromGregorian(year, month, day);
  if (gd) {
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, dbVarName, GWEN_Date_GetString(gd));
    GWEN_Date_free(gd);
  }
}

/** Append optional CAMT Chrgs/TtlChrgsAndTaxAmt or unambiguous single Chrgs/Rcrd/Amt money object.
 * @param b JSON output. @param name Field name. @param parent Ntry or TxDtls node. @return Zero or error.
 */
static int aqbCamtCharges(GWEN_BUFFER *b, const char *name, GWEN_XMLNODE *parent) {
  GWEN_XMLNODE *chrgs=GWEN_XMLNode_GetNodeByXPath(parent, "Chrgs", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  GWEN_XMLNODE *ttl=chrgs?GWEN_XMLNode_GetNodeByXPath(chrgs, "TtlChrgsAndTaxAmt", GWEN_PATH_FLAGS_NAMEMUSTEXIST):NULL;
  if (ttl) return aqbCamtMoney(b, name, chrgs, "TtlChrgsAndTaxAmt");
  if (chrgs) {
    GWEN_XMLNODE *r1=GWEN_XMLNode_FindFirstTag(chrgs, "Rcrd", NULL, NULL);
    GWEN_XMLNODE *r2=r1?GWEN_XMLNode_FindNextTag(r1, "Rcrd", NULL, NULL):NULL;
    if (r1 && !r2 && GWEN_XMLNode_GetNodeByXPath(r1, "Amt", GWEN_PATH_FLAGS_NAMEMUSTEXIST))
      return aqbCamtMoney(b, name, r1, "Amt");
  }
  if (GWEN_Buffer_AppendString(b, ",")<0 || aqbJsonString(b, name)<0 || GWEN_Buffer_AppendString(b, ":null")<0)
    return GWEN_ERROR_BUFFER_OVERFLOW;
  return 0;
}

/** Append structured ISO 20022 BkTxCd object (Domn/Fmly/SubFmlyCd + Prtry/Cd + Issr), or null.
 * @param b JSON output. @param name Field name. @param parent Ntry or TxDtls node. @return Zero or error.
 */
static int aqbCamtBankTxCode(GWEN_BUFFER *b, const char *name, GWEN_XMLNODE *parent) {
  GWEN_XMLNODE *btc=GWEN_XMLNode_GetNodeByXPath(parent, "BkTxCd", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  int rv;
  if (GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  rv=aqbJsonString(b, name);
  if (rv<0) return rv;
  if (!btc) return GWEN_Buffer_AppendString(b, ":null")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
  if (GWEN_Buffer_AppendString(b, ":{")<0) return GWEN_ERROR_BUFFER_OVERFLOW;
  rv=aqbJsonString(b, "domain");
  if (rv<0 || GWEN_Buffer_AppendString(b, ":")<0) return GWEN_ERROR_INVALID;
  rv=aqbJsonString(b, aqbCamtText(btc, "Domn/Cd"));
  if (rv<0 ||
      aqbJsonField(b, "family", aqbCamtText(btc, "Domn/Fmly/Cd"))<0 ||
      aqbJsonField(b, "subFamily", aqbCamtText(btc, "Domn/Fmly/SubFmlyCd"))<0 ||
      aqbJsonField(b, "proprietaryCode", aqbCamtText(btc, "Prtry/Cd"))<0 ||
      aqbJsonField(b, "proprietaryIssuer", aqbCamtText(btc, "Prtry/Issr"))<0 ||
      GWEN_Buffer_AppendString(b, "}")<0) {
    return GWEN_ERROR_INVALID;
  }
  return GWEN_Buffer_GetUsedBytes(b)>1048576?GWEN_ERROR_INVALID:0;
}

/** Append SEPA/CAMT RtrInf return reason metadata, or null when absent.
 * @param b JSON output. @param detail TxDtls XML. @return Zero or error.
 */
static int aqbCamtReturnInfo(GWEN_BUFFER *b, GWEN_XMLNODE *detail) {
  GWEN_XMLNODE *rtr=GWEN_XMLNode_GetNodeByXPath(detail, "RtrInf", GWEN_PATH_FLAGS_NAMEMUSTEXIST);
  const char *rsn;
  const char *orgtr;
  const char *domn;
  const char *fmly;
  const char *subf;
  const char *prtryCode;
  const char *origBtc=NULL;
  char obuf[128];
  GWEN_XMLNODE *a;
  int firstInfo=1, items=0, rv;
  if (!rtr) return GWEN_Buffer_AppendString(b, ",\"returnInfo\":null")<0?GWEN_ERROR_BUFFER_OVERFLOW:0;
  rsn=aqbCamtText(rtr, "Rsn/Cd");
  if (!rsn) rsn=aqbCamtText(rtr, "Rsn/Prtry");
  orgtr=aqbCamtText(rtr, "Orgtr/Nm");
  if (!orgtr) orgtr=aqbCamtText(rtr, "Orgtr/Pty/Nm");
  domn=aqbCamtText(rtr, "OrgnlBkTxCd/Domn/Cd");
  fmly=aqbCamtText(rtr, "OrgnlBkTxCd/Domn/Fmly/Cd");
  subf=aqbCamtText(rtr, "OrgnlBkTxCd/Domn/Fmly/SubFmlyCd");
  prtryCode=aqbCamtText(rtr, "OrgnlBkTxCd/Prtry/Cd");
  if (domn && fmly && subf) {
    int n=snprintf(obuf, sizeof(obuf), "%s-%s-%s", domn, fmly, subf);
    if (n>0 && (size_t)n<sizeof(obuf)) origBtc=obuf;
  }
  if (!origBtc && prtryCode && *prtryCode) origBtc=prtryCode;
  if (GWEN_Buffer_AppendString(b, ",\"returnInfo\":{\"reasonCode\":")<0) return GWEN_ERROR_INVALID;
  rv=aqbJsonString(b, rsn);
  if (rv<0 ||
      aqbJsonField(b, "originatorName", orgtr)<0 ||
      aqbJsonField(b, "originalBankTransactionCode", origBtc)<0 ||
      GWEN_Buffer_AppendString(b, ",\"additionalInformation\":[")<0) {
    return GWEN_ERROR_INVALID;
  }
  a=GWEN_XMLNode_FindFirstTag(rtr, "AddtlInf", NULL, NULL);
  while (a) {
    if (++items>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) return GWEN_ERROR_INVALID;
    if (!firstInfo && GWEN_Buffer_AppendString(b, ",")<0) return GWEN_ERROR_INVALID;
    firstInfo=0;
    if (aqbJsonString(b, GWEN_XMLNode_GetCharValue(a, NULL, ""))<0) return GWEN_ERROR_INVALID;
    a=GWEN_XMLNode_FindNextTag(a, "AddtlInf", NULL, NULL);
  }
  if (GWEN_Buffer_AppendString(b, "]}")<0) return GWEN_ERROR_INVALID;
  return GWEN_Buffer_GetUsedBytes(b)>1048576?GWEN_ERROR_INVALID:0;
}

/** Store one entry's original details on its single native booking transaction.
 * @param entry Ntry XML. @param db Native transaction DB. @param source CAMT namespace.
 * @return Zero on success; error if size/detail/buffer limits or UTF-8 validation fail, without partial results.
 */
static int aqbCamtEntry(GWEN_XMLNODE *entry, GWEN_DB_NODE *db, const char *source) {
  GWEN_BUFFER *b=GWEN_Buffer_new(0, 1024, 0, 1);
  GWEN_XMLNODE *group;
  GWEN_XMLNODE *firstDetail=NULL;
  int count=0, groups=0, batch=0;
  const char *direction=aqbCamtText(entry, "CdtDbtInd");
  const char *entryStatus;
  char tailBuf[64];
  int tailLen;
  if (!b) return GWEN_ERROR_MEMORY_FULL;
  GWEN_Buffer_SubMode(b, GWEN_BUFFER_MODE_ABORT_ON_MEMFULL);
  GWEN_Buffer_SetHardLimit(b, 1048576 + 4096);
  if (aqbCamtSetEntryValue(entry, db, direction)<0) { GWEN_Buffer_free(b); return GWEN_ERROR_INVALID; }
  aqbCamtApplyDateTimeFallback(entry, db, "BookgDt/DtTm", "date");
  aqbCamtApplyDateTimeFallback(entry, db, "ValDt/DtTm", "valutaDate");
  entryStatus=aqbCamtText(entry, "Sts/Cd");
  if (!entryStatus) entryStatus=aqbCamtText(entry, "Sts");
  if (entryStatus && (strcasecmp(entryStatus, "PDNG")==0 || strcasecmp(entryStatus, "FUTR")==0)) {
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "type", "notedStatement");
  }
  if (GWEN_Buffer_AppendString(b, "{\"version\":1")<0 ||
      aqbJsonField(b, "source", source)<0 ||
      aqbJsonField(b, "entryReference", aqbCamtText(entry, "NtryRef"))<0 ||
      aqbJsonField(b, "accountServicerReference", aqbCamtText(entry, "AcctSvcrRef"))<0 ||
      aqbJsonField(b, "creditDebitIndicator", direction)<0 ||
      aqbJsonField(b, "entryInformation", aqbCamtText(entry, "AddtlNtryInf"))<0 ||
      aqbJsonField(b, "bookingDateTime", aqbCamtText(entry, "BookgDt/DtTm"))<0 ||
      aqbJsonField(b, "valutaDateTime", aqbCamtText(entry, "ValDt/DtTm"))<0 ||
      aqbJsonField(b, "status", entryStatus)<0 ||
      aqbCamtMoney(b, "entryAmount", entry, "Amt")<0 ||
      aqbCamtCharges(b, "chargesAmount", entry)<0 ||
      aqbCamtBankTxCode(b, "bankTransactionCode", entry)<0 ||
      GWEN_Buffer_AppendString(b, ",\"groups\":[")<0) {
    GWEN_Buffer_free(b); return GWEN_ERROR_INVALID;
  }
  group=GWEN_XMLNode_FindFirstTag(entry, "NtryDtls", NULL, NULL);
  while (group) {
    GWEN_XMLNODE *btch=GWEN_XMLNode_FindFirstTag(group, "Btch", NULL, NULL);
    if (groups++ && GWEN_Buffer_AppendString(b, ",")<0) { GWEN_Buffer_free(b); return GWEN_ERROR_INVALID; }
    if (btch) batch=1;
    if (GWEN_Buffer_AppendString(b, "{\"declaredCount\":")<0 ||
        aqbJsonString(b, aqbCamtText(group, "Btch/NbOfTxs"))<0 ||
        aqbJsonField(b, "paymentInformationId", aqbCamtText(group, "Btch/PmtInfId"))<0 ||
        aqbJsonField(b, "messageId", aqbCamtText(group, "Btch/MsgId"))<0 ||
        aqbCamtMoney(b, "totalAmount", group, "Btch/TtlAmt")<0 ||
        GWEN_Buffer_AppendString(b, "}")<0) {
      GWEN_Buffer_free(b); return GWEN_ERROR_INVALID;
    }
    group=GWEN_XMLNode_FindNextTag(group, "NtryDtls", NULL, NULL);
    if (groups>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) { GWEN_Buffer_free(b); return GWEN_ERROR_INVALID; }
  }
  if (GWEN_Buffer_AppendString(b, "],\"details\":[")<0) { GWEN_Buffer_free(b); return GWEN_ERROR_INVALID; }
  groups=0;
  group=GWEN_XMLNode_FindFirstTag(entry, "NtryDtls", NULL, NULL);
  while (group) {
    GWEN_XMLNODE *detail=GWEN_XMLNode_FindFirstTag(group, "TxDtls", NULL, NULL);
    while (detail) {
      const char *s=aqbCamtText(detail, "CdtDbtInd");
      const char *purpCode=aqbCamtText(detail, "Purp/Cd");
      char hdrBuf[64];
      int hdrLen;
      if (!firstDetail) firstDetail=detail;
      if (!purpCode) purpCode=aqbCamtText(detail, "Purp/Prtry");
      if (count && GWEN_Buffer_AppendString(b, ",")<0) { GWEN_Buffer_free(b); return GWEN_ERROR_INVALID; }
      hdrLen=snprintf(hdrBuf, sizeof(hdrBuf), "{\"index\":%d,\"groupIndex\":%d", count++, groups);
      if (hdrLen<0 || (size_t)hdrLen>=sizeof(hdrBuf) ||
          GWEN_Buffer_AppendBytes(b, hdrBuf, (uint32_t)hdrLen)<0 ||
          aqbJsonField(b, "creditDebitIndicator", s?s:direction)<0 ||
          aqbJsonField(b, "debtorName", aqbCamtName(detail, "Dbtr"))<0 ||
          aqbJsonField(b, "debtorIban", aqbCamtText(detail, "RltdPties/DbtrAcct/Id/IBAN"))<0 ||
          aqbJsonField(b, "creditorName", aqbCamtName(detail, "Cdtr"))<0 ||
          aqbJsonField(b, "creditorIban", aqbCamtText(detail, "RltdPties/CdtrAcct/Id/IBAN"))<0 ||
          aqbJsonField(b, "ultimateDebtor", aqbCamtName(detail, "UltmtDbtr"))<0 ||
          aqbJsonField(b, "ultimateCreditor", aqbCamtName(detail, "UltmtCdtr"))<0 ||
          aqbJsonField(b, "endToEndReference", aqbCamtText(detail, "Refs/EndToEndId"))<0 ||
          aqbJsonField(b, "mandateId", aqbCamtText(detail, "Refs/MndtId"))<0 ||
          aqbJsonField(b, "transactionId", aqbCamtText(detail, "Refs/TxId"))<0 ||
          aqbJsonField(b, "instructionId", aqbCamtText(detail, "Refs/InstrId"))<0 ||
          aqbJsonField(b, "paymentInformationId", aqbCamtText(detail, "Refs/PmtInfId"))<0 ||
          aqbJsonField(b, "accountServicerReference", aqbCamtText(detail, "Refs/AcctSvcrRef"))<0 ||
          aqbJsonField(b, "uetr", aqbCamtText(detail, "Refs/UETR"))<0 ||
          aqbJsonField(b, "purposeCode", purpCode)<0 ||
          aqbJsonField(b, "additionalInformation", aqbCamtText(detail, "AddtlTxInf"))<0 ||
          aqbCamtMoney(b, "transactionAmount", detail, "AmtDtls/TxAmt/Amt")<0 ||
          aqbCamtMoney(b, "directAmount", detail, "Amt")<0 ||
          aqbCamtMoney(b, "instructedAmount", detail, "AmtDtls/InstdAmt/Amt")<0 ||
          aqbCamtMoney(b, "counterValueAmount", detail, "AmtDtls/CntrValAmt/Amt")<0 ||
          aqbCamtCharges(b, "chargesAmount", detail)<0 ||
          aqbCamtBankTxCode(b, "bankTransactionCode", detail)<0 ||
          aqbCamtReturnInfo(b, detail)<0 ||
          aqbCamtRemittance(b, detail)<0 ||
          GWEN_Buffer_AppendString(b, "}")<0) {
        GWEN_Buffer_free(b); return GWEN_ERROR_INVALID;
      }
      if (count>10000 || GWEN_Buffer_GetUsedBytes(b)>1048576) {
        GWEN_Buffer_free(b); return GWEN_ERROR_INVALID;
      }
      detail=GWEN_XMLNode_FindNextTag(detail, "TxDtls", NULL, NULL);
    }
    groups++; group=GWEN_XMLNode_FindNextTag(group, "NtryDtls", NULL, NULL);
  }
  tailLen=snprintf(tailBuf, sizeof(tailBuf), "],\"isBatch\":%s}", (batch || count>1 || groups>1)?"true":"false");
  if (tailLen<0 || (size_t)tailLen>=sizeof(tailBuf) ||
      GWEN_Buffer_AppendBytes(b, tailBuf, (uint32_t)tailLen)<0 ||
      GWEN_Buffer_GetUsedBytes(b)>1048576) {
    GWEN_Buffer_free(b); return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "camtDetailsJson", GWEN_Buffer_GetStart(b));
  /* Never attribute first child party/reference or mixed-batch local counterparty info to parent booking. */
  if (batch || count>1 || groups>1) {
    const char *entryInfo=aqbCamtText(entry, "AddtlNtryInf");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localName", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localIban", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "localBic", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteName", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteIban", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteBic", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "endToEndReference", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "mandateId", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "creditorSchemeId", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "fiId", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "ultimateCreditor", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "ultimateDebtor", "");
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "purpose", entryInfo?entryInfo:"");
  }
  else if (count==1 && firstDetail && direction) {
    /* Recover party info and agent BICs for camt.053/054.001.08 single entries where upstream XML schemas use wrong XPaths. */
    const char *existingRemote=GWEN_DB_GetCharValue(db, "remoteName", 0, NULL);
    const char *existingIban=GWEN_DB_GetCharValue(db, "remoteIban", 0, NULL);
    const char *existingBic=GWEN_DB_GetCharValue(db, "remoteBic", 0, NULL);
    const char *rName=NULL;
    const char *rIban=NULL;
    const char *rBic=NULL;
    if (strcasecmp(direction, "CRDT")==0) {
      rName=aqbCamtName(firstDetail, "Dbtr");
      rIban=aqbCamtText(firstDetail, "RltdPties/DbtrAcct/Id/IBAN");
      rBic=aqbCamtText(firstDetail, "RltdAgts/DbtrAgt/FinInstnId/BICFI");
      if (!rBic) rBic=aqbCamtText(firstDetail, "RltdAgts/DbtrAgt/FinInstnId/BIC");
    }
    else if (strcasecmp(direction, "DBIT")==0) {
      rName=aqbCamtName(firstDetail, "Cdtr");
      rIban=aqbCamtText(firstDetail, "RltdPties/CdtrAcct/Id/IBAN");
      rBic=aqbCamtText(firstDetail, "RltdAgts/CdtrAgt/FinInstnId/BICFI");
      if (!rBic) rBic=aqbCamtText(firstDetail, "RltdAgts/CdtrAgt/FinInstnId/BIC");
    }
    if (!(existingRemote && *existingRemote) && rName && *rName) {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteName", rName);
    }
    if (!(existingIban && *existingIban) && rIban && *rIban) {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteIban", rIban);
    }
    if (!(existingBic && *existingBic) && rBic && *rBic) {
      GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "remoteBic", rBic);
    }
  }
  GWEN_Buffer_free(b); return 0;
}

/** Attach original CAMT entries to schema-created native rows in matching report/entry order.
 * @param xml Parsed bank document. @param db Schema DB. @param schemaName Optional schema hint.
 * @return Zero or a fail-closed mapping error.
 */
static int aqbCamtPreserve(GWEN_XMLNODE *xml, GWEN_DB_NODE *db, const char *schemaName) {
  GWEN_XMLNODE *doc=GWEN_XMLNode_FindFirstTag(xml, "Document", NULL, NULL), *message=NULL, *report;
  const char *ns, *source, *reportName=NULL;
  GWEN_DB_NODE *account;
  if (!doc) return (schemaName && strncasecmp(schemaName, "camt_", 5)==0)?GWEN_ERROR_INVALID:0;
  ns=GWEN_XMLNode_GetProperty(doc, "xmlns", NULL);
  if (ns && strstr(ns, ":camt.")) source=ns;
  else if (schemaName && strncasecmp(schemaName, "camt_", 5)==0) source=schemaName;
  else return 0;
  message=GWEN_XMLNode_FindFirstTag(doc, "BkToCstmrStmt", NULL, NULL); if (message) reportName="Stmt";
  if (!message) { message=GWEN_XMLNode_FindFirstTag(doc, "BkToCstmrAcctRpt", NULL, NULL); if (message) reportName="Rpt"; }
  if (!message) { message=GWEN_XMLNode_FindFirstTag(doc, "BkToCstmrDbtCdtNtfctn", NULL, NULL); if (message) reportName="Ntfctn"; }
  if (!message) return GWEN_ERROR_INVALID;
  report=GWEN_XMLNode_FindFirstTag(message, reportName, NULL, NULL);
  account=GWEN_DB_FindFirstGroup(db, "account");
  while (report) {
    GWEN_XMLNODE *entry; GWEN_DB_NODE *transaction;
    const char *iban=aqbCamtText(report, "Acct/Id/IBAN"), *mapped;
    if (!account) return GWEN_ERROR_INVALID;
    mapped=GWEN_DB_GetCharValue(account, "iban", 0, NULL);
    if ((iban && (!mapped || strcmp(iban, mapped))) || (!iban && mapped && *mapped)) return GWEN_ERROR_INVALID;
    entry=GWEN_XMLNode_FindFirstTag(report, "Ntry", NULL, NULL); transaction=GWEN_DB_FindFirstGroup(account, "transaction");
    while (entry) {
      const char *amount=aqbCamtText(entry, "Amt");
      if (!amount || !*amount) return GWEN_ERROR_INVALID;
      if (!transaction || aqbCamtEntry(entry, transaction, source)<0) return GWEN_ERROR_INVALID;
      transaction=GWEN_DB_FindNextGroup(transaction, "transaction");
      entry=GWEN_XMLNode_FindNextTag(entry, "Ntry", NULL, NULL);
    }
    if (transaction) return GWEN_ERROR_INVALID;
    account=GWEN_DB_FindNextGroup(account, "account"); report=GWEN_XMLNode_FindNextTag(report, reportName, NULL, NULL);
  }
  return account?GWEN_ERROR_INVALID:0;
}
