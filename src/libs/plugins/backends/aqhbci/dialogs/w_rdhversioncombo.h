/***************************************************************************
 begin       : Wed May 01 2024
 copyright   : (C) 2024 by Martin Preuss
 email       : martin@aqbanking.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/

#ifndef AQHBCI_WIDGET_RDHVERSIONCOMBO_H
#define AQHBCI_WIDGET_RDHVERSIONCOMBO_H


#include <gwenhywfar/dialog.h>


void AH_Widget_RdhVersionComboSetup(GWEN_DIALOG *dlg, const char *widgetName);
void AH_Widget_RdhVersionComboSetCurrent(GWEN_DIALOG *dlg, const char *widgetName, int v);

/**
 * Return selected RDH/RAH version (bits 0-7) and crypt mode (bit 8-15)
 */
int AH_Widget_RdhVersionComboGetCurrent(GWEN_DIALOG *dlg, const char *widgetName);



#endif
