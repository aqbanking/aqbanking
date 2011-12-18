/***************************************************************************
 begin       : Thu Jul 03 2003
 copyright   : (C) 2003-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gwenhywfar/debug.h>
#include <gwenhywfar/text.h>

#include <gwenhywfar/syncio_file.h>

// #include <aqhbci/version.h>
#include <aqhbci/msgengine.h>
#include "loganalyzer.h"

#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <cstring>
#ifdef HAVE_STRINGS_H
# include <strings.h>
#endif


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if OS_WIN32
# include <windows.h>
//# define strcasecmp _stricmp
#endif

#include <list>
#include <string>
using namespace std;

#define MYNAME "hbcixml2"
#define PRG_VERSION_INFO \
  MYNAME " v1.99  (part of AqHBCI v"AQHBCI_VERSION_STRING")\n"\
  "(c) 2005 Martin Preuss<martin@libchipcard.de>\n" \
  "This program is free software licensed under GPL.\n"\
  "See COPYING for details.\n"


void usage(const char *prg) {
  fprintf(stderr,
	  MYNAME " - A tool to work on data in a HBCI XML file.\n"
	  "(c) 2003 Martin Preuss<martin@libchipcard.de>\n"
	  "This library is free software; you can redistribute it and/or\n"
	  "modify it under the terms of the GNU Lesser General Public\n"
	  "License as published by the Free Software Foundation; either\n"
	  "version 2.1 of the License, or (at your option) any later version.\n"
	  "\n"
	  "Usage:\n"
	  "%s COMMAND [OPTIONS]\n"
	  " COMMAND may be one of these:\n"
	  "  show   : shows the variables used by a given Job\n"
          "           (the job name must be given by \"-j\")\n"
          "  list   : lists all available jobs\n"
          "  analyze: analyze a log file. You can use this to anonymize\n"
          "           the log file before sendign it as a bug report\n"
          "\n"
	  " General Options:\n"
          " -f FILE  - the HBCI description file to load (xml-file)\n"
          "            repeat this option for each file to load\n"
          "            If this option is omitted the default XML files\n"
          "            are loaded\n"
          " -t TYPE  - the type of node inside the XML file\n"
          "            use \"job\" to inspect jobs,\n"
          "            \"seg\" to inspect segments\n"
          " -hv VER  - HBCI version to use (defaults to 210)\n"
          "            This effects which XML files will be loaded\n"
          "            upons startup (only if no -f option is given)\n"
          " -m MODE  - security mode to use with \"show\":\n"
	  "             DDV (chipcard mode with DDV cards)\n"
          "             RDH (keyfile mode) \n"
          "\n"
          " Special Options for \"show\":\n"
          " -j JOB   - name of the job/segment you are interested in\n"
          " -p       - shows even those variables which are automatically\n"
          "            preset by the message engine.\n"
          " -P       - shows even those variables which are marked as\n"
          "            being hidden\n"
          " -mv VER  - message/job/segment version to show \n"
          "            (0 uses the first available)\n"
          "\n"
          " Special Options for \"analyze\":\n"
          " --trustlevel L - the higher this level the more you trust the\n"
          "                  recipient of the output logfile.\n"
          " --analyze F    - name of the file to analyze\n"
          " -ol F          - name of anonymized output logfile\n"
          " -od F          - name of anonymized parsed logfile\n"
          " -os F          - name of SWIFT MT940/942 file to export\n"
          "\n"
          " --logfile FILE   - use given FILE as log file\n"
          " --logtype TYPE   - use given TYPE as log type\n"
          "                    These are the valid types:\n"
          "                     stderr (log to standard error channel)\n"
          "                     file   (log to the file given by --logfile)\n"
#ifdef HAVE_SYSLOG_H
          "                     syslog (log via syslog)\n"
#endif
          "                    Default is stderr\n"
          " --loglevel LEVEL - set the loglevel\n"
          "                    Valid levels are:\n"
          "                     emergency, alert, critical, error,\n"
          "                     warning, notice, info and debug\n"
          "                    Default is \"warning\".\n"
          "\n"
          "The simplest usage of this program is:\n"
          "  " MYNAME " show -j JobDialogInit\n"
          "This shows the properties used by the job \"JobInit\". Other example:\n"
	  "  " MYNAME " show -t seg -j Balance\n"
	  "shows the response segment of a JobGetBalance.\n",
          prg);
}


struct s_args {
  list<string> xmlfiles;  // -f
  string mode;            // -m
  string group;           // -j
  string typ;             // -t
  bool showPresets;       // -p
  bool showHidden;        // -P
  int version;            // -mv
  int hversion;           // -hv
  string analyzeFile;          // --analyze
  string outFile;              // -ol
  string parseFile;            // -od
  string swiftFile;            // -os
  int trustLevel;              // --trustlevel
  string logFile;              // --logfile
  GWEN_LOGGER_LOGTYPE logType; // --logtype
  GWEN_LOGGER_LEVEL logLevel;  // --loglevel
  list<string> params;
};


int checkArgs(s_args &args, int argc, char **argv) {
  int i;
  string tmp;

  i=2;
  args.showPresets=false;
  args.showHidden=false;
  args.mode="DDV";
  args.typ="job";
  args.version=0;
  args.hversion=210;
  args.logFile=MYNAME ".log";
  args.logType=GWEN_LoggerType_Console;
  args.logLevel=GWEN_LoggerLevel_Warning;
  args.trustLevel=0;

  if (argc<2) {
    usage(argv[0]);
    return 1;
  }
  args.params.push_back(argv[1]);
  while (i<argc){
    tmp=argv[i];
    if (tmp=="-f") {
      i++;
      if (i>=argc)
	return 1;
      args.xmlfiles.push_back(argv[i]);
    }
    else if (tmp=="-j") {
      i++;
      if (i>=argc)
	return 1;
      args.group=argv[i];
    }
    else if (tmp=="-t") {
      i++;
      if (i>=argc)
	return 1;
      args.typ=argv[i];
    }
    else if (tmp=="-m") {
      i++;
      if (i>=argc)
	return 1;
      args.mode=argv[i];
    }
    else if (tmp=="-p") {
      args.showPresets=true;
    }
    else if (tmp=="-P") {
      args.showHidden=true;
    }
    else if (tmp=="-mv") {
      i++;
      if (i>=argc)
        return 1;
      args.version=atoi(argv[i]);
    }
    else if (tmp=="-hv") {
      i++;
      if (i>=argc)
        return 1;
      args.hversion=atoi(argv[i]);
    }
    else if (tmp=="--analyze") {
      i++;
      if (i>=argc)
        return 1;
      args.analyzeFile=argv[i];
    }
    else if (tmp=="-ol") {
      i++;
      if (i>=argc)
        return 1;
      args.outFile=argv[i];
    }
    else if (tmp=="-od") {
      i++;
      if (i>=argc)
        return 1;
      args.parseFile=argv[i];
    }
    else if (tmp=="-os") {
      i++;
      if (i>=argc)
        return 1;
      args.swiftFile=argv[i];
    }
    else if (tmp=="--trustlevel") {
      i++;
      if (i>=argc)
        return 1;
      args.trustLevel=atoi(argv[i]);
    }
    else if (tmp=="--logtype") {
      i++;
      if (i>=argc)
	return -1;
      if (strcmp(argv[i],"stderr")==0)
        args.logType=GWEN_LoggerType_Console;
      else if (strcmp(argv[i],"file")==0)
	args.logType=GWEN_LoggerType_File;
#ifdef HAVE_SYSLOG_H
      else if (strcmp(argv[i],"syslog")==0)
	args.logType=GWEN_LoggerType_Syslog;
#endif
      else {
        fprintf(stderr,"Unknown log type \"%s\"\n",
		argv[i]);
	return -1;
      }
    }

    else if (tmp=="--loglevel") {
      i++;
      if (i>=argc)
	return -1;
      if (strcmp(argv[i], "emergency")==0)
	args.logLevel=GWEN_LoggerLevel_Emergency;
      else if (strcmp(argv[i], "alert")==0)
	args.logLevel=GWEN_LoggerLevel_Alert;
      else if (strcmp(argv[i], "critical")==0)
	args.logLevel=GWEN_LoggerLevel_Critical;
      else if (strcmp(argv[i], "error")==0)
	args.logLevel=GWEN_LoggerLevel_Error;
      else if (strcmp(argv[i], "warning")==0)
	args.logLevel=GWEN_LoggerLevel_Warning;
      else if (strcmp(argv[i], "notice")==0)
	args.logLevel=GWEN_LoggerLevel_Notice;
      else if (strcmp(argv[i], "info")==0)
	args.logLevel=GWEN_LoggerLevel_Info;
      else if (strcmp(argv[i], "debug")==0)
	args.logLevel=GWEN_LoggerLevel_Debug;
      else {
	fprintf(stderr,
                "Unknown log level \"%s\"\n",
		argv[i]);
	return -1;
      }
    }
    else if (tmp=="-h" || tmp=="--help") {
      usage(argv[0]);
      return -1;
    }
    else if (tmp=="-V" || tmp=="--version") {
      fprintf(stdout, PRG_VERSION_INFO);
      return -1;
    }
    else {
      fprintf(stderr,"unknown argument: %s\n",tmp.c_str());
      return -1;
    }
    i++;
  } // while
  // that's it

  if (args.params.empty()) {
    usage(argv[0]);
    return 1;
  }
  return 0;
}



int dumpNode(GWEN_XMLNODE *n,
             const s_args &args,
             unsigned int flags,
             unsigned int ind,
             int groupsOnly) {
  if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
    const char *name;
    unsigned int i;

    name=GWEN_XMLNode_GetData(n);
    if (strcasecmp(name, "ELEM")==0) {
      if (!groupsOnly) {
        // found an element
        const char *path;

        path=GWEN_XMLNode_GetProperty(n, "GWEN_path", "");
        if (*path) {
          unsigned int minnum;
          unsigned int maxnum;
          unsigned int minsize;
          unsigned int maxsize;
          bool hide;
          const char *type, *bintype, *binsubtype;

          minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum", "1"));
          maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum", "1"));
          minsize=atoi(GWEN_XMLNode_GetProperty(n, "minsize", "1"));
          maxsize=atoi(GWEN_XMLNode_GetProperty(n, "maxsize", "0"));
          hide=atoi(GWEN_XMLNode_GetProperty(n, "hide", "0"));
          type=GWEN_XMLNode_GetProperty(n, "type", "1");
          bintype=GWEN_XMLNode_GetProperty(n, "bintype", 0);
          binsubtype=GWEN_XMLNode_GetProperty(n, "binsubtype", 0);

          if (!hide || args.showHidden) {
            // indent
            for (i=0; i<ind; i++)
              printf(" ");

            printf("%s   (type \"%s\"", path, type);
            if (bintype) {
              printf(".%s", bintype);
              if (binsubtype) {
                printf(".%s", binsubtype);
              }
            }
            printf(", need %d", minnum);
            if (maxnum!=minnum)
              printf("-%d", maxnum);
            if (minsize!=0 && maxsize!=0) {
              printf(", size %d", minsize);
              if (maxsize!=minsize)
                printf("-%d", maxsize);
            }
            else if (minsize!=0 && maxsize==0) {
              printf(", size %d-xx", minsize);
            }
            else if (minsize==0 && maxsize!=0) {
              printf(", size xx-%d", maxsize);
            }
            if (atoi(GWEN_XMLNode_GetProperty(n, "GWEN_set", "0")))
              printf(", [set]");
            printf(")\n");
          } // if !hide
        }
        else {
          DBG_INFO(0, "No path for element");
        }
      }
    }
    else if (strcasecmp(name, "VALUES")==0) {
    }
    else {
      // found a group
      unsigned int minnum;
      unsigned int maxnum;
//      unsigned int minsize;
//      unsigned int maxsize;
      const char *type;
      const char *path;
      bool hide;
      GWEN_XMLNODE *nn;

      path=GWEN_XMLNode_GetProperty(n, "GWEN_path", "");

      minnum=atoi(GWEN_XMLNode_GetProperty(n, "minnum", "1"));
      maxnum=atoi(GWEN_XMLNode_GetProperty(n, "maxnum", "1"));
//      minsize=atoi(GWEN_XMLNode_GetProperty(n, "minsize", "1"));
//      maxsize=atoi(GWEN_XMLNode_GetProperty(n, "maxsize", "1"));
      hide=atoi(GWEN_XMLNode_GetProperty(n, "hide", "0"));
      type=GWEN_XMLNode_GetProperty(n, "type", "1");

      if (!hide || args.showHidden) {
        if (minnum==1 && maxnum==1) {
          if (!groupsOnly) {
            // dump all children with the same indendation
            nn=GWEN_XMLNode_GetChild(n);
            while(nn) {
              if (dumpNode(nn, args, flags, ind, 0))
                return 1;
              nn=GWEN_XMLNode_Next(nn);
            } // while
          }
        }
        else {
          if (groupsOnly) {
            // indent
            for (i=0; i<ind; i++)
              printf(" ");
            printf("---------------------------------------\n");
            for (i=0; i<ind; i++)
              printf(" ");
            printf("Group");
            if (*path)
              printf(" %s", path);
            printf("  (type \"%s\"", type);
            printf(", need %d", minnum);
            if (maxnum!=minnum)
              printf("-%d", maxnum);
            printf(")\n");

            // dump all children
            nn=GWEN_XMLNode_GetChild(n);
            while(nn) {
              if (dumpNode(nn, args, flags, ind+2, 0))
                return 1;
              nn=GWEN_XMLNode_Next(nn);
            } // while
            nn=GWEN_XMLNode_GetChild(n);
            while(nn) {
              if (dumpNode(nn, args, flags, ind+2, 1))
                return 1;
              nn=GWEN_XMLNode_Next(nn);
            } // while
          } // if groupsonly
        }
      } // if !hide
    }
  } // if group

  return 0;
}


int show(const s_args &args) {
  GWEN_XMLNODE *defs;
  GWEN_MSGENGINE *e;
  unsigned int flags;
  list<string>::const_iterator it;
  GWEN_XMLNODE *listNode;
  GWEN_XMLNODE *n;
  int version;

  flags=0;
  flags|=GWEN_MSGENGINE_SHOW_FLAGS_NOSET;
  if (args.showPresets)
    flags&=~GWEN_MSGENGINE_SHOW_FLAGS_NOSET;
  e=AH_MsgEngine_new();

  GWEN_MsgEngine_SetMode(e, args.mode.c_str());

  /* read defs */
  for (it=args.xmlfiles.begin();
       it!=args.xmlfiles.end();
       it++) {
    defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"defs");
    DBG_DEBUG(0, "Reading file %s", (*it).c_str());
    if (GWEN_XML_ReadFile(defs, (*it).c_str(), GWEN_XML_FLAGS_DEFAULT)){
      fprintf(stderr,"Error parsing.\n");
      GWEN_MsgEngine_free(e);
      return 2;
    }
    GWEN_MsgEngine_AddDefinitions(e, defs);
    GWEN_XMLNode_free(defs);
  } // for


  if (args.logLevel>=GWEN_LoggerLevel_Debug)
    GWEN_XMLNode_Dump(GWEN_MsgEngine_GetDefinitions(e), 1);

  listNode=GWEN_MsgEngine_ListMessage(e,
                                      args.typ.c_str(),
                                      args.group.c_str(),
                                      args.version,
                                      flags);
  if (!listNode) {
    fprintf(stderr, "Error listing message.\n");
    GWEN_MsgEngine_free(e);
    return 2;
  }
  GWEN_MsgEngine_free(e);

  DBG_INFO(0, "Listnode:");
  if (args.logLevel>=GWEN_LoggerLevel_Info)
    GWEN_XMLNode_Dump(listNode, 1);

  version=atoi(GWEN_XMLNode_GetProperty(listNode, "version", "-1"));

  if (version==-1)
    version=args.version;
  printf("Description of %s \"%s\" (version %d)\n",
         args.typ.c_str(),
         args.group.c_str(),
         version);
  printf("------------------------------------------------------------\n");
  if (strcasecmp(args.typ.c_str(),"job")==0) {
    const char *response;
    const char *bpdjob;

    response=GWEN_XMLNode_GetProperty(listNode, "response", 0);
    bpdjob=GWEN_XMLNode_GetProperty(listNode, "params", 0);
    if (response)
      printf("Job response segment  : \"%s\"\n", response);
    if (bpdjob)
      printf("Job parameter segment : \"%s\"\n", bpdjob);
  }

  printf("Variables:\n");

  n=GWEN_XMLNode_GetChild(listNode);
  while(n) {
    dumpNode(n, args, flags, 2, 0);
    n=GWEN_XMLNode_Next(n);
  } // while

  n=GWEN_XMLNode_GetChild(listNode);
  while(n) {
    dumpNode(n, args, flags, 2, 1);
    n=GWEN_XMLNode_Next(n);
  } // while

  GWEN_XMLNode_free(listNode);

  return 0;
}



