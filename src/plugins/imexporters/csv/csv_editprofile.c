/***************************************************************************
    begin       : Sat Jan 13 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "csv_editprofile_p.h"
#include "i18n_l.h"

#include <gwenhywfar/gwenhywfar.h>
#include <gwenhywfar/misc.h>
#include <gwenhywfar/pathmanager.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/syncio_file.h>
#include <gwenhywfar/syncio_buffered.h>
#include <gwenhywfar/stringlist.h>
#include <gwenhywfar/text.h>

#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>


#define DIALOG_MINWIDTH  400
#define DIALOG_MINHEIGHT 400

#define DIALOG_MAX_COLUMNS   30
#define DIALOG_MAX_TESTLINES 100



GWEN_INHERIT(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG)


static const char *csv_dateFormats[]={
  "DD.MM.YYYY",
  "dD.mM.YYYY",
  "MM/DD/YYYY",
  "mM/dD/YYYY",
  "DD/MM/YYYY",
  "dD/mM/YYYY",
  "YYYY/MM/DD",
  "YYYY/mM/dD",
  NULL
};


static const char *csv_delimiters[]={
  "TAB", I18S("Tabulator (default)"),
  "SPACE", I18S("Space"),
  ",", I18S("Komma (,)"),
  ";", I18S("Semicolon (;)"),
  ":", I18S("Colon (:)"),
  NULL
};


static const char *csv_subjects[]={
  "transactions", I18S("Booked Transactions (default)"),
  "notedTransactions", I18S("Noted Transactions"),
  NULL
};



static const char *csv_amountFormats[]={
  "rational", I18S("Rational (default)"),
  "float", I18S("Float"),
  NULL
};


static const char *csv_columns[]={
  "",                     I18S("-- empty --"),
  "localCountry",         I18S("Local Country Code"),
  "localBankCode",        I18S("Local Bank Code"),
  "localBranchId",        I18S("Local Branch Id"),
  "localAccountNumber",   I18S("Local Account Number"),
  "localSuffix",          I18S("Local Account Suffix"),
  "localIban",            I18S("Local IBAN"),
  "localName",            I18S("Local Name (e.g. your name)"),
  "localBIC",             I18S("Local BIC"),
  "remoteCountry",        I18S("Remote Country Code"),
  "remoteBankCode",       I18S("Remote Bank Code"),
  "remoteBranchId",       I18S("Remote Branch Id"),
  "remoteAccountNumber",  I18S("Remote Account Number"),
  "remoteSuffix",         I18S("Remote Account Suffix"),
  "remoteIban",           I18S("Remote IBAN"),
  "remoteName[0]",        I18S("Remote Name (First Line)"),
  "remoteName[1]",        I18S("Remote Name (Second Line)"),
  "remoteBIC",            I18S("Remote BIC"),
  "uniqueId",             I18S("Unique Transaction Id"),
  "idForApplication",     I18S("Id assigned by Application"),
//  "groupId",              I18S("Group Id"),
  "valutaDate",           I18S("Valuta Date"),
  "date",                 I18S("Booking Date"),
  "value/value",          I18S("Amount (Value)"),
  "value/currency",       I18S("Amount (Currency)"),
  "fees/value",           I18S("Fees (Amount)"),
  "fees/currency",        I18S("Fees (Currency)"),
  "textKey",              I18S("Textkey"),
  "textKeyExt",           I18S("Textkey Extensions"),
  "transactionKey",       I18S("Transaction Key"),
  "customerReference",    I18S("Customer Reference"),
  "bankReference",        I18S("Bank Reference"),
  "transactionCode",      I18S("Transaction Code"),
  "transactionText",      I18S("Transaction Text (not purpose!)"),
  "primanota",            I18S("Primanota"),
  "fiId",                 I18S("Id assigned by Finance Institute"),
  "purpose[0]",           I18S("Purpose (1st Line)"),
  "purpose[1]",           I18S("Purpose (2nd Line)"),
  "purpose[2]",           I18S("Purpose (3rd Line)"),
  "purpose[3]",           I18S("Purpose (4th Line)"),
  "purpose[4]",           I18S("Purpose (5th Line)"),
  "purpose[5]",           I18S("Purpose (6th Line)"),
  "purpose[6]",           I18S("Purpose (7th Line)"),
  "purpose[7]",           I18S("Purpose (8th Line)"),
  "purpose[8]",           I18S("Purpose (9th Line)"),
  "purpose[9]",           I18S("Purpose (10th Line)"),
  "category[0]",          I18S("Category (1st Line)"),
  "category[1]",          I18S("Category (2nd Line)"),
  "category[2]",          I18S("Category (3rd Line)"),
  "category[3]",          I18S("Category (4th Line)"),
  "category[4]",          I18S("Category (5th Line)"),
  "category[5]",          I18S("Category (6th Line)"),
  "category[6]",          I18S("Category (7th Line)"),
  "category[7]",          I18S("Category (8th Line)"),
  "period",               I18S("Period (Standing Order)"),
  "cycle",                I18S("Cycle (Standing Order)"),
  "executionDay",         I18S("Execution Day (Standing Order)"),
  "firstExecutionDate",   I18S("Date of First Execution (Standing Order)"),
  "lastExecutionDate",    I18S("Date of Last Execution (Standing Order)"),
  "nextExecutionDate",    I18S("Date of Next Execution (Standing Order)"),
  "type",                 I18S("Type"),
  "subtype",              I18S("Subtype"),
  "status",               I18S("Status"),
  "remoteAddrStreet",     I18S("Remote Address: Street"),
  "remoteAddrZipCode",    I18S("Remote Address: Zipcode"),
  "remoteAddrCity",       I18S("Remote Address: City"),
  "remotePhone",          I18S("Remote Address: Phone Number"),
  "unitId",               I18S("Unit Id (Stock)"),
  "unitIdNameSpace",      I18S("Namespace of Unit Id (Securities)"),
  "units/value",          I18S("Amount of Units (Securities) (value)"),
  "units/currency",       I18S("Amount of Units (Securities) (currency)"),
  "unitprice/value",      I18S("Price per Unit (Securities) (value)"),
  "unitprice/currency",   I18S("Price per Unit (Securities) (currency)"),
  "commission/value",     I18S("Commission (Securities) (value)"),
  "commission/currency",  I18S("Commission (Securities) (currency)"),
  "bankAccountId",        I18S("Bank Account Id"),
  "posNeg",               I18S("Positive/Negative Mark"),
  NULL
};



GWEN_DIALOG *AB_CSV_EditProfileDialog_new(AB_IMEXPORTER *ie,
					  GWEN_DB_NODE *dbProfile,
					  const char *testFileName) {
  AB_BANKING *ab;
  GWEN_DIALOG *dlg;
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;
  GWEN_BUFFER *fbuf;
  int rv;

  assert(ie);
  assert(dbProfile);

  ab=AB_ImExporter_GetBanking(ie);
  dlg=GWEN_Dialog_new("ab_csv_edit_profile");
  GWEN_NEW_OBJECT(AB_CSV_EDIT_PROFILE_DIALOG, xdlg);
  GWEN_INHERIT_SETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg, xdlg,
		       AB_CSV_EditProfileDialog_FreeData);
  GWEN_Dialog_SetSignalHandler(dlg, AB_CSV_EditProfileDialog_SignalHandler);

  /* get path of dialog description file */
  fbuf=GWEN_Buffer_new(0, 256, 0, 1);
  rv=GWEN_PathManager_FindFile(AB_PM_LIBNAME, AB_PM_DATADIR,
			       "aqbanking/imexporters/csv/dialogs/csv_editprofile.dlg",
			       fbuf);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "Dialog description file not found (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }

  /* read dialog from dialog description file */
  rv=GWEN_Dialog_ReadXmlFile(dlg, GWEN_Buffer_GetStart(fbuf));
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d).", rv);
    GWEN_Buffer_free(fbuf);
    GWEN_Dialog_free(dlg);
    return NULL;
  }
  GWEN_Buffer_free(fbuf);

  xdlg->banking=ab;
  xdlg->imExporter=ie;
  xdlg->testFileName=testFileName;
  xdlg->dbProfile=dbProfile;

  xdlg->columns=GWEN_StringList_new();

  /* done */
  return dlg;
}



