/***************************************************************************
 $RCSfile$
 -------------------
 begin       : Mon Jan 07 2008
 copyright   : (C) 2008 by Martin Preuss
 email       : martin@libchipcard.de
 copyright   : (C) 2013 by Paul Conrady
 email       : c.p.conrady@gmail.com

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AIO_OFX_G_MFINFO_P_H
#define AIO_OFX_G_MFINFO_P_H


#include "g_mfinfo_l.h"


int AIO_OfxGroup_MFINFO_StartTag(AIO_OFX_GROUP *g, const char *tagName);
int AIO_OfxGroup_MFINFO_EndSubGroup(AIO_OFX_GROUP *g, AIO_OFX_GROUP *sg);

#endif


