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


#ifndef AB_ACCOUNTSTATUS_HPP
#define AB_ACCOUNTSTATUS_HPP

#include <aqbanking/accstatus.h>

#include <aqbankingpp/cxxwrap.hpp>
#include <aqbankingpp/balance.hpp>
#include <aqbankingpp/time.hpp>
#include <aqbankingpp/value.hpp>
  
namespace AB
{

/** A wrapper class around the AB_ACCOUNT_STATUS type */
class AccountStatus
{
public:
  typedef AB_ACCOUNT_STATUS wrapped_type;
private:
  wrapped_type* m_ptr;
public:

  AB_CXXWRAP_CONSTRUCTOR0(AccountStatus, AB_AccountStatus);
  AB_CXXWRAP_CONSTRUCTORS(AccountStatus, AB_AccountStatus);

  Time AB_CXXWRAP_GET0_CONST(getTime, AB_AccountStatus_GetTime);
  Value AB_CXXWRAP_GET0_CONST(getBankLine, AB_AccountStatus_GetBankLine);
  Value AB_CXXWRAP_GET0_CONST(getDisposable, AB_AccountStatus_GetDisposable);
  Value AB_CXXWRAP_GET0_CONST(getDisposed, AB_AccountStatus_GetDisposed);
  Balance AB_CXXWRAP_GET0_CONST(getBookedBalance, AB_AccountStatus_GetBookedBalance);
  Balance AB_CXXWRAP_GET0_CONST(getNotedBalance, AB_AccountStatus_GetNotedBalance);
};

} // END namespace AB

#endif // AB_ACCOUNTSTATUS_HPP