void GWENHYWFAR_CB AB_CSV_EditProfileDialog_FreeData(void *bp, void *p) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;

  xdlg=(AB_CSV_EDIT_PROFILE_DIALOG*) p;
  assert(xdlg);

  GWEN_Buffer_free(xdlg->dataBuffer);

  GWEN_FREE_OBJECT(xdlg);
}



static const char *getCharValueFromDoubleStringsCombo(GWEN_DIALOG *dlg,
						      const char *comboBoxName,
						      const char **strings) {
  int i;
  int j;

  /* count number of string entries */
  for (j=0; ; j+=2) {
    if (strings[j]==NULL)
      break;
  }
  j>>=1;

  i=GWEN_Dialog_GetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, -1);
  if (i<0 || i>=j) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Index %d in %s out of range (%d)", i, comboBoxName, j);
    return NULL;
  }

  return strings[i*2];
}



static int readTestData(GWEN_DIALOG *dlg) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;
  GWEN_SYNCIO *sio;
  GWEN_SYNCIO *baseIo;
  GWEN_BUFFER *dbuf;
  int i;
  int ignoreLines=0;
  int rv;
  const char *delimiter;
  GWEN_BUFFER *wbuffer;
  const char *s;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  GWEN_StringList_Clear(xdlg->columns);

  /* create file IO */
  sio=GWEN_SyncIo_File_new(xdlg->testFileName, GWEN_SyncIo_File_CreationMode_OpenExisting);
  GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
  baseIo=sio;

  /* create buffered IO on top of file io to allow for reading of lines below */
  sio=GWEN_SyncIo_Buffered_new(baseIo);

  dbuf=GWEN_Buffer_new(0, 1024, 0, 1);

  /* skip lines */
  ignoreLines=GWEN_Dialog_GetIntProperty(dlg, "ignoreLinesSpin", GWEN_DialogProperty_Value, 0, 0);
  i=GWEN_Dialog_GetIntProperty(dlg, "headerCheck", GWEN_DialogProperty_Value, 0, 0);
  if (i)
    ignoreLines++;

  delimiter=getCharValueFromDoubleStringsCombo(dlg, "delimiterCombo", csv_delimiters);
  if (!(delimiter && *delimiter))
    delimiter="TAB";
  if (strcasecmp(delimiter, "TAB")==0)
    delimiter="\t";
  else if (strcasecmp(delimiter, "SPACE")==0)
    delimiter=" ";

  /* open file */
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(dbuf);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  for (i=0; i<ignoreLines; i++) {
    rv=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, dbuf);
    if (rv<0) {
      DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading: %d", rv);
      GWEN_Buffer_free(dbuf);
      GWEN_SyncIo_Disconnect(sio);
      GWEN_SyncIo_free(sio);
      return rv;
    }
    GWEN_Buffer_Reset(dbuf);
  }

  /* read single data line */
  rv=GWEN_SyncIo_Buffered_ReadLineToBuffer(sio, dbuf);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Error reading: %d", rv);
    GWEN_Buffer_free(dbuf);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_Dialog_SetCharProperty(dlg, "dataEdit", GWEN_DialogProperty_Value, 0,
			      GWEN_Buffer_GetStart(dbuf), 0);

  /* we don't need the io layer any longer */
  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  wbuffer=GWEN_Buffer_new(0, 256, 0, 1);
  s=GWEN_Buffer_GetStart(dbuf);
  while(*s) {
    rv=GWEN_Text_GetWordToBuffer(s, delimiter, wbuffer,
				 GWEN_TEXT_FLAGS_DEL_LEADING_BLANKS |
				 GWEN_TEXT_FLAGS_DEL_TRAILING_BLANKS |
				 GWEN_TEXT_FLAGS_NULL_IS_DELIMITER |
				 GWEN_TEXT_FLAGS_DEL_QUOTES,
				 &s);
    if (rv) {
      DBG_DEBUG(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(wbuffer);
      GWEN_Buffer_free(dbuf);
      return rv;
    }
    GWEN_StringList_AppendString(xdlg->columns, GWEN_Buffer_GetStart(wbuffer), 0, 0);
    GWEN_Buffer_Reset(wbuffer);
    if (*s) {
      if (strchr(delimiter, *s))
	s++;
    }
  } /* while */
  GWEN_Buffer_free(wbuffer);

  for (i=0; i<DIALOG_MAX_COLUMNS; i++) {
    char editName[32];

    snprintf(editName, sizeof(editName)-1, "col%dDataEdit", i+1);
    s=GWEN_StringList_StringAt(xdlg->columns, i);
    if (s==NULL)
      break;
    GWEN_Dialog_SetCharProperty(dlg, editName, GWEN_DialogProperty_Value, 0, s, 0);
  }

  return 0;
}



