/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : March 3 2011
    copyright   : (C) 2011 by Christian Stimming
    email       : christian@cstimming.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#include "stringlist.hpp"

namespace AB
{

typedef std::vector<std::string> SVector;

StringList::StringList(const std::vector<std::string>& sv)
  : m_ptr(GWEN_StringList_new())
{
  for (SVector::const_iterator iter = sv.begin();
       iter != sv.end(); ++iter)
    {
      push_back(*iter);
    }
}

static void *cb_func(const char* value, void *user_data)
{
  SVector* result = reinterpret_cast<SVector*>(user_data);
  result->push_back(value);
  return NULL;
}

SVector StringList::toVector() const
{
  SVector result;
  GWEN_StringList_ForEach(m_ptr, &cb_func, &result);
  return result;
}


} // END namespace AB
