/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_GENERIC_P_H
#define AIO_OFX_G_GENERIC_P_H


#include "g_generic_l.h"


static int AIO_OfxGroup_Generic_EndTag(AIO_OFX_GROUP *g, const char *tagName);
static int AIO_OfxGroup_Generic_AddData(AIO_OFX_GROUP *g, const char *data);
static int AIO_OfxGroup_Generic_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);

#endif

