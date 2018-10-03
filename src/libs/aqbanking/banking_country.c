/***************************************************************************
 begin       : Wed Oct 03 2018
 copyright   : (C) 2018 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
 ***************************************************************************/



const AB_COUNTRY *AB_Banking_FindCountryByName(AB_BANKING *ab, const char *name){
  assert(ab);
  return AB_Country_FindByName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByLocalName(AB_BANKING *ab, const char *name){
  assert(ab);
  return AB_Country_FindByLocalName(name);
}



const AB_COUNTRY *AB_Banking_FindCountryByCode(AB_BANKING *ab, const char *code){
  assert(ab);
  return AB_Country_FindByCode(code);
}



const AB_COUNTRY *AB_Banking_FindCountryByNumeric(AB_BANKING *ab, int numid){
  assert(ab);
  return AB_Country_FindByNumeric(numid);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByName(AB_BANKING *ab, const char *name){
  assert(ab);
  return AB_Country_ListByName(name);
}



AB_COUNTRY_CONSTLIST2 *AB_Banking_ListCountriesByLocalName(AB_BANKING *ab, const char *name){
  assert(ab);
  return AB_Country_ListByLocalName(name);
}




