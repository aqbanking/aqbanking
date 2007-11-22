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
#include <gwenhywfar/io_file.h>
#include <gwenhywfar/iolayer.h>
#include <gwenhywfar/iomanager.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>

#ifdef OS_WIN32
# define mkdir(a,b) mkdir(a)
#endif
using namespace std;


LogAnalyzer::LogFile::LogMessage::LogMessage(GWEN_DB_NODE *header,
                                             const string &body)
:_header(header)
,_message(body) {

}



LogAnalyzer::LogFile::LogMessage::~LogMessage(){
  GWEN_DB_Group_free(_header);
}



int LogAnalyzer::LogFile::LogMessage::toFile(const string &fname) {
  int fd;
  int rv;
  GWEN_IO_LAYER *io;

  fd=open(fname.c_str(),
          O_RDWR | O_CREAT | O_APPEND,
          S_IRUSR | S_IWUSR);
  if (fd==-1) {
    DBG_ERROR(0, "fopen(%s): %s", fname.c_str(), strerror(errno));
    return GWEN_ERROR_IO;
  }

  /* create io layer for this file */
  io=GWEN_Io_LayerFile_new(-1, fd);
  assert(io);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* write header */
  rv=GWEN_DB_WriteToIo(_header, io,
		       GWEN_DB_FLAGS_WRITE_SUBGROUPS |
		       GWEN_DB_FLAGS_DETAILED_GROUPS |
		       GWEN_DB_FLAGS_USE_COLON|
		       GWEN_DB_FLAGS_OMIT_TYPES,
		       0,
		       2000);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* append empty line to separate header from data */
  rv=GWEN_Io_Layer_WriteChar(io, '\n',
			     GWEN_IO_REQUEST_FLAGS_WRITEALL,
			     0,
			     2000);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* write data */
  rv=GWEN_Io_Layer_WriteBytes(io,
			      (const uint8_t*)_message.data(),
			      _message.length(),
			      GWEN_IO_REQUEST_FLAGS_WRITEALL,
			      0,
			      2000);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* append CR for better readability */
  rv=GWEN_Io_Layer_WriteChar(io, '\n',
			     GWEN_IO_REQUEST_FLAGS_WRITEALL,
			     0,
			     2000);
  if (rv<0) {
    DBG_INFO(0, "here (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  /* close layer */
  rv=GWEN_Io_Layer_DisconnectRecursively(io, NULL, 0, 0, 30000);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    return rv;
  }

  GWEN_Io_Layer_free(io);

  return 0;
}






LogAnalyzer::LogFile::LogFile(const string &fname)
:_fileName(fname){
  GWEN_IO_LAYER *io;
  int fd;
  int rv;
  uint8_t buffer[1024];

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

  /* create io layer for this file */
  io=GWEN_Io_LayerFile_new(fd, -1);
  assert(io);

  rv=GWEN_Io_Manager_RegisterLayer(io);
  if (rv) {
    DBG_ERROR(GWEN_LOGDOMAIN, "Internal error: Could not register io layer (%d)", rv);
    GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					GWEN_IO_REQUEST_FLAGS_FORCE,
					0, 2000);
    GWEN_Io_Layer_free(io);
    throw Error("LogAnalyzer::Logfile::LogFile",
		ERROR_LEVEL_NORMAL,
		HBCI_ERROR_CODE_UNKNOWN,
		ERROR_ADVISE_DONTKNOW,
		"Error registering io layer",
		fname);
  }

  for (;;) {
    Pointer<LogMessage> msg;
    GWEN_DB_NODE *hd;
    unsigned int size;
    string body;

    // read header
    hd=GWEN_DB_Group_new("Header");
    rv=GWEN_DB_ReadFromIo(hd, io,
			  GWEN_DB_FLAGS_HTTP |
			  GWEN_DB_FLAGS_UNTIL_EMPTY_LINE,
			  0,
			  2000);
    if (rv<0) {
      if (rv==GWEN_ERROR_EOF)
	break;
      else {
	GWEN_DB_Group_free(hd);
	GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					    GWEN_IO_REQUEST_FLAGS_FORCE,
					    0, 2000);
	GWEN_Io_Layer_free(io);
	throw Error("LogAnalyzer::Logfile::LogFile",
		    ERROR_LEVEL_NORMAL,
		    HBCI_ERROR_CODE_UNKNOWN,
		    ERROR_ADVISE_DONTKNOW,
		    "Error reading header",
		    fname);
      }
    }
    // read body
    size=GWEN_DB_GetIntValue(hd, "size", 0, 0);
    while(size) {
      string tmp;
      unsigned int lsize;

      lsize=size;
      if (lsize>sizeof(buffer))
	lsize=sizeof(buffer);
      rv=GWEN_Io_Layer_ReadBytes(io,
				 buffer,
				 lsize,
				 0,
				 0,
				 2000);
      if (rv<1) {
	GWEN_DB_Group_free(hd);
	GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					    GWEN_IO_REQUEST_FLAGS_FORCE,
					    0, 2000);
	GWEN_Io_Layer_free(io);
	throw Error("LogAnalyzer::Logfile::LogFile",
		    ERROR_LEVEL_NORMAL,
		    HBCI_ERROR_CODE_UNKNOWN,
		    ERROR_ADVISE_DONTKNOW,
		    "Error reading body",
		    fname);
      }
      body+=string((const char*)buffer, rv);
      size-=rv;
    } // while

    /* read closing LF */
    rv=GWEN_Io_Layer_ReadBytes(io,
			       buffer,
			       1,
			       0,
			       0,
			       2000);
    if (rv<1) {
      GWEN_DB_Group_free(hd);
      GWEN_Io_Layer_DisconnectRecursively(io, NULL,
					  GWEN_IO_REQUEST_FLAGS_FORCE,
					  0, 2000);
      GWEN_Io_Layer_free(io);
      throw Error("LogAnalyzer::Logfile::LogFile",
		  ERROR_LEVEL_NORMAL,
		  HBCI_ERROR_CODE_UNKNOWN,
		  ERROR_ADVISE_DONTKNOW,
		  "Error reading body",
		  fname);
    }

    msg=new LogMessage(hd, body);
    DBG_INFO(0, "Adding message");
    _logMessages.push_back(msg);

  }

  GWEN_Io_Layer_DisconnectRecursively(io, NULL,
				      0,
				      0, 2000);
  GWEN_Io_Layer_free(io);
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
  GWEN_DIRECTORY *dlogs;
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





