/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_ACCTINFORS_P_H
#define AIO_OFX_G_ACCTINFORS_P_H


#include "g_acctinfors_l.h"


int AIO_OfxGroup_ACCTINFORS_StartTag(AIO_OFX_GROUP *g, const char *tagName);
int AIO_OfxGroup_ACCTINFORS_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);

#endif


