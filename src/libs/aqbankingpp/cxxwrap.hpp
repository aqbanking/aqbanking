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


#ifndef AB_CXXWRAP_HPP
#define AB_CXXWRAP_HPP

/**
 * \file
 *
 * This file contains macros that simplify the wrapping of aqbanking's
 * data types in a C++ class. The macros assume the following:
 *
 * - The wrapped C type is available as a typedef "wrapped_type"
 * - The pointer to the wrapped C object is called m_ptr
 *
 * The only additional assumptions are necessary in the
 * AB_CXXWRAP_CONSTRUCTORS() macro.
 */


/** Wraps a getter function with 0 arguments, const */
#define AB_CXXWRAP_GET0_CONST(cxxname, cname)	\
  cxxname() const								\
  { return cname(m_ptr); }

/** Wraps a getter function with 0 arguments */
#define AB_CXXWRAP_GET0(cxxname, cname)			\
  cxxname()										\
  { return cname(m_ptr); }

/** Wraps a getter function with 1 argument, const */
#define AB_CXXWRAP_GET1_CONST(cxxname, type1, cname)	\
  cxxname(type1 arg1) const								\
  { return cname(m_ptr, arg1); }

/** Wraps a getter function with 1 argument */
#define AB_CXXWRAP_GET1(cxxname, type1, cname)	\
  cxxname(type1 arg1)							\
  { return cname(m_ptr, arg1); }

/** Wraps a setter function with 0 argument */
#define AB_CXXWRAP_SET0(cxxname, cname)			\
  void cxxname()								\
  { cname(m_ptr); }

/** Wraps a setter function with 1 argument */
#define AB_CXXWRAP_SET1(cxxname, type1, cname)	\
  void cxxname(type1 arg1)						\
  { cname(m_ptr, arg1); }

/** Wraps the default C++ constructor with zero arguments. This macro
	only works if FOO_new() is available. Some of the FOO_new()
	functions take additional arguments, in which case this macro
	doesn't work. */
#define AB_CXXWRAP_CONSTRUCTOR0(cxxname, cprefix) \
  cxxname()										  \
	: m_ptr(cprefix##_new()) {}

/** Wraps the set of C++ constructors, destructor, and assignment operator.
 *
 * This macro additionally assumes that the C type FOO has a set of
 * constructor/ destructor/ copy functions which are called FOO_free()
 * and FOO_dup(), respectively.
 */
#define AB_CXXWRAP_CONSTRUCTORS(cxxname, cprefix) \
  ~cxxname()									  \
  { cprefix##_free(m_ptr); }					  \
  cxxname(const wrapped_type *other)			  \
	: m_ptr(cprefix##_dup(other)) {}			  \
  cxxname(const cxxname& other)					  \
	: m_ptr(cprefix##_dup(other.m_ptr)) {}		  \
  cxxname& operator=(const cxxname& other)		  \
  {												  \
	if (&other == this)							  \
	  return *this;								  \
	cprefix##_free(m_ptr);						  \
	m_ptr = cprefix##_dup(other.m_ptr);			  \
	return *this;								  \
  }												  \
  operator const wrapped_type*() const			  \
  { return m_ptr; }								  \
  operator wrapped_type*()						  \
  { return m_ptr; }								  \
  const wrapped_type* ptr() const				  \
  { return m_ptr; }								  \
  wrapped_type* ptr()							  \
  { return m_ptr; }


#endif // AB_CXXWRAP_HPP

