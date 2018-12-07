/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_POSSTOCK_P_H
#define AIO_OFX_G_POSSTOCK_P_H


#include "g_posstock_l.h"


static int AIO_OfxGroup_POSSTOCK_StartTag(AIO_OFX_GROUP *g,
					  const char *tagName);

static int AIO_OfxGroup_POSSTOCK_EndSubGroup(AIO_OFX_GROUP *g,
					     AIO_OFX_GROUP *sg);

#endif

