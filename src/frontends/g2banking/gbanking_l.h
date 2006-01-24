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



#ifndef GBANKING_L_H
#define GBANKING_L_H

#include "gbanking.h"

#include <glade/glade-xml.h>


GladeXML *GBanking_GladeXml_new(AB_BANKING *ab,
                                const char *relFname,
                                const char *wname);



#endif /* GBANKING_L_H */









