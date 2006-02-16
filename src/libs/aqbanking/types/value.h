/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Apr 05 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQBANKING_VALUE_H
#define AQBANKING_VALUE_H

#include <gwenhywfar/db.h>
#include <aqbanking/error.h> /* for AQBANKING_API */

#ifdef __cplusplus
extern "C" {
#endif

/** An abstract monetary value in HBCI. It has an amount and a
    currency string. 

    Right now the amount is stored as a "double" floating-point value,
    but in the future this might be changed to a fixed-point
    representation to avoid rounding errors in financial
    calculations. Therefore some basic arithmetic operations are
    already supported, and more are likely to come. */
typedef struct AB_VALUE AB_VALUE;

/** @name Constructor/Destructor */
/*@{*/
/** Create a new value with the given amount and the given ISO-4217
 * currency. (Constructor)
 *
 * @param value The amount
 *
 * @param currency The currency as ISO-4217 string, e.g. "EUR" for
 * Euro; may be NULL
*/
AQBANKING_API 
AB_VALUE *AB_Value_new(double value, const char *currency);

/** Create a duplicate of the given value (Copy constructor). */
AQBANKING_API 
AB_VALUE *AB_Value_dup(const AB_VALUE *v);

/** Create a value from the given string. FIXME: describe string
    format here. */
AQBANKING_API 
AB_VALUE *AB_Value_fromString(const char *s);

/** Create a value from the given GWEN_DB. */
AQBANKING_API 
AB_VALUE *AB_Value_fromDb(GWEN_DB_NODE *db);

/** Write the given value into the given GWEN_DB. */
AQBANKING_API 
int AB_Value_toDb(const AB_VALUE *v, GWEN_DB_NODE *db);

/** Free the given value (Destructor). */
AQBANKING_API 
void AB_Value_free(AB_VALUE *v);
/*@}*/


/** @name Getters/Setters */
/*@{*/
/** Returns the value part as a double. */
AQBANKING_API 
double AB_Value_GetValue(const AB_VALUE *v);

/** Set the value of this AB_VALUE from a double. */
AQBANKING_API 
void AB_Value_SetValue(AB_VALUE *v, double d);

/** Returns the ISO-4217 currency string of the given value,
    e.g. "EUR" for Euro. WATCH OUT: The currency string may be
    NULL!  */
AQBANKING_API 
const char *AB_Value_GetCurrency(const AB_VALUE *v);

/** Set the ISO-4217 currency string of this value, e.g. "EUR" for
    Euro. The currency string may be NULL. */
AQBANKING_API 
void AB_Value_SetCurrency(AB_VALUE *v, const char *s);
/*@}*/


/** @name Predicates/Comparisons */
/*@{*/
/** Predicate: Returns nonzero (TRUE) if the given value is
    valid. FIXME: Describe what "valid" means. */
AQBANKING_API 
int AB_Value_IsValid(const AB_VALUE *v);

/** Predicate: Returns nonzero (TRUE) if the value is negative (lesser
    than zero), or zero (FALSE) otherwise. */
AQBANKING_API 
int AB_Value_IsNegative(const AB_VALUE *v);

/** Predicate: Returns nonzero (TRUE) if the value is positive
    (greater or equal to zero), or zero (FALSE) otherwise. */
AQBANKING_API 
int AB_Value_IsPositive(const AB_VALUE *v);

/** Predicate: Returns nonzero (TRUE) if the value is equal to zero,
    or zero (FALSE) otherwise. */
AQBANKING_API 
int AB_Value_IsZero(const AB_VALUE *v);

/** Equality predicate: Returns nonzero (TRUE) if v1 is equal to v2,
    or zero (FALSE) otherwise. (Note: Right now this function ignores
    the currency field.) */
AQBANKING_API 
int AB_Value_IsEqual(const AB_VALUE  *v1, const AB_VALUE *v2);

/** Compare function: Returns +1 if v1>v2, zero if v1 == v2, and -1 if
    v1<v2. (Note: Right now this function ignores the currency
    field.) */
AQBANKING_API 
int AB_Value_Compare(const AB_VALUE  *v1, const AB_VALUE *v2);
/*@}*/


/** @name Arithmetic operations */
/*@{*/
/** Add the value @c vToAdd to the given value <i>v</i>, i.e. assign
    v=v+vToAdd. Returns zero on success, or -1 if any input argument
    is invalid. (Note: Right now this function ignores the currency
    field.) */
AQBANKING_API
int AB_Value_AddValue(AB_VALUE  *v, const AB_VALUE *vToAdd);

/** Subtract the value @c vToSub from the given value <i>v</i>,
    i.e. assign v=v-vToSub. Returns zero on succes, or -1 if any
    input argument is invalid. (Note: Right now this function ignores
    the currency field.) */
AQBANKING_API 
int AB_Value_SubValue(AB_VALUE  *v, const AB_VALUE *vToSub);

/** Negate the sign of the given value, i.e. assign v=-v. Returns zero
    on succes, or -1 if any input argument is invalid. */
AQBANKING_API 
int AB_Value_Negate(AB_VALUE *v);
/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* AQBANKING_VALUE_H */


