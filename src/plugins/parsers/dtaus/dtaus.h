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

#ifndef AQHBCIBANK_DTAUS_H
#define AQHBCIBANK_DTAUS_H

/**
 * @defgroup MOD_PLUGIN_DTAUS DTAUS Parser
 * @ingroup MOD_PLUGINS
 *
 * <p>
 * This DBIO plugin allows parsing and creating of DTAUS records.
 * </p>
 * <p>
 * Every DBIO plugin uses a GWEN_DB which contains the configuration for
 * the given plugin. For this plugin the configuration is as follows:
 * </p>
 * <table>
 *
 * <tr>
 *   <td><b>Name</b></td>
 *   <td><b>Type</b></td>
 *   <td><b>Content</b></td>
 *   <td><b>Mode</b></td>
 * </tr>
 *
 * <tr>
 *   <td><b>type</b></td>
 *   <td>char</td>
 *   <td><i>transfer</i>, <i>debitnote</i>: type of transactions</td>
 *   <td>required</td>
 * </tr>
 *
 * <tr>
 *   <td><b>bankCode</b></td>
 *   <td>char</td>
 *   <td>bank code of the executing credit institute</td>
 *   <td>required</td>
 * </tr>
 *
 * <tr>
 *   <td><b>accountId</b></td>
 *   <td>char</td>
 *   <td>id of the account to be used for the transactions</td>
 *   <td>required</td>
 * </tr>
 *
 * <tr>
 *   <td><b>currency</b></td>
 *   <td>char</td>
 *   <td><i>EUR</i>, <i>DEM</i>: currency of all transactions</td>
 *   <td>required</td>
 * </tr>
 *
 * <tr>
 *   <td><b>execDate</b></td>
 *   <td>char</td>
 *   <td>date of execution for the transactions involved</td>
 *   <td>optional</td>
 * </tr>
 *
 * <tr>
 *   <td><b>custRef</b></td>
 *   <td>char</td>
 *   <td>customer reference</td>
 *   <td>optional</td>
 * </tr>
 *
 * </table>
 *
 */



#endif /* AQHBCIBANK_DTAUS_H */