int listAll(const s_args &args) {
  GWEN_XMLNODE *defs;
  GWEN_MSGENGINE *e;
  list<string>::const_iterator it;
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *cn;
  char tgbuffer[128];
  char tdbuffer[128];

  e=AH_MsgEngine_new();
  GWEN_MsgEngine_SetMode(e, args.mode.c_str());

  /* read defs */
  for (it=args.xmlfiles.begin();
       it!=args.xmlfiles.end();
       it++) {
    defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"defs");
    if (GWEN_XML_ReadFile(defs, (*it).c_str(), GWEN_XML_FLAGS_DEFAULT)){
      fprintf(stderr,"Error parsing.\n");
      GWEN_MsgEngine_free(e);
      return 2;
    }
    GWEN_MsgEngine_AddDefinitions(e, defs);
    GWEN_XMLNode_free(defs);
  } // for


  if (args.logLevel>=GWEN_LoggerLevel_Info)
    GWEN_XMLNode_Dump(GWEN_MsgEngine_GetDefinitions(e), 1);


  n=GWEN_MsgEngine_GetDefinitions(e);
  if (!n) {
    fprintf(stderr, "No definitions found.\n");
    GWEN_MsgEngine_free(e);
    return 3;
  }

  if (args.typ.length()+1>=sizeof(tgbuffer) ||
      args.typ.length()+3>=sizeof(tdbuffer)) {
    fprintf(stderr, "Typename too long !\n");
    return 1;
  }
  sprintf(tgbuffer, "%ss", args.typ.c_str());
  sprintf(tdbuffer, "%sdef", args.typ.c_str());

  n=GWEN_XMLNode_GetChild(n);
  while(n) {
    const char *name;

    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      name=GWEN_XMLNode_GetData(n);
      if (strcasecmp(name, tgbuffer)==0) {
        // we have a matching group of definitions
        cn=GWEN_XMLNode_GetChild(n);
        while(cn) {
          const char *cname;

          if (GWEN_XMLNode_GetType(cn)==GWEN_XMLNodeTypeTag) {
            cname=GWEN_XMLNode_GetData(cn);
            if (strcasecmp(cname, tdbuffer)==0) {
              // we have a matching definition
              const char *cid;
              const char *bywhat;
              int version;
              int crypt;
              int sign;
              const char *mode;
              int internal;

              version=atoi(GWEN_XMLNode_GetProperty(cn, "version", "0"));
              crypt=atoi(GWEN_XMLNode_GetProperty(cn, "crypt", "1"));
              sign=atoi(GWEN_XMLNode_GetProperty(cn, "sign", "1"));
              mode=GWEN_XMLNode_GetProperty(cn, "mode", 0);
              internal=atoi(GWEN_XMLNode_GetProperty(cn, "internal", "0"));

              bywhat="id";
              cid=GWEN_XMLNode_GetProperty(cn, bywhat, 0);
              if (!cid) {
                bywhat="name";
                cid=GWEN_XMLNode_GetProperty(cn, bywhat, 0);
              }
              if (!cid) {
                bywhat="code";
                cid=GWEN_XMLNode_GetProperty(cn, bywhat, 0);
              }
              printf("\"%s\"", cid);
              if (version)
                printf(", version %d", version);
              if (strcasecmp(bywhat, "id")!=0)
                printf(", selected by %s", bywhat);
              if (!sign)
                printf(", nosign");
              if (!crypt)
                printf(", nocrypt");
              if (mode)
                printf(", %s mode", mode);
              if (internal)
                printf(", internal");
              printf("\n");
            }
          }
          cn=GWEN_XMLNode_Next(cn);
        }
      }
    }
    n=GWEN_XMLNode_Next(n);
  }

  GWEN_MsgEngine_free(e);
  return 0;
}



