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


#ifndef AH_MEDIUMHEADER_P_H
#define AH_MEDIUMHEADER_P_H

#define AH_MEDIUMHEADER_MAGIC "#AHM"

#include "mediumheader_l.h"


struct AH_MEDIUMHEADER {
  char *typName;
  GWEN_TYPE_UINT32 vmajor;
  GWEN_TYPE_UINT32 vminor;
};


#endif /* AH_MEDIUMHEADER_P_H */


