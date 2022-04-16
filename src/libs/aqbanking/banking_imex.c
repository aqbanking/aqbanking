/***************************************************************************
 begin       : Mon Mar 01 2004
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

/* This file is included by banking.c */

#include "aqbanking/backendsupport/swiftdescr.h"


#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CSV
# include "src/libs/plugins/imexporters/csv/csv.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_ERI2
# include "src/libs/plugins/imexporters/eri2/eri2.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OFX
# include "src/libs/plugins/imexporters/ofx/ofx.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OPENHBCI1
# include "src/libs/plugins/imexporters/openhbci1/openhbci1.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SWIFT
# include "src/libs/plugins/imexporters/swift/swift.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XMLDB
# include "src/libs/plugins/imexporters/xmldb/xmldb.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_YELLOWNET
# include "src/libs/plugins/imexporters/yellownet/yellownet.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SEPA
# include "src/libs/plugins/imexporters/sepa/sepa.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CTXFILE
# include "src/libs/plugins/imexporters/ctxfile/ctxfile.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_Q43
# include "src/libs/plugins/imexporters/q43/q43.h"
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CAMT
# include "src/libs/plugins/imexporters/camt/camt.h"
#endif


#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XML
# include "src/libs/plugins/imexporters/xml/xml.h"
#endif



static GWEN_DB_NODE *_getProfileFromFileOrSystem(AB_BANKING *ab,
						 const char *importerName,
						 const char *profileName,
						 const char *profileFile);



AB_IMEXPORTER *AB_Banking__CreateImExporterPlugin(AB_BANKING *ab, const char *modname)
{
  if (modname && *modname) {
#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CSV
    if (strcasecmp(modname, "csv")==0)
      return AB_ImExporterCSV_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_ERI2
    if (strcasecmp(modname, "eri2")==0)
      return AB_ImExporterERI2_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OFX
    if (strcasecmp(modname, "ofx")==0)
      return AB_ImExporterOFX_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_OPENHBCI1
    if (strcasecmp(modname, "ofx")==0)
      return AB_ImExporterOpenHBCI1_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SWIFT
    if (strcasecmp(modname, "swift")==0)
      return AB_ImExporterSWIFT_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XMLDB
    if (strcasecmp(modname, "xmldb")==0)
      return AB_ImExporterXMLDB_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_YELLOWNET
    if (strcasecmp(modname, "yellownet")==0)
      return AB_ImExporterYellowNet_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_SEPA
    if (strcasecmp(modname, "sepa")==0)
      return AB_ImExporterSEPA_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CTXFILE
    if (strcasecmp(modname, "ctxfile")==0)
      return AB_ImExporterCtxFile_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_Q43
    if (strcasecmp(modname, "q43")==0)
      return AB_ImExporterQ43_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_CAMT
    if (strcasecmp(modname, "camt")==0)
      return AB_ImExporterCAMT_new(ab);
#endif

#ifdef AQBANKING_WITH_PLUGIN_IMEXPORTER_XML
    if (strcasecmp(modname, "xml")==0)
      return AB_ImExporterXML_new(ab);
#endif

    DBG_ERROR(AQBANKING_LOGDOMAIN, "Plugin [%s] not compiled-in", modname);
  }

  return NULL;
}



AB_IMEXPORTER *AB_Banking_FindImExporter(AB_BANKING *ab, const char *name)
{
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);
  ie=AB_ImExporter_List_First(ab_imexporters);
  while (ie) {
    if (strcasecmp(AB_ImExporter_GetName(ie), name)==0)
      break;
    ie=AB_ImExporter_List_Next(ie);
  } /* while */

  return ie;
}



AB_IMEXPORTER *AB_Banking_GetImExporter(AB_BANKING *ab, const char *name)
{
  AB_IMEXPORTER *ie;

  assert(ab);
  assert(name);

  ie=AB_Banking_FindImExporter(ab, name);
  if (ie)
    return ie;
  ie=AB_Banking__CreateImExporterPlugin(ab, name);
  if (ie)
    AB_ImExporter_List_Add(ie, ab_imexporters);

  return ie;
}



