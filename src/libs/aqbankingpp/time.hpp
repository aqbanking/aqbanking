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


#ifndef AB_TIME_HPP
#define AB_TIME_HPP

#include <gwenhywfar/gwentime.h>
#include <aqbankingpp/cxxwrap.hpp>
  
namespace AB
{

/** A wrapper class around the GWEN_TIME type */
class Time
{
public:
  typedef GWEN_TIME wrapped_type;
private:
  wrapped_type* m_ptr;
public:

  Time(int year,
	   int month,
	   int day,
	   int hour,
	   int min,
	   int sec,
	   int inUtc)
	: m_ptr(GWEN_Time_new(year, month, day,
						  hour, min, sec, inUtc))
  {}
  AB_CXXWRAP_CONSTRUCTORS(Time, GWEN_Time);

  uint32_t AB_CXXWRAP_GET0_CONST(seconds, GWEN_Time_Seconds);
  double AB_CXXWRAP_GET0_CONST(milliseconds, GWEN_Time_Milliseconds);
  struct tm AB_CXXWRAP_GET0_CONST(toTm, GWEN_Time_toTm);
  time_t AB_CXXWRAP_GET0_CONST(toTime_t, GWEN_Time_toTime_t);

  /**
   * Returns the broken down date as local date.
   */
  int getBrokenDownDate(int& day, int& month, int& year)
  {
	return GWEN_Time_GetBrokenDownDate(m_ptr, &day, &month, &year);
  }
  /**
   * Returns the broken down time as UTC date (Greenwhich Mean time).
   */
  int getBrokenDownUtcDate(int& day, int& month, int& year)
  {
	return GWEN_Time_GetBrokenDownUtcDate(m_ptr, &day, &month, &year);
  }

  static Time currentTime()
  {
	return GWEN_CurrentTime();
  }
};

} // END namespace AB

#endif // AB_TIME_HPP
