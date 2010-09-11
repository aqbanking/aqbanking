/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "msgengine_p.h"
#include "aqhbci_l.h"
#include "hbci_l.h"
#include "user_l.h"
#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/dbio.h>

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>


GWEN_INHERIT(GWEN_MSGENGINE, AH_MSGENGINE);


int AH_MsgEngine_TypeRead(GWEN_MSGENGINE *e,
                          GWEN_BUFFER *msgbuf,
                          GWEN_XMLNODE *node,
                          GWEN_BUFFER *vbuf,
                          char escapeChar,
                          const char *delimiters){
  AH_MSGENGINE *x;
  const char *type;
  int v;

  DBG_VERBOUS(AQHBCI_LOGDOMAIN, "AH_MsgEngine_TypeRead");
  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);


  type=GWEN_XMLNode_GetProperty(node, "type","");
  if (strcasecmp(type, "date")==0) {
    char buffer[9];
    unsigned int i;

    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Supporting type \"date\"");
    memset(buffer,0, sizeof(buffer));
    if (GWEN_Buffer_GetBytesLeft(msgbuf)<8) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Too few bytes for date (%d<8)",
                GWEN_Buffer_GetBytesLeft(msgbuf));
      return -1;
    }
    for (i=0; i<8; i++) {
      int c;

      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      if (!isdigit(c)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-digit found in date");
        return -1;
      }
      buffer[i]=c;
      if (i==3) {
        /* year is full, check it */
        v=atoi(buffer);
        if (v<1970 || v>2100) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Year out of range (%d)", v);
          return -1;
        }
      } /* if i==3 */
      else if (i==5) {
        /* month is full, check it */
        v=atoi(buffer+4);
        if (v<1 || v>12) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Month out of range (%d)", v);
          return -1;
        }
      } /* if i==6 */
    }
    v=atoi(buffer+6);
    if (v<1 || v>31) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Day out of range (%d)", v);
      return -1;
    }
    /* date read, now store it */
    if (GWEN_Buffer_AppendBytes(vbuf, buffer, 8)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
      return -1;
    }
    return 0;
  } /* if date */

  else if (strcasecmp(type, "time")==0) {
    char buffer[7];
    unsigned int i;

    DBG_INFO(AQHBCI_LOGDOMAIN, "Supporting type \"time\"");
    memset(buffer,0, sizeof(buffer));
    if (GWEN_Buffer_GetBytesLeft(msgbuf)<6) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Too few bytes for time (%d<6)",
                GWEN_Buffer_GetBytesLeft(msgbuf));
      return -1;
    }
    for (i=0; i<6; i++) {
      int c;

      c=GWEN_Buffer_ReadByte(msgbuf);
      if (c==-1)
        return -1;
      if (!isdigit(c)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-digit found in date");
        return -1;
      }
      buffer[i]=c;
      if (i==1) {
        /* hour is full, check it */
        v=atoi(buffer);
        if (v>23) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Hour out of range (%d)", v);
          return -1;
        }
      } /* if i==1 */
      else if (i==3) {
        /* minutes full, check */
        v=atoi(buffer+2);
        if (v>59) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Minutes out of range (%d)", v);
          return -1;
        }
      } /* if i==3 */
    }
    v=atoi(buffer+4);
    if (v>59) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Seconds out of range (%d)", v);
      return -1;
    }
    /* date read, now store it */
    if (GWEN_Buffer_AppendBytes(vbuf, buffer, 6)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
      return -1;
    }
    return 0;
  }
  else {
    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Type \"%s\" not supported by HBCI MsgEngine", type);
    return 1;
  }
}



int AH_MsgEngine_BinTypeWrite(GWEN_MSGENGINE *e,
                              GWEN_XMLNODE *node,
                              GWEN_DB_NODE *gr,
                              GWEN_BUFFER *dbuf) {
  const char *s;

  s=GWEN_XMLNode_GetProperty(node, "binType", 0);
  if (s && *s) {
    if (strcasecmp(s, "dtaus")==0) {
      int rv;
      GWEN_DBIO *dbio;
      GWEN_DB_NODE *dbCfg;
      GWEN_DB_NODE *dbTransfers;

      dbio=GWEN_DBIO_GetPlugin("dtaus");
      if (!dbio) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "DTAUS parser plugin not available");
	return GWEN_ERROR_NOT_SUPPORTED;
      }

      s=GWEN_XMLNode_GetProperty(node, "name", 0);
      assert(s);
    
      dbCfg=GWEN_DB_GetGroup(gr, GWEN_PATH_FLAGS_NAMEMUSTEXIST, s);
      assert(dbCfg);

      dbTransfers=GWEN_DB_GetGroup(dbCfg, GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                                   "transactions");

      rv=GWEN_DBIO_ExportToBuffer(dbio, dbuf,
				  dbTransfers, dbCfg,
				  GWEN_DB_FLAGS_DEFAULT);
      if (rv<0) {
	DBG_ERROR(AQHBCI_LOGDOMAIN, "Error creating DTAUS object");
	return rv;
      }

      return 0;
    }
  }

  return 1;
}