void AB_Banking_FillGapsInTransaction(AB_BANKING *ab, AB_ACCOUNT *a, AB_TRANSACTION *t)
{
  assert(t);

  if (a) {
    const char *s;

    /* local account */
    s=AB_Account_GetCountry(a);
    if (!s || !*s)
      s="de";
    AB_Transaction_SetLocalCountry(t, s);
    AB_Transaction_SetRemoteCountry(t, s);

    s=AB_Account_GetBankCode(a);
    if (s && *s)
      AB_Transaction_SetLocalBankCode(t, s);

    s=AB_Account_GetAccountNumber(a);
    if (s && *s)
      AB_Transaction_SetLocalAccountNumber(t, s);

    s=AB_Account_GetOwnerName(a);
    if (s && *s)
      AB_Transaction_SetLocalName(t, s);

    s=AB_Account_GetBic(a);
    if (s && *s)
      AB_Transaction_SetLocalBic(t, s);

    s=AB_Account_GetIban(a);
    if (s && *s)
      AB_Transaction_SetLocalIban(t, s);
  }
}



int AB_Banking_GetEditImExporterProfileDialog(AB_BANKING *ab,
                                              const char *imExporterName,
                                              GWEN_DB_NODE *dbProfile,
                                              const char *testFileName,
                                              GWEN_DIALOG **pDlg)
{
  AB_IMEXPORTER *ie;
  int rv;

  ie=AB_Banking_GetImExporter(ab, imExporterName);
  if (ie==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return GWEN_ERROR_NO_DATA;
  }

  if (AB_ImExporter_GetFlags(ie) & AB_IMEXPORTER_FLAGS_GETPROFILEEDITOR_SUPPORTED) {
    GWEN_DIALOG *dlg=NULL;

    rv=AB_ImExporter_GetEditProfileDialog(ie, dbProfile, testFileName, &dlg);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      return rv;
    }

    *pDlg=dlg;
    return 0;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "EditProfileDialog not supported by imExporter \"%s\"", imExporterName);
    return GWEN_ERROR_NOT_SUPPORTED;
  }
}