int checkAll(const s_args &args) {
  GWEN_XMLNODE *defs;
  GWEN_MSGENGINE *e;
  list<string>::const_iterator it;
  GWEN_XMLNODE *n;
  GWEN_XMLNODE *cn;
  char tgbuffer[128];
  char tdbuffer[128];
  int errors=0;

  e=AH_MsgEngine_new();
  GWEN_MsgEngine_SetMode(e, args.mode.c_str());

  /* read defs */
  for (it=args.xmlfiles.begin();
       it!=args.xmlfiles.end();
       it++) {
    defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"defs");
    if (GWEN_XML_ReadFile(defs, (*it).c_str(), GWEN_XML_FLAGS_DEFAULT)){
      fprintf(stderr,"Error parsing.\n");
      GWEN_MsgEngine_free(e);
      return 2;
    }
    GWEN_MsgEngine_AddDefinitions(e, defs);
    GWEN_XMLNode_free(defs);
  } // for


  if (args.logLevel>=GWEN_LoggerLevel_Info)
    GWEN_XMLNode_Dump(GWEN_MsgEngine_GetDefinitions(e), 1);


  n=GWEN_MsgEngine_GetDefinitions(e);
  if (!n) {
    fprintf(stderr, "No definitions found.\n");
    GWEN_MsgEngine_free(e);
    return 3;
  }

  if (args.typ.length()+1>=sizeof(tgbuffer) ||
      args.typ.length()+3>=sizeof(tdbuffer)) {
    fprintf(stderr, "Typename too long !\n");
    return 1;
  }
  sprintf(tgbuffer, "%ss", args.typ.c_str());
  sprintf(tdbuffer, "%sdef", args.typ.c_str());

  n=GWEN_XMLNode_GetChild(n);
  while(n) {
    const char *name;

    if (GWEN_XMLNode_GetType(n)==GWEN_XMLNodeTypeTag) {
      name=GWEN_XMLNode_GetData(n);
      if (strcasecmp(name, tgbuffer)==0) {
        // we have a matching group of definitions
        cn=GWEN_XMLNode_GetChild(n);
        while(cn) {
          const char *cname;

          if (GWEN_XMLNode_GetType(cn)==GWEN_XMLNodeTypeTag) {
            cname=GWEN_XMLNode_GetData(cn);
            if (strcasecmp(cname, tdbuffer)==0) {
              // we have a matching definition
              bool isError=false;
              bool isInternal=false;

              isInternal=(atoi(GWEN_XMLNode_GetProperty(cn,
                                                        "internal",
                                                        "0"))!=0);
              if (GWEN_XMLNode_FindFirstTag(cn, "MESSAGE", 0, 0)) {
                /* multi-message job, check further */
              }
              else {
                /* single-message job, check further */
                if (GWEN_XMLNode_GetProperty(cn, "code", 0)==0 &&
                    !isInternal) {
                  fprintf(stderr, "This element has no \"code\" attribute\n");
                  isError=true;
                }
              }

              if (isError) {
                GWEN_XMLNode_Dump(cn, 2);
                errors++;
              }
            }
          }
          cn=GWEN_XMLNode_Next(cn);
        }
      }
    }
    n=GWEN_XMLNode_Next(n);
  }

  GWEN_MsgEngine_free(e);

  if (errors) {
    fprintf(stderr, "Found %d error(s)\n", errors);
    return 2;
  }

  fprintf(stderr, "No errors found\n");
  return 0;
}



