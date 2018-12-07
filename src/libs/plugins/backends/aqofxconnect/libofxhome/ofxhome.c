/***************************************************************************
    begin       : Tue Aug 17 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "ofxhome_p.h"

#include <aqbanking/version.h>

#include <gwenhywfar/debug.h>
#include <gwenhywfar/misc.h>


#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>


#define OFX_CACHE_TIME_HR 2




OFXHOME *OfxHome_new(const char *dataFolder) {
  OFXHOME *ofh;

  GWEN_NEW_OBJECT(OFXHOME, ofh);

  if (dataFolder)
    ofh->dataFolder=strdup(dataFolder);

  ofh->dataList=OH_InstituteData_List_new();

  return ofh;
}



void OfxHome_free(OFXHOME *ofh) {
  if (ofh) {
    free(ofh->dataFolder);
    OH_InstituteSpec_List_free(ofh->specList);
    OH_InstituteData_List_free(ofh->dataList);

    GWEN_FREE_OBJECT(ofh);
  }
}



int OfxHome_SetupHttpSession(OFXHOME *ofh, GWEN_HTTP_SESSION *sess) {

  GWEN_HttpSession_SetHttpVMajor(sess, 1);
  GWEN_HttpSession_SetHttpVMinor(sess, 1);
  GWEN_HttpSession_SetHttpUserAgent(sess, "AqBanking/" AQBANKING_VERSION_STRING);

  return 0;
}



int OfxHome_DownloadSpecs(OFXHOME *ofh, OH_INSTITUTE_SPEC_LIST *sl) {
  GWEN_HTTP_SESSION *sess;
  int rv;
  GWEN_BUFFER *xbuf;
  GWEN_XMLNODE *nroot;
  GWEN_XMLNODE *n;

  /* prepare session */
  sess=GWEN_HttpSession_new("http://www.ofxhome.com/api.php?all=yes", "http", 80);
  rv=OfxHome_SetupHttpSession(ofh, sess);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* send request (no body) */
  rv=GWEN_HttpSession_SendPacket(sess, "GET", NULL, 0);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* get response */
  xbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, xbuf);
  if (rv<0 || rv<200 || rv>=300) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* fini */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse list */
  nroot=GWEN_XMLNode_fromString(GWEN_Buffer_GetStart(xbuf),
                                GWEN_Buffer_GetUsedBytes(xbuf),
                                GWEN_XML_FLAGS_DEFAULT |
                                GWEN_XML_FLAGS_HANDLE_HEADERS |
                                GWEN_XML_FLAGS_TOLERANT_ENDTAGS |
                                GWEN_XML_FLAGS_HANDLE_NAMESPACES);
  if (nroot==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_Dump(xbuf, 2);
    GWEN_Buffer_free(xbuf);
    return GWEN_ERROR_BAD_DATA;
  }
  GWEN_Buffer_free(xbuf);

  n=GWEN_XMLNode_FindFirstTag(nroot, "institutionlist", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "XML tree does not contain an \"institutionlist\" element");
    GWEN_XMLNode_Dump(nroot, 2);
    GWEN_XMLNode_free(nroot);
    return GWEN_ERROR_NO_DATA;
  }

  n=GWEN_XMLNode_FindFirstTag(n, "institutionid", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "XML tree does not contain an \"institutionid\" element");
    GWEN_XMLNode_Dump(nroot, 2);
    GWEN_XMLNode_free(nroot);
    return GWEN_ERROR_NO_DATA;
  }
  else {
    while(n) {
      OH_INSTITUTE_SPEC *os;

      os=OH_InstituteSpec_fromXml(n);
      if (os==NULL) {
        DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "element does not contain a valid institute spec");
        GWEN_XMLNode_Dump(n, 2);
      }
      else
        OH_InstituteSpec_List_Add(os, sl);
      /* previously we needed to use "FindFirstTag" because of the malformed response */
      n=GWEN_XMLNode_FindNextTag(n, "institutionid", NULL, NULL);
    }
  }
  GWEN_XMLNode_free(nroot);

  return 0;
}



int OfxHome_SaveSpecs(OFXHOME *ofh, const OH_INSTITUTE_SPEC_LIST *sl) {
  GWEN_DB_NODE *db;
  OH_INSTITUTE_SPEC *os;
  GWEN_BUFFER *nbuf;
  int rv;

  /* store institutes in db */
  db=GWEN_DB_Group_new("institutes");
  os=OH_InstituteSpec_List_First(sl);
  while(os) {
    GWEN_DB_NODE *dbT;
    int rv;

    dbT=GWEN_DB_Group_new("institute");
    rv=OH_InstituteSpec_toDb(os, dbT);
    if (rv<0) {
      DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
      GWEN_DB_Group_free(dbT);
      GWEN_DB_Group_free(db);
      return rv;
    }
    GWEN_DB_AddGroup(db, dbT);

    os=OH_InstituteSpec_List_Next(os);
  }

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(nbuf, "institutes.conf");

  /* write file */
  rv=GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(nbuf), GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_LOCKFILE);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* cleanup, done */
  GWEN_Buffer_free(nbuf);
  GWEN_DB_Group_free(db);

  return 0;
}



