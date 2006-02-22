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

#ifndef AH_BPD_P_H
#define AH_BPD_P_H

#include "bpd_l.h"


#define AH_BPD_MAXHBCIVERSIONS 16
#define AH_BPD_MAXLANGUAGES    16


struct AH_BPD {
  int bpdVersion;
  GWEN_DB_NODE *bpdJobs;

  char *bankName;
  char *bankAddr;
  int bankPort;
  AH_BPD_ADDR_TYPE addrType;

  int jobTypesPerMsg;
  int maxMsgSize;

  int hbciVersions[AH_BPD_MAXHBCIVERSIONS+1];
  int languages[AH_BPD_MAXLANGUAGES+1];

  int isDirty;

  AH_BPD_ADDR_LIST *addrList;
};


struct AH_BPD_ADDR {
  GWEN_LIST_ELEMENT(AH_BPD_ADDR)
  AH_BPD_ADDR_TYPE type;
  char *addr;
  char *suffix;
  AH_BPD_FILTER_TYPE ftype;
  int fversion;
};



#endif /* AH_BPD_P_H */



