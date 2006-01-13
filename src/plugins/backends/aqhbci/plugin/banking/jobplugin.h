/***************************************************************************
 $RCSfile$
                             -------------------
    cvs         : $Id$
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AH_JOBPLUGIN_H
#define AH_JOBPLUGIN_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/inherit.h>
#include <gwenhywfar/libloader.h>
#include <gwenhywfar/buffer.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct AH_JOBPLUGIN AH_JOBPLUGIN;
#ifdef __cplusplus
}
#endif


#include <aqbanking/banking.h>
#include <aqbanking/job.h>
#include <aqhbci/aqhbci.h>
#include <aqhbci/hbci.h>
#include <aqhbci/job.h>
#include <aqhbci/provider.h>



#ifdef __cplusplus
extern "C" {
#endif





GWEN_LIST_FUNCTION_LIB_DEFS(AH_JOBPLUGIN, AH_JobPlugin, AQHBCI_API);
GWEN_INHERIT_FUNCTION_LIB_DEFS(AH_JOBPLUGIN, AQHBCI_API);


/** @name Prototypes For Virtual Functions
 *
 * The functions in this group are wrappers which in most cases directly
 * call the implementations of the functions.
 */
/*@{*/

typedef AH_JOBPLUGIN* (*AH_JOBPLUGIN_NEWFN)(AH_PROVIDER *pro);

typedef AH_JOB*
  (*AH_JOBPLUGIN_FACTORYFN)(AH_JOBPLUGIN *jp,
                            AB_JOB_TYPE jt,
                            AB_USER *u,
                            AB_ACCOUNT *a);

typedef int
  (*AH_JOBPLUGIN_CHECKFN)(AH_JOBPLUGIN *jp,
                          AB_JOB_TYPE jt);

/*@}*/


AQHBCI_API AH_JOBPLUGIN *AH_JobPlugin_new(AH_PROVIDER *pro,
                                          const char *name);

AQHBCI_API void AH_JobPlugin_free(AH_JOBPLUGIN *jp);


AQHBCI_API GWEN_LIBLOADER *AH_JobPlugin_GetLibLoader(const AH_JOBPLUGIN *jp);

AQHBCI_API
  void AH_JobPlugin_SetLibLoader(AH_JOBPLUGIN *jp, GWEN_LIBLOADER *ll);

AQHBCI_API AH_PROVIDER *AH_JobPlugin_GetProvider(const AH_JOBPLUGIN *jp);


/** @name Virtual Functions
 *
 * The functions in this group are wrappers which in most cases directly
 * call the implementations of the functions.
 */
/*@{*/
AQHBCI_API AH_JOB *AH_JobPlugin_Factory(AH_JOBPLUGIN *jp,
                                        AB_JOB_TYPE jt,
                                        AB_USER *u,
                                        AB_ACCOUNT *a);

AQHBCI_API int AH_JobPlugin_CheckType(AH_JOBPLUGIN *jp,
                                      AB_JOB_TYPE jt);

AQHBCI_API const char *AH_JobPlugin_GetName(const AH_JOBPLUGIN *jp);

AQHBCI_API const char *AH_JobPlugin_GetDescription(const AH_JOBPLUGIN *jp);
AQHBCI_API void AH_JobPlugin_SetDescription(AH_JOBPLUGIN *jp, const char *s);


/** @name Setters For Virtual Functions
 *
 */
/*@{*/
AQHBCI_API
  void AH_JobPlugin_SetFactoryFn(AH_JOBPLUGIN *jp, AH_JOBPLUGIN_FACTORYFN f);

AQHBCI_API
  void AH_JobPlugin_SetCheckFn(AH_JOBPLUGIN *jp, AH_JOBPLUGIN_CHECKFN f);
/*@}*/ /* defgroup */



#ifdef __cplusplus
}
#endif

#endif /* AH_JOBPLUGIN_H */



