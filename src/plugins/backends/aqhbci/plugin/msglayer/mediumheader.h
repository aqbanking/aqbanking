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


#ifndef AH_MEDIUMHEADER_H
#define AH_MEDIUMHEADER_H

#include <gwenhywfar/buffer.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct AH_MEDIUMHEADER AH_MEDIUMHEADER;

#define AH_MEDIUMHEADER_SIZE 128


AH_MEDIUMHEADER *AH_MediumHeader_new(const char *mtype,
                                     GWEN_TYPE_UINT32 vmajor,
                                     GWEN_TYPE_UINT32 vminor);

void AH_MediumHeader_free(AH_MEDIUMHEADER *mh);


AH_MEDIUMHEADER *AH_MediumHeader_FromBuffer(GWEN_BUFFER *hbuf);

void AH_MediumHeader_ToBuffer(AH_MEDIUMHEADER *mh, GWEN_BUFFER *hbuf);


const char *AH_MediumHeader_GetMediumType(const AH_MEDIUMHEADER *mh);
GWEN_TYPE_UINT32 AH_MediumHeader_GetMajorVersion(const AH_MEDIUMHEADER *mh);
GWEN_TYPE_UINT32 AH_MediumHeader_GetMinorVersion(const AH_MEDIUMHEADER *mh);



#ifdef __cplusplus
}
#endif



#endif /* AH_MEDIUMHEADER_H */