static void setUpComboFromSingleStrings(GWEN_DIALOG *dlg,
					const char *comboBoxName,
					const char **strings,
					const char *s) {
  int i;
  int j;
  const char *t;

  GWEN_Dialog_SetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  j=-1;
  for (i=0; ; i++) {
    t=strings[i];
    if (t==NULL)
      break;
    GWEN_Dialog_SetCharProperty(dlg, comboBoxName, GWEN_DialogProperty_AddValue, 0, t, 0);
    if (s && *s && strcmp(s, t)==0)
      j=i;
  }
  if (j==-1) {
    if (s && *s) {
      GWEN_Dialog_SetCharProperty(dlg, comboBoxName, GWEN_DialogProperty_AddValue, 0, s, 0);
      j=i;
    }
    else
      j=0;
  }
  GWEN_Dialog_SetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, j, 0);
}



static void setUpComboFromDoubleStrings(GWEN_DIALOG *dlg,
					const char *comboBoxName,
					const char **strings,
					const char *s) {
  int i;
  int j;
  const char *t1;
  const char *t2;

  GWEN_Dialog_SetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_ClearValues, 0, 0, 0);
  j=-1;
  for (i=0; ; i+=2) {
    t1=strings[i];
    if (t1==NULL)
      break;
    t2=strings[i+1];
    GWEN_Dialog_SetCharProperty(dlg, comboBoxName, GWEN_DialogProperty_AddValue, 0, I18N(t2), 0);
    if (s && *s && strcasecmp(s, t1)==0)
      j=i/2;
  }
  if (j==-1) {
    if (s && *s) {
      GWEN_Dialog_SetCharProperty(dlg, comboBoxName, GWEN_DialogProperty_AddValue, 0, s, 0);
      j=i/2;
    }
    else
      j=0;
  }
  GWEN_Dialog_SetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, j, 0);
}



