/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : Mon March 1 2011
    copyright   : (C) 2011 by Christian Stimming
    email       : christian@cstimming.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AB_VALUE_HPP
#define AB_VALUE_HPP

#include <aqbanking/value.h>
#include <aqbankingpp/cxxwrap.hpp>
#include <string>
  
namespace AB
{

/** A wrapper class around the AB_VALUE rational number type */
class /*AQBANKING_API*/ Value
// note: AQBANKING_API isn't needed as long as this class is defined
// purely in the header.
{
public:
  typedef AB_VALUE wrapped_type;
private:
  wrapped_type* m_ptr;
public:

  AB_CXXWRAP_CONSTRUCTOR0(Value, AB_Value);
  AB_CXXWRAP_CONSTRUCTORS(Value, AB_Value);

  /** Extra constructor: Create this value from a double. \see
      AB_Value_fromDouble() */
  Value(double d)
	: m_ptr(AB_Value_fromDouble(d))
  {}

  /** Extra constructor: Create this value from two integer values for
      numerator and denominator. \see AB_Value_fromInt() */
  Value(long int num, long int denom)
	: m_ptr(AB_Value_fromInt(num, denom))
  {}

  /** Conversion to string. \see AB_Value_toString() */
  void toString(GWEN_BUFFER *buf) const
  {
	AB_Value_toString(m_ptr, buf);
  }

  /** Conversion to string. \see AB_Value_toString() */
  std::string toString() const
  {
	GWEN_BUFFER *buf = GWEN_Buffer_new(NULL, 100, 0, 0);
	toString(buf);
	std::string result(GWEN_Buffer_GetStart(buf));
	GWEN_Buffer_free(buf);
	return result;
  }


  long int AB_CXXWRAP_GET0_CONST(getNum, AB_Value_Num);
  long int AB_CXXWRAP_GET0_CONST(getDenom, AB_Value_Denom);
  double AB_CXXWRAP_GET0_CONST(getValueAsDouble, AB_Value_GetValueAsDouble);

  AB_CXXWRAP_SET1(setValueFromDouble, double, AB_Value_SetValueFromDouble);
  AB_CXXWRAP_SET0(setZero, AB_Value_SetZero);

  bool AB_CXXWRAP_GET0_CONST(isZero, AB_Value_IsZero);
  bool AB_CXXWRAP_GET0_CONST(isNegative, AB_Value_IsNegative);
  bool AB_CXXWRAP_GET0_CONST(isPositive, AB_Value_IsPositive);

  int AB_CXXWRAP_GET1_CONST(compare, const Value&, AB_Value_Compare);
  bool AB_CXXWRAP_GET1_CONST(equal, const Value&, AB_Value_Equal);

  int AB_CXXWRAP_GET1(addValue, const Value&, AB_Value_AddValue);
  int AB_CXXWRAP_GET1(subValue, const Value&, AB_Value_SubValue);
  int AB_CXXWRAP_GET1(multValue, const Value&, AB_Value_MultValue);
  int AB_CXXWRAP_GET1(divValue, const Value&, AB_Value_DivValue);

  int AB_CXXWRAP_GET0(negate, AB_Value_Negate);

  std::string AB_CXXWRAP_GET0_CONST(getCurrency, AB_Value_GetCurrency);
  void setCurrency(const std::string& s)
  {
	AB_Value_SetCurrency(m_ptr, s.c_str());
  }

  /** Conversion from string. \see AB_Value_fromString() */
  static Value fromString(const std::string& s)
  {
	return Value(AB_Value_fromString(s.c_str()));
  }
};

bool operator==(const Value& v1, const Value& v2)
{
  return v1.equal(v2);
}
bool operator!=(const Value& v1, const Value& v2)
{
  return !(v1 == v2);
}
bool operator>(const Value& v1, const Value& v2)
{
  return v1.compare(v2) > 0;
}
bool operator<(const Value& v1, const Value& v2)
{
  return v1.compare(v2) < 0;
}

} // END namespace AB


#endif // AB_VALUE_HPP

