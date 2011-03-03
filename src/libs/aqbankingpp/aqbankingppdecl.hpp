/***************************************************************************
 $RCSfile$
                             -------------------
    begin       : Mon March 2 2011
    copyright   : (C) 2011 by Christian Stimming
    email       : christian@cstimming.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/


#ifndef AB_AQBANKINGPPDECL_HPP
#define AB_AQBANKINGPPDECL_HPP

#if defined(_WIN32) || defined(_MSC_VER)
# define AQBANKINGPP_IS_WINDOWS
#endif

#ifdef AQBANKINGPP_IS_WINDOWS
# ifdef export_AQBANKINGPP
#  define AQBANKINGPP_DECL __declspec (dllexport)
# else
#  define AQBANKINGPP_DECL __declspec (dllimport)
# endif
#else
# define AQBANKINGPP_DECL
#endif

#endif // AB_STRINGLIST_HPP
