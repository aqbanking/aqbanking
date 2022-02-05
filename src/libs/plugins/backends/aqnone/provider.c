/***************************************************************************
    begin       : Mon Mar 01 2004
    copyright   : (C) 2018 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "provider_p.h"



AB_PROVIDER *AN_Provider_new(AB_BANKING *ab)
{
  AB_PROVIDER *pro;

  pro=AB_Provider_new(ab, "aqnone");
  AB_Provider_SetInitFn(pro, AN_Provider_Init);
  AB_Provider_SetFiniFn(pro, AN_Provider_Fini);

  return pro;
}



int AN_Provider_Init(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  return 0;
}



int AN_Provider_Fini(AB_PROVIDER *pro, GWEN_DB_NODE *dbData)
{
  return 0;
}



