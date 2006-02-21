

/** @defgroup G_AB_LIBRARY AqBanking Library */

/** @defgroup G_AB_C_INTERFACE API for C
 *  @ingroup G_AB_LIBRARY
 */

/** @defgroup G_AB_FRONTENDS Frontends */

/** @defgroup G_AB_QBANKING QT Frontend (QBanking)
 * @ingroup G_AB_FRONTENDS
 */

/** @defgroup G_AB_KBANKING KDE Frontend (KBanking)
 * @ingroup G_AB_FRONTENDS
 */

/** @defgroup G_AB_G2BANKING GTK2 Frontend (G2Banking)
 * @ingroup G_AB_FRONTENDS
 */
/** @defgroup G_AB_G2BANKING_VIEWS Views
 * @ingroup G_AB_G2BANKING
 */
/** @defgroup G_AB_G2BANKING_WIDGETS Widgets
 * @ingroup G_AB_G2BANKING
 */

/** @defgroup G_AB_CBANKING Console Frontend (CBanking)
 * @ingroup G_AB_FRONTENDS
 */


/** @defgroup G_AB_BACKENDS Backends */



/** @mainpage AqBanking Documentation Main Page

This is AqBanking, a modular library for Online Banking and related
tasks.

This library was designed by Martin Preuss<martin@aquamaniac.de> to
provide a generic way for applications to use Online Banking
(e.g. HBCI), and to import/export financial data (such as OFX, SWIFT,
DTAUS). It is written in C (see @ref G_AB_C_INTERFACE).

If you are writing an application and want to use AqBanking's
features, there are two possibilities:

 - Use the "Im/Exporter Layer API", which requires the least coding
 effort on the application side. See @ref G_AB_BANKING_IMEXPORT_API

 - Or you can use the "Main Interface API" (also called "High level
 API" sometimes), which offers the highest flexibility by its access
 to @ref AB_JOB objects but requires slightly more coding effort. See
 @ref G_AB_BANKING, and there is also a full introduction into the
 program flow of that interface: @ref G_APP_INTRO

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

  <b>Lowlevel</b>: This level is represented by the function group
  AB_PROVIDER (see @ref G_AB_PROVIDER). These functions should
  <i>not</i> be accessed by an application; instead, they are accessed
  by <i>plugins</i>. The plugins (e.g. AqHBCI) implement the Provider
  functions which actually implement the online banking
  functionality. ("Provider" is simply another word for "backend".)
  Currently only AqHBCI for HBCI is available. However the API has
  been designed to be as wide open as possible. This layer also
  includes the simple API (consisting of a single function for now)
  for plugins that are importing transactions from a file.
    
  </li>
  <li>

  <b>Midlevel</b>: This is the glue between lowlevel and
  highlevel. This is the layer which holds the list of manageable
  accounts and distributes jobs across the providers. This is also not
  used by the application.

  </li>
  <li>

  <b>Highlevel</b> (or "Main Interface"): These are the functions used
  by the application (see @ref G_AB_BANKING and @ref G_APP_INTRO) This
  layer offers:

    <ul>
      <li>Managing Backends/Providers</li>
      <li>Managing Accounts</li>
      <li>Accepting @ref AB_JOB objects for online banking jobs from
      the application and sending it to the corresponding
      provider</li>
    </ul>
    The application only needs to ask AqBanking for a list of accounts
    and to map those accounts to its own accounts.

  </li>
  <li>

  <b>ImExporter Layer</b> (previously this was occasionally called
  "Application Layer"): This layer offers an even simpler API to
  applications, where the calls to all lower layers are combined, and
  the application will <i>not</i> deal with @ref AB_JOB objects. This
  may be useful when adding AqBanking support to applications which
  have not been designed to work with AqBanking in the first place
  (see @ref G_AB_BANKING_IMEXPORT_API)

  </li>
</ul>


For the backends/providers, AqBanking provides callbacks for some
simple user interaction functions, independently of the actual
graphical or text frontend. (see @ref G_AB_PROVIDER) This has the
additional advantage that any new backend/provider will then
immediately be supported by all applications.
  */

/**
@page G_APP_INTRO Introduction into application programming with AqBanking
@verbinclude 03-APPS

 */


/** @defgroup G_TUTORIALS Tutorials  */



