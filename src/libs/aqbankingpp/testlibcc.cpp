#include <gwenhywfar/buffer.h>
#include <aqbanking/banking.h>
#include "value.hpp"
#include "balance.hpp"
#include "time.hpp"
#include "accountstatus.hpp"
#include "stringlist.hpp"

const char *input = "1,361.54";

int
main(int argc, char *argv[])
{
	AB::Value value;
	GWEN_BUFFER *buf, *buf2;
	int result = 0;

	if (argc > 1) input = argv[1];
	value = AB::Value::fromString(input);

	buf = GWEN_Buffer_new(NULL, 300, 0, 0);
	value.toString(buf);
	if (strcmp(GWEN_Buffer_GetStart(buf), "136154/100") != 0)
	  result = -1;

	buf2 = GWEN_Buffer_new(NULL, 300, 0, 0);
	AB_Value_toHumanReadableString2(value.ptr(), buf2, 2, 0);
	if ((strcmp(GWEN_Buffer_GetStart(buf2), "1361.54") != 0)
	    && (strcmp(GWEN_Buffer_GetStart(buf2), "1361,54") != 0))
	  result = -1;

	printf("Storing %s internally as rational number %s; as double: %s\n",
	       input, GWEN_Buffer_GetStart(buf), GWEN_Buffer_GetStart(buf2));

	GWEN_Buffer_free(buf);
	GWEN_Buffer_free(buf2);

	return result;
}
