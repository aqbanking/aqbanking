/***************************************************************************
  $RCSfile$
  -------------------
  cvs         : $Id$
  begin       : Sat Oct 25 2003
  copyright   : (C) 2003 by Martin Preuss
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

#include "loganalyzer.h"

#include <gwenhywfar/debug.h>
#include <gwenhywfar/directory.h>
#include <gwenhywfar/path.h>
#include <gwenhywfar/bufferedio.h>
#include <gwenhywfar/bio_buffer.h>

#ifndef WIN32
# include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
# include <io.h>
# include <direct.h>
# define open _open
# define close _close
# define mkdir(aa,bb) _mkdir((aa))
//# define S_IRUSR 0
//# define S_IWUSR 0
//# define S_IRWXU 0
//# define S_ISDIR(x) ((x) & _S_IFDIR) -- mingw-3.4.4 now has that defined.
//# define S_ISREG(x) ((x) & _S_IFREG)
#endif

LogAnalyzer::LogFile::LogMessage::LogMessage(GWEN_DB_NODE *header,
                                             const string &body)
:_header(header)
,_message(body) {

}



LogAnalyzer::LogFile::LogMessage::~LogMessage(){
  GWEN_DB_Group_free(_header);
}



string LogAnalyzer::LogFile::LogMessage::toString() const {
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  unsigned int size;
  const char *p;
  unsigned int pos;
  GWEN_BUFFER *buf;
  string result;

  buf=GWEN_Buffer_new(0, 1024, 0, 1);
  bio=GWEN_BufferedIO_Buffer2_new(buf, 0);
  GWEN_BufferedIO_SetWriteBuffer(bio, 0, 1024);
  if (GWEN_DB_WriteToStream(_header, bio,
                            GWEN_DB_FLAGS_WRITE_SUBGROUPS |
                            GWEN_DB_FLAGS_DETAILED_GROUPS |
                            GWEN_DB_FLAGS_USE_COLON|
                            GWEN_DB_FLAGS_OMIT_TYPES)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return "";
  }

  // append empty line to separate header from data
  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)){
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return "";
  }
  size=_message.length();

  p=_message.data();
  pos=0;
  while(pos<size) {
    unsigned int lsize;

    lsize=size-pos;
    err=GWEN_BufferedIO_WriteRaw(bio, p+pos, &lsize);
    if (!GWEN_Error_IsOk(err)) {
      DBG_INFO(0, "called from here");
      GWEN_BufferedIO_Abandon(bio);
      GWEN_BufferedIO_free(bio);
      GWEN_Buffer_free(buf);
      return "";
    }
    pos+=lsize;
  } // while

  // append CR for better readability
  err=GWEN_BufferedIO_WriteLine(bio, "");
  if (!GWEN_Error_IsOk(err)){
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_Abandon(bio);
    GWEN_BufferedIO_free(bio);
    GWEN_Buffer_free(buf);
    return "";
  }

  if (GWEN_BufferedIO_Close(bio)) {
    DBG_INFO(0, "called from here");
  }
  GWEN_BufferedIO_free(bio);

  result=string(GWEN_Buffer_GetStart(buf),
                GWEN_Buffer_GetUsedBytes(buf));
  GWEN_Buffer_free(buf);
  return result;
}






LogAnalyzer::LogFile::LogFile(const string &fname)
:_fileName(fname){
  GWEN_BUFFEREDIO *bio;
  GWEN_ERRORCODE err;
  int fd;
  int rv;

  fd=open(fname.c_str(), O_RDONLY);
  if (fd==-1) {
    DBG_ERROR(0, "Error opening file \"%s\": %s",
              fname.c_str(),
              strerror(errno));
    throw Error("LogAnalyzer::Logfile::LogFile",
                ERROR_LEVEL_NORMAL,
                HBCI_ERROR_CODE_UNKNOWN,
                ERROR_ADVISE_DONTKNOW,
                "Error opening file",
                fname);
  }

  bio=GWEN_BufferedIO_File_new(fd);
  GWEN_BufferedIO_SetReadBuffer(bio, 0, 1024);

  while (!GWEN_BufferedIO_CheckEOF(bio)) {
    Pointer<LogMessage> msg;
    GWEN_DB_NODE *hd;
    unsigned int size;
    string body;

    hd=GWEN_DB_Group_new("Header");
    rv=GWEN_DB_ReadFromStream(hd, bio, GWEN_DB_FLAGS_HTTP);
    if (rv) {
      GWEN_DB_Group_free(hd);
      GWEN_BufferedIO_free(bio);
      throw Error("LogAnalyzer::Logfile::LogFile",
                  ERROR_LEVEL_NORMAL,
                  HBCI_ERROR_CODE_UNKNOWN,
                  ERROR_ADVISE_DONTKNOW,
                  "Error reading header",
                  fname);
    }
    // read body
    size=GWEN_DB_GetIntValue(hd, "size", 0, 0);
    while(size) {
      string tmp;
      unsigned int lsize;
      char buffer[1024];

      lsize=size;
      if (lsize>sizeof(buffer))
        lsize=sizeof(buffer);
      if (GWEN_BufferedIO_ReadRaw(bio, buffer, &lsize)) {
        GWEN_DB_Group_free(hd);
        GWEN_BufferedIO_free(bio);
        throw Error("LogAnalyzer::Logfile::LogFile",
                    ERROR_LEVEL_NORMAL,
                    HBCI_ERROR_CODE_UNKNOWN,
                    ERROR_ADVISE_DONTKNOW,
                    "Error reading body",
                    fname);
      }
      body+=string(buffer, lsize);
      size-=lsize;
    } // while
    if (GWEN_BufferedIO_ReadChar(bio)==-1) {
      GWEN_DB_Group_free(hd);
      GWEN_BufferedIO_free(bio);
      throw Error("LogAnalyzer::Logfile::LogFile",
                  ERROR_LEVEL_NORMAL,
                  HBCI_ERROR_CODE_UNKNOWN,
                  ERROR_ADVISE_DONTKNOW,
                  "Error reading newline after body",
                  fname);
    }
    msg=new LogMessage(hd, body);
    DBG_INFO(0, "Adding message");
    _logMessages.push_back(msg);
  } // while

  err=GWEN_BufferedIO_Close(bio);
  if (!GWEN_Error_IsOk(err)) {
    DBG_INFO(0, "called from here");
    GWEN_BufferedIO_free(bio);
    throw Error("LogAnalyzer::Logfile::LogFile",
                ERROR_LEVEL_NORMAL,
                HBCI_ERROR_CODE_UNKNOWN,
                ERROR_ADVISE_DONTKNOW,
                "Error closing file",
                fname);
  }
  GWEN_BufferedIO_free(bio);
}



LogAnalyzer::LogFile::~LogFile(){
}







string LogAnalyzer::_getPath() {
  string dname;
  char buffer[256];
  void *p;
  char numbuf[16];

  dname=_baseDir;
  dname+="/backends/aqhbci/data/banks/";
  snprintf(numbuf, sizeof(numbuf), "%d", _country);
  dname+=numbuf;
  dname+="/";
  dname+=_bankCode;
  dname+="/logs/";

  DBG_INFO(0, "Searching in \"%s\"", dname.c_str());
  if (dname.length()>=sizeof(buffer)) {
    DBG_ERROR(0, "Path too long");
    return "";
  }

  buffer[0]=0;
  p=buffer;
  p=GWEN_Path_Handle(dname.c_str(), p,
                     GWEN_PATH_FLAGS_NAMEMUSTEXIST,
                     _handlePathElement);
  if (p==0)
    return "";
  return (const char*)p;
}


void *LogAnalyzer::_handlePathElement(const char *entry,
                                      void *data,
                                      unsigned int flags){
  char *p;
  struct stat st;
  bool exists;

  p=(char*)data;
  if ((strlen(p)+strlen(entry)+2)>=256) {
    DBG_ERROR(0, "Buffer too small");
    return 0;
  }
  strcat(p, "/");
  strcat(p, entry);

  // check for existence of the file/folder

  DBG_DEBUG(0, "Checking entry \"%s\"", p);
  if (stat(p, &st)) {
    exists=false;
    DBG_DEBUG(0, "stat: %s (%s)", strerror(errno), p);
    if ((flags & GWEN_PATH_FLAGS_PATHMUSTEXIST) ||
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_NAMEMUSTEXIST))) {
      DBG_ERROR(0, "Path \"%s\" does not exist (it should)", p);
      return 0;
    }
  }
  else {
    DBG_DEBUG(0, "Checking for type");
    exists=true;
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      if (!S_ISREG(st.st_mode)) {
        DBG_ERROR(0, "%s not a regular file", p);
        return 0;
      }
    }
    else {
      if (!S_ISDIR(st.st_mode)) {
	DBG_ERROR(0, "%s not a direcory", p);
        return 0;
      }
    }
    if ((flags & GWEN_PATH_FLAGS_PATHMUSTNOTEXIST) ||
        ((flags & GWEN_PATH_FLAGS_LAST) &&
         (flags & GWEN_PATH_FLAGS_NAMEMUSTNOTEXIST))) {
      DBG_ERROR(0, "Path \"%s\" does not exist (it should)", p);
      return 0;
    }
  } // if stat is ok

  if (!exists) {
    DBG_DEBUG(0, "Entry \"%s\" does not exist", p);
    if (flags & GWEN_PATH_FLAGS_VARIABLE) {
      // create file
      int fd;

      DBG_DEBUG(0, "Creating file \"%s\"", p);
      fd=open(p, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
      if (fd==-1) {
        DBG_ERROR(0, "open: %s (%s)", strerror(errno), p);
        return 0;
      }
      close(fd);
      DBG_DEBUG(0, "Sucessfully created");
    }
    else {
      // create dir
      DBG_DEBUG(0, "Creating folder \"%s\"", p);
      if (mkdir(p, S_IRWXU)) {
        DBG_ERROR(0, "mkdir: %s (%s)", strerror(errno), p);
        return 0;
      }
    }
  } // if exists
  else {
    DBG_DEBUG(0, "Entry \"%s\" exists", p);
  }
  DBG_DEBUG(0, "Returning this: %s", p);
  return p;
}



LogAnalyzer::LogAnalyzer(const string &baseDir,
                         unsigned int country,
                         const string &bank)
:_baseDir(baseDir)
,_country(country)
,_bankCode(bank) {
  GWEN_DIRECTORYDATA *dlogs;
  string dname;

  dname=_getPath();
  if (!dname.empty()) {
    dlogs=GWEN_Directory_new();
    if (!GWEN_Directory_Open(dlogs, dname.c_str())) {
      char nbuffer[256];

      while(!GWEN_Directory_Read(dlogs,
                                 nbuffer,
                                 sizeof(nbuffer))) {
        int i;

        i=strlen(nbuffer);
        if (i>4) {
          if (strcmp(nbuffer+i-4, ".log")==0) {
            string fname;

            fname=dname+"/"+nbuffer;
            DBG_NOTICE(0, "Added file \"%s\"", fname.c_str());
            _logFiles.push_back(fname);
          }
        }
      } // while read
      if (GWEN_Directory_Close(dlogs)) {
        GWEN_Directory_free(dlogs);
        throw Error("LogAnalyzer::LogAnalyzer",
                    ERROR_LEVEL_NORMAL,
                    HBCI_ERROR_CODE_UNKNOWN,
                    ERROR_ADVISE_DONTKNOW,
                    "Error closing dir",
                    dname);
      }
    } // if open succeeds
    GWEN_Directory_free(dlogs);
  } // if !empty
}



LogAnalyzer::~LogAnalyzer(){
}


Pointer<LogAnalyzer::LogFile> LogAnalyzer::getFirstLogFile(){
  Pointer<LogAnalyzer::LogFile> lf;

  _lfit=_logFiles.begin();
  if (_lfit!=_logFiles.end()) {
    lf=new LogFile(*_lfit);
    _lfit++;
  }
  return lf;
}


Pointer<LogAnalyzer::LogFile> LogAnalyzer::getNextLogFile(){
  Pointer<LogAnalyzer::LogFile> lf;

  if (_lfit!=_logFiles.end()) {
    lf=new LogFile(*_lfit);
    _lfit++;
  }
  return lf;
}





