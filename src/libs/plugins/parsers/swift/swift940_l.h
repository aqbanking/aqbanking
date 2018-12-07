/***************************************************************************
 begin       : Fri Apr 02 2004
 copyright   : (C) 2004-2010 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef AQHBCIBANK_SWIFT940_L_H
#define AQHBCIBANK_SWIFT940_L_H

#include "swift_l.h"


int AHB_SWIFT940_Import(AHB_SWIFT_TAG_LIST *tl,
			GWEN_DB_NODE *data,
			GWEN_DB_NODE *cfg,
			uint32_t flags);


#endif /* AQHBCIBANK_SWIFT940_P_H */