int AH_MsgEngine_TypeWrite(GWEN_MSGENGINE *e,
                           GWEN_BUFFER *gbuf,
                           GWEN_BUFFER *data,
                           GWEN_XMLNODE *node){
  AH_MSGENGINE *x;
  const char *type;
  int v;

  DBG_VERBOUS(AQHBCI_LOGDOMAIN, "AH_MsgEngine_TypeWrite");
  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);

  type=GWEN_XMLNode_GetProperty(node, "type","");
  if (strcasecmp(type, "date")==0) {
    char buffer[9];
    unsigned int i;

    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Supporting type \"date\"");
    memset(buffer,0, sizeof(buffer));
    if (GWEN_Buffer_GetBytesLeft(data)<8) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Too few bytes for date (%d<8)",
                GWEN_Buffer_GetBytesLeft(data));
      return -1;
    }
    for (i=0; i<8; i++) {
      int c;

      c=GWEN_Buffer_ReadByte(data);
      if (c==-1)
        return -1;
      if (!isdigit(c)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-digit found in date");
        return -1;
      }
      buffer[i]=c;
      if (i==3) {
        /* year is full, check it */
        v=atoi(buffer);
        if (v<1970 || v>2100) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Year out of range (%d)", v);
          return -1;
        }
      } /* if i==3 */
      else if (i==5) {
        /* month is full, check it */
        v=atoi(buffer+4);
        if (v<1 || v>12) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Month out of range (%d)", v);
          return -1;
        }
      } /* if i==5 */
    }
    v=atoi(buffer+6);
    if (v<1 || v>31) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Day out of range (%d)", v);
      return -1;
    }
    /* date read, now store it */
    if (GWEN_Buffer_AppendBytes(gbuf, buffer, 8)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
      return -1;
    }
    return 0;
  } /* if date */

  else if (strcasecmp(type, "time")==0) {
    char buffer[7];
    unsigned int i;

    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Supporting type \"time\"");
    memset(buffer,0, sizeof(buffer));
    if (GWEN_Buffer_GetBytesLeft(data)<6) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Too few bytes for time (%d<6)",
                GWEN_Buffer_GetBytesLeft(data));
      return -1;
    }
    for (i=0; i<6; i++) {
      int c;

      c=GWEN_Buffer_ReadByte(data);
      if (c==-1)
        return -1;
      if (!isdigit(c)) {
        DBG_ERROR(AQHBCI_LOGDOMAIN, "Non-digit found in date");
        return -1;
      }
      buffer[i]=c;
      if (i==1) {
        /* hour is full, check it */
        v=atoi(buffer);
        if (v>23) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Hour out of range (%d)", v);
          return -1;
        }
      } /* if i==1 */
      else if (i==3) {
        /* minutes full, check */
        v=atoi(buffer+2);
        if (v>59) {
          DBG_ERROR(AQHBCI_LOGDOMAIN, "Minutes out of range (%d)", v);
          return -1;
        }
      } /* if i==3 */
    }
    v=atoi(buffer+4);
    if (v>59) {
      DBG_ERROR(AQHBCI_LOGDOMAIN, "Seconds out of range (%d)", v);
      return -1;
    }
    /* date read, now store it */
    if (GWEN_Buffer_AppendBytes(gbuf, buffer, 6)) {
      DBG_INFO(AQHBCI_LOGDOMAIN, "called from here");
      return -1;
    }
    return 0;
  }
  else {
    DBG_VERBOUS(AQHBCI_LOGDOMAIN,
              "Type \"%s\" not supported by MsgEngineHBCI", type);
    return 1;
  }
}



GWEN_DB_NODE_TYPE AH_MsgEngine_TypeCheck(GWEN_MSGENGINE *e,
					 const char *tname){
  AH_MSGENGINE *x;

  DBG_VERBOUS(AQHBCI_LOGDOMAIN, "AH_MsgEngine_TypeCheck");

  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);

  if (strcasecmp(tname, "date")==0 ||
      strcasecmp(tname, "time")==0)
    return GWEN_DB_NodeType_ValueChar;
  else
    return GWEN_DB_NodeType_Unknown;
}



