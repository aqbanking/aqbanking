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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "country_p.h"
#include "i18n_l.h"

#include <gwenhywfar/text.h>
#include <gwenhywfar/debug.h>


GWEN_CONSTLIST2_FUNCTIONS(AB_COUNTRY, AB_Country)


static AB_COUNTRY ab_country_list[]= {
{ I18N_NOOP("AFGHANISTAN"), "AF", 4 },
{ I18N_NOOP("AND ISLANDS"), "AX", 248 },
{ I18N_NOOP("ALBANIA"), "AL", 8 },
{ I18N_NOOP("ALGERIA"), "DZ", 12 },
{ I18N_NOOP("AMERICAN SAMOA"), "AS", 16 },
{ I18N_NOOP("ANDORRA"), "AD", 20 },
{ I18N_NOOP("ANGOLA"), "AO", 24 },
{ I18N_NOOP("ANGUILLA"), "AI", 660 },
{ I18N_NOOP("ANTARCTICA"), "AQ", 10 },
{ I18N_NOOP("ANTIGUA AND BARBUDA"), "AG", 28 },
{ I18N_NOOP("ARGENTINA"), "AR", 32 },
{ I18N_NOOP("ARMENIA"), "AM", 51 },
{ I18N_NOOP("ARUBA"), "AW", 533 },
{ I18N_NOOP("AUSTRALIA"), "AU", 36 },
{ I18N_NOOP("AUSTRIA"), "AT", 40 },
{ I18N_NOOP("AZERBAIJAN"), "AZ", 31 },
{ I18N_NOOP("BAHAMAS"), "BS", 44 },
{ I18N_NOOP("BAHRAIN"), "BH", 48 },
{ I18N_NOOP("BANGLADESH"), "BD", 50 },
{ I18N_NOOP("BARBADOS"), "BB", 52 },
{ I18N_NOOP("BELARUS"), "BY", 112 },
{ I18N_NOOP("BELGIUM"), "BE", 56 },
{ I18N_NOOP("BELIZE"), "BZ", 84 },
{ I18N_NOOP("BENIN"), "BJ", 204 },
{ I18N_NOOP("BERMUDA"), "BM", 60 },
{ I18N_NOOP("BHUTAN"), "BT", 64 },
{ I18N_NOOP("BOLIVIA"), "BO", 68 },
{ I18N_NOOP("BOSNIA AND HERZEGOVINA"), "BA", 70 },
{ I18N_NOOP("BOTSWANA"), "BW", 72 },
{ I18N_NOOP("BOUVET ISLAND"), "BV", 74 },
{ I18N_NOOP("BRAZIL"), "BR", 76 },
{ I18N_NOOP("BRITISH INDIAN OCEAN TERRITORY"), "IO", 86 },
{ I18N_NOOP("BRUNEI DARUSSALAM"), "BN", 96 },
{ I18N_NOOP("BULGARIA"), "BG", 100 },
{ I18N_NOOP("BURKINA FASO"), "BF", 854 },
{ I18N_NOOP("BURUNDI"), "BI", 108 },
{ I18N_NOOP("CAMBODIA"), "KH", 116 },
{ I18N_NOOP("CAMEROON"), "CM", 120 },
{ I18N_NOOP("CANADA"), "CA", 124 },
{ I18N_NOOP("CAPE VERDE"), "CV", 132 },
{ I18N_NOOP("CAYMAN ISLANDS"), "KY", 136 },
{ I18N_NOOP("CENTRAL AFRICAN REPUBLIC"), "CF", 140 },
{ I18N_NOOP("CHAD"), "TD", 148 },
{ I18N_NOOP("CHILE"), "CL", 152 },
{ I18N_NOOP("CHINA"), "CN", 156 },
{ I18N_NOOP("CHRISTMAS ISLAND"), "CX", 162 },
{ I18N_NOOP("COCOS (KEELING) ISLANDS"), "CC", 166 },
{ I18N_NOOP("COLOMBIA"), "CO", 170 },
{ I18N_NOOP("COMOROS"), "KM", 174 },
{ I18N_NOOP("CONGO"), "CG", 178 },
{ I18N_NOOP("CONGO, THE DEMOCRATIC REPUBLIC OF THE"), "CD", 180 },
{ I18N_NOOP("COOK ISLANDS"), "CK", 184 },
{ I18N_NOOP("COSTA RICA"), "CR", 188 },
{ I18N_NOOP("COTE D'IVOIRE"), "CI", 384 },
{ I18N_NOOP("CROATIA"), "HR", 191 },
{ I18N_NOOP("CUBA"), "CU", 192 },
{ I18N_NOOP("CYPRUS"), "CY", 196 },
{ I18N_NOOP("CZECH REPUBLIC"), "CZ", 203 },
{ I18N_NOOP("DENMARK"), "DK", 208 },
{ I18N_NOOP("DJIBOUTI"), "DJ", 262 },
{ I18N_NOOP("DOMINICA"), "DM", 212 },
{ I18N_NOOP("DOMINICAN REPUBLIC"), "DO", 214 },
{ I18N_NOOP("ECUADOR"), "EC", 218 },
{ I18N_NOOP("EGYPT"), "EG", 818 },
{ I18N_NOOP("EL SALVADOR"), "SV", 222 },
{ I18N_NOOP("EQUATORIAL GUINEA"), "GQ", 226 },
{ I18N_NOOP("ERITREA"), "ER", 232 },
{ I18N_NOOP("ESTONIA"), "EE", 233 },
{ I18N_NOOP("ETHIOPIA"), "ET", 231 },
{ I18N_NOOP("FALKLAND ISLANDS (MALVINAS)"), "FK", 238 },
{ I18N_NOOP("FAROE ISLANDS"), "FO", 234 },
{ I18N_NOOP("FIJI"), "FJ", 242 },
{ I18N_NOOP("FINLAND"), "FI", 246 },
{ I18N_NOOP("FRANCE"), "FR", 250 },
{ I18N_NOOP("FRENCH GUIANA"), "GF", 254 },
{ I18N_NOOP("FRENCH POLYNESIA"), "PF", 258 },
{ I18N_NOOP("FRENCH SOUTHERN TERRITORIES"), "TF", 260 },
{ I18N_NOOP("GABON"), "GA", 266 },
{ I18N_NOOP("GAMBIA"), "GM", 270 },
{ I18N_NOOP("GEORGIA"), "GE", 268 },
{ I18N_NOOP("GERMANY"), "DE", 280 },
{ I18N_NOOP("GERMANY"), "DE", 276 },
{ I18N_NOOP("GHANA"), "GH", 288 },
{ I18N_NOOP("GIBRALTAR"), "GI", 292 },
{ I18N_NOOP("GREECE"), "GR", 300 },
{ I18N_NOOP("GREENLAND"), "GL", 304 },
{ I18N_NOOP("GRENADA"), "GD", 308 },
{ I18N_NOOP("GUADELOUPE"), "GP", 312 },
{ I18N_NOOP("GUAM"), "GU", 316 },
{ I18N_NOOP("GUATEMALA"), "GT", 320 },
{ I18N_NOOP("GUINEA"), "GN", 324 },
{ I18N_NOOP("GUINEA-BISSAU"), "GW", 624 },
{ I18N_NOOP("GUYANA"), "GY", 328 },
{ I18N_NOOP("HAITI"), "HT", 332 },
{ I18N_NOOP("HEARD ISLAND AND MCDONALD ISLANDS"), "HM", 334 },
{ I18N_NOOP("HOLY SEE (VATICAN CITY STATE)"), "VA", 336 },
{ I18N_NOOP("HONDURAS"), "HN", 340 },
{ I18N_NOOP("HONG KONG"), "HK", 344 },
{ I18N_NOOP("HUNGARY"), "HU", 348 },
{ I18N_NOOP("ICELAND"), "IS", 352 },
{ I18N_NOOP("INDIA"), "IN", 356 },
{ I18N_NOOP("INDONESIA"), "ID", 360 },
{ I18N_NOOP("IRAN, ISLAMIC REPUBLIC OF"), "IR", 364 },
{ I18N_NOOP("IRAQ"), "IQ", 368 },
{ I18N_NOOP("IRELAND"), "IE", 372 },
{ I18N_NOOP("ISRAEL"), "IL", 376 },
{ I18N_NOOP("ITALY"), "IT", 380 },
{ I18N_NOOP("JAMAICA"), "JM", 388 },
{ I18N_NOOP("JAPAN"), "JP", 392 },
{ I18N_NOOP("JORDAN"), "JO", 400 },
{ I18N_NOOP("KAZAKHSTAN"), "KZ", 398 },
{ I18N_NOOP("KENYA"), "KE", 404 },
{ I18N_NOOP("KIRIBATI"), "KI", 296 },
{ I18N_NOOP("KOREA, DEMOCRATIC PEOPLE'S REPUBLIC OF"), "KP", 408 },
{ I18N_NOOP("KOREA, REPUBLIC OF"), "KR", 410 },
{ I18N_NOOP("KUWAIT"), "KW", 414 },
{ I18N_NOOP("KYRGYZSTAN"), "KG", 417 },
{ I18N_NOOP("LAO PEOPLE'S DEMOCRATIC REPUBLIC"), "LA", 418 },
{ I18N_NOOP("LATVIA"), "LV", 428 },
{ I18N_NOOP("LEBANON"), "LB", 422 },
{ I18N_NOOP("LESOTHO"), "LS", 426 },
{ I18N_NOOP("LIBERIA"), "LR", 430 },
{ I18N_NOOP("LIBYAN ARAB JAMAHIRIYA"), "LY", 434 },
{ I18N_NOOP("LIECHTENSTEIN"), "LI", 438 },
{ I18N_NOOP("LITHUANIA"), "LT", 440 },
{ I18N_NOOP("LUXEMBOURG"), "LU", 442 },
{ I18N_NOOP("MACAO"), "MO", 446 },
{ I18N_NOOP("MACEDONIA, THE FORMER YUGOSLAV REPUBLIC OF"), "MK", 807 },
{ I18N_NOOP("MADAGASCAR"), "MG", 450 },
{ I18N_NOOP("MALAWI"), "MW", 454 },
{ I18N_NOOP("MALAYSIA"), "MY", 458 },
{ I18N_NOOP("MALDIVES"), "MV", 462 },
{ I18N_NOOP("MALI"), "ML", 466 },
{ I18N_NOOP("MALTA"), "MT", 470 },
{ I18N_NOOP("MARSHALL ISLANDS"), "MH", 584 },
{ I18N_NOOP("MARTINIQUE"), "MQ", 474 },
{ I18N_NOOP("MAURITANIA"), "MR", 478 },
{ I18N_NOOP("MAURITIUS"), "MU", 480 },
{ I18N_NOOP("MAYOTTE"), "YT", 175 },
{ I18N_NOOP("MEXICO"), "MX", 484 },
{ I18N_NOOP("MICRONESIA, FEDERATED STATES OF"), "FM", 583 },
{ I18N_NOOP("MOLDOVA, REPUBLIC OF"), "MD", 498 },
{ I18N_NOOP("MONACO"), "MC", 492 },
{ I18N_NOOP("MONGOLIA"), "MN", 496 },
{ I18N_NOOP("MONTSERRAT"), "MS", 500 },
{ I18N_NOOP("MOROCCO"), "MA", 504 },
{ I18N_NOOP("MOZAMBIQUE"), "MZ", 508 },
{ I18N_NOOP("MYANMAR"), "MM", 104 },
{ I18N_NOOP("NAMIBIA"), "NA", 516 },
{ I18N_NOOP("NAURU"), "NR", 520 },
{ I18N_NOOP("NEPAL"), "NP", 524 },
{ I18N_NOOP("NETHERLANDS"), "NL", 528 },
{ I18N_NOOP("NETHERLANDS ANTILLES"), "AN", 530 },
{ I18N_NOOP("NEW CALEDONIA"), "NC", 540 },
{ I18N_NOOP("NEW ZEALAND"), "NZ", 554 },
{ I18N_NOOP("NICARAGUA"), "NI", 558 },
{ I18N_NOOP("NIGER"), "NE", 562 },
{ I18N_NOOP("NIGERIA"), "NG", 566 },
{ I18N_NOOP("NIUE"), "NU", 570 },
{ I18N_NOOP("NORFOLK ISLAND"), "NF", 574 },
{ I18N_NOOP("NORTHERN MARIANA ISLANDS"), "MP", 580 },
{ I18N_NOOP("NORWAY"), "NO", 578 },
{ I18N_NOOP("OMAN"), "OM", 512 },
{ I18N_NOOP("PAKISTAN"), "PK", 586 },
{ I18N_NOOP("PALAU"), "PW", 585 },
{ I18N_NOOP("PALESTINIAN TERRITORY, OCCUPIED"), "PS", 275 },
{ I18N_NOOP("PANAMA"), "PA", 591 },
{ I18N_NOOP("PAPUA NEW GUINEA"), "PG", 598 },
{ I18N_NOOP("PARAGUAY"), "PY", 600 },
{ I18N_NOOP("PERU"), "PE", 604 },
{ I18N_NOOP("PHILIPPINES"), "PH", 608 },
{ I18N_NOOP("PITCAIRN"), "PN", 612 },
{ I18N_NOOP("POLAND"), "PL", 616 },
{ I18N_NOOP("PORTUGAL"), "PT", 620 },
{ I18N_NOOP("PUERTO RICO"), "PR", 630 },
{ I18N_NOOP("QATAR"), "QA", 634 },
{ I18N_NOOP("REUNION"), "RE", 638 },
{ I18N_NOOP("ROMANIA"), "RO", 642 },
{ I18N_NOOP("RUSSIAN FEDERATION"), "RU", 643 },
{ I18N_NOOP("RWANDA"), "RW", 646 },
{ I18N_NOOP("SAINT HELENA"), "SH", 654 },
{ I18N_NOOP("SAINT KITTS AND NEVIS"), "KN", 659 },
{ I18N_NOOP("SAINT LUCIA"), "LC", 662 },
{ I18N_NOOP("SAINT PIERRE AND MIQUELON"), "PM", 666 },
{ I18N_NOOP("SAINT VINCENT AND THE GRENADINES"), "VC", 670 },
{ I18N_NOOP("SAMOA"), "WS", 882 },
{ I18N_NOOP("SAN MARINO"), "SM", 674 },
{ I18N_NOOP("SAO TOME AND PRINCIPE"), "ST", 678 },
{ I18N_NOOP("SAUDI ARABIA"), "SA", 682 },
{ I18N_NOOP("SENEGAL"), "SN", 686 },
{ I18N_NOOP("SERBIA AND MONTENEGRO"), "CS", 891 },
{ I18N_NOOP("SEYCHELLES"), "SC", 690 },
{ I18N_NOOP("SIERRA LEONE"), "SL", 694 },
{ I18N_NOOP("SINGAPORE"), "SG", 702 },
{ I18N_NOOP("SLOVAKIA"), "SK", 703 },
{ I18N_NOOP("SLOVENIA"), "SI", 705 },
{ I18N_NOOP("SOLOMON ISLANDS"), "SB", 90 },
{ I18N_NOOP("SOMALIA"), "SO", 706 },
{ I18N_NOOP("SOUTH AFRICA"), "ZA", 710 },
{ I18N_NOOP("SOUTH GEORGIA AND THE SOUTH SANDWICH ISLANDS"), "GS", 239 },
{ I18N_NOOP("SPAIN"), "ES", 724 },
{ I18N_NOOP("SRI LANKA"), "LK", 144 },
{ I18N_NOOP("SUDAN"), "SD", 736 },
{ I18N_NOOP("SURINAME"), "SR", 740 },
{ I18N_NOOP("SVALBARD AND JAN MAYEN"), "SJ", 744 },
{ I18N_NOOP("SWAZILAND"), "SZ", 748 },
{ I18N_NOOP("SWEDEN"), "SE", 752 },
{ I18N_NOOP("SWITZERLAND"), "CH", 756 },
{ I18N_NOOP("SYRIAN ARAB REPUBLIC"), "SY", 760 },
{ I18N_NOOP("TAIWAN, PROVINCE OF CHINA"), "TW", 158 },
{ I18N_NOOP("TAJIKISTAN"), "TJ", 762 },
{ I18N_NOOP("TANZANIA, UNITED REPUBLIC OF"), "TZ", 834 },
{ I18N_NOOP("THAILAND"), "TH", 764 },
{ I18N_NOOP("TIMOR-LESTE"), "TL", 626 },
{ I18N_NOOP("TOGO"), "TG", 768 },
{ I18N_NOOP("TOKELAU"), "TK", 772 },
{ I18N_NOOP("TONGA"), "TO", 776 },
{ I18N_NOOP("TRINIDAD AND TOBAGO"), "TT", 780 },
{ I18N_NOOP("TUNISIA"), "TN", 788 },
{ I18N_NOOP("TURKEY"), "TR", 792 },
{ I18N_NOOP("TURKMENISTAN"), "TM", 795 },
{ I18N_NOOP("TURKS AND CAICOS ISLANDS"), "TC", 796 },
{ I18N_NOOP("TUVALU"), "TV", 798 },
{ I18N_NOOP("UGANDA"), "UG", 800 },
{ I18N_NOOP("UKRAINE"), "UA", 804 },
{ I18N_NOOP("UNITED ARAB EMIRATES"), "AE", 784 },
{ I18N_NOOP("UNITED KINGDOM"), "GB", 826 },
{ I18N_NOOP("UNITED STATES"), "US", 840 },
{ I18N_NOOP("UNITED STATES MINOR OUTLYING ISLANDS"), "UM", 581 },
{ I18N_NOOP("URUGUAY"), "UY", 858 },
{ I18N_NOOP("UZBEKISTAN"), "UZ", 860 },
{ I18N_NOOP("VANUATU"), "VU", 548 },
{ I18N_NOOP("VENEZUELA"), "VE", 862 },
{ I18N_NOOP("VIET NAM"), "VN", 704 },
{ I18N_NOOP("VIRGIN ISLANDS, BRITISH"), "VG", 92 },
{ I18N_NOOP("VIRGIN ISLANDS, U.S."), "VI", 850 },
{ I18N_NOOP("WALLIS AND FUTUNA"), "WF", 876 },
{ I18N_NOOP("WESTERN SAHARA"), "EH", 732 },
{ I18N_NOOP("YEMEN"), "YE", 887 },
{ I18N_NOOP("ZAMBIA"), "ZM", 894 },
{ I18N_NOOP("ZIMBABWE"), "ZW", 716 },
{ 0, 0, 0 }
};




