#include <gwenhywfar/buffer.h>
#include <aqbanking/banking.h>
#include <stdio.h>
#include <string.h>

char *input = "1,361.54";

int main(int argc, char *argv[])
{
  AB_VALUE *value;
  GWEN_BUFFER *buf, *buf2;
  int result = 0;

  if (argc > 1)
    input = argv[1];
  value = AB_Value_fromString(input);

  buf = GWEN_Buffer_new(NULL, 300, 0, 0);
  AB_Value_toString(value, buf);
  if (strcmp(GWEN_Buffer_GetStart(buf), "136154/100") != 0)
    result = -1;

  buf2 = GWEN_Buffer_new(NULL, 300, 0, 0);
  AB_Value_toHumanReadableString(value, buf2, 2, 0);
  if ((strcmp(GWEN_Buffer_GetStart(buf2), "1361.54") != 0)
      && (strcmp(GWEN_Buffer_GetStart(buf2), "1361,54") != 0))
    result = -1;

  printf("Storing %s internally as rational number %s; as double: %s\n",
         input, GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetStart(buf2));

  GWEN_Buffer_free(buf);
  GWEN_Buffer_free(buf2);
  AB_Value_free(value);

  if (argc <= 1) {
    AB_VALUE *v25;
    AB_VALUE *v64;
    AB_VALUE *bad;
    GWEN_BUFFER *vbuf;

    v25 = AB_Value_fromString("0.1234567890123456789012345");
    if (!v25)
      return -1;
    vbuf = GWEN_Buffer_new(NULL, 300, 0, 1);
    AB_Value_toString(v25, vbuf);
    if (strcmp(GWEN_Buffer_GetStart(vbuf), "1234567890123456789012345/10000000000000000000000000") != 0)
      result = -1;
    GWEN_Buffer_free(vbuf);
    AB_Value_free(v25);

    v64 = AB_Value_fromString("1.0000000000000000000000000000000000000000000000000000000000000001");
    if (!v64)
      return -1;
    if (AB_Value_GetValueAsDouble(v64) < 0.99 || AB_Value_GetValueAsDouble(v64) > 1.01)
      result = -1;
    AB_Value_free(v64);

    bad = AB_Value_fromString("123/0");
    if (bad) {
      AB_Value_free(bad);
      return -1;
    }
    bad = AB_Value_fromString(".");
    if (bad) {
      AB_Value_free(bad);
      return -1;
    }
  }

  return result;
}
