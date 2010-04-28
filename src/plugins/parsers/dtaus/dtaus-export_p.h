/***************************************************************************
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQHBCIBANK_DTAUS_EXPORT_P_H
#define AQHBCIBANK_DTAUS_EXPORT_P_H


#define AHB_DTAUS_HARDLIMIT (256*1024)


#include <aqbanking/value.h>


int AHB_DTAUS__ToDTA(int c);
int AHB_DTAUS__AddWord(GWEN_BUFFER *dst,
                       unsigned int size,
                       const char *s);
int AHB_DTAUS__AddDtaWord(GWEN_BUFFER *dst,
			  unsigned int size,
			  const char *s);

int AHB_DTAUS__AddNum(GWEN_BUFFER *dst,
                      unsigned int size,
                      const char *s);

double AHB_DTAUS__string2double(const char *s);

int AHB_DTAUS__CreateSetA(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg);
int AHB_DTAUS__CreateSetC(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg,
                          GWEN_DB_NODE *xa,
			  AB_VALUE *sumEUR,
                          AB_VALUE *sumDEM,
                          AB_VALUE *sumBankCodes,
                          AB_VALUE *sumAccountIds);
int AHB_DTAUS__CreateSetE(GWEN_BUFFER *dst,
                          GWEN_DB_NODE *cfg,
                          int csets,
                          AB_VALUE *sumEUR,
                          AB_VALUE *sumDEM,
                          AB_VALUE *sumBankCodes,
                          AB_VALUE *sumAccountIds);


int AHB_DTAUS__Export(GWEN_DBIO *dbio,
		      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *data,
		      GWEN_DB_NODE *cfg,
		      uint32_t flags);




#endif /* AQHBCIBANK_DTAUS_EXPORT_P_H */

