/***************************************************************************
 $RCSfile: adminjobs.h,v $
                             -------------------
    cvs         : $Id: adminjobs.h,v 1.3 2006/01/13 13:59:58 cstim Exp $
    begin       : Mon Mar 01 2004
    copyright   : (C) 2004 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQEBICS_MSG_ZIP_H
#define AQEBICS_MSG_ZIP_H

#include <aqebics/aqebics.h>

#include <gwenhywfar/buffer.h>


int EB_Zip_Deflate(const char *ptr, unsigned int size, GWEN_BUFFER *buf);
int EB_Zip_Inflate(const char *ptr, unsigned int size, GWEN_BUFFER *buf);



#endif



