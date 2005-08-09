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


#include "wizard.h"
#include <qbanking/qbanking.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qwizard.h>
#include <qcombobox.h>
#include <qtextbrowser.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qpalette.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qprinter.h>
#include <qsimplerichtext.h>
#include <qtextview.h>
#include <qlabel.h>
#include <qdatetime.h>

#include <aqhbci/hbci.h>
#include <aqhbci/provider.h>
#include <aqbanking/banking_be.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/gwentime.h>
#include <gwenhywfar/nettransportssl.h>
#include <gwenhywfar/netconnectionhttp.h>



bool Wizard::initServerCertPage() {
  QObject::connect((QObject*)(getCertButton),
                   SIGNAL(clicked()),
                   this,
                   SLOT(slotGetCert()));
  return true;
}




bool Wizard::doServerCertPage(QWidget *p) {
  return true;
}



bool Wizard::undoServerCertPage(QWidget *p) {
  return true;
}



GWEN_NETTRANSPORTSSL_ASKADDCERT_RESULT
Wizard::_askAddCert(GWEN_NETTRANSPORT *tr,
                    GWEN_DB_NODE *cert){
  QString t;
  const char *s;
  GWEN_TYPE_UINT32 ti;
  int rv;

  t="<qt><p>";
  t+=QWidget::tr("Received an unknown certificate:");
  t+="</p>";
  t+="<table>";

  t+="<tr><td>";
  t+=QWidget::tr("Name");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "commonName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("Organization");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "organizationName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("Department");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "organizationalUnitName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("Country");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "countryName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("City");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "localityName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("State");
  t+="</td><td>";
  s=GWEN_DB_GetCharValue(cert, "stateOrProvinceName", 0, 0);
  if (s)
    t+=s;
  else
    t+="(unknown)";
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("Valid after");
  t+="</td><td>";
  ti=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(cert, "notBefore", 0, 0);
  if (ti) {
    GWEN_TIME *gt;
    int year, month, day;
    int hour, min, sec;

    gt=GWEN_Time_fromSeconds(ti);
    if (!GWEN_Time_GetBrokenDownDate(gt, &day, &month, &year)) {
      QDate d(year, month+1, day);

      t+=d.toString();
    }
    t+=" ";
    if (!GWEN_Time_GetBrokenDownTime(gt, &hour, &min, &sec)) {
      QTime d(hour, min, sec);

      t+=d.toString();
    }
    GWEN_Time_free(gt);
  }
  t+="</td></tr>";

  t+="<tr><td>";
  t+=QWidget::tr("Valid until");
  t+="</td><td>";
  ti=(GWEN_TYPE_UINT32)GWEN_DB_GetIntValue(cert, "notAfter", 0, 0);
  if (ti) {
    GWEN_TIME *gt;
    int year, month, day;
    int hour, min, sec;

    gt=GWEN_Time_fromSeconds(ti);
    if (GWEN_Time_GetBrokenDownDate(gt, &day, &month, &year)) {
      QDate d(year, month+1, day);

      t+=d.toString();
    }
    t+=" ";
    if (GWEN_Time_GetBrokenDownTime(gt, &hour, &min, &sec)) {
      QTime d(hour, min, sec);

      t+=d.toString();
    }
    GWEN_Time_free(gt);
  }
  t+="</td></tr></table>";


  t+="<br><tr><td colspan=\"2\"><p>";
  t+=QWidget::tr("Do you accept this certificate?");
  t+="</p></qt>";

  rv=QMessageBox::warning(0, QWidget::tr("New Certificate"),
                          t,
                          QWidget::tr("Yes"),
                          QWidget::tr("No"));
  if (rv!=0) {
    DBG_NOTICE(0, "User rejected certificate");
    return GWEN_NetTransportSSL_AskAddCertResultNo;
  }

  rv=QMessageBox::warning(0,
                          QWidget::tr("New Certificate"),
                          QWidget::tr("Do you accept this certificate "
                                      "permanently\n"
                                      "or for this session only (temporarily)?"
                                     ),
                          QWidget::tr("Permanently"),
                          QWidget::tr("Temporarily"));
  if (rv==0) {
    DBG_NOTICE(0, "Trusting permanently");
    return GWEN_NetTransportSSL_AskAddCertResultPerm;
  }
  else {
    DBG_NOTICE(0, "Trusting temporarily");
    return GWEN_NetTransportSSL_AskAddCertResultTmp;
  }
}






