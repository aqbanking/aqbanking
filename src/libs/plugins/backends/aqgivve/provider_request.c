/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2023 by Rico Rommel
    email       : rico@bierrommel.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "provider.h"
#include "provider_request_p.h"
#include "gwenhywfar/json.h"
#include "gwenhywfar/json_read.h"
#include "meta.h"
#include "merchant.h"
#include <gwenhywfar/syncio_http.h>
#include <gwenhywfar/gui.h>
#include <ctype.h>


/* ------------------------------------------------------------------------------------------------
 * forward declarations
 * ------------------------------------------------------------------------------------------------
 */

static void GWENHYWFAR_CB _freeData(void *bp, void *p);
static GWEN_JSON_ELEM *_sendRequest(char *method, char *url, GWEN_DB_NODE *header_params, char *send_data_buf);

static int GWENHYWFAR_CB _cbInitSyncIo(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio);
static GWEN_DATE *_parseDate(const char *date_str);
static AB_VALUE *_parseMoney(GWEN_JSON_ELEM *value_elem);
static AB_BALANCE *_parseBalance(GWEN_JSON_ELEM *balance_elem);
static AB_TRANSACTION *_parseTransaction(GWEN_JSON_ELEM *data_elem);
static char *_createDateFilter(const GWEN_DATE *date, const char *op);
static GWEN_JSON_ELEM *_getElement(GWEN_JSON_ELEM *parent, char *key);



/* ------------------------------------------------------------------------------------------------
 * code
 * ------------------------------------------------------------------------------------------------
 */

GWEN_INHERIT(GWEN_HTTP_SESSION, AG_HTTP_SESSION_HEADER);



void _freeData(void *bp, void *p)
{
  free(p);
}



int _cbInitSyncIo(GWEN_HTTP_SESSION *sess, GWEN_SYNCIO *sio)
{
  AG_HTTP_SESSION_HEADER *xsess;


  xsess = GWEN_INHERIT_GETDATA(GWEN_HTTP_SESSION, AG_HTTP_SESSION_HEADER, sess);

  GWEN_DB_NODE *header = GWEN_SyncIo_Http_GetDbHeaderOut(sio);

  if (xsess->url) {
    GWEN_DB_NODE *command = GWEN_SyncIo_Http_GetDbCommandOut(sio);
    GWEN_DB_SetCharValue(command, GWEN_DB_FLAGS_OVERWRITE_VARS, "url", xsess->url);
  }


  GWEN_DB_NODE *var = GWEN_DB_GetFirstVar(xsess->header);

  while (var) {
    GWEN_DB_NODE *val = GWEN_DB_GetFirstValue(var);
    GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, GWEN_DB_VariableName(var),
                         GWEN_DB_GetCharValueFromNode(val));
    var = GWEN_DB_GetNextVar(var);
  }

  return 0;
}



