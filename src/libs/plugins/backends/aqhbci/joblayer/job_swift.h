/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AH_JOB_SWIFT_H
#define AH_JOB_SWIFT_H

#include "aqhbci/joblayer/job_l.h"

#include <aqbanking/backendsupport/swiftdescr.h>



/**
 * Return SWIFT descriptors which are supported by both the job's BPD/UPD and the XML importer.
 *
 * @return list of supported descriptiors
 * @param j job object
 * @param paramDbGroupName db group within the job params db (below @ref AH_Job_GetParams)
 * @param paramDbVarName db variable within the job params db (below @ref AH_Job_GetParams)
 * @param family document family (e.g. "camt", "pain")
 * @param version1 major version (as "001" in pain.001.002.03). Please don't use leading zeros here!!
 */
AB_SWIFT_DESCR_LIST *AH_Job_GetSwiftDescriptorsSupportedByJob(AH_JOB *j,
                                                              const char *paramDbGroupName,
                                                              const char *paramDbVarName,
                                                              const char *family,
                                                              int version1);

/**
 * Return SWIFT descriptors which are supported by both the user and the XML importer.
 *
 * @return list of supported descriptiors
 * @param j job object
 * @param family document family (e.g. "camt", "pain")
 * @param version1 major version (as "001" in pain.001.002.03). Please don't use leading zeros here!!
 */
AB_SWIFT_DESCR_LIST *AH_Job_GetSwiftDescriptorsSupportedByUser(AH_JOB *j, const char *family, int version1);




#endif