const char *AH_MsgEngine_GetCharValue(GWEN_MSGENGINE *e,
                                      const char *name,
                                      const char *defValue){
  AH_MSGENGINE *x;
  AH_HBCI *h;

  DBG_VERBOUS(AQHBCI_LOGDOMAIN, "AH_MsgEngine_GetCharValue");
  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);

  DBG_INFO(AQHBCI_LOGDOMAIN, "Mode is: \"%s\"", GWEN_MsgEngine_GetMode(e));
  DBG_INFO(AQHBCI_LOGDOMAIN, "Variable is: \"%s\"", name);
  assert(x->user);
  h=AH_User_GetHbci(x->user);
  assert(h);

  if (strcasecmp(name, "product")==0)
    return AH_HBCI_GetProductName(h);
  else if (strcasecmp(name, "productversion")==0)
    return AH_HBCI_GetProductVersion(h);

  if (strcasecmp(name, "customerid")==0)
    return AB_User_GetCustomerId(x->user);
  else if (strcasecmp(name, "userid")==0)
    return AB_User_GetUserId(x->user);
  else if (strcasecmp(name, "bankcode")==0)
    return AB_User_GetBankCode(x->user);
  else if (strcasecmp(name, "systemId")==0){
    const char *p;

    DBG_WARN(AQHBCI_LOGDOMAIN,
             "SystemId requested (deprecated), in mode \"%s\" variable \"%s\"", GWEN_MsgEngine_GetMode(e), name);
    p=AH_User_GetSystemId(x->user);
    if (p)
      return p;
    return "0";
  }
  else {
    DBG_VERBOUS(AQHBCI_LOGDOMAIN,
                "Unknown char variable \"%s\", returning default value",
		name);
    return defValue;
  }
}



int AH_MsgEngine_GetIntValue(GWEN_MSGENGINE *e,
                             const char *name,
                             int defValue){
  AH_MSGENGINE *x;

  DBG_VERBOUS(AQHBCI_LOGDOMAIN, "AH_MsgEngine_GetIntValue");
  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);

  assert(x->user);

  if (strcasecmp(name, "country")==0) {
    const char *s;
    const AB_COUNTRY *cnt;

    s=AB_User_GetCountry(x->user);
    if (!s || !*s)
      return defValue;
    cnt=AB_Banking_FindCountryByName(AB_User_GetBanking(x->user), s);
    if (cnt)
      return AB_Country_GetNumericCode(cnt);
    return defValue;
  }
  else if (strcasecmp(name, "updversion")==0)
    return AH_User_GetUpdVersion(x->user);
  else if (strcasecmp(name, "bpdversion")==0)
    return AH_User_GetBpdVersion(x->user);
  else {
    DBG_VERBOUS(AQHBCI_LOGDOMAIN, "Unknown int variable \"%s\", returning default value",
		name);
    return defValue;
  }
}



AH_MSGENGINE *AH_MsgEngine_Data_new() {
  AH_MSGENGINE *x;

  GWEN_NEW_OBJECT(AH_MSGENGINE, x);

  return x;
}



void AH_MsgEngine_Data_free(AH_MSGENGINE *x) {
  assert(x);
  DBG_INFO(AQHBCI_LOGDOMAIN, "Destroying AH_MSGENGINE");
  GWEN_FREE_OBJECT(x);
}



void GWENHYWFAR_CB AH_MsgEngine_FreeData(void *bp, void *p) {
  AH_MSGENGINE *x;

  x=(AH_MSGENGINE*)p;
  AH_MsgEngine_Data_free(x);
}



void AH_MsgEngine_SetUser(GWEN_MSGENGINE *e, AB_USER *u){
  AH_MSGENGINE *x;

  assert(e);
  x=GWEN_INHERIT_GETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e);
  assert(x);
  x->user=u;
}



GWEN_MSGENGINE *AH_MsgEngine_new(){
  GWEN_MSGENGINE *e;
  AH_MSGENGINE *x;

  e=GWEN_MsgEngine_new();
  assert(e);
  x=AH_MsgEngine_Data_new();
  GWEN_INHERIT_SETDATA(GWEN_MSGENGINE, AH_MSGENGINE, e, x,
                       AH_MsgEngine_FreeData);
  GWEN_MsgEngine_SetTypeReadFunction(e, AH_MsgEngine_TypeRead);
  GWEN_MsgEngine_SetTypeWriteFunction(e, AH_MsgEngine_TypeWrite);
  GWEN_MsgEngine_SetTypeCheckFunction(e, AH_MsgEngine_TypeCheck);
  GWEN_MsgEngine_SetBinTypeWriteFunction(e, AH_MsgEngine_BinTypeWrite);
  GWEN_MsgEngine_SetGetCharValueFunction(e, AH_MsgEngine_GetCharValue);
  GWEN_MsgEngine_SetGetIntValueFunction(e, AH_MsgEngine_GetIntValue);
  GWEN_MsgEngine_SetEscapeChar(e, '?');
  return e;
}





