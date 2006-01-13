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


#ifndef AH_HBCI_UPDATES_L_H
#define AH_HBCI_UPDATES_L_H

#include "hbci_l.h"
#include <gwenhywfar/types.h>


int AH_HBCI_UpdateDb(AH_HBCI *hbci, GWEN_DB_NODE *db);

int AH_HBCI_Update2(AH_HBCI *hbci,
                    GWEN_DB_NODE *db,
                    GWEN_TYPE_UINT32 oldVersion,
                    GWEN_TYPE_UINT32 currentVersion);



#endif