static int setDbValueFromDoubleStringsCombo(GWEN_DIALOG *dlg,
					    GWEN_DB_NODE *db,
                                            const char *varName,
					    const char *comboBoxName,
					    const char **strings) {
  int i;
  int j;

  /* count number of string entries */
  for (j=0; ; j+=2) {
    if (strings[j]==NULL)
      break;
  }
  j>>=1;

  i=GWEN_Dialog_GetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, -1);
  if (i<0 || i>=j) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Index %d in %s out of range (%d)", i, comboBoxName, j);
    return GWEN_ERROR_INVALID;
  }
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, strings[i*2]);
  return 0;
}



static int setColumnValueFromCombo(GWEN_DIALOG *dlg,
				   GWEN_DB_NODE *db,
				   const char *varName,
				   const char *comboBoxName,
				   const char **strings) {
  int i;
  int j;

  /* count number of string entries */
  for (j=0; ; j+=2) {
    if (strings[j]==NULL)
      break;
  }
  j>>=1;

  i=GWEN_Dialog_GetIntProperty(dlg, comboBoxName, GWEN_DialogProperty_Value, 0, -1);
  if (i<0 || i>=j) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Index %d of %s out of range (%d)", i, comboBoxName, j);
    return GWEN_ERROR_INVALID;
  }
  if (i!=0)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, varName, strings[i*2]);
  return 0;
}



