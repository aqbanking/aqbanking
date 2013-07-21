/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Mon Mar 01 2004
 copyright   : (C) 2004 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 * This file is part of the project "AqBanking".                           *
 * Please see toplevel file COPYING of that project for license details.   *
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
{ I18N_NOOP("Albania"), "AL", 8, I18N_NOOP("Lek"),  "ALL" },
{ I18N_NOOP("Algeria"), "DZ", 12, I18N_NOOP("Algerian Dinar"),  "DZD" },
{ I18N_NOOP("American Samoa"), "AS", 16, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Andorra"), "AD", 20, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Angola"), "AO", 24, I18N_NOOP("New Kwanza "),  "AON" },
{ I18N_NOOP("Anguilla"), "AI", 660, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Antarctica"), "AQ", 10, I18N_NOOP("Norwegian Krone"),  "NOK" },
{ I18N_NOOP("Antigua and Barbuda"), "AG", 28, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Argentina"), "AR", 32, I18N_NOOP("Austral and Argenintinian Neuvo Peso "),  "ARA" },
{ I18N_NOOP("Armenia"), "AM", 51, I18N_NOOP("Dram "),  "AMD" },
{ I18N_NOOP("Aruba"), "AW", 533, I18N_NOOP("Aruban Guilder "),  "AWG" },
{ I18N_NOOP("Australia"), "AU", 36, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Austria"), "AT", 40, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Azerbaijan"), "AZ", 31, I18N_NOOP("Azerbaijani Manat "),  "AZM" },
{ I18N_NOOP("Bahrain"), "BH", 48, I18N_NOOP("Bahraini Dinar"),  "BHD" },
{ I18N_NOOP("Bangladesh"), "BD", 50, I18N_NOOP("Taka"),  "BDT" },
{ I18N_NOOP("Barbados"), "BB", 52, I18N_NOOP("Barbados Dollar"),  "BBD" },
{ I18N_NOOP("Belgium"), "BE", 56, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Belize"), "BZ", 84, I18N_NOOP("Belize Dollar"),  "BZD" },
{ I18N_NOOP("Benin"), "BJ", 204, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Bermuda"), "BM", 60, I18N_NOOP("Bermudian Dollar"),  "BMD" },
{ I18N_NOOP("Bhutan"), "BT", 64, I18N_NOOP("Ngultrum "),  "BTN" },
{ I18N_NOOP("Bolivia"), "BO", 68, I18N_NOOP("Boliviano and Bolivian Peso"),  "BOB" },
{ I18N_NOOP("Bosnia & Herzegowina"), "BA", 70, I18N_NOOP("Convertible Mark"),  "BAM" },
{ I18N_NOOP("Botswana"), "BW", 72, I18N_NOOP("Pula"),  "BWP" },
{ I18N_NOOP("Bouvet Island"), "BV", 74, I18N_NOOP("Norwegian Krone"),  "NOK" },
{ I18N_NOOP("Brazil"), "BR", 76, I18N_NOOP("Cruzeiro Real"),  "BRR" },
{ I18N_NOOP("British Indian Ocean Territory"), "IO", 86, I18N_NOOP("Pound Sterling "),  "GBP" },
{ I18N_NOOP("Brunei Darussalam"), "BN", 96, I18N_NOOP("Brunei Dollar"),  "BND" },
{ I18N_NOOP("Bulgaria"), "BG", 100, I18N_NOOP("Lev"),  "BGL" },
{ I18N_NOOP("Burkina Faso"), "BF", 854, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Burundi"), "BI", 108, I18N_NOOP("Burundi Franc"),  "BIF" },
{ I18N_NOOP("Cameroon"), "CM", 120, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Canada"), "CA", 124, I18N_NOOP("Canadian Dollar"),  "CAD" },
{ I18N_NOOP("Cape Verde"), "CV", 132, I18N_NOOP("Escudo Caboverdiano"),  "CVE" },
{ I18N_NOOP("Cayman Islands"), "KY", 136, I18N_NOOP("Cayman Islands Dollar"),  "KYD" },
{ I18N_NOOP("Central African Republic"), "CF", 140, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Chad"), "TD", 148, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Chili"), "CL", 152, I18N_NOOP("Unidades de Fomento and Chilean Peso"),  "CLF" },
{ I18N_NOOP("China"), "CN", 156, I18N_NOOP("Yuan Renminbi"),  "CNY" },
{ I18N_NOOP("Christmas Island"), "CX", 162, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Cocos "), "CC", 166, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Colombia"), "CO", 170, I18N_NOOP("Colombian Peso"),  "COP" },
{ I18N_NOOP("Comoros"), "KM", 174, I18N_NOOP("Comorian Franc"),  "KMF" },
{ I18N_NOOP("Congo"), "CG", 178, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Cook Islands"), "CK", 184, I18N_NOOP("New Zealand Dollar"),  "NZD" },
{ I18N_NOOP("Costa Rica"), "CR", 188, I18N_NOOP("Costa Rican Col&oacute;n"),  "CRC" },
{ I18N_NOOP("Cuba"), "CU", 192, I18N_NOOP("Cuban Peso"),  "CUP" },
{ I18N_NOOP("Cyprus"), "CY", 196, I18N_NOOP("Cypriot Pound"),  "CYP" },
{ I18N_NOOP("Djibouti"), "DJ", 262, I18N_NOOP("Djibouti Franc"),  "DJF" },
{ I18N_NOOP("Dominica"), "DM", 212, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Dominican Republic"), "DO", 214, I18N_NOOP("Dominican Republic Peso"),  "DOP" },
{ I18N_NOOP("Ecuador"), "EC", 218, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Egypt"), "EG", 818, I18N_NOOP("Egytian Pound"),  "EGP" },
{ I18N_NOOP("El Salvador"), "SV", 222, I18N_NOOP("El Salvadorian Col&oacute;n"),  "SVC" },
{ I18N_NOOP("Equatorial Guinea"), "GQ", 226, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine and Ekwele"),  "XAF" },
{ I18N_NOOP("Eritrea"), "ER", 232, I18N_NOOP("Eritreian Nakfa, Ethiopian Birr"),  "ERN" },
{ I18N_NOOP("Estonia"), "EE", 233, I18N_NOOP("Kroon"),  "EEK" },
{ I18N_NOOP("Ethiopia"), "ET", 231, I18N_NOOP("Birr"),  "ETB" },
{ I18N_NOOP("European Community"), "??", 0, I18N_NOOP("Euro "),  "EUR" },
{ I18N_NOOP("Falkland Islands"), "FK", 238, I18N_NOOP("Falkland Pound"),  "FKP" },
{ I18N_NOOP("Faroe Islands"), "FO", 234, I18N_NOOP("Danish Krone"),  "DKK" },
{ I18N_NOOP("Fiji Islands"), "FJ", 242, I18N_NOOP("Fiji Dollar"),  "FJD" },
{ I18N_NOOP("Finland"), "FI", 246, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("France"), "FR", 250, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("France, Metropolitan"), "FX", 0, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("French Guiana"), "GF", 254, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("French Polynesia"), "PF", 258, I18N_NOOP("Franc des Comptoirs fran&ccedil;ais du Pacifique"),  "XPF" },
{ I18N_NOOP("French Southern and Antarctic Territories"), "TF", 260, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Gambia"), "GM", 270, I18N_NOOP("Dalasi"),  "GMD" },
{ I18N_NOOP("Georgia"), "GE", 268, I18N_NOOP("Lari "),  "GEL" },
{ I18N_NOOP("Germany "), "DE", 280, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Germany "), "DE", 276, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Ghana"), "GH", 288, I18N_NOOP("Cedi"),  "GHC" },
{ I18N_NOOP("Gibraltar"), "GI", 292, I18N_NOOP("Gibraltar Pound"),  "GIP" },
{ I18N_NOOP("Greece"), "GR", 300, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Greenland"), "GL", 304, I18N_NOOP("Danish Krone"),  "DKK" },
{ I18N_NOOP("Grenada"), "GD", 308, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Guadeloupe"), "GP", 312, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Guam"), "GU", 316, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Guatemala"), "GT", 320, I18N_NOOP("Quetzal"),  "GTQ" },
{ I18N_NOOP("Guinea"), "GN", 324, I18N_NOOP("Guinea Syli"),  "GNS" },
{ I18N_NOOP("Guinea-Bissau"), "GW", 624, I18N_NOOP("Guinea-Bissau Peso and Franc de la Communaut&eacute; financi&egrave;re africaine"),  "GWP" },
{ I18N_NOOP("Guyana"), "GY", 328, I18N_NOOP("Guyana Dollar"),  "GYD" },
{ I18N_NOOP("Heard and McDonald Islands"), "HM", 334, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Honduras"), "HN", 340, I18N_NOOP("Lempira"),  "HNL" },
{ I18N_NOOP("Hong Kong"), "HK", 344, I18N_NOOP("Hong Kong Dollar"),  "HKD" },
{ I18N_NOOP("Hungary"), "HU", 348, I18N_NOOP("Forint"),  "HUF" },
{ I18N_NOOP("India"), "IN", 356, I18N_NOOP("Indian Rupee"),  "INR" },
{ I18N_NOOP("Iran, Islamic Republic of"), "IR", 364, I18N_NOOP("Iranian Rial"),  "IRR" },
{ I18N_NOOP("Iraq"), "IQ", 368, I18N_NOOP("Iraqi Dinar"),  "IQD" },
{ I18N_NOOP("Ireland"), "IE", 372, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Israel"), "IL", 376, I18N_NOOP("Shekel"),  "ILS" },
{ I18N_NOOP("Italy"), "IT", 380, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Japan"), "JP", 392, I18N_NOOP("Yen"),  "JPY" },
{ I18N_NOOP("Jordan"), "JO", 400, I18N_NOOP("Jordanian Dinar"),  "JOD" },
{ I18N_NOOP("Kazakhstan"), "KZ", 398, I18N_NOOP("Tenge "),  "KZT" },
{ I18N_NOOP("Kenya"), "KE", 404, I18N_NOOP("Kenyan Shilling"),  "KES" },
{ I18N_NOOP("Kiribati"), "KI", 296, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Korea, Democratic People's Republic of "), "KP", 408, I18N_NOOP("North Korean Won"),  "KPW" },
{ I18N_NOOP("Korea, Republic of "), "KR", 410, I18N_NOOP("South Korean Won"),  "KRW" },
{ I18N_NOOP("Kuwait"), "KW", 414, I18N_NOOP("Kuwaiti Dinar"),  "KWD" },
{ I18N_NOOP("Kyrgyzstan"), "KG", 417, I18N_NOOP("Kyrgyzstani Som"),  "KGS" },
{ I18N_NOOP("Latvia"), "LV", 428, I18N_NOOP("Lats"),  "LVL" },
{ I18N_NOOP("Lebanon"), "LB", 422, I18N_NOOP("Lebanese Pound"),  "LBP" },
{ I18N_NOOP("Lesotho"), "LS", 426, I18N_NOOP("Loti, Maloti and South African Rand"),  "LSL" },
{ I18N_NOOP("Liberia"), "LR", 430, I18N_NOOP("Liberian Dollar"),  "LRD" },
{ I18N_NOOP("Libyan Arab Jamahiriya"), "LY", 434, I18N_NOOP("Libyan Dinar"),  "LYD" },
{ I18N_NOOP("Liechtenstein"), "LI", 438, I18N_NOOP("Swiss Franc"),  "CHF" },
{ I18N_NOOP("Lithuania"), "LT", 440, I18N_NOOP("Litas"),  "LTL" },
{ I18N_NOOP("Luxembourg"), "LU", 442, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Macedonia, the Former Yugoslav Republic of"), "MK", 807, I18N_NOOP("Macedonian Dinar"),  "MKD" },
{ I18N_NOOP("Madagascar"), "MG", 450, I18N_NOOP("Malagasy Franc"),  "MGF" },
{ I18N_NOOP("Malawi"), "MW", 454, I18N_NOOP("Malawian Kwacha"),  "MWK" },
{ I18N_NOOP("Malaysia"), "MY", 458, I18N_NOOP("Ringgit "),  "MYR" },
{ I18N_NOOP("Maldives"), "MV", 462, I18N_NOOP("Rufiyaa"),  "MVR" },
{ I18N_NOOP("Mali"), "ML", 466, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine and Malian Franc"),  "XAF" },
{ I18N_NOOP("Malta"), "MT", 470, I18N_NOOP("Maltese Lira "),  "MTL" },
{ I18N_NOOP("Marshall Islands"), "MH", 584, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Martinique"), "MQ", 474, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Mauritania"), "MR", 478, I18N_NOOP("Ouguiya"),  "MRO" },
{ I18N_NOOP("Mauritius"), "MU", 480, I18N_NOOP("Mauritius Rupee"),  "MUR" },
{ I18N_NOOP("Mayotte"), "YT", 175, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Mexico"), "MX", 484, I18N_NOOP("Mexican New Peso "),  "MXN" },
{ I18N_NOOP("Micronesia, Federated States of"), "FM", 583, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Moldova, Republic of"), "MD", 498, I18N_NOOP("Moldovian Leu"),  "MDL" },
{ I18N_NOOP("Monaco"), "MC", 492, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Mongolia"), "MN", 496, I18N_NOOP("Tugrik"),  "MNT" },
{ I18N_NOOP("Montserrat"), "MS", 500, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Morocco"), "MA", 504, I18N_NOOP("Moroccan Dirham"),  "MAD" },
{ I18N_NOOP("Mozambique"), "MZ", 508, I18N_NOOP("Metical"),  "MZM" },
{ I18N_NOOP("Nauru"), "NR", 520, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Nepal"), "NP", 524, I18N_NOOP("Nepalese Rupee"),  "NPR" },
{ I18N_NOOP("Netherlands"), "NL", 528, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Netherlands Antilles"), "AN", 530, I18N_NOOP("Netherlands Antilles Guilder "),  "ANG" },
{ I18N_NOOP("New Caledonia"), "NC", 540, I18N_NOOP("Franc des Comptoirs fran&ccedil;ais du Pacifique"),  "XPF" },
{ I18N_NOOP("New Zealand"), "NZ", 554, I18N_NOOP("New Zealand Dollar"),  "NZD" },
{ I18N_NOOP("Nicaragua"), "NI", 558, I18N_NOOP("C&oacute;rdoba"),  "NIC" },
{ I18N_NOOP("Niger"), "NE", 562, I18N_NOOP("West African Franc and Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XOF" },
{ I18N_NOOP("Nigeria"), "NG", 566, I18N_NOOP("Naira"),  "NGN" },
{ I18N_NOOP("Niue"), "NU", 570, I18N_NOOP("New Zealand Dollar"),  "NZD" },
{ I18N_NOOP("Norfolk Island"), "NF", 574, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Northern Mariana Islands"), "MP", 580, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Norway"), "NO", 578, I18N_NOOP("Norwegian Krone"),  "NOK" },
{ I18N_NOOP("Palau"), "PW", 585, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Panama"), "PA", 591, I18N_NOOP("Balboa and US Dollar"),  "PAB" },
{ I18N_NOOP("Papua New Guinea"), "PG", 598, I18N_NOOP("Kina"),  "PGK" },
{ I18N_NOOP("Paraguay"), "PY", 600, I18N_NOOP("Guarani"),  "PYG" },
{ I18N_NOOP("Peru"), "PE", 604, I18N_NOOP("Inti and New Sol "),  "PEI" },
{ I18N_NOOP("Philippines"), "PH", 608, I18N_NOOP("Philippines Peso"),  "PHP" },
{ I18N_NOOP("Pitcairn Island"), "PN", 612, I18N_NOOP("New Zealand Dollar"),  "NZD" },
{ I18N_NOOP("Poland"), "PL", 616, I18N_NOOP("New Zloty "),  "PLN" },
{ I18N_NOOP("Portugal"), "PT", 620, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Puerto Rico"), "PR", 630, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Romania"), "RO", 642, I18N_NOOP("Romanian Leu"),  "ROL" },
{ I18N_NOOP("Russian Federation"), "RU", 643, I18N_NOOP("Russian Federation Rouble"),  "RUR" },
{ I18N_NOOP("Rwanda"), "RW", 646, I18N_NOOP("Rwanda Franc"),  "RWF" },
{ I18N_NOOP("Saint Kitts "), "KN", 659, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Saint Lucia"), "LC", 662, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Saint Vincent and the Grenadines"), "VC", 670, I18N_NOOP("East Caribbean Dollar"),  "XCD" },
{ I18N_NOOP("Samoa"), "WS", 882, I18N_NOOP("Tala"),  "WST" },
{ I18N_NOOP("San Marino"), "SM", 674, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("S&atilde;o Tom&eacute; and Pr&iacute;ncipe"), "ST", 678, I18N_NOOP("Dobra"),  "STD" },
{ I18N_NOOP("Saudi Arabia"), "SA", 682, I18N_NOOP("Saudi Riyal"),  "SAR" },
{ I18N_NOOP("Senegal"), "SN", 686, I18N_NOOP("West African Franc and Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XOF" },
{ I18N_NOOP("Seychelles"), "SC", 690, I18N_NOOP("Seychelles Rupee"),  "SCR" },
{ I18N_NOOP("Sierra Leone"), "SL", 694, I18N_NOOP("Leone"),  "SLL" },
{ I18N_NOOP("Singapore"), "SG", 702, I18N_NOOP("Singapore Dollar"),  "SGD" },
{ I18N_NOOP("Slovenia"), "SI", 705, I18N_NOOP("Tolar"),  "SIT" },
{ I18N_NOOP("Solomon Islands"), "SB", 90, I18N_NOOP("Solomon Islands Dollar"),  "SBD" },
{ I18N_NOOP("Somalia"), "SO", 706, I18N_NOOP("Somali Shilling"),  "SOS" },
{ I18N_NOOP("South Africa"), "ZA", 710, I18N_NOOP("Rand"),  "ZAR" },
{ I18N_NOOP("South Georgia and the South Sandwich Islands"), "GS", 239, I18N_NOOP("Pound Sterling"),  "GBP" },
{ I18N_NOOP("Spain"), "ES", 724, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Sri Lanka"), "LK", 144, I18N_NOOP("Sri Lankan Rupee"),  "LKR" },
{ I18N_NOOP("St Helena"), "SH", 654, I18N_NOOP("St Helena Pound"),  "SHP" },
{ I18N_NOOP("St Pierre and Miquelon"), "PM", 666, I18N_NOOP("Euro"),  "EUR" },
{ I18N_NOOP("Sudan"), "SD", 736, I18N_NOOP("Sudanese Pound and Sudanese Dinar"),  "SDP" },
{ I18N_NOOP("Suriname"), "SR", 740, I18N_NOOP("Surinam Guilder "),  "SRG" },
{ I18N_NOOP("Svalbard and Jan Mayen Islands"), "SJ", 744, I18N_NOOP("Norwegian Krone"),  "NOK" },
{ I18N_NOOP("Swaziland"), "SZ", 748, I18N_NOOP("Lilangeni"),  "SZL" },
{ I18N_NOOP("Sweden"), "SE", 752, I18N_NOOP("Swedish Krona"),  "SEK" },
{ I18N_NOOP("Switzerland"), "CH", 756, I18N_NOOP("Swiss Franc"),  "CHF" },
{ I18N_NOOP("Syrian Arab Republic"), "SY", 760, I18N_NOOP("Syrian Pound"),  "SYP" },
{ I18N_NOOP("Tajikistan"), "TJ", 762, I18N_NOOP("Tajik Rouble "),  "TJR" },
{ I18N_NOOP("Tanzania, United Republic of"), "TZ", 834, I18N_NOOP("Tanzanian Shilling"),  "TZS" },
{ I18N_NOOP("Thailand"), "TH", 764, I18N_NOOP("Baht"),  "THB" },
{ I18N_NOOP("Togo"), "TG", 768, I18N_NOOP("Franc de la Communaut&eacute; financi&egrave;re africaine"),  "XAF" },
{ I18N_NOOP("Tokelau"), "TK", 772, I18N_NOOP("New Zealand Dollar"),  "NZD" },
{ I18N_NOOP("Tonga"), "TO", 776, I18N_NOOP("Pa'anga"),  "TOP" },
{ I18N_NOOP("Trinidad and Tobago"), "TT", 780, I18N_NOOP("Trinidad and Tobago Dollar"),  "TTD" },
{ I18N_NOOP("Tunisia"), "TN", 788, I18N_NOOP("Tunisian Dinar"),  "TND" },
{ I18N_NOOP("Turkey"), "TR", 792, I18N_NOOP("Turkish Lira"),  "TRL" },
{ I18N_NOOP("Turkmenistan"), "TM", 795, I18N_NOOP("Turkmenistani Manat"),  "TMM" },
{ I18N_NOOP("Turks and Caicos Islands"), "TC", 796, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Tuvalu"), "TV", 798, I18N_NOOP("Australian Dollar"),  "AUD" },
{ I18N_NOOP("Ukraine"), "UA", 804, I18N_NOOP("Hryvna and Karbovanet"),  "UAH" },
{ I18N_NOOP("Union of Soviet Socialist Republics"), "SU", 0, I18N_NOOP("USSR Rouble"),  "SUR" },
{ I18N_NOOP("United Arab Emirates"), "AE", 784, I18N_NOOP("UAE Dirham"),  "AED" },
{ I18N_NOOP("United Kingdom"), "GB", 826, I18N_NOOP("Pound Sterling"),  "GBP" },
{ I18N_NOOP("United States of America"), "US", 840, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("United States Minor Outlying Islands"), "UM", 581, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Uruguay"), "UY", 858, I18N_NOOP("Uruguayan New Peso "),  "UYU" },
{ I18N_NOOP("Uzbekistan"), "UZ", 860, I18N_NOOP("Uzbekistani Som "),  "UZS" },
{ I18N_NOOP("Venezuela"), "VE", 862, I18N_NOOP("Bolivar"),  "VEB" },
{ I18N_NOOP("Viet Nam"), "VN", 704, I18N_NOOP("D&ocirc;ng"),  "VND" },
{ I18N_NOOP("Virgin Islands "), "VG", 92, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("Virgin Islands "), "VI", 850, I18N_NOOP("US Dollar"),  "USD" },
{ I18N_NOOP("West Africa"), "XO", 0, I18N_NOOP("West African Franc"),  "XOF" },
{ I18N_NOOP("Western Sahara"), "EH", 732, I18N_NOOP("Moroccan Dirham and Mauritanian Ouguiya"),  "MAD" },
{ I18N_NOOP("Yugoslavia"), "YU", 0, I18N_NOOP("New Dinar"),  "YUD" },
{ I18N_NOOP("Zambia"), "ZM", 894, I18N_NOOP("Zambian Kwacha"),  "ZMK" },
{ I18N_NOOP("Zimbabwe"), "ZW", 716, I18N_NOOP("Zimbabwe Dollar"),  "ZWD" },
  /* Watch out: The last element *has* to be this all-zero
     element, or else the iterations will not find the end of this
     list. */
{ 0, 0, 0, 0, 0 }
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



const char *AB_Country_GetCurrencyName(const AB_COUNTRY *cntry){
  assert(cntry);
  return cntry->currencyName;
}



const char *AB_Country_GetCurrencyCode(const AB_COUNTRY *cntry){
  assert(cntry);
  return cntry->currencyCode;
}



const char *AB_Country_GetLocalCurrencyName(const AB_COUNTRY *cntry){
  assert(cntry);
  return I18N(cntry->currencyName);
}