int AB_Banking_Import(AB_BANKING *ab,
                      const char *importerName,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *dbProfile)
{
  AB_IMEXPORTER *ie;
  int rv;

  ie=AB_Banking_GetImExporter(ab, importerName);
  if (ie==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_ImExporter_Import(ie, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_Export(AB_BANKING *ab,
                      const char *exporterName,
                      AB_IMEXPORTER_CONTEXT *ctx,
                      GWEN_SYNCIO *sio,
                      GWEN_DB_NODE *dbProfile)
{
  AB_IMEXPORTER *ie;
  int rv;

  ie=AB_Banking_GetImExporter(ab, exporterName);
  if (ie==NULL) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_ImExporter_Export(ie, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int AB_Banking_ImportLoadProfile(AB_BANKING *ab,
                                 const char *importerName,
                                 AB_IMEXPORTER_CONTEXT *ctx,
                                 GWEN_SYNCIO *sio,
                                 const char *profileName,
                                 const char *profileFile)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, importerName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_Import(ab, importerName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}



int AB_Banking_ExportLoadProfile(AB_BANKING *ab,
                                 const char *exporterName,
                                 AB_IMEXPORTER_CONTEXT *ctx,
                                 GWEN_SYNCIO *sio,
                                 const char *profileName,
                                 const char *profileFile)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, exporterName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_Export(ab, exporterName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}




int AB_Banking_ImportFromFile(AB_BANKING *ab,
                              const char *importerName,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              const char *inputFileName,
                              GWEN_DB_NODE *dbProfile)
{
  GWEN_SYNCIO *sio;
  int rv;

  if (inputFileName) {
    sio=GWEN_SyncIo_File_new(inputFileName, GWEN_SyncIo_File_CreationMode_OpenExisting);
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FILE_FLAGS_READ);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
      GWEN_SyncIo_free(sio);
      return rv;
    }
  }
  else {
    sio=GWEN_SyncIo_File_fromStdin();
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FLAGS_DONTCLOSE);
  }

  rv=AB_Banking_Import(ab, importerName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  GWEN_SyncIo_Disconnect(sio);
  GWEN_SyncIo_free(sio);

  return 0;
}



int AB_Banking_ExportToFile(AB_BANKING *ab,
                            const char *exporterName,
                            AB_IMEXPORTER_CONTEXT *ctx,
                            const char *outputFileName,
                            GWEN_DB_NODE *dbProfile)
{
  GWEN_SYNCIO *sio;
  int rv;

  if (outputFileName) {
    sio=GWEN_SyncIo_File_new(outputFileName, GWEN_SyncIo_File_CreationMode_CreateAlways);
    GWEN_SyncIo_AddFlags(sio,
                         GWEN_SYNCIO_FILE_FLAGS_READ |
                         GWEN_SYNCIO_FILE_FLAGS_WRITE |
                         GWEN_SYNCIO_FILE_FLAGS_UREAD |
                         GWEN_SYNCIO_FILE_FLAGS_UWRITE |
                         GWEN_SYNCIO_FILE_FLAGS_GREAD |
                         GWEN_SYNCIO_FILE_FLAGS_GWRITE);
    rv=GWEN_SyncIo_Connect(sio);
    if (rv<0) {
      DBG_WARN(AQBANKING_LOGDOMAIN, "Failed to Connect() syncio (%d)", rv);
      GWEN_SyncIo_free(sio);
      return rv;
    }
  }
  else {
    sio=GWEN_SyncIo_File_fromStdout();
    GWEN_SyncIo_AddFlags(sio, GWEN_SYNCIO_FLAGS_DONTCLOSE);
  }

  rv=AB_Banking_Export(ab, exporterName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Failed to export (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return rv;
  }

  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_WARN(AQBANKING_LOGDOMAIN, "Failed to Disconnect() syncio (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_free(sio);

  return 0;
}



int AB_Banking_ImportFromFileLoadProfile(AB_BANKING *ab,
                                         const char *importerName,
                                         AB_IMEXPORTER_CONTEXT *ctx,
                                         const char *profileName,
                                         const char *profileFile,
                                         const char *inputFileName)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, importerName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_ImportFromFile(ab, importerName, ctx, inputFileName, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}



int AB_Banking_ExportToFileLoadProfile(AB_BANKING *ab,
                                       const char *exporterName,
                                       AB_IMEXPORTER_CONTEXT *ctx,
                                       const char *outputFileName,
                                       const char *profileName,
                                       const char *profileFile)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, exporterName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_ExportToFile(ab, exporterName, ctx, outputFileName, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}



int AB_Banking_ImportFromBuffer(AB_BANKING *ab,
                                const char *importerName,
                                AB_IMEXPORTER_CONTEXT *ctx,
                                const uint8_t *dataPtr,
                                uint32_t dataLen,
                                GWEN_DB_NODE *dbProfile)
{
  GWEN_BUFFER *buf;
  GWEN_SYNCIO *sio;
  int rv;

  buf=GWEN_Buffer_new((char *) dataPtr, dataLen, dataLen, 0);
  GWEN_Buffer_SetMode(buf, GWEN_BUFFER_MODE_READONLY);

  sio=GWEN_SyncIo_Memory_new(buf, 0);

  rv=AB_Banking_Import(ab, importerName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    GWEN_Buffer_free(buf);
    return rv;
  }

  GWEN_SyncIo_free(sio);
  GWEN_Buffer_free(buf);

  return 0;
}



int AB_Banking_ExportToBuffer(AB_BANKING *ab,
                              const char *exporterName,
                              AB_IMEXPORTER_CONTEXT *ctx,
                              GWEN_BUFFER *outputBuffer,
                              GWEN_DB_NODE *dbProfile)
{
  int rv;
  GWEN_SYNCIO *sio;

  sio=GWEN_SyncIo_Memory_new(outputBuffer, 0);
  rv=AB_Banking_Export(ab, exporterName, ctx, sio, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return rv;
  }
  GWEN_SyncIo_free(sio);

  return 0;
}



int AB_Banking_ImportFromBufferLoadProfile(AB_BANKING *ab,
                                           const char *importerName,
                                           AB_IMEXPORTER_CONTEXT *ctx,
                                           const char *profileName,
                                           const char *profileFile,
                                           const uint8_t *dataPtr,
                                           uint32_t dataLen)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, importerName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_ImportFromBuffer(ab, importerName, ctx, dataPtr, dataLen, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}



int AB_Banking_ExportToBufferLoadProfile(AB_BANKING *ab,
                                         const char *exporterName,
                                         AB_IMEXPORTER_CONTEXT *ctx,
                                         GWEN_BUFFER *outputBuffer,
                                         const char *profileName,
                                         const char *profileFile)
{
  GWEN_DB_NODE *dbProfile;
  int rv;

  dbProfile=_getProfileFromFileOrSystem(ab, exporterName, profileName, profileFile);
  if (dbProfile==NULL) {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "Profile [%s] not found", profileName?profileName:"(null)");
    return GWEN_ERROR_NO_DATA;
  }

  rv=AB_Banking_ExportToBuffer(ab, exporterName, ctx, outputBuffer, dbProfile);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(dbProfile);
    return rv;
  }
  GWEN_DB_Group_free(dbProfile);

  return 0;
}



GWEN_PLUGIN_DESCRIPTION_LIST2 *AB_Banking_GetImExporterDescrs(AB_BANKING *ab)
{
  assert(ab);
  if (ab_pluginManagerImExporter) {
    GWEN_PLUGIN_DESCRIPTION_LIST2 *l;

    l=GWEN_PluginManager_GetPluginDescrs(ab_pluginManagerImExporter);
    return l;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN, "No plugin manager for imexporters");
    return NULL;
  }
}



int AB_Banking__ReadImExporterProfiles(AB_BANKING *ab,
                                       const char *path,
                                       GWEN_DB_NODE *db,
                                       int isGlobal)
{
  GWEN_DIRECTORY *d;
  GWEN_BUFFER *nbuf;
  char nbuffer[64];
  unsigned int pathLen;

  if (!path)
    path="";

  /* create path */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(nbuf, path);
  pathLen=GWEN_Buffer_GetUsedBytes(nbuf);

  d=GWEN_Directory_new();
  if (GWEN_Directory_Open(d, GWEN_Buffer_GetStart(nbuf))) {
    DBG_INFO(AQBANKING_LOGDOMAIN,
             "Path \"%s\" is not available",
             GWEN_Buffer_GetStart(nbuf));
    GWEN_Buffer_free(nbuf);
    GWEN_Directory_free(d);
    return GWEN_ERROR_NOT_FOUND;
  }

  while (!GWEN_Directory_Read(d,
                              nbuffer,
                              sizeof(nbuffer))) {
    if (strcmp(nbuffer, ".") &&
        strcmp(nbuffer, "..")) {
      int nlen;

      nlen=strlen(nbuffer);
      if (nlen>4) {
        if (strcasecmp(nbuffer+nlen-5, ".conf")==0) {
          struct stat st;

          GWEN_Buffer_Crop(nbuf, 0, pathLen);
          GWEN_Buffer_SetPos(nbuf, pathLen);
          GWEN_Buffer_AppendString(nbuf, DIRSEP);
          GWEN_Buffer_AppendString(nbuf, nbuffer);

          if (stat(GWEN_Buffer_GetStart(nbuf), &st)) {
            DBG_ERROR(AQBANKING_LOGDOMAIN, "stat(%s): %s",
                      GWEN_Buffer_GetStart(nbuf),
                      strerror(errno));
          }
          else {
            if (!S_ISDIR(st.st_mode)) {
              GWEN_DB_NODE *dbT;

              dbT=GWEN_DB_Group_new("profile");
              if (GWEN_DB_ReadFile(dbT,
                                   GWEN_Buffer_GetStart(nbuf),
                                   GWEN_DB_FLAGS_DEFAULT |
                                   GWEN_PATH_FLAGS_CREATE_GROUP)) {
                DBG_ERROR(AQBANKING_LOGDOMAIN,
                          "Could not read file \"%s\"",
                          GWEN_Buffer_GetStart(nbuf));
              }
              else {
                const char *s;

                s=GWEN_DB_GetCharValue(dbT, "name", 0, 0);
                if (!s) {
                  DBG_ERROR(AQBANKING_LOGDOMAIN,
                            "Bad file \"%s\" (no name)",
                            GWEN_Buffer_GetStart(nbuf));
                }
                else {
                  GWEN_DB_NODE *dbTarget;
                  DBG_INFO(AQBANKING_LOGDOMAIN,
                           "File \"%s\" contains profile \"%s\"",
                           GWEN_Buffer_GetStart(nbuf), s);

                  dbTarget=GWEN_DB_GetGroup(db,
                                            GWEN_DB_FLAGS_OVERWRITE_GROUPS,
                                            s);
                  assert(dbTarget);
                  GWEN_DB_AddGroupChildren(dbTarget, dbT);
                  GWEN_DB_SetIntValue(dbTarget, GWEN_DB_FLAGS_OVERWRITE_VARS, "isGlobal", isGlobal);
                  GWEN_DB_SetCharValue(dbTarget, GWEN_DB_FLAGS_OVERWRITE_VARS, "fileName", nbuffer);
                } /* if name */
              } /* if file successfully read */
              GWEN_DB_Group_free(dbT);
            } /* if !dir */
          } /* if stat was ok */
        } /* if conf */
      } /* if name has more than 4 chars */
    } /* if not "." and not ".." */
  } /* while */
  GWEN_Directory_Close(d);
  GWEN_Directory_free(d);
  GWEN_Buffer_free(nbuf);

  return 0;
}



GWEN_DB_NODE *AB_Banking_GetImExporterProfiles(AB_BANKING *ab,
                                               const char *name)
{
  GWEN_BUFFER *buf;
  GWEN_DB_NODE *db;
  int rv;
  GWEN_STRINGLIST *sl;
  GWEN_STRINGLISTENTRY *sentry;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  db=GWEN_DB_Group_new("profiles");

  sl=AB_Banking_GetGlobalDataDirs();
  assert(sl);

  sentry=GWEN_StringList_FirstEntry(sl);
  assert(sentry);

  while (sentry) {
    const char *pkgdatadir;

    pkgdatadir = GWEN_StringListEntry_Data(sentry);
    assert(pkgdatadir);

    /* read global profiles */
    GWEN_Buffer_AppendString(buf, pkgdatadir);
    GWEN_Buffer_AppendString(buf,
                             DIRSEP
                             "aqbanking"
                             DIRSEP
                             AB_IMEXPORTER_FOLDER
                             DIRSEP);
    if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Bad name for importer/exporter");
      GWEN_StringList_free(sl);
      GWEN_DB_Group_free(db);
      GWEN_Buffer_free(buf);
      return 0;
    }
    GWEN_Buffer_AppendString(buf, DIRSEP "profiles");
    rv=AB_Banking__ReadImExporterProfiles(ab,
                                          GWEN_Buffer_GetStart(buf),
                                          db,
                                          1);
    if (rv && rv!=GWEN_ERROR_NOT_FOUND) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Error reading global profiles");
      GWEN_StringList_free(sl);
      GWEN_DB_Group_free(db);
      GWEN_Buffer_free(buf);
      return 0;
    }
    GWEN_Buffer_Reset(buf);
    sentry=GWEN_StringListEntry_Next(sentry);
  }
  GWEN_StringList_free(sl);

  /* read local user profiles */
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP AB_IMEXPORTER_FOLDER DIRSEP);
  if (GWEN_Text_EscapeToBufferTolerant(name, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "profiles");

  rv=AB_Banking__ReadImExporterProfiles(ab,
                                        GWEN_Buffer_GetStart(buf),
                                        db,
                                        0);
  if (rv && rv!=GWEN_ERROR_NOT_FOUND) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error reading users profiles");
    GWEN_DB_Group_free(db);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_free(buf);

  return db;
}