GWEN_JSON_ELEM *_sendRequest(char *method, char *url, GWEN_DB_NODE *header_params, char *send_data_buf)
{
  char *url_base = "https://www.givve.com";

  size_t send_data_len = 0;

  if (send_data_buf) {
    send_data_len = strlen(send_data_buf);
  }


  GWEN_JSON_ELEM *root_elem = NULL;

  GWEN_HTTP_SESSION *sess;
  GWEN_BUFFER *tbuf;
  int rv;
  sess=GWEN_HttpSession_new(url_base, " https", 443);
  if (sess==NULL) {
    DBG_ERROR(AQGIVVE_LOGDOMAIN, "Could not create http session for ");
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  AG_HTTP_SESSION_HEADER *xsess;
  GWEN_NEW_OBJECT(AG_HTTP_SESSION_HEADER, xsess);
  GWEN_INHERIT_SETDATA(GWEN_HTTP_SESSION, AG_HTTP_SESSION_HEADER, sess, xsess, _freeData);

  xsess->header = header_params;
  xsess->url = url;

  GWEN_HttpSession_SetInitSyncIoFn(sess, _cbInitSyncIo);

  rv = GWEN_HttpSession_Init(sess);
  if (rv < 0) {
    DBG_INFO(AQGIVVE_LOGDOMAIN, "Session init failed: %d", rv);
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  rv=GWEN_HttpSession_SendPacket(sess, method, (const uint8_t *) send_data_buf, send_data_len);
  if (rv < 0) {
    DBG_INFO(AQGIVVE_LOGDOMAIN, "Send data failed: %d", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  tbuf=GWEN_Buffer_new(0, 50000, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, tbuf);
  if (rv < 0) {
    DBG_INFO(AQGIVVE_LOGDOMAIN, "Receive data failed: %d", rv);
    GWEN_HttpSession_Fini(sess);
    GWEN_Buffer_free(tbuf);
    GWEN_HttpSession_free(sess);
    return NULL;
  }

  root_elem = GWEN_JsonElement_fromString(GWEN_Buffer_GetStart(tbuf));

  GWEN_HttpSession_Fini(sess);
  GWEN_Buffer_free(tbuf);
  GWEN_HttpSession_free(sess);
  return root_elem;

}



char *AG_Provider_Request_GetToken(AB_USER *user)
{
  char *token = NULL;
  char text[512];
  char pw[129];
  char pw_token[1024];

  const char *identifier = AB_User_GetUserId(user);

  snprintf(text, sizeof(text)-1,
           "Please enter the password for user %s\n<html>Please enter the password for user <i>%s</i></br></html>", identifier,
           identifier);

  GWEN_Gui_GetPassword(0, pw_token, "Enter Password", text, pw, 4, sizeof(pw)-1, GWEN_Gui_PasswordMethod_Text, NULL, 0);

  char request[1024];

  snprintf(request, 1024, "{\"identifier\": \"%s\", \"password\": \"%s\", \"accessors\": [ \"voucher_owner\"]}",
           identifier, pw);

  GWEN_DB_NODE *header = GWEN_DB_Group_new("header");

  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept", "application/json");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept-Version", "v2");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Content-Type", "application/json");
  GWEN_JSON_ELEM *json_root = _sendRequest("POST", "/api/authorizations", header, request);

  if (json_root) {
    GWEN_JSON_ELEM *json_data = _getElement(json_root, "data");

    if (json_data) {
      GWEN_JSON_ELEM *json_token_type = GWEN_JsonElement_GetElementByPath(json_data, "token_type", 0);
      GWEN_JSON_ELEM *json_token = GWEN_JsonElement_GetElementByPath(json_data, "access_token", 0);

      if (json_token && json_token_type) {
        const char *token_type_str = GWEN_JsonElement_GetData(json_token_type);
        const char *token_str = GWEN_JsonElement_GetData(json_token);

        size_t token_max_len = strlen(token_type_str) + strlen(token_str) + 5;
        token = malloc(token_max_len);

        strncpy(token, token_type_str, token_max_len);
        strcat(token, " ");
        strncat(token, token_str, token_max_len - strlen(token_type_str) -1) ;

        token[0] = toupper(token[0]);

        printf("token: %s", token);

        GWEN_JsonElement_free(json_token);
        GWEN_JsonElement_free(json_token_type);
      }
      GWEN_JsonElement_free(json_data);

    }
    GWEN_JsonElement_free(json_root);

  }
  if (!token) {
    DBG_INFO(AQGIVVE_LOGDOMAIN, "no token received ");

  }
  GWEN_DB_Group_free(header);
  return token;
}



AG_VOUCHERLIST *AG_Provider_Request_GetVoucherList(char *token)
{

  int total_pages = 1;
  int current_page = 1;
  char path[512];

  AG_VOUCHERLIST *card_list = AG_VOUCHERLIST_new();

  GWEN_DB_NODE *header = GWEN_DB_Group_new("header");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept", "application/json");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept-Version", "v2");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Authorization", token);


  while (current_page <= total_pages) {

    snprintf(path, sizeof(path) - 1, "/api/vouchers?page[number]=%d",  current_page);

    GWEN_JSON_ELEM *json_root = _sendRequest("GET", path, header, NULL);

    if (json_root) {

      //Get number of entries
      AG_META *meta = AG_META_FromJsonElem(_getElement(json_root, "meta"));


      if (meta) {
        int total_entries = AG_META_GetTotalEntries(meta);
        AG_META_free(meta);

        GWEN_JSON_ELEM *json_data = _getElement(json_root, "data");
        if (json_data && (total_entries > 0)) {
          int n = 0;
          GWEN_JSON_ELEM *json_card = GWEN_JsonElement_Tree2_GetFirstChild(json_data);
          while (json_card) {
            GWEN_JSON_ELEM *json_id = GWEN_JsonElement_GetElementByPath(json_card, "id", 0);
            const char *id = GWEN_JsonElement_GetData(json_id);
            AG_VOUCHEROWNER *owner = NULL;

            GWEN_JSON_ELEM *json_owner = GWEN_JsonElement_GetElementByPath(json_card, "owner", 0);
            if (json_owner) {
              GWEN_JSON_ELEM *json_name = GWEN_JsonElement_GetElementByPath(json_owner, "name", 0);
              const char *owner_name = GWEN_JsonElement_GetData(json_name);
              owner = AG_VOUCHEROWNER_new(owner_name);

            }
            AG_VOUCHER *card = AG_VOUCHER_new(id, owner);
            AG_VOUCHERLIST_AddCard(card_list, card);

            DBG_INFO(AQGIVVE_LOGDOMAIN, "found card %d: %s", n, id);
            json_card = GWEN_JsonElement_Tree2_GetNext(json_card);
            n++;
          }

        }

      }
      GWEN_JsonElement_free(json_root);
    }
    current_page++;
  }

  return card_list;
}



AB_BALANCE *AG_Provider_Request_GetBalance(AB_ACCOUNT *account, char *token)
{

  const char *id = AB_Account_GetAccountNumber(account);
  char path[512];

  snprintf(path, sizeof(path) - 1, "/api/vouchers/%s", id);

  GWEN_DB_NODE *header = GWEN_DB_Group_new("header");

  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept", "application/json");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept-Version", "v2");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Authorization", token);
  GWEN_JSON_ELEM *json_root = _sendRequest("GET", path, header, NULL);

  AB_BALANCE *bal = NULL;

  if (json_root) {
    GWEN_JSON_ELEM *json_data = _getElement(json_root, "data");
    if (json_data) {
      bal = _parseBalance(GWEN_JsonElement_GetElementByPath(json_data, "balance", 0));
      if (bal) {
        GWEN_JSON_ELEM *json_date = GWEN_JsonElement_GetElementByPath(json_data, "updated_at", 0);
        if (json_date) {
          AB_Balance_SetDate(bal, _parseDate(GWEN_JsonElement_GetData(json_date)));
          GWEN_JsonElement_free(json_date);
        }

      }
      GWEN_JsonElement_free(json_data);

    }
    GWEN_JsonElement_free(json_root);

  }
  GWEN_DB_Group_free(header);
  return bal;

}



AB_TRANSACTION_LIST *AG_Provider_Request_GetTransactions(AB_ACCOUNT *account, const GWEN_DATE *start_date,
                                                         const GWEN_DATE *end_date, char *token)
{
  int total_pages = 1;
  int current_page = 1;
  char path[512];

  AB_TRANSACTION_LIST *trans_list = AB_Account_List_new();

  const char *id = AB_Account_GetAccountNumber(account);

  GWEN_DB_NODE *header = GWEN_DB_Group_new("header");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept", "application/json");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Accept-Version", "v2");
  GWEN_DB_SetCharValue(header, GWEN_DB_FLAGS_OVERWRITE_VARS, "Authorization", token);

  while (current_page <= total_pages) {

    char *filter_start = _createDateFilter(start_date, "$gte");
    char *filter_end = _createDateFilter(end_date, "$lte");

    snprintf(path, sizeof(path) - 1, "/api/vouchers/%s/transactions/?page[number]=%d&filter[status][$in]=Settled%s%s", id,
             current_page, filter_start, filter_end);

    free(filter_start);
    free(filter_end);

    GWEN_JSON_ELEM *json_root = _sendRequest("GET", path, header, NULL);

    if (json_root) {
      GWEN_JSON_ELEM *json_meta = _getElement(json_root, "meta");
      if (json_meta) {
        AG_META *meta = AG_META_FromJsonElem(json_meta);

        if (meta) {
          total_pages =  AG_META_GetTotalPages(meta);
          AG_META_free(meta);
        }
        GWEN_JsonElement_free(json_meta);

      }

      GWEN_JSON_ELEM *json_data = _getElement(json_root, "data");

      if (json_data) {
        GWEN_JSON_ELEM *json_transaction = GWEN_JsonElement_Tree2_GetFirstChild(json_data);
        while (json_transaction) {

          AB_TRANSACTION *t = _parseTransaction(json_transaction);


          if (t) {
            AB_Transaction_List_Add(t, trans_list);
          }
          json_transaction = GWEN_JsonElement_Tree2_GetNext(json_transaction);
        }
        GWEN_JsonElement_free(json_data);
      }
      GWEN_JsonElement_free(json_root);
    }
    current_page++;
  }

  return trans_list;
}



