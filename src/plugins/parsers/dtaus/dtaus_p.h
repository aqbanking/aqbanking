/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Apr 29 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQHBCIBANK_DTAUS_P_H
#define AQHBCIBANK_DTAUS_P_H

#include <gwenhywfar/dbio.h>


GWEN_DBIO *dtaus_factory();


GWEN_DBIO_CHECKFILE_RESULT AHB_DTAUS__ReallyCheckFile(GWEN_BUFFER *src,
                                                      unsigned int pos);
GWEN_DBIO_CHECKFILE_RESULT AHB_DTAUS__CheckFile(GWEN_DBIO *dbio,
                                                const char *fname);



#endif /* AQHBCIBANK_DTAUS_P_H */