int OfxHome_LoadSpecs(OFXHOME *ofh, OH_INSTITUTE_SPEC_LIST *sl) {
  GWEN_DB_NODE *db;
  GWEN_DB_NODE *dbT;
  GWEN_BUFFER *nbuf;
  int rv;

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(nbuf, "institutes.conf");

  /* read file */
  db=GWEN_DB_Group_new("institutes");
  rv=GWEN_DB_ReadFile(db,
                      GWEN_Buffer_GetStart(nbuf),
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_LOCKFILE |
                      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_Buffer_free(nbuf);

  dbT=GWEN_DB_GetFirstGroup(db);
  while(dbT) {
    OH_INSTITUTE_SPEC *os;

    os=OH_InstituteSpec_fromDb(dbT);
    if (os)
      OH_InstituteSpec_List_Add(os, sl);
    else {
      DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "Group does not contain a valid institute spec");
      GWEN_DB_Dump(dbT, 2);
    }
    dbT=GWEN_DB_GetNextGroup(dbT);
  }

  /* cleanup, done */
  GWEN_DB_Group_free(db);

  return 0;
}



int OfxHome_CheckSpecsCache(OFXHOME *ofh, int hours) {
  GWEN_BUFFER *nbuf;
  int rv;
  struct stat st;

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  GWEN_Buffer_AppendString(nbuf, "institutes.conf");

  rv=stat(GWEN_Buffer_GetStart(nbuf), &st);
  GWEN_Buffer_free(nbuf);
  if (rv) {
    /* not in cache */
    return -1;
  }
  else {
    time_t t0;
    double diff;

    t0=time(NULL);
    diff=difftime(t0, st.st_mtime)/(60.0*60.0);
    if (diff<hours)
      /* cache entry still ok */
      return 1;
    /* cache outdated */
    return 0;
  }

  return 0;
}



const OH_INSTITUTE_SPEC_LIST *OfxHome_GetSpecs(OFXHOME *ofh) {
  if (ofh->specList==NULL) {
    OH_INSTITUTE_SPEC_LIST *sl;
    int rv;

    sl=OH_InstituteSpec_List_new();
    rv=OfxHome_CheckSpecsCache(ofh, OFX_CACHE_TIME_HR);
    if (rv<1) {
      /* no valid data in cache, download */
      rv=OfxHome_DownloadSpecs(ofh, sl);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        OH_InstituteSpec_List_free(sl);
        return NULL;
      }

      /* save data */
      rv=OfxHome_SaveSpecs(ofh, sl);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        OH_InstituteSpec_List_free(sl);
        return NULL;
      }
    }
    else {
      /* valid data in cache, load it */
      rv=OfxHome_LoadSpecs(ofh, sl);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        OH_InstituteSpec_List_free(sl);
        return NULL;
      }
    }
    ofh->specList=sl;
  }

  return ofh->specList;
}



int OfxHome_DownloadData(OFXHOME *ofh, int fid, OH_INSTITUTE_DATA **pData) {
  GWEN_HTTP_SESSION *sess;
  int rv;
  GWEN_BUFFER *xbuf;
  GWEN_XMLNODE *nroot;
  GWEN_XMLNODE *n;
  char urlbuf[256];
  OH_INSTITUTE_DATA *od;

  /* prepare session */
  snprintf(urlbuf, sizeof(urlbuf)-1, "http://www.ofxhome.com/api.php?lookup=%d", fid);
  urlbuf[sizeof(urlbuf)-1]=0;

  sess=GWEN_HttpSession_new(urlbuf, "http", 80);
  rv=OfxHome_SetupHttpSession(ofh, sess);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  rv=GWEN_HttpSession_Init(sess);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* send request (no body) */
  rv=GWEN_HttpSession_SendPacket(sess, "GET", NULL, 0);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* get response */
  xbuf=GWEN_Buffer_new(0, 1024, 0, 1);
  rv=GWEN_HttpSession_RecvPacket(sess, xbuf);
  if (rv<0 || rv<200 || rv>=300) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(xbuf);
    GWEN_HttpSession_free(sess);
    return rv;
  }

  /* fini */
  GWEN_HttpSession_Fini(sess);
  GWEN_HttpSession_free(sess);

  /* parse list */
  nroot=GWEN_XMLNode_fromString(GWEN_Buffer_GetStart(xbuf),
                                GWEN_Buffer_GetUsedBytes(xbuf),
                                GWEN_XML_FLAGS_DEFAULT |
                                GWEN_XML_FLAGS_HANDLE_HEADERS |
                                GWEN_XML_FLAGS_HANDLE_NAMESPACES);
  if (nroot==NULL) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_Dump(xbuf, 2);
    GWEN_Buffer_free(xbuf);
    return rv;
  }
  GWEN_Buffer_free(xbuf);

  n=GWEN_XMLNode_FindFirstTag(nroot, "institution", NULL, NULL);
  if (n==NULL) {
    DBG_ERROR(AQOFXCONNECT_LOGDOMAIN, "XML tree does not contain an \"institution\" element");
    GWEN_XMLNode_Dump(n, 2);
    GWEN_XMLNode_free(nroot);
    return GWEN_ERROR_BAD_DATA;
  }
  od=OH_InstituteData_fromXml(n);
  if (od==NULL) {
    DBG_WARN(AQOFXCONNECT_LOGDOMAIN, "element does not contain valid institute data");
    GWEN_XMLNode_Dump(n, 2);
    GWEN_XMLNode_free(nroot);
    return GWEN_ERROR_BAD_DATA;
  }
  else
    *pData=od;
  GWEN_XMLNode_free(nroot);

  return 0;
}



