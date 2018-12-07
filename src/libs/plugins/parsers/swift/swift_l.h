/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004,2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT_l_H
#define AQHBCIBANK_SWIFT_l_H

#include <gwenhywfar/misc.h>
#include <gwenhywfar/dbio.h>


#define AHB_SWIFT_MAXLINELEN 512


typedef struct AHB_SWIFT_TAG AHB_SWIFT_TAG;
typedef struct AHB_SWIFT_SUBTAG AHB_SWIFT_SUBTAG;


GWEN_LIST_FUNCTION_DEFS(AHB_SWIFT_TAG, AHB_SWIFT_Tag);


const char *AHB_SWIFT_Tag_GetId(const AHB_SWIFT_TAG *tg);
const char *AHB_SWIFT_Tag_GetData(const AHB_SWIFT_TAG *tg);


GWEN_LIST_FUNCTION_DEFS(AHB_SWIFT_SUBTAG, AHB_SWIFT_SubTag);


AHB_SWIFT_SUBTAG *AHB_SWIFT_SubTag_new(int id, const char *content, int clen);
void AHB_SWIFT_SubTag_free(AHB_SWIFT_SUBTAG *stg);
int AHB_SWIFT_SubTag_GetId(const AHB_SWIFT_SUBTAG *stg);
const char *AHB_SWIFT_SubTag_GetData(const AHB_SWIFT_SUBTAG *stg);
AHB_SWIFT_SUBTAG *AHB_SWIFT_FindSubTagById(const AHB_SWIFT_SUBTAG_LIST *stlist, int id);

void AHB_SWIFT_SubTag_Condense(AHB_SWIFT_SUBTAG *stg, int keepMultipleBlanks);

int AHB_SWIFT_GetNextSubTag(const char **sptr, AHB_SWIFT_SUBTAG **tptr);
int AHB_SWIFT_ParseSubTags(const char *s, AHB_SWIFT_SUBTAG_LIST *stlist, int keepMultipleBlanks);


int AHB_SWIFT_Condense(char *buffer, int keepDoubleBlanks);


#endif /* AQHBCIBANK_SWIFT_L_H */



