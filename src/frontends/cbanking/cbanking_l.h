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


#ifndef CBANKING_CBANKING_L_H
#define CBANKING_CBANKING_L_H

#include <gwenhywfar/buffer.h>

#include "cbanking.h"


void CBanking_GetRawText(AB_BANKING *ab,
                         const char *text,
                         GWEN_BUFFER *tbuf);




#endif