void _logMessage(const string &fname,
                 const string &msg,
                 GWEN_DB_NODE *hd) {
  int rv;
  GWEN_SYNCIO *sio;

  sio=GWEN_SyncIo_File_new(fname.c_str(), GWEN_SyncIo_File_CreationMode_CreateAlways);
  GWEN_SyncIo_AddFlags(sio,
		       GWEN_SYNCIO_FILE_FLAGS_READ |
		       GWEN_SYNCIO_FILE_FLAGS_WRITE |
		       GWEN_SYNCIO_FILE_FLAGS_UREAD |
		       GWEN_SYNCIO_FILE_FLAGS_UWRITE |
		       GWEN_SYNCIO_FILE_FLAGS_APPEND);
  rv=GWEN_SyncIo_Connect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return;
  }

  rv=GWEN_DB_WriteToIo(hd, sio,
		       GWEN_DB_FLAGS_WRITE_SUBGROUPS |
		       GWEN_DB_FLAGS_DETAILED_GROUPS |
		       GWEN_DB_FLAGS_USE_COLON|
		       GWEN_DB_FLAGS_OMIT_TYPES);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return;
  }

  /* append empty line to separate header from data */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return;
  }

  /* write data */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) msg.data(), msg.length());
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return;
  }

  /* append CR for better readability */
  rv=GWEN_SyncIo_WriteForced(sio, (const uint8_t*) "\n", 1);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_SyncIo_Disconnect(sio);
    GWEN_SyncIo_free(sio);
    return;
  }

  /* close layer */
  rv=GWEN_SyncIo_Disconnect(sio);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_SyncIo_free(sio);
    return;
  }

  GWEN_SyncIo_free(sio);
}



