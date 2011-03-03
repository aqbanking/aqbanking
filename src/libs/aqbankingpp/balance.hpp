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


#ifndef AB_BALANCE_HPP
#define AB_BALANCE_HPP

#include <aqbanking/balance.h>

#include <aqbankingpp/cxxwrap.hpp>
#include <aqbankingpp/time.hpp>
#include <aqbankingpp/value.hpp>
  
namespace AB
{

/** A wrapper class around the \ref AB_BALANCE type */
class Balance
{
public:
  typedef AB_BALANCE wrapped_type;
private:
  wrapped_type* m_ptr;
public:

  Balance(const Value& v, const Time& t)
	: m_ptr(AB_Balance_new(v, t))
  {}
  AB_CXXWRAP_CONSTRUCTORS(Balance, AB_Balance);
  Value AB_CXXWRAP_GET0_CONST(getValue, AB_Balance_GetValue);
  AB_CXXWRAP_SET1(setValue, const Value&, AB_Balance_SetValue);
  const GWEN_TIME *AB_CXXWRAP_GET0_CONST(getTime, AB_Balance_GetTime);
};

} // END namespace AB

#endif // AB_BALANCE_HPP