AB_BALANCE *_parseBalance(GWEN_JSON_ELEM *balance_elem)
{
  AB_BALANCE *bal = NULL;
  AB_VALUE *vc = _parseMoney(balance_elem);
  if (vc) {
    bal=AB_Balance_new();
    AB_Balance_SetType(bal, AB_Balance_TypeBooked);
    AB_Balance_SetValue(bal, vc);
    AB_Value_free(vc);
  }
  return bal;
}



GWEN_DATE *_parseDate(const char *date_str)
{
  GWEN_DATE *date =NULL;
  char *buf = strdup(date_str);

  char *ptr = strchr(buf, 'T');
  if (ptr) {
    *ptr = '\0';
    date = GWEN_Date_fromStringWithTemplate(buf, "YYYY-MM-DD");
  }
  free(buf);
  return date;
}



AB_VALUE *_parseMoney(GWEN_JSON_ELEM *value_elem)
{
  AB_VALUE *val = NULL;

  if (value_elem) {
    GWEN_JSON_ELEM *json_cents = GWEN_JsonElement_GetElementByPath(value_elem, "cents", 0);
    GWEN_JSON_ELEM *json_currency = GWEN_JsonElement_GetElementByPath(value_elem, "currency", 0);
    if (json_cents && json_currency) {
      const char *cent_str = GWEN_JsonElement_GetData(json_cents);
      const char *currency = GWEN_JsonElement_GetData(json_currency) ;

      double d = strtod(cent_str, NULL);
      val = AB_Value_fromDouble(d / 100);
      AB_Value_SetCurrency(val, currency);
    }
  }

  return val;

}



