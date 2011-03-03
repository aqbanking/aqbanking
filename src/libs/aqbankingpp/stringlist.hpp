/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : Mon March 2 2011
    copyright   : (C) 2011 by Christian Stimming
    email       : christian@cstimming.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AB_STRINGLIST_HPP
#define AB_STRINGLIST_HPP

#include <gwenhywfar/stringlist.h>

#include <aqbankingpp/cxxwrap.hpp>
#include <aqbankingpp/aqbankingppdecl.hpp>
#include <string>
#include <vector>
  
namespace AB
{

/** A wrapper class around the GWEN_STRINGLIST type */
class AQBANKINGPP_DECL StringList
{
public:
  typedef GWEN_STRINGLIST wrapped_type;
  typedef std::size_t size_type;
  typedef std::string value_type;

private:
  wrapped_type* m_ptr;
public:

  AB_CXXWRAP_CONSTRUCTOR0(StringList, GWEN_StringList);
  AB_CXXWRAP_CONSTRUCTORS(StringList, GWEN_StringList);
  StringList(const std::vector<std::string>& other);

  AB_CXXWRAP_SET0(clear, GWEN_StringList_Clear);
  size_type AB_CXXWRAP_GET0_CONST(size, GWEN_StringList_Count);
  bool empty() const { return size() == 0; }
  std::string AB_CXXWRAP_GET0_CONST(front, GWEN_StringList_FirstString);
  std::string AB_CXXWRAP_GET1_CONST(at, size_type, GWEN_StringList_StringAt);
  std::string operator[](size_type i) const { return at(i); }
  void push_back(const std::string& s)
  {
    GWEN_StringList_AppendString(m_ptr, s.c_str(), false, false);
  }
  void push_front(const std::string& s)
  {
    GWEN_StringList_InsertString(m_ptr, s.c_str(), false, false);
  }
  std::vector<std::string> toVector() const;

};

} // END namespace AB

#endif // AB_STRINGLIST_HPP
