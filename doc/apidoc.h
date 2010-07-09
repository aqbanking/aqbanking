

/** @defgroup G_AB_LIBRARY AqBanking Library */

/** @defgroup G_AB_C_INTERFACE API for Applications
 *  @ingroup G_AB_LIBRARY
 */

/** @defgroup G_AB_BANKING Main Interface
 *  @ingroup G_AB_C_INTERFACE
 */

/** @defgroup G_AB_INFO Bank/Country Info
 *  @ingroup G_AB_C_INTERFACE
 */

/** @defgroup G_AB_IMEXPORTER Generic Im- and Exporter
 * @ingroup G_AB_C_INTERFACE
 */

/** @defgroup G_AB_ONLINE_BANKING Online Banking
 *  @ingroup G_AB_C_INTERFACE
 */

/** @defgroup G_AB_ACCOUNT Accounts
 * @ingroup G_AB_ONLINE_BANKING
 */

/** @defgroup G_AB_DIALOGS Platform-indepentent Dialogs
 *  @ingroup G_AB_C_INTERFACE
 *
 * Dialogs are only available if the application created and set a GWEN_GUI
 * object with support for the GWEN_DIALOG framework.
 *
 * Currently these are the following implementations:
 * <ul>
 *   <li>FOX16 (see @ref FOX16_Gui)</li>
 *   <li>QT3 (see @ref QT3_Gui)</li>
 *   <li>QT4 (see @ref QT4_Gui)</li>
 *   <li>GTK2 (see @ref Gtk2_Gui_new)</li>
 * </ul>
 *
 *
 * The following is a demonstration of the use of AqBankings new dialogs
 * (using the generic file importer):
 *
 * @code
 *
 *   GWEN_DIALOG *dlg;
 *   AB_IMEXPORTER_CONTEXT *ctx;
 *   int rv;

 *   ctx=AB_ImExporterContext_new();
 *   dlg=AB_ImporterDialog_new(banking,
 *       		      ctx,
 *       		      I18N("Your file has been successfully imported.\n"
 *       			   "Click the finish button below to import the "
 *       			   "data into the application\n"));
 *
 *   if (dlg==NULL) {
 *     fprintf(stderr, "Could not create dialog\n");
 *     return 1;
 *   }
 *
 *   rv=GWEN_Gui_ExecDialog(dlg, 0);
 *   if (rv==0) {
 *     GWEN_Dialog_free(dlg);
 *     AB_ImExporterContext_free(ctx);
 *     return 1;
 *   }
 *   GWEN_Dialog_free(dlg);
 *
 * @endcode
 *
 */


/** @defgroup G_AB_BE_INTERFACE API for Backends
 * @ingroup G_AB_LIBRARY
 */
/** @defgroup G_AB_BE_BANKING Main Interface
 * @ingroup G_AB_BE_INTERFACE
 */
/** @defgroup G_AB_PROVIDER Backend Objects
 * @ingroup G_AB_BE_INTERFACE
 */



/** @defgroup G_AB_GUI Gwenhywfar GUI Extension */


/** @defgroup G_AB_BACKENDS Backends
 */



/** @mainpage AqBanking Documentation Main Page

This is AqBanking, a modular library for Online Banking and related
tasks.

This library was designed by Martin Preuss<martin@aquamaniac.de> to
provide a generic way for applications to use Online Banking
(e.g. HBCI, EBICS), and to import/export financial data (such as OFX, SWIFT,
DTAUS). It is written in C (see @ref G_AB_C_INTERFACE).

A general Note: All strings exchanged from and to AqBanking are
expected in UTF-8 encoding unless stated otherwise. Please do not pass
Latin-1 strings (i.e. with latin1-Umlauts) into AqBanking or expect
the returned strings (which might be translated into German in UTF-8)
in that way. For converting UTF-8 into or from other encodings, see
the system function iconv(3) in iconv.h.


@section AB_features Features

AqBanking is very modular. It simply provides the means to manage
online accounts and to import/export financial data. AqBanking itself
does not implement the actual online banking protocols -- this is done
in extra plugins which serve as <i>backends</i>/<i>providers</i>. This
library is organized in multiple layers:

<ul>
  <li>
    <b>API for Applications</b>:
    This contains the complete API to be used by applications (see 
    @ref G_AB_C_INTERFACE). It is subdivided into several groups and 
    includes (among other things):
    <ul>
      <li>Managing Backends/Providers</li>
      <li>Managing Accounts</li>
      <li>Accepting @ref AB_JOB objects for online banking jobs from
      the application and sending them to the corresponding
      provider</li>
    </ul>
  </li>

  <li>
    <b>API for Backends</b>
    This level is represented by the function group @ref G_AB_BE_INTERFACE).
    These functions should <i>not</i> be accessed by an application; instead,
    they are accessed by <i>plugins</i>. The plugins (e.g. AqHBCI) implement
    the Provider functions which actually implement the online banking
    functionality. ("Provider" is simply another word for "backend".)
    The API has been designed to be as wide open as possible, and currently
    it is used by plugins for HBCI, OFX Direct Connect, YellowNet, 
    DTAUS and GeldKarte providers.
  </li>
</ul>

*/

/**
@page G_APP_INTRO Introduction into application programming with AqBanking
@verbinclude 03-APPS

 */


/** @defgroup G_TUTORIALS Tutorials  */



