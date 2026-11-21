/* Bounds and underflow regression tests for SWIFT MT535 and MT940 parsers. */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "swift535_p.h"
#include "swift940_61.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK_EXPR(expr) \
  do { \
    if (!(expr)) { \
      fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, (int)__LINE__, #expr); \
      g_failures++; \
    } \
  } while (0)

int main(GWEN_UNUSED int argc, GWEN_UNUSED char **argv)
{
  char *longPayload;
  AHB_SWIFT_TAG *tg;
  GWEN_DB_NODE *data;
  GWEN_DB_NODE *cfg;
  GWEN_DB_NODE *origGroup;
  GWEN_DB_NODE *feesGroup;
  const char *s;
  int rv;

  cfg = GWEN_DB_Group_new("cfg");
  CHECK_EXPR(cfg != NULL);

  longPayload = (char *)malloc(4500);
  CHECK_EXPR(longPayload != NULL);
  if (longPayload) {
    memcpy(longPayload, "ISIN   DE0005140008\n/DE/514000\n", 31);
    memset(longPayload + 31, 'S', 4200);
    longPayload[4231] = '\0';

    tg = AHB_SWIFT_Tag_new("35B", longPayload);
    data = GWEN_DB_Group_new("data");
    rv = AHB_SWIFT535_Parse_35B(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
    CHECK_EXPR(rv == 0);
    s = GWEN_DB_GetCharValue(data, "uniqueId", 0, NULL);
    CHECK_EXPR(s != NULL && strcmp(s, "DE0005140008") == 0);
    s = GWEN_DB_GetCharValue(data, "name", 0, NULL);
    CHECK_EXPR(s != NULL && strlen(s) == 4200);
    GWEN_DB_Group_free(data);
    AHB_SWIFT_Tag_free(tg);
    free(longPayload);
  }

  tg = AHB_SWIFT_Tag_new("98C", ":PRIC//1 1 1");
  data = GWEN_DB_Group_new("data");
  rv = AHB_SWIFT535_Parse_98C(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
  CHECK_EXPR(rv == 0);
  CHECK_EXPR(GWEN_DB_GetCharValue(data, "unitPriceDateTime", 0, NULL) == NULL);
  GWEN_DB_Group_free(data);
  AHB_SWIFT_Tag_free(tg);

  tg = AHB_SWIFT_Tag_new("98C", ":PRIC//20269999256161");
  data = GWEN_DB_Group_new("data");
  rv = AHB_SWIFT535_Parse_98C(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
  CHECK_EXPR(rv == GWEN_ERROR_BAD_DATA);
  GWEN_DB_Group_free(data);
  AHB_SWIFT_Tag_free(tg);

  tg = AHB_SWIFT_Tag_new("98A", ":PRIC//20269999");
  data = GWEN_DB_Group_new("data");
  rv = AHB_SWIFT535_Parse_98A(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
  CHECK_EXPR(rv == GWEN_ERROR_BAD_DATA);
  GWEN_DB_Group_free(data);
  AHB_SWIFT_Tag_free(tg);

  {
    static const char *const badLines[] = {
      "2601020102C100,00NTRFNONREF\n/OCMT/E",
      "2601020102C100,00NTRFNONREF\n/OCMT/EU",
      "2601020102C100,00NTRFNONREF\n/OCMT/EUR/",
      "2601020102C100,00NTRFNONREF\n/CHGS/X"
    };
    size_t idx;

    for (idx = 0; idx < sizeof(badLines) / sizeof(badLines[0]); idx++) {
      tg = AHB_SWIFT_Tag_new("61", badLines[idx]);
      data = GWEN_DB_Group_new("data");
      rv = AHB_SWIFT940_Parse_61(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
      CHECK_EXPR(rv < 0);
      GWEN_DB_Group_free(data);
      AHB_SWIFT_Tag_free(tg);
    }
  }

  tg = AHB_SWIFT_Tag_new("61", "2601020102C100,00NTRFNONREF\n/OCMT/EUR12,34/CHGS/EUR/56,78/");
  data = GWEN_DB_Group_new("data");
  rv = AHB_SWIFT940_Parse_61(tg, GWEN_DB_FLAGS_DEFAULT, data, cfg);
  CHECK_EXPR(rv == 0);
  origGroup = GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "origvalue");
  feesGroup = GWEN_DB_GetGroup(data, GWEN_PATH_FLAGS_NAMEMUSTEXIST, "fees");
  CHECK_EXPR(origGroup != NULL);
  CHECK_EXPR(feesGroup != NULL);
  if (origGroup) {
    CHECK_EXPR(strcmp(GWEN_DB_GetCharValue(origGroup, "currency", 0, ""), "EUR") == 0);
    CHECK_EXPR(strcmp(GWEN_DB_GetCharValue(origGroup, "value", 0, ""), "12,34") == 0);
  }
  if (feesGroup) {
    CHECK_EXPR(strcmp(GWEN_DB_GetCharValue(feesGroup, "currency", 0, ""), "EUR") == 0);
    CHECK_EXPR(strcmp(GWEN_DB_GetCharValue(feesGroup, "value", 0, ""), "56,78") == 0);
  }
  GWEN_DB_Group_free(data);
  AHB_SWIFT_Tag_free(tg);

  GWEN_DB_Group_free(cfg);
  return g_failures ? 1 : 0;
}
