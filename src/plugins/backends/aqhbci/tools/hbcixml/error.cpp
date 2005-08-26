/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Sun Nov 18 2001
    copyright   : (C) 2001 by Martin Preuss
    email       : openhbci@aquamaniac.de

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
#include <config.h>
#endif

#include <cassert>
#include <cstdio>
#include "error.h"
using namespace std;

namespace HBCI {

Error::Error(string where,
	     string message,
	     int code)
    :_where(where)
    ,_level(ERROR_LEVEL_NORMAL)
    ,_code(code)
    ,_advise(ERROR_ADVISE_DONTKNOW)
    ,_message(message)
{
}



Error::Error()
:_level(ERROR_LEVEL_NONE)
,_code(0)
,_advise(ERROR_ADVISE_DONTKNOW)
{
}


Error::Error(string where,
	     ErrorLevel level,
	     int code,
	     ErrorAdvise advise,
	     string message,
	     string info)
:_where(where)
,_level(level)
,_code(code)
,_advise(advise)
,_message(message)
,_info(info)
{
}


Error::Error(const string &where,
	     const Error &err){
  *this=err;
  if (_reportedFrom.empty())
    _reportedFrom=where;
  else
    _reportedFrom=where+"/"+_reportedFrom;
}


Error::~Error(){
}


string Error::errorString() const {
  string result;

  if (_level==ERROR_LEVEL_NONE)
    result="NONE";
  else {
    char numbuf[16];

    result+=_message;
    result+=" (";
    snprintf(numbuf, sizeof(numbuf), "%d", _code);
    result+=numbuf;
    result+=") at ";
    result+=_where;
    if (!_info.empty()) {
      result+=" Info: ";
      result+=_info;
    }
    if (!_reportedFrom.empty()) {
      result+=" reported from ";
      result+=_reportedFrom;
    }
  }
  return result;
}


} // namespace HBCI

