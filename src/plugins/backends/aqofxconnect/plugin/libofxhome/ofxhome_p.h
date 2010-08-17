/***************************************************************************
    begin       : Tue Aug 17 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/


#ifndef OFXHOME_P_H
#define OFXHOME_P_H


#include "ofxhome.h"

#include <gwenhywfar/httpsession.h>



struct OFXHOME {
  OH_INSTITUTE_SPEC_LIST *specList;
  OH_INSTITUTE_DATA_LIST *dataList;

  char *dataFolder;
};



int OfxHome_SetupHttpSession(OFXHOME *ofh, GWEN_HTTP_SESSION *sess);

int OfxHome_DownloadSpecs(OFXHOME *ofh, OH_INSTITUTE_SPEC_LIST *sl);
int OfxHome_LoadSpecs(OFXHOME *ofh, OH_INSTITUTE_SPEC_LIST *sl);
int OfxHome_SaveSpecs(OFXHOME *ofh, const OH_INSTITUTE_SPEC_LIST *sl);

/**
 * @param ofh pointer to OFXHOME object
 * @param hours cache validity time in hours
 * @return -1 if data is not in cache
 *          0 if data in cache but outdated,
 *          1 if valid data in cache
 */
int OfxHome_CheckSpecsCache(OFXHOME *ofh, int hours);


int OfxHome_DownloadData(OFXHOME *ofh, int fid, OH_INSTITUTE_DATA **pData);
int OfxHome_LoadData(OFXHOME *ofh, int fid, OH_INSTITUTE_DATA **pData);
int OfxHome_SaveData(OFXHOME *ofh, const OH_INSTITUTE_DATA *od);
int OfxHome_CheckDataCache(OFXHOME *ofh, int fid, int hours);


#endif