int OfxHome_SaveData(OFXHOME *ofh, const OH_INSTITUTE_DATA *od) {
  GWEN_DB_NODE *db;
  GWEN_BUFFER *nbuf;
  int rv;
  char numbuf[32];

  /* store institutes in db */
  db=GWEN_DB_Group_new("institute");
  rv=OH_InstituteData_toDb(od, db);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  snprintf(numbuf, sizeof(numbuf)-1, "%d", OH_InstituteData_GetId(od));
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(nbuf, numbuf);
  GWEN_Buffer_AppendString(nbuf, ".conf");

  /* write file */
  rv=GWEN_DB_WriteFile(db, GWEN_Buffer_GetStart(nbuf), GWEN_DB_FLAGS_DEFAULT | GWEN_DB_FLAGS_LOCKFILE);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* cleanup, done */
  GWEN_Buffer_free(nbuf);
  GWEN_DB_Group_free(db);

  return 0;
}



int OfxHome_LoadData(OFXHOME *ofh, int fid, OH_INSTITUTE_DATA **pData) {
  GWEN_DB_NODE *db;
  GWEN_BUFFER *nbuf;
  int rv;
  char numbuf[32];
  OH_INSTITUTE_DATA *od;

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  snprintf(numbuf, sizeof(numbuf)-1, "%d", fid);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(nbuf, numbuf);
  GWEN_Buffer_AppendString(nbuf, ".conf");

  /* read file */
  db=GWEN_DB_Group_new("institute");
  rv=GWEN_DB_ReadFile(db,
                      GWEN_Buffer_GetStart(nbuf),
                      GWEN_DB_FLAGS_DEFAULT |
                      GWEN_DB_FLAGS_LOCKFILE |
                      GWEN_PATH_FLAGS_CREATE_GROUP);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_Buffer_free(nbuf);
    GWEN_DB_Group_free(db);
    return rv;
  }
  GWEN_Buffer_free(nbuf);

  /* store institutes in db */
  od=OH_InstituteData_fromDb(db);
  if (rv<0) {
    DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
    GWEN_DB_Group_free(db);
    return rv;
  }

  /* cleanup, done */
  GWEN_DB_Group_free(db);
  *pData=od;

  return 0;
}



int OfxHome_CheckDataCache(OFXHOME *ofh, int fid, int hours) {
  GWEN_BUFFER *nbuf;
  int rv;
  char numbuf[32];
  struct stat st;

  /* create filename */
  nbuf=GWEN_Buffer_new(0, 256, 0, 1);
  if (ofh->dataFolder) {
    GWEN_Buffer_AppendString(nbuf, ofh->dataFolder);
    GWEN_Buffer_AppendString(nbuf, GWEN_DIR_SEPARATOR_S);
  }
  snprintf(numbuf, sizeof(numbuf)-1, "%d", fid);
  numbuf[sizeof(numbuf)-1]=0;
  GWEN_Buffer_AppendString(nbuf, numbuf);
  GWEN_Buffer_AppendString(nbuf, ".conf");

  rv=stat(GWEN_Buffer_GetStart(nbuf), &st);
  GWEN_Buffer_free(nbuf);
  if (rv) {
    /* not in cache */
    return -1;
  }
  else {
    time_t t0;
    double diff;

    t0=time(NULL);
    diff=difftime(t0, st.st_mtime)/(60.0*60.0);
    if (diff<hours)
      /* cache entry still ok */
      return 1;
    /* cache outdated */
    return 0;
  }

  return 0;
}



const OH_INSTITUTE_DATA *OfxHome_GetData(OFXHOME *ofh, int fid) {
  OH_INSTITUTE_DATA *d=NULL;

  d=OH_InstituteData_List_GetById(ofh->dataList, fid);
  if (d==NULL) {
    int rv;

    rv=OfxHome_CheckDataCache(ofh, fid, OFX_CACHE_TIME_HR);
    if (rv<1) {
      /* no valid data in cache, download */
      rv=OfxHome_DownloadData(ofh, fid, &d);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        return NULL;
      }

      /* save data */
      rv=OfxHome_SaveData(ofh, d);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        OH_InstituteData_free(d);
        return NULL;
      }
    }
    else {
      /* valid data in cache, load it */
      rv=OfxHome_LoadData(ofh, fid, &d);
      if (rv<0) {
        DBG_INFO(AQOFXCONNECT_LOGDOMAIN, "here (%d)", rv);
        return NULL;
      }
    }
    OH_InstituteData_List_Add(d, ofh->dataList);
  }

  return d;
}







