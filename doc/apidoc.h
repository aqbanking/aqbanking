

/** @defgroup AB_C_INTERFACE API for C  */
/** @defgroup AB_CPP_INTERFACE API for C++ */


/** @mainpage AqBanking Documentation Main Page

<p>
This is AqBanking, a modular Banking API.
</p>

<p>
This library was designed by Martin Preuss<martin@aquamaniac.de> to provide a
generic way for applications to use Online Banking (e.g. HBCI).
</p>

<p>
Though it is written in C a C++ interface to the main functions is
provided (see @ref AB_C_INTERFACE and @ref AB_CPP_INTERFACE).
</p>

@section AB_features Features

<p>
AqBanking is very modular. It simply provides the means to manage online
accounts.
</p>

AqBanking consists of multiple layers:
<ul>
  <li>
    Lowlevel: This level is represented by the function group
    AB_PROVIDER (see @ref AB_PROVIDER). Providers are plugins which actually
    implement the online banking functionality (as OpenHBCI previously did 
    and AqHBCI now does).
    <br>
    Currently only HBCI is provided. However the API has been designed to be
    as wide open as possible. This layer also includes the simple API
    (consisting of a single function for now) for importing transactions from
    a file.
    
  </li>
  <li>
    Midlevel: This is the glue between lowlevel and highlevel. This is the
    layer which holds the list of manageable accounts and distributes jobs
    across the providers.
  </li>
  <li>
    Highlevel: These are the functions used by the application. this layer
    offers:
    <ul>
      <li>returning a list of providers</li>
      <li>returning a list of accounts</li>
      <li>receiving jobs from the application and sending it to the
        corresponding provider</li>
    </ul>
    The application only needs to ask AqBanking for a list of accounts
    and to map those accounts to its own accounts.
  </li>
</ul>


<p>
For providers (backends) AqBanking provides an easy interface to any
application thus supporting graphical (KDE, GNOME) as well as console
applications to be used with AqBanking.
</p>

<p>
If a new provider is added to AqBanking then it immediately is supported by
<b>all</b> applications.
</p>



 */