void Wizard::slotGetCert() {
  int country;
  const char *bankId;
  GWEN_BUFFER *nbuf;
  char *p;
  GWEN_NETTRANSPORT *tr;
  GWEN_NETCONNECTION *conn;
  GWEN_SOCKET *sk;
  GWEN_INETADDRESS *addr;
  GWEN_DB_NODE *dbCert;
  int rv;
  GWEN_ERRORCODE err;
  GWEN_TYPE_UINT32 wid;
  AB_PROVIDER *pro;
  int alwaysAskForCert;

  country=AH_Bank_GetCountry(_bank);
  bankId=AH_Bank_GetBankId(_bank);
  nbuf=GWEN_Buffer_new(0, 64, 0, 1);
  AH_HBCI_AddBankCertFolder(_hbci, _bank, nbuf);

  AH_HBCI_RemoveAllBankCerts(_hbci, _bank);

  sk=GWEN_Socket_new(GWEN_SocketTypeTCP);
  tr=GWEN_NetTransportSSL_new(sk,
                              GWEN_Buffer_GetStart(nbuf),
                              GWEN_Buffer_GetStart(nbuf),
                              0, 0, 0, 1);
  GWEN_Buffer_Reset(nbuf);
  GWEN_Buffer_AppendString(nbuf, getServerAddr().latin1());
  p=strchr(GWEN_Buffer_GetStart(nbuf), '/');
  if (p)
    *p=0;
  addr=GWEN_InetAddr_new(GWEN_AddressFamilyIP);
  err=GWEN_InetAddr_SetAddress(addr, GWEN_Buffer_GetStart(nbuf));
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "Resolving hostname \"%s\"",
	     GWEN_Buffer_GetStart(nbuf));
    err=GWEN_InetAddr_SetName(addr, GWEN_Buffer_GetStart(nbuf));
    if (!GWEN_Error_IsOk(err)) {
      DBG_ERROR(0,
		"Error resolving hostname \"%s\":",
		GWEN_Buffer_GetStart(nbuf));
      DBG_ERROR_ERR(0, err);
      QMessageBox::critical(0,
			    QWidget::tr("Network Error"),
			    QWidget::tr("Could not resolve the given "
					"address.\n"
					"May there is a typo?"),
			    QWidget::tr("Dismiss"),0,0,0);
      GWEN_NetTransport_free(tr);
      GWEN_InetAddr_free(addr);
      GWEN_Buffer_free(nbuf);
      return;
    }
  }

  GWEN_InetAddr_SetPort(addr, 443);
  GWEN_NetTransport_SetPeerAddr(tr, addr);
  GWEN_InetAddr_free(addr);
  /* use HTTP 1.0 */
  conn=GWEN_NetConnectionHTTP_new(tr, 1, 999,
                                  1,0);
  assert(conn);
  GWEN_NetConnection_SetUserMark(conn, 999);
  if (p)
    *p='/';


  // connect
  pro=AB_Banking_GetProvider(_app->getCInterface(), "aqhbci");
  alwaysAskForCert=AB_Banking_GetAlwaysAskForCert(_app->getCInterface());
  AB_Banking_SetAlwaysAskForCert(_app->getCInterface(), 1);
  wid=AB_Banking_ShowBox(_app->getCInterface(),
			 0,
			 QWidget::tr("Please Wait").latin1(),
			 QWidget::tr("Retrieving certificate...").latin1());
  rv=GWEN_NetConnection_Connect_Wait(conn, 15);
  if (wid)
    AB_Banking_HideBox(_app->getCInterface(), wid);
  AB_Banking_SetAlwaysAskForCert(_app->getCInterface(), alwaysAskForCert);
  if (rv) {
    DBG_ERROR(0, "Could not connect");
    QMessageBox::critical(0,
                          QWidget::tr("Network Error"),
                          QWidget::tr("Could not connect to the bank's "
                                      "server.\n"
                                      "Please check the logs."
                                     ),
                          QWidget::tr("Dismiss"),0,0,0);
    GWEN_NetConnection_StartDisconnect(conn);
    GWEN_NetConnection_free(conn);
    return;
  }

  dbCert=GWEN_NetTransportSSL_GetPeerCertificate(tr);
  GWEN_NetConnection_StartDisconnect(conn);
  GWEN_NetConnection_free(conn);
  if (!dbCert) {
    QMessageBox::critical(0,
			  QWidget::tr("No Certificate"),
			  QWidget::tr(
    "<qt>"
    "<p>"
    "The server does not seem to use valid SSL certificates."
    "</p>"
    "<p>"
    "<font color=red>"
    "We strongly suggest not using PIN/TAN mode with this server, "
    "because without a valid SSL certificate you can never be sure "
    "about to whom you are talking."
    "</font>"
    "</p>"
    "<p>"
    "However, you may continue using this bank server at your own risk, "
    "but you have been <b>warned</b>!"
				      "</p>"
    "</qt>"
				     ),
                          QWidget::tr("Dismiss"),0,0,0);

  }
  else {
    QMessageBox::information(0,
                             QWidget::tr("Certificate Accepted"),
                             QWidget::tr("Either the certificate is ok or "
                                         "you accepted it anyway.\n"
                                         "In either case a usable "
                                         "certificate is present."
                                        ),
                             QWidget::tr("Dismiss"),0,0,0);
  }
}