const AB_COUNTRY *AB_Country_FindByName(const char *name){
  AB_COUNTRY *c;

  c=ab_country_list;
  while(c->name) {
    if (-1!=GWEN_Text_ComparePattern(c->name, name, 0)) {
      return c;
    }
    c++;
  }

  return 0;
}



const AB_COUNTRY *AB_Country_FindByLocalName(const char *name){
  AB_COUNTRY *c;

  c=ab_country_list;
  while(c->name) {
    if (-1!=GWEN_Text_ComparePattern(AB_Country_GetLocalName(c), name, 0)) {
      return c;
    }
    c++;
  }

  return 0;
}



const AB_COUNTRY *AB_Country_FindByCode(const char *code){
  AB_COUNTRY *c;

  c=ab_country_list;
  while(c->name) {
    if (-1!=GWEN_Text_ComparePattern(c->code, code, 0)) {
      return c;
    }
    c++;
  }

  return 0;
}



const AB_COUNTRY *AB_Country_FindByNumeric(int numid){
  AB_COUNTRY *c;

  c=ab_country_list;
  while(c->name) {
    if (c->numericCode==numid)
      return c;
    c++;
  }

  return 0;
}



const char *AB_Country_GetName(const AB_COUNTRY *cntry){
  assert(cntry);
  return cntry->name;
}



