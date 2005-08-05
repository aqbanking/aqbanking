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

#ifndef AQMONEY_LOGANALYZER_H
#define AQMONEY_LOGANALYZER_H

#include "pointer.h"
#include <gwenhywfar/db.h>

#include <list>
#include <string>
using namespace std;
using namespace HBCI;


/**
 *
 */
class LogAnalyzer {
public:
  class LogFile {
  public:
    class LogMessage {
    public:
    private:
      GWEN_DB_NODE *_header;
      string _message;
    public:
      LogMessage(GWEN_DB_NODE *header, const string &body);
      ~LogMessage();
      GWEN_DB_NODE *header() { return _header;};
      const string &message() const { return _message;};
      int toFile(const string &fname);
    };

  private:
    list<Pointer<LogMessage> > _logMessages;
    string _fileName;
  public:
    LogFile(const string &file);
    ~LogFile();
    list<Pointer<LogMessage> > logMessages() { return _logMessages;};
    const string &fileName() const { return _fileName; };
  };

private:
  string _baseDir;
  unsigned int _country;
  string _bankCode;
  list<string> _logFiles;
  list<string>::iterator _lfit;

  string _getPath();
  static void *_handlePathElement(const char *entry,
                                  void *data,
                                  unsigned int flags);

public:
  LogAnalyzer(const string &baseDir,
              unsigned int country,
              const string &bank);
  ~LogAnalyzer();

  Pointer<LogFile> getFirstLogFile();
  Pointer<LogFile> getNextLogFile();

};



#endif