int analyzeLog(const s_args &args) {
  GWEN_XMLNODE *defs;
  GWEN_MSGENGINE *e;
  list<string>::const_iterator it;
  Pointer<LogAnalyzer::LogFile> logfile;
  list<Pointer<LogAnalyzer::LogFile::LogMessage> > lmsgs;
  list<Pointer<LogAnalyzer::LogFile::LogMessage> >::iterator lmit;
  GWEN_MSGENGINE_TRUSTEDDATA *trustedData;
  GWEN_MSGENGINE_TRUSTEDDATA *ntd;
  GWEN_DB_NODE *allgr;

  e=AH_MsgEngine_new();

  GWEN_MsgEngine_SetMode(e, args.mode.c_str());

  /* read defs */
  for (it=args.xmlfiles.begin();
       it!=args.xmlfiles.end();
       it++) {
    defs=GWEN_XMLNode_new(GWEN_XMLNodeTypeTag,"defs");
    DBG_DEBUG(0, "Reading file %s", (*it).c_str());
    if (GWEN_XML_ReadFile(defs, (*it).c_str(), GWEN_XML_FLAGS_DEFAULT)){
      fprintf(stderr,"Error parsing.\n");
      GWEN_MsgEngine_free(e);
      return 2;
    }
    GWEN_MsgEngine_AddDefinitions(e, defs);
    GWEN_XMLNode_free(defs);
  } // for

  try {
    logfile=new LogAnalyzer::LogFile(args.analyzeFile);
  }
  catch (Error xerr) {
    fprintf(stderr, "Error: %s\n", xerr.errorString().c_str());
    GWEN_MsgEngine_free(e);
    return 2;
  }

  allgr=GWEN_DB_Group_new("messages");
  lmsgs=logfile.ref().logMessages();
  for (lmit=lmsgs.begin(); lmit!=lmsgs.end(); lmit++) {
    GWEN_DB_NODE *gr;
    GWEN_DB_NODE *hd;
    GWEN_DB_NODE *repl;
    GWEN_BUFFER *mbuf;
    int rv;
    string lstr;
    string mode;

    gr=GWEN_DB_Group_new("message");
    lstr=(*lmit).ref().message();
    hd=(*lmit).ref().header();
    mode=GWEN_DB_GetCharValue(hd, "mode",0, args.mode.c_str());
    GWEN_MsgEngine_SetMode(e, mode.c_str());
    mbuf=GWEN_Buffer_new((char*)((*lmit).ref().message().data()),
                         (*lmit).ref().message().length(),
                         (*lmit).ref().message().length(),
                         0);

    DBG_INFO(0, "Reading message");
    rv=GWEN_MsgEngine_ReadMessage(e, "SEG", mbuf, gr,
                                  GWEN_MSGENGINE_READ_FLAGS_TRUSTINFO);
    if (rv) {
      fprintf(stderr, "ERROR.\n");
      return 2;
    }
    GWEN_Buffer_free(mbuf);

    // work on trust data
    trustedData=GWEN_MsgEngine_TakeTrustInfo(e);
    if (trustedData) {
      if (GWEN_MsgEngine_TrustedData_CreateReplacements(trustedData)) {
        fprintf(stderr, "Could not anonymize log (createReplacements)\n");
        GWEN_MsgEngine_TrustedData_free(trustedData);
	GWEN_MsgEngine_free(e);
        return 0;
      }
    }

    if (!args.swiftFile.empty()) {
      int bnum=0;
      int nnum=0;

      if ((strcasecmp(GWEN_DB_GetCharValue(gr, "logHeader/sender", 0,
					   "bank"), "bank")==0) &&
	  (strcasecmp(GWEN_DB_GetCharValue(gr, "logHeader/crypt", 0,
					   "no"), "no")==0)
	 ) {
	GWEN_DB_NODE *dbT;

	dbT=GWEN_DB_FindFirstGroup(gr, "transactions");
	while(dbT) {
	  const void *p;
	  unsigned int len;

	  p=GWEN_DB_GetBinValue(dbT, "booked", 0, 0, 0, &len);
	  if (p && len) {
	    GWEN_BUFFER *fbuf;
	    FILE *f;

	    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(fbuf, args.swiftFile.c_str());
	    GWEN_Buffer_AppendString(fbuf, ".booked");
	    if (bnum!=0) {
	      char numbuf[32];

	      snprintf(numbuf, sizeof(numbuf), "%d", bnum);
	      GWEN_Buffer_AppendString(fbuf, ".");
	      GWEN_Buffer_AppendString(fbuf, numbuf);
	    }
	    f=fopen(GWEN_Buffer_GetStart(fbuf), "w+");
	    if (!f) {
	      DBG_ERROR(0, "fopen(%s): %s",
			GWEN_Buffer_GetStart(fbuf),
			strerror(errno));
	      return 2;
	    }
	    if (1!=fwrite(p, len, 1, f)) {
	      DBG_ERROR(0, "fwrite(%s, %d bytes): %s",
			GWEN_Buffer_GetStart(fbuf),
			len,
			strerror(errno));
	      return 2;
	    }
	    if (fclose(f)) {
	      DBG_ERROR(0, "fclose(%s): %s",
			GWEN_Buffer_GetStart(fbuf),
			strerror(errno));
	      return 2;
	    }
	    GWEN_Buffer_free(fbuf);
	    bnum++;
	  }

	  p=GWEN_DB_GetBinValue(dbT, "noted", 0, 0, 0, &len);
	  if (p && len) {
	    GWEN_BUFFER *fbuf;
	    FILE *f;

	    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
	    GWEN_Buffer_AppendString(fbuf, args.swiftFile.c_str());
	    GWEN_Buffer_AppendString(fbuf, ".noted");
	    if (nnum!=0) {
	      char numbuf[32];

	      snprintf(numbuf, sizeof(numbuf), "%d", nnum);
	      GWEN_Buffer_AppendString(fbuf, ".");
	      GWEN_Buffer_AppendString(fbuf, numbuf);
	    }
	    f=fopen(GWEN_Buffer_GetStart(fbuf), "w+");
	    if (!f) {
	      DBG_ERROR(0, "fopen(%s): %s",
			GWEN_Buffer_GetStart(fbuf),
			strerror(errno));
	      return 2;
	    }
	    if (1!=fwrite(p, len, 1, f)) {
	      DBG_ERROR(0, "fwrite(%s, %d bytes): %s",
			GWEN_Buffer_GetStart(fbuf),
			len,
			strerror(errno));
	      return 2;
	    }
	    if (fclose(f)) {
	      DBG_ERROR(0, "fclose(%s): %s",
			GWEN_Buffer_GetStart(fbuf),
			strerror(errno));
	      return 2;
	    }
	    GWEN_Buffer_free(fbuf);
	    nnum++;
	  }
	  dbT=GWEN_DB_FindNextGroup(dbT, "transactions");
	}
      }
    } // if swiftFile

    // anonymize
    ntd=trustedData;
    repl=GWEN_DB_GetGroup(hd, GWEN_DB_FLAGS_DEFAULT, "replacements");
    assert(repl);
    while(ntd) {
      if (GWEN_MsgEngine_TrustedData_GetTrustLevel(ntd)>args.trustLevel) {
        int pos;
        unsigned int size;
        char rbuffer[3];
        const char *rpstr;

        rpstr=GWEN_MsgEngine_TrustedData_GetReplacement(ntd);
        assert(rpstr);
        assert(*rpstr);
        size=strlen(rpstr);
        if (size==1) {
          rbuffer[0]=rpstr[0];
          rbuffer[1]=0;
        }
        else {
          rbuffer[0]=rpstr[0];
          rbuffer[1]=rpstr[1];
          rbuffer[2]=0;
        }
        GWEN_DB_SetCharValue(repl,
                             GWEN_DB_FLAGS_DEFAULT |
                             GWEN_PATH_FLAGS_CREATE_VAR,
                             rbuffer,
                             GWEN_MsgEngine_TrustedData_GetDescription(ntd));
        size=GWEN_MsgEngine_TrustedData_GetSize(ntd);
        pos=GWEN_MsgEngine_TrustedData_GetFirstPos(ntd);
        while(pos>=0) {
          DBG_INFO(0, "Replacing %d bytes at %d", size, pos);
          lstr.replace(pos, size,
                       GWEN_MsgEngine_TrustedData_GetReplacement(ntd));
          pos=GWEN_MsgEngine_TrustedData_GetNextPos(ntd);
        } // while pos
      }
      ntd=GWEN_MsgEngine_TrustedData_GetNext(ntd);
    } // while ntd

    // log anonymized message
    if (!args.outFile.empty())
      _logMessage(args.outFile,
                  lstr,
                  hd);

    // parse anonymized message and store it
    if (!args.parseFile.empty()) {
      GWEN_BUFFER *nmbuf;
      GWEN_DB_NODE *ngr;
      GWEN_DB_NODE *hdgr;

      ngr=GWEN_DB_GetGroup(allgr, GWEN_DB_FLAGS_DEFAULT |
                           GWEN_PATH_FLAGS_CREATE_GROUP,
                           "message");
      assert(ngr);
      hdgr=GWEN_DB_GetGroup(ngr, GWEN_DB_FLAGS_DEFAULT,
                            "logheader");
      GWEN_DB_AddGroupChildren(hdgr, hd);
      nmbuf=GWEN_Buffer_new((char*)lstr.data(),
                            lstr.length(),
                            lstr.length(),
                            0);

      DBG_INFO(0, "Rereading message");
      rv=GWEN_MsgEngine_ReadMessage(e, "SEG", nmbuf, ngr,
                                    GWEN_MSGENGINE_READ_FLAGS_DEFAULT);
      if (rv) {
        fprintf(stderr, "ERROR parsing message.\n");
        GWEN_Buffer_free(nmbuf);
        GWEN_DB_Group_free(allgr);
	GWEN_MsgEngine_free(e);
        return 2;
      }
      GWEN_Buffer_free(nmbuf);
    } // if parseFile

    // free trust info
    ntd=trustedData;
    while(ntd) {
      GWEN_MSGENGINE_TRUSTEDDATA *nnn;

      nnn=GWEN_MsgEngine_TrustedData_GetNext(ntd);
      GWEN_MsgEngine_TrustedData_free(ntd);
      ntd=nnn;
    } // while
  } // for

  if (!args.parseFile.empty()) {
    if (GWEN_DB_WriteFile(allgr, args.parseFile.c_str(),
			  GWEN_DB_FLAGS_DEFAULT)) {
      fprintf(stderr, "ERROR saving message.\n");
      GWEN_DB_Group_free(allgr);
      GWEN_MsgEngine_free(e);
      return 2;
    }
  }
  GWEN_MsgEngine_free(e);
  return 0;
}



int main(int argc, char **argv) {
  s_args args;
  int rv;
  string cmd;

  rv=checkArgs(args,argc,argv);
  if (rv==-1)
    return 0;
  else if (rv)
    return rv;

  if (args.xmlfiles.empty()) {
    string fname;

    // fill with default files

    fname=XMLDATA_DIR;
    fname+="/hbci.xml";
    args.xmlfiles.push_back(fname);
  }


  if (GWEN_Logger_Open(0,
                       MYNAME,
                       args.logFile.c_str(),
                       args.logType,
		       GWEN_LoggerFacility_User)) {
    fprintf(stderr, "Could not start logging, aborting.\n");
    return 2;
  }
  GWEN_Logger_SetLevel(0, args.logLevel);

  cmd=args.params.front();
  if (cmd=="show") {
    rv=show(args);
  }
  else if (cmd=="list") {
    rv=listAll(args);
  }
  else if (cmd=="analyze") {
    rv=analyzeLog(args);
  }
  else if (cmd=="check") {
    rv=checkAll(args);
  }
  else {
    usage(argv[0]);
    return 1;
  }

  return rv;
}