int AB_Banking_SaveLocalImExporterProfile(AB_BANKING *ab,
                                          const char *imexporterName,
                                          GWEN_DB_NODE *dbProfile,
                                          const char *fname)
{
  GWEN_BUFFER *buf;
  int rv;

  buf=GWEN_Buffer_new(0, 256, 0, 1);

  /* get folder for local user profiles */
  rv=AB_Banking_GetUserDataDir(ab, buf);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_Buffer_free(buf);
    return rv;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP AB_IMEXPORTER_FOLDER DIRSEP);
  rv=GWEN_Text_EscapeToBufferTolerant(imexporterName, buf);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Bad name for importer/exporter (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP "profiles");

  /* make sure the path exists */
  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(buf), GWEN_PATH_FLAGS_CHECKROOT);
  if (rv<0) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(buf);
    return rv;
  }

  GWEN_Buffer_AppendString(buf, DIRSEP);
  if (fname && *fname)
    GWEN_Buffer_AppendString(buf, fname);
  else {
    const char *s;

    s=GWEN_DB_GetCharValue(dbProfile, "name", 0, NULL);
    if (s && *s) {
      FILE *f;

      rv=GWEN_Text_EscapeToBufferTolerant(s, buf);
      if (rv<0) {
        DBG_ERROR(AQBANKING_LOGDOMAIN,
                  "Bad profile name (%d)", rv);
        GWEN_Buffer_free(buf);
        return rv;
      }
      GWEN_Buffer_AppendString(buf, ".conf");

      f=fopen(GWEN_Buffer_GetStart(buf), "r");
      if (f) {
        DBG_ERROR(AQBANKING_LOGDOMAIN, "There already is a profile of that name");
        GWEN_Buffer_free(buf);
        fclose(f);
        return GWEN_ERROR_INVALID;
      }
    }
    else {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Missing profile name");
      GWEN_Buffer_free(buf);
      return GWEN_ERROR_INVALID;
    }
  }

  rv=GWEN_DB_WriteFile(dbProfile,
                       GWEN_Buffer_GetStart(buf),
                       GWEN_DB_FLAGS_DEFAULT);
  if (rv<0) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Error writing users profile (%d)", rv);
    GWEN_Buffer_free(buf);
    return 0;
  }
  GWEN_Buffer_free(buf);

  return 0;
}



