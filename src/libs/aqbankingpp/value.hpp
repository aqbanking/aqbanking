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

  Value()
	: m_ptr(AB_Value_new())
  {}
  Value(const wrapped_type *ov)
	: m_ptr(AB_Value_dup(ov))
  {}
  Value(const Value& ov)
	: m_ptr(AB_Value_dup(ov.ptr()))
  {}
  Value(double d)
	: m_ptr(AB_Value_fromDouble(d))
  {}
  Value(long int num, long int denom)
	: m_ptr(AB_Value_fromInt(num, denom))
  {}
  Value& operator=(const Value& ov)
  {
	if (&ov == this)
	  return *this;
	AB_Value_free(m_ptr);
	m_ptr = AB_Value_dup(ov.ptr());
	return *this;
  }
  ~Value()
  {
	AB_Value_free(m_ptr);
  }
  operator const wrapped_type*() const
  {
	return m_ptr;
  }
  const wrapped_type* ptr() const
  {
	return m_ptr;
  }
  long int getNum() const { return AB_Value_Num(m_ptr); }
  long int getDenom() const { return AB_Value_Denom(m_ptr); }

  void toString(GWEN_BUFFER *buf) const
  {
	AB_Value_toString(m_ptr, buf);
  }

  /**
   * This function returns the value as a double.
   * You should not feed another AB_VALUE from this double, because the
   * conversion from an AB_VALUE to a double might be lossy!
   */
  double getValueAsDouble() const
  {
	return AB_Value_GetValueAsDouble(m_ptr);
  }
  void setValueFromDouble(double d)
  {
	AB_Value_SetValueFromDouble(m_ptr, d);
  }
  void setZero()
  {
	AB_Value_SetZero(m_ptr);
  }
  bool isZero() const
  {
	return AB_Value_IsZero(m_ptr);
  }
  bool isNegative() const
  {
	return AB_Value_IsNegative(m_ptr);
  }
  bool isPositive() const
  {
	return AB_Value_IsPositive(m_ptr);
  }
  int compare(const Value& other) const
  {
	return AB_Value_Compare(m_ptr, other.m_ptr);
  }
  bool equal(const Value& other) const
  {
	return AB_Value_Equal(m_ptr, other.m_ptr);
  }
  int addValue(const Value& other)
  {
	return AB_Value_AddValue(m_ptr, other);
  }
  int subValue(const Value& other)
  {
	return AB_Value_SubValue(m_ptr, other);
  }
  int multValue(const Value& other)
  {
	return AB_Value_MultValue(m_ptr, other);
  }
  int divValue(const Value& other)
  {
	return AB_Value_DivValue(m_ptr, other);
  }
  int negate(const Value& other)
  {
	return AB_Value_Negate(m_ptr);
  }
  const char* getCurrency() const
  {
	return AB_Value_GetCurrency(m_ptr);
  }
  void setCurrency(const std::string& s)
  {
	AB_Value_SetCurrency(m_ptr, s.c_str());
  }

  static Value fromString(const std::string& s)
  {
	return Value(AB_Value_fromString(s.c_str()));
  }
private:
  wrapped_type* m_ptr;
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

