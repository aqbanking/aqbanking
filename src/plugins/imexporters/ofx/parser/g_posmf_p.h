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


#ifndef AIO_OFX_G_POSMF_P_H
#define AIO_OFX_G_POSMF_P_H


#include "g_posmf_l.h"


static int AIO_OfxGroup_POSMF_StartTag(AIO_OFX_GROUP *g,
				       const char *tagName);

static int AIO_OfxGroup_POSMF_EndSubGroup(AIO_OFX_GROUP *g,
					  AIO_OFX_GROUP *sg);

#endif