int AB_Banking_FindDataFileForImExporter(AB_BANKING *ab, const char *imExpName, const char *fileName,
                                         GWEN_BUFFER *fullPathBuffer)
{
  GWEN_BUFFER *buf;
  int rv;
  GWEN_STRINGLIST *sl;

  buf=GWEN_Buffer_new(0, 256, 0, 1);
  GWEN_Buffer_AppendString(buf,
                           DIRSEP
                           "aqbanking"
                           DIRSEP
                           AB_IMEXPORTER_FOLDER
                           DIRSEP);
  GWEN_Buffer_AppendString(buf, imExpName);
  GWEN_Buffer_AppendString(buf, DIRSEP "data" DIRSEP);
  GWEN_Buffer_AppendString(buf, fileName);


  sl=AB_Banking_GetGlobalDataDirs();
  assert(sl);

  rv=GWEN_Directory_FindFileInPaths(sl, GWEN_Buffer_GetStart(buf), fullPathBuffer);
  if (rv==0) {
    GWEN_Buffer_free(buf);
    return rv;
  }
  GWEN_Buffer_Reset(buf);

  /* try local storage */
  if (AB_Banking_GetUserDataDir(ab, buf)) {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "Could not get user data dir");
    GWEN_Buffer_free(buf);
    return GWEN_ERROR_NOT_FOUND;
  }
  GWEN_Buffer_AppendString(buf, DIRSEP AB_IMEXPORTER_FOLDER DIRSEP);
  GWEN_Buffer_AppendString(buf, fileName);

  rv=GWEN_Directory_GetPath(GWEN_Buffer_GetStart(buf), GWEN_PATH_FLAGS_PATHMUSTEXIST);
  if (rv==0) {
    GWEN_Buffer_AppendString(fullPathBuffer, GWEN_Buffer_GetStart(buf));
    GWEN_Buffer_free(buf);
    return 0;
  }

  GWEN_Buffer_free(buf);

  return GWEN_ERROR_NOT_FOUND;
}



