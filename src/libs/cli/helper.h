/***************************************************************************
 begin       : Fri Nov 14 2025
 copyright   : (C) 2026 by Ralf Habacker
 email       : ralf.habacker@freenet.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef AQCLI_HELPER_H
#define AQCLI_HELPER_H

#include <aqbanking/error.h>

#include <gwenhywfar/db.h>
#include <gwenhywfar/args.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Processes arguments for individual commands
 * Upon request, the list of available arguments is output,
 * or a message is output in case of an error.
 */
int AB_Cmd_Handle_Args(int argc, char **argv, const GWEN_ARGS *args, GWEN_DB_NODE *db);

/**
 * Processes arguments for a command line application
 * Upon request, the version and syntax of the command line and a list of available
 * arguments are output, or a message is output in case of an error.
 */
int AB_App_Handle_Args(int argc, char **argv, const GWEN_ARGS *args, GWEN_DB_NODE *db);

#ifdef __cplusplus
}
#endif

#endif // AQCLI_HELPER_H