void AB_CSV_EditProfileDialog_Init(GWEN_DIALOG *dlg) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;
  int i;
  const char *s;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  /* setup dialog size */
  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* read width */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_width", 0, -1);
  if (i>=DIALOG_MINWIDTH)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, i, 0);

  /* read height */
  i=GWEN_DB_GetIntValue(dbPrefs, "dialog_height", 0, -1);
  if (i>=DIALOG_MINHEIGHT)
    GWEN_Dialog_SetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, i, 0);

  GWEN_Dialog_SetCharProperty(dlg,
			      "",
			      GWEN_DialogProperty_Title,
			      0,
			      I18N("Edit CSV Profile"),
			      0);

  /* setup dialog widgets */
  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "name", 0, NULL);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "nameEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "version", 0, NULL);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "versionEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "shortDescr", 0, NULL);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "shortDescrEdit", GWEN_DialogProperty_Value, 0, s, 0);

  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "longDescr", 0, NULL);
  if (s && *s)
    GWEN_Dialog_SetCharProperty(dlg, "longDescrEdit", GWEN_DialogProperty_Value, 0, s, 0);

  i=GWEN_DB_GetIntValue(xdlg->dbProfile, "import", 0, 1);
  GWEN_Dialog_SetIntProperty(dlg, "importCheck", GWEN_DialogProperty_Value, 0, i?1:0, 0);

  i=GWEN_DB_GetIntValue(xdlg->dbProfile, "export", 0, 1);
  GWEN_Dialog_SetIntProperty(dlg, "exportCheck", GWEN_DialogProperty_Value, 0, i?1:0, 0);

  GWEN_Dialog_SetIntProperty(dlg, "ignoreLinesSpin", GWEN_DialogProperty_MinValue, 0, 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "ignoreLinesSpin", GWEN_DialogProperty_MaxValue, 0, 1000, 0);
  i=GWEN_DB_GetIntValue(xdlg->dbProfile, "params/ignoreLines", 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "ignoreLinesSpin", GWEN_DialogProperty_Value, 0, i, 0);

  i=GWEN_DB_GetIntValue(xdlg->dbProfile, "params/title", 0, 0);
  GWEN_Dialog_SetIntProperty(dlg, "headerCheck", GWEN_DialogProperty_Value, 0, i?1:0, 0);

  i=GWEN_DB_GetIntValue(xdlg->dbProfile, "params/quote", 0, 1);
  GWEN_Dialog_SetIntProperty(dlg, "quoteCheck", GWEN_DialogProperty_Value, 0, i?1:0, 0);

  /* setup delimiter combo box */
  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "params/delimiter", 0, NULL);
  setUpComboFromDoubleStrings(dlg, "delimiterCombo", csv_delimiters, s);

  /* setup subject combo box */
  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "subject", 0, NULL);
  setUpComboFromDoubleStrings(dlg, "subjectCombo", csv_subjects, s);

  /* setup date format combo box */
  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "dateFormat", 0, NULL);
  setUpComboFromSingleStrings(dlg, "dateFormatCombo", csv_dateFormats, s);

  /* setup amount format combo box */
  s=GWEN_DB_GetCharValue(xdlg->dbProfile, "valueFormat", 0, NULL);
  setUpComboFromDoubleStrings(dlg, "amountFormatCombo", csv_amountFormats, s);


  /* setup columns page */
  for (i=0; i<DIALOG_MAX_COLUMNS; i++) {
    char varName[32];
    char comboName[32];
    char editName[32];

    snprintf(varName, sizeof(varName)-1, "params/columns/%d", i+1);
    snprintf(comboName, sizeof(comboName)-1, "col%dCombo", i+1);
    snprintf(editName, sizeof(editName)-1, "col%dDataEdit", i+1);

    s=GWEN_DB_GetCharValue(xdlg->dbProfile, varName, 0, NULL);
    setUpComboFromDoubleStrings(dlg, comboName, csv_columns, s);
  }

  if (xdlg->testFileName)
    readTestData(dlg);

}