GWEN_STRINGLIST *AB_Banking_ListDataFilesForImExporter(AB_BANKING *ab, const char *imExpName, const char *fileMask)
{
  int rv;
  GWEN_BUFFER *pathBuffer;
  GWEN_STRINGLIST *slGlobalDataDirs;
  GWEN_STRINGLIST *slMatchingFiles;
  GWEN_STRINGLISTENTRY *seGlobalDataDir;

  slGlobalDataDirs=AB_Banking_GetGlobalDataDirs();
  assert(slGlobalDataDirs);

  slMatchingFiles=GWEN_StringList_new();

  pathBuffer=GWEN_Buffer_new(0, 256, 0, 1);
  seGlobalDataDir=GWEN_StringList_FirstEntry(slGlobalDataDirs);
  while (seGlobalDataDir) {
    GWEN_Buffer_AppendString(pathBuffer, GWEN_StringListEntry_Data(seGlobalDataDir));
    GWEN_Buffer_AppendString(pathBuffer,
                             DIRSEP
                             "aqbanking"
                             DIRSEP
                             AB_IMEXPORTER_FOLDER
                             DIRSEP);
    GWEN_Buffer_AppendString(pathBuffer, imExpName);
    GWEN_Buffer_AppendString(pathBuffer, DIRSEP "data");

    rv=GWEN_Directory_GetMatchingFilesRecursively(GWEN_Buffer_GetStart(pathBuffer), slMatchingFiles, fileMask);
    if (rv<0) {
      DBG_INFO(AQBANKING_LOGDOMAIN,
               "Error listing matching files in folder \"%s\", ignoring",
               GWEN_Buffer_GetStart(pathBuffer));
    }
    GWEN_Buffer_Reset(pathBuffer);

    seGlobalDataDir=GWEN_StringListEntry_Next(seGlobalDataDir);
  } /* while(seGlobalDataDir) */
  GWEN_Buffer_free(pathBuffer);


  if (GWEN_StringList_Count(slMatchingFiles)<1) {
    DBG_INFO(AQBANKING_LOGDOMAIN, "No matching data files");
    GWEN_StringList_free(slMatchingFiles);
    return NULL;
  }

  return slMatchingFiles;
}