const char *AB_Country_GetLocalName(const AB_COUNTRY *cntry){
  assert(cntry);
  return I18N(cntry->name);
}



const char *AB_Country_GetCode(const AB_COUNTRY *cntry){
  assert(cntry);
  return cntry->code;
}



int AB_Country_GetNumericCode(const AB_COUNTRY *cntry){
  assert(cntry);
  return cntry->numericCode;
}



AB_COUNTRY_CONSTLIST2 *AB_Country_ListByName(const char *name){
  const AB_COUNTRY *c;
  AB_COUNTRY_CONSTLIST2 *cl;

  c=ab_country_list;
  cl=AB_Country_ConstList2_new();
  while(c->name) {
    if (-1!=GWEN_Text_ComparePattern(c->name, name, 0)) {
      AB_Country_ConstList2_PushBack(cl, c);
    }
    c++;
  }
  if (AB_Country_ConstList2_GetSize(cl)==0) {
    AB_Country_ConstList2_free(cl);
    return 0;
  }
  return cl;
}


AB_COUNTRY_CONSTLIST2 *AB_Country_ListByLocalName(const char *name){
  const AB_COUNTRY *c;
  AB_COUNTRY_CONSTLIST2 *cl;

  c=ab_country_list;
  cl=AB_Country_ConstList2_new();
  while(c->name) {
    if (-1!=GWEN_Text_ComparePattern(AB_Country_GetLocalName(c), name, 0)){
      AB_Country_ConstList2_PushBack(cl, c);
    }
    c++;
  }
  if (AB_Country_ConstList2_GetSize(cl)==0) {
    AB_Country_ConstList2_free(cl);
    return 0;
  }
  return cl;
}







