/***************************************************************************
 begin       : Mon Jan 13 2020
 copyright   : (C) 2020 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "n_utils.h"
#include "aqofxconnect/user.h"
#include "aqbanking/i18n_l.h"

#include <gwenhywfar/gui.h>
#include <gwenhywfar/text.h>
#include <gwenhywfar/cryptkeysym.h>




void AO_Provider_Util_SetDateValue(GWEN_XMLNODE *xmlNode, const GWEN_DATE *da, uint32_t userFlags, const char *varName)
{
  if (da) {
    GWEN_BUFFER *tbuf;

    tbuf=GWEN_Buffer_new(0, 32, 0, 1);
    if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000", tbuf);
    else
      GWEN_Date_toStringWithTemplate(da, "YYYYMMDD000000.000", tbuf);
    GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(tbuf));
    GWEN_Buffer_free(tbuf);
  }
}



void AO_Provider_Util_SetTimeValue(GWEN_XMLNODE *xmlNode, const GWEN_TIME *ti, uint32_t userFlags, const char *varName)
{
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 32, 0, 1);
  if (userFlags & AO_USER_FLAGS_SEND_SHORT_DATE)
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss", tbuf);
  else
    GWEN_Time_toString(ti, "YYYYMMDDhhmmss.000", tbuf);

  GWEN_XMLNode_SetCharValue(xmlNode, varName, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void AO_Provider_Util_SetCurrentTimeValue(GWEN_XMLNODE *xmlNode, uint32_t userFlags, const char *varName)
{
  GWEN_TIME *ti;

  ti=GWEN_CurrentTime();
  assert(ti);
  AO_Provider_Util_SetTimeValue(xmlNode, ti, userFlags, varName);
  GWEN_Time_free(ti);
}



void AO_Provider_Util_ListAccounts(AB_IMEXPORTER_CONTEXT *ctx)
{
  if (ctx) {
    AB_IMEXPORTER_ACCOUNTINFO_LIST *accInfoList;

    accInfoList=AB_ImExporterContext_GetAccountInfoList(ctx);
    if (accInfoList) {
      AB_IMEXPORTER_ACCOUNTINFO *accInfo;

      accInfo=AB_ImExporterAccountInfo_List_First(accInfoList);
      while (accInfo) {
        const char *sBankCode;
        const char *sBankName;
        const char *sAccountNumber;
        const char *sAccountName;

        sBankCode=AB_ImExporterAccountInfo_GetBankCode(accInfo);
        sBankName=AB_ImExporterAccountInfo_GetBankName(accInfo);
        sAccountNumber=AB_ImExporterAccountInfo_GetAccountNumber(accInfo);
        sAccountName=AB_ImExporterAccountInfo_GetAccountName(accInfo);

        GWEN_Gui_ProgressLog2(0,
                              GWEN_LoggerLevel_Notice,
                              I18N("Received account %s/%s (%s/%s)"),
                              sBankCode?sBankCode:"(no bank code)",
                              sAccountNumber?sAccountNumber:"(no account number)",
                              sBankName?sBankName:"(no bank name)",
                              sAccountName?sAccountName:"(no account name)");

        accInfo=AB_ImExporterAccountInfo_List_Next(accInfo);
      }
    }
  }
}


/* XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
 * 012345678901234567890123456789012345
 *           1         2         3
 * 0        9    14   19   24
 *         8    13   18   23
*/
char *AO_Provider_Util_GenerateUuid()
{
  GWEN_CRYPT_KEY *temporaryCryptKey;
  uint8_t *ptrKeyData;
  uint32_t lenKeyData;
  uint8_t rawUuid[16];
  char *returnBuffer;

  temporaryCryptKey=GWEN_Crypt_KeyBlowFish_Generate(GWEN_Crypt_CryptMode_Cbc, 16, 0);
  if (temporaryCryptKey==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No random data generated");
    return NULL;
  }

  ptrKeyData=GWEN_Crypt_KeyBlowFish_GetKeyDataPtr(temporaryCryptKey);
  lenKeyData=GWEN_Crypt_KeyBlowFish_GetKeyDataLen(temporaryCryptKey);
  if (ptrKeyData==NULL || lenKeyData<16) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Too few bytes in random data");
    GWEN_Crypt_Key_free(temporaryCryptKey);
    return NULL;
  }
  memmove(rawUuid, ptrKeyData, 16);
  GWEN_Crypt_Key_free(temporaryCryptKey);

  /* some adjustments for RFC 4122 */
  rawUuid[6]=0x40 | (rawUuid[6] & 0xf);  /* mark as randomly generated */
  rawUuid[8]=0x80 | (rawUuid[8] & 0x3f); /* set high nibble to 8, 9, A or B */

  returnBuffer=(char *) malloc(37);
  assert(returnBuffer);

  GWEN_Text_ToHex((const char *) rawUuid, 4, returnBuffer, 9);
  returnBuffer[8]='-';

  GWEN_Text_ToHex((const char *) rawUuid+4, 2, returnBuffer+9, 5);
  returnBuffer[13]='-';

  GWEN_Text_ToHex((const char *) rawUuid+6, 2, returnBuffer+14, 5);
  returnBuffer[18]='-';

  GWEN_Text_ToHex((const char *) rawUuid+8, 2, returnBuffer+19, 5);
  returnBuffer[23]='-';

  GWEN_Text_ToHex((const char *) rawUuid+10, 6, returnBuffer+24, 13);
  returnBuffer[36]=0;

  return returnBuffer;
}