GWEN_DB_NODE *AB_Banking_GetImExporterProfile(AB_BANKING *ab,
                                              const char *imExporterName,
                                              const char *profileName)
{
  GWEN_DB_NODE *dbProfiles;

  dbProfiles=AB_Banking_GetImExporterProfiles(ab, imExporterName);
  if (dbProfiles) {
    GWEN_DB_NODE *dbProfile;

    dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
    while (dbProfile) {
      const char *name;

      name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
      assert(name);
      if (strcasecmp(name, profileName)==0)
        break;
      dbProfile=GWEN_DB_GetNextGroup(dbProfile);
    }
    if (!dbProfile) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Profile \"%s\" for exporter \"%s\" not found",
                profileName, imExporterName);
      GWEN_DB_Group_free(dbProfiles);
      return NULL;
    }

    GWEN_DB_UnlinkGroup(dbProfile);
    GWEN_DB_Group_free(dbProfiles);
    return dbProfile;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "No profiles found for exporter \"%s\"",
              imExporterName);
    return NULL;
  }

  return NULL;
}



GWEN_DB_NODE *AB_Banking_FindMatchingSwiftImExporterProfile(AB_BANKING *ab,
                                                            const char *imExporterName,
                                                            const char *family,
                                                            int version1,
                                                            int version2,
                                                            int version3)
{
  GWEN_DB_NODE *dbProfiles;

  dbProfiles=AB_Banking_GetImExporterProfiles(ab, imExporterName);
  if (dbProfiles) {
    GWEN_DB_NODE *dbProfile;

    dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
    while (dbProfile) {
      const char *name;
      AB_SWIFT_DESCR *swiftDescr;

      name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
      assert(name);

      swiftDescr=AB_SwiftDescr_FromString(name);
      if (swiftDescr) {
        if (AB_SwiftDescr_Matches(swiftDescr, family, version1, version2, version3)) {
          AB_SwiftDescr_free(swiftDescr);
          break;
        }
      }
      AB_SwiftDescr_free(swiftDescr);

      dbProfile=GWEN_DB_GetNextGroup(dbProfile);
    }
    if (!dbProfile) {
      DBG_ERROR(AQBANKING_LOGDOMAIN,
                "Profile \"%s.%03d.%03d.%02d\" for exporter \"%s\" not found",
                family, version1, version2, version3,
                imExporterName);
      GWEN_DB_Group_free(dbProfiles);
      return NULL;
    }

    GWEN_DB_UnlinkGroup(dbProfile);
    GWEN_DB_Group_free(dbProfiles);
    return dbProfile;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "No profiles found for exporter \"%s\"",
              imExporterName);
    return NULL;
  }

  return NULL;
}



