

/** @defgroup G_AB_C_INTERFACE API for C  */
/** @defgroup G_AB_CPP_INTERFACE API for C++ */


/** @mainpage AqBanking Documentation Main Page

<p>
This is AqBanking, a modular Banking API.
</p>

<p>
This library was designed by Martin Preuss<martin@aquamaniac.de> to provide a
generic way for applications to use Online Banking (e.g. HBCI) and to 
import/export financial data (such as SWIFT, DTAUS).
</p>

<p>
It is written in C but a C++ interface to the main functions is also
provided (see @ref G_AB_C_INTERFACE and @ref G_AB_CPP_INTERFACE).
</p>

<p>
If you want to add support for AqBanking to existing applications you
may want to have a look at the API designed for this purpose 
(@ref G_AB_BANKING_HL).
</p>

<p>
If you are writing an application based on AqBanking you may additionally
want to look into the main API (see @ref G_AB_BANKING).
</p>

@section AB_features Features

<p>
AqBanking is very modular. It simply provides the means to manage online
accounts and to import/export financial data.
</p>

AqBanking consists of multiple layers:
<ul>
  <li>
    @b Lowlevel: This level is represented by the function group
    AB_PROVIDER (see @ref G_AB_PROVIDER). Providers are plugins which actually
    implement the online banking functionality (as OpenHBCI previously did 
    and AqHBCI now does).
    <br>
    Currently only HBCI is provided. However the API has been designed to be
    as wide open as possible. This layer also includes the simple API
    (consisting of a single function for now) for importing transactions from
    a file.
    
  </li>
  <li>
    @b Midlevel: This is the glue between lowlevel and highlevel. This is the
    layer which holds the list of manageable accounts and distributes jobs
    across the providers.
  </li>
  <li>
    @b Highlevel: These are the functions used by the application 
    (AB_BANKING, see @ref G_AB_BANKING)
    This layer offers:
    <ul>
      <li>returning a list of providers</li>
      <li>returning a list of accounts</li>
      <li>receiving jobs from the application and sending it to the
        corresponding provider</li>
    </ul>
    The application only needs to ask AqBanking for a list of accounts
    and to map those accounts to its own accounts.
  </li>
  <li>
    @b Application Layer: This layer combines calls to functions of the other
    layers to provide a simpler API to applications. This is especially
    usefull when adding AqBanking support to applications which have not been
    designed to work with AqBanking in the first place 
    (see @ref G_AB_BANKING_HL)
  </li>
</ul>


<p>
For providers (backends) AqBanking provides an easy interface to any
application thus supporting graphical (KDE, GNOME) as well as console
applications to be used with AqBanking (see @ref G_AB_PROVIDER)
</p>

<p>
If a new provider is added to AqBanking then it immediately is supported by
<b>all</b> applications.
</p>



 */