void AB_CSV_EditProfileDialog_Fini(GWEN_DIALOG *dlg) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;
  int i;
  GWEN_DB_NODE *dbPrefs;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  dbPrefs=GWEN_Dialog_GetPreferences(dlg);

  /* store dialog width */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Width, 0, -1);
  if (i<DIALOG_MINWIDTH)
    i=DIALOG_MINWIDTH;
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_width",
		      i);

  /* store dialog height */
  i=GWEN_Dialog_GetIntProperty(dlg, "", GWEN_DialogProperty_Height, 0, -1);
  if (i<DIALOG_MINHEIGHT)
    i=DIALOG_MINHEIGHT;
  GWEN_DB_SetIntValue(dbPrefs,
		      GWEN_DB_FLAGS_OVERWRITE_VARS,
		      "dialog_height",
		      i);
}



int AB_CSV_EditProfileDialog_fromGui(GWEN_DIALOG *dlg, GWEN_DB_NODE *db) {
  const char *s;
  int i;

  s=GWEN_Dialog_GetCharProperty(dlg, "nameEdit", GWEN_DialogProperty_Value, 0, NULL);
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name", s);
  else {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			I18N("Input Error"),
			I18N("Please enter a name for the profile."),
			I18N("Continue"), 0, 0, 0);
    /* change focus */
    GWEN_Dialog_SetIntProperty(dlg, "nameEdit", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "versionEdit", GWEN_DialogProperty_Value, 0, "");
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "version", s);

  s=GWEN_Dialog_GetCharProperty(dlg, "shortDescrEdit", GWEN_DialogProperty_Value, 0, "");
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "shortDescr", s);

  s=GWEN_Dialog_GetCharProperty(dlg, "longDescrEdit", GWEN_DialogProperty_Value, 0, "");
  if (s && *s)
    GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "longDescr", s);

  i=GWEN_Dialog_GetIntProperty(dlg, "importCheck", GWEN_DialogProperty_Value, 0, 1);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "import", i);

  i=GWEN_Dialog_GetIntProperty(dlg, "exportCheck", GWEN_DialogProperty_Value, 0, 1);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "export", i);

  i=GWEN_Dialog_GetIntProperty(dlg, "ignoreLinesSpin", GWEN_DialogProperty_Value, 0, 0);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "params/ignoreLines", i);

  i=setDbValueFromDoubleStringsCombo(dlg, db, "params/delimiter", "delimiterCombo", csv_delimiters);
  if (i<0) {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			I18N("Input Error"),
			I18N("Please select a field delimiter."),
			I18N("Continue"), 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "tabBook", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "delimiterCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_BAD_DATA;
  }

  i=setDbValueFromDoubleStringsCombo(dlg, db, "subject", "subjectCombo", csv_subjects);
  if (i<0) {
    GWEN_Dialog_SetIntProperty(dlg, "tabBook", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "subjectCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_BAD_DATA;
  }

  s=GWEN_Dialog_GetCharProperty(dlg, "dateFormatCombo", GWEN_DialogProperty_Value, 0, "");
  if (!(s && *s)) {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			I18N("Input Error"),
			I18N("Please select a date format."),
			I18N("Continue"), 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "tabBook", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "dateFormatCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_BAD_DATA;
  }
  GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "dateFormat", s);

  i=setDbValueFromDoubleStringsCombo(dlg, db, "valueFormat", "amountFormatCombo", csv_amountFormats);
  if (i<0) {
    GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			I18N("Input Error"),
			I18N("Please select a value format."),
			I18N("Continue"), 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "tabBook", GWEN_DialogProperty_Value, 0, 0, 0);
    GWEN_Dialog_SetIntProperty(dlg, "amountFormatCombo", GWEN_DialogProperty_Focus, 0, 1, 0);
    return GWEN_ERROR_BAD_DATA;
  }

  i=GWEN_Dialog_GetIntProperty(dlg, "headerCheck", GWEN_DialogProperty_Value, 0, 0);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "params/title", i);

  i=GWEN_Dialog_GetIntProperty(dlg, "quoteCheck", GWEN_DialogProperty_Value, 0, 1);
  GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "params/quote", i);

  /* get columns */
  GWEN_DB_ClearGroup(db, "params/columns");
  for (i=0; i<DIALOG_MAX_COLUMNS; i++) {
    char varName[32];
    char comboName[32];
    int rv;

    snprintf(varName, sizeof(varName)-1, "params/columns/%d", i+1);
    snprintf(comboName, sizeof(comboName)-1, "col%dCombo", i+1);

    rv=setColumnValueFromCombo(dlg, db, varName, comboName, csv_columns);
    if (rv<0) {
      GWEN_Gui_MessageBox(GWEN_GUI_MSG_FLAGS_TYPE_ERROR |
			  GWEN_GUI_MSG_FLAGS_CONFIRM_B1 |
			  GWEN_GUI_MSG_FLAGS_SEVERITY_NORMAL,
			  I18N("Input Error"),
			  I18N("Please select a valid column type."),
			  I18N("Continue"), 0, 0, 0);
      GWEN_Dialog_SetIntProperty(dlg, "tabBook", GWEN_DialogProperty_Value, 0, 1, 0);
      GWEN_Dialog_SetIntProperty(dlg, comboName, GWEN_DialogProperty_Focus, 0, 1, 0);
      return GWEN_ERROR_BAD_DATA;
    }
  }

  return 0;
}