AB_SWIFT_DESCR_LIST *AB_Banking_GetSwiftDescriptorsForImExporter(AB_BANKING *ab, const char *imExporterName)
{
  GWEN_DB_NODE *dbProfiles;

  dbProfiles=AB_Banking_GetImExporterProfiles(ab, imExporterName);
  if (dbProfiles) {
    GWEN_DB_NODE *dbProfile;
    AB_SWIFT_DESCR_LIST *descrList;

    descrList=AB_SwiftDescr_List_new();
    dbProfile=GWEN_DB_GetFirstGroup(dbProfiles);
    while (dbProfile) {
      const char *name;
      AB_SWIFT_DESCR *descr;

      name=GWEN_DB_GetCharValue(dbProfile, "name", 0, 0);
      assert(name);

      descr=AB_SwiftDescr_FromString(name);
      if (descr) {
        DBG_INFO(AQBANKING_LOGDOMAIN, "Adding matching profile [%s]", name);
        AB_SwiftDescr_SetAlias1(descr, name);
        AB_SwiftDescr_List_Add(descr, descrList);
      }

      dbProfile=GWEN_DB_GetNextGroup(dbProfile);
    }
    GWEN_DB_Group_free(dbProfiles);

    if (AB_SwiftDescr_List_GetCount(descrList)==0) {
      AB_SwiftDescr_List_free(descrList);
      return NULL;
    }

    return descrList;
  }
  else {
    DBG_ERROR(AQBANKING_LOGDOMAIN,
              "No profiles found for exporter \"%s\"",
              imExporterName);
    return NULL;
  }

  return NULL;
}



GWEN_DB_NODE *_getProfileFromFileOrSystem(AB_BANKING *ab,
                                          const char *importerName,
                                          const char *profileName,
                                          const char *profileFile)
{
  GWEN_DB_NODE *dbProfile=NULL;

  if (profileFile && *profileFile) {
    int rv;

    dbProfile=GWEN_DB_Group_new("profile");
    rv=GWEN_DB_ReadFile(dbProfile, profileFile, GWEN_DB_FLAGS_DEFAULT);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbProfile);
      return NULL;
    }
  }
  else if (profileName && *profileName) {
    dbProfile=AB_Banking_GetImExporterProfile(ab, importerName, profileName);
  }
  else {
    dbProfile=AB_Banking_GetImExporterProfile(ab, importerName, "default");
  }

  if (dbProfile==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Unable to load profile for imexporter \"%s\" (file=%s, name=%s)",
	      importerName,
	      profileFile?profileFile:"-none-",
	      profileName?profileName:"-none");
  }

  return dbProfile;
}