AB_TRANSACTION *_parseTransaction(GWEN_JSON_ELEM *data_elem)
{
  AB_TRANSACTION *t = AB_Transaction_new();
  GWEN_JSON_ELEM *json_amount = GWEN_JsonElement_GetElementByPath(data_elem, "amount", 0);
  if (json_amount) {
    AB_VALUE *v = _parseMoney(json_amount);
    if (v) {
      AB_Transaction_SetValue(t, v);
    }
  }

  GWEN_JSON_ELEM *json_date = GWEN_JsonElement_GetElementByPath(data_elem, "booked_at", 0);
  if (json_date) {
    GWEN_DATE *d = _parseDate(GWEN_JsonElement_GetData(json_date));
    if (d) {
      AB_Transaction_SetDate(t, d);
      AB_Transaction_SetValutaDate(t, d);
    }
  }

  GWEN_JSON_ELEM *json_id = GWEN_JsonElement_GetElementByPath(data_elem, "id", 0);
  if (json_id) {
    AB_Transaction_SetFiId(t, GWEN_JsonElement_GetData(json_id));
  }

  GWEN_JSON_ELEM *json_fee = GWEN_JsonElement_GetElementByPath(data_elem, "fixed_fee", 0);
  if (json_fee) {
    AB_VALUE *v = _parseMoney(json_fee);
    if (v) {
      AB_Transaction_SetFees(t, v);
    }
  }

  GWEN_JSON_ELEM *json_desc = GWEN_JsonElement_GetElementByPath(data_elem, "description", 0);
  if (json_desc) {
    AB_Transaction_SetPurpose(t, GWEN_JsonElement_GetData(json_desc));

  }

  const char *type_str = NULL;

  GWEN_JSON_ELEM *json_type = GWEN_JsonElement_GetElementByPath(data_elem, "type", 0);

  if (json_type) {
    type_str = GWEN_JsonElement_GetData(json_type);
    AB_Transaction_SetTransactionText(t, type_str);
  }

  if (strcmp(type_str, "load") == 0) {
    AB_Transaction_SetRemoteName(t, "Aufladung");
  }
  else {
    GWEN_JSON_ELEM *json_merchant = GWEN_JsonElement_GetElementByPath(data_elem, "merchant", 0);
    if (json_merchant) {
      AG_MERCHANT *m = AG_MERCHANT_FromJsonElem(json_merchant);
      if (m) {
        AB_Transaction_SetRemoteName(t, AG_MERCHANT_GetName(m));
      }
    }
  }

  AB_Transaction_SetStatus(t, AB_Transaction_StatusAccepted);
  AB_Transaction_SetType(t, AB_Transaction_TypeStatement);
  AB_Transaction_SetSubType(t, AB_Transaction_SubTypeNone);

  return t;
}



char *_createDateFilter(const GWEN_DATE *date, const char *op)
{
  char *filter = malloc(60);
  if (date && op) {
    GWEN_BUFFER *date_buf = GWEN_Buffer_new(0, 20, 0, 1);
    GWEN_Date_toStringWithTemplate(date, "YYYY-MM-DD", date_buf);
    snprintf(filter, 60, "&filter[created_at][%s]=%s", op, GWEN_Buffer_GetStart(date_buf));

    GWEN_Buffer_free(date_buf);
  }
  else {
    filter[0] = '\0';
  }

  return filter;
}



GWEN_JSON_ELEM *_getElement(GWEN_JSON_ELEM *parent, char *key)
{
  GWEN_JSON_ELEM *json_el = NULL;

  GWEN_JSON_ELEM *json_child = GWEN_JsonElement_Tree2_GetFirstChild(parent);

  while (!json_el && json_child) {
    if (json_child) {
      if (strcmp(GWEN_JsonElement_GetData(json_child), key) == 0) {
        json_el = GWEN_JsonElement_Tree2_GetFirstChild(json_child);
      }
      json_child = GWEN_JsonElement_Tree2_GetNext(json_child);
    }
  }

  return json_el;
}