int AB_CSV_EditProfileDialog_HandleActivated(GWEN_DIALOG *dlg, const char *sender) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  if (strcasecmp(sender, "okButton")==0) {
    GWEN_DB_NODE *db;
    int rv;

    db=GWEN_DB_Group_new("profile");
    rv=AB_CSV_EditProfileDialog_fromGui(dlg, db);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(db);
      return GWEN_DialogEvent_ResultHandled;
    }
    GWEN_DB_ClearGroup(xdlg->dbProfile, NULL);
    GWEN_DB_AddGroupChildren(xdlg->dbProfile, db);
    GWEN_DB_Group_free(db);
    return GWEN_DialogEvent_ResultAccept;
  }
  else if (strcasecmp(sender, "abortButton")==0)
    return GWEN_DialogEvent_ResultReject;
  else if (strcasecmp(sender, "helpButton")==0) {
  }
  else if (strcasecmp(sender, "headerCheck")==0) {
    if (xdlg->testFileName)
      readTestData(dlg);
    return GWEN_DialogEvent_ResultHandled;
  }

  return GWEN_DialogEvent_ResultHandled;
}




int AB_CSV_EditProfileDialog_HandleValueChanged(GWEN_DIALOG *dlg, const char *sender){
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  if (strcasecmp(sender, "ignoreLinesSpin")==0 ||
      strcasecmp(sender, "delimiterCombo")==0 ||
      strcasecmp(sender, "headerCheck")==0 ||
      strcasecmp(sender, "quoteCheck")==0) {
    if (xdlg->testFileName)
      readTestData(dlg);
  }

  return GWEN_DialogEvent_ResultHandled;
}



int GWENHYWFAR_CB AB_CSV_EditProfileDialog_SignalHandler(GWEN_DIALOG *dlg,
							 GWEN_DIALOG_EVENTTYPE t,
							 const char *sender) {
  AB_CSV_EDIT_PROFILE_DIALOG *xdlg;

  assert(dlg);
  xdlg=GWEN_INHERIT_GETDATA(GWEN_DIALOG, AB_CSV_EDIT_PROFILE_DIALOG, dlg);
  assert(xdlg);

  switch(t) {
  case GWEN_DialogEvent_TypeInit:
    AB_CSV_EditProfileDialog_Init(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeFini:
    AB_CSV_EditProfileDialog_Fini(dlg);
    return GWEN_DialogEvent_ResultHandled;;

  case GWEN_DialogEvent_TypeValueChanged:
    return AB_CSV_EditProfileDialog_HandleValueChanged(dlg, sender);

  case GWEN_DialogEvent_TypeActivated:
    return AB_CSV_EditProfileDialog_HandleActivated(dlg, sender);

  case GWEN_DialogEvent_TypeEnabled:
  case GWEN_DialogEvent_TypeDisabled:
  case GWEN_DialogEvent_TypeClose:

  case GWEN_DialogEvent_TypeLast:
    return GWEN_DialogEvent_ResultNotHandled;

  }

  return GWEN_DialogEvent_ResultNotHandled;
}






