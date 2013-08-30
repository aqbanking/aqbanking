INCLUDES = -I$(aqbanking_symlinkdir) $(gwenhywfar_includes) -I$(top_srcdir)/src/libs/aqbanking

noinst_LTLIBRARIES=libofxparser.la

libofxparser_la_SOURCES=\
 ofxxmlctx.c \
 ofxgroup.c \
 g_acctinfo.c \
 g_acctinfors.c \
 g_acctinfotrnrs.c \
 g_bal.c \
 g_bankacc.c \
 g_bankacctinfo.c \
 g_bankmsgsrsv1.c \
 g_banktranlist.c \
 g_buymf.c \
 g_buystock.c \
 g_creditcardmsgsrsv1.c \
 g_document.c \
 g_generic.c \
 g_ignore.c \
 g_income.c \
 g_invacc.c \
 g_invbuy.c \
 g_invpos.c \
 g_invposlist.c \
 g_invstmtmsgsrsv1.c \
 g_invstmtrs.c \
 g_invstmttrnrs.c \
 g_invtran.c \
 g_invtranlist.c \
 g_mfinfo.c \
 g_ofx.c \
 g_posmf.c \
 g_posstock.c \
 g_reinvest.c \
 g_secid.c \
 g_secinfo.c \
 g_seclist.c \
 g_seclistmsgsrsv1.c \
 g_signonmsgsrsv1.c \
 g_signupmsgsrsv1.c \
 g_sonrs.c \
 g_status.c \
 g_stmtrn.c \
 g_stmtrs.c \
 g_stmttrnrs.c \
 g_stockinfo.c

noinst_HEADERS=\
 ofxxmlctx_l.h ofxxmlctx_p.h \
 ofxgroup_l.h ofxgroup_p.h \
 g_acctinfo_l.h g_acctinfo_p.h \
 g_acctinfors_l.h g_acctinfors_p.h \
 g_acctinfotrnrs_l.h g_acctinfotrnrs_p.h \
 g_bal_l.h g_bal_p.h \
 g_bankacc_l.h g_bankacc_p.h \
 g_bankacctinfo_l.h g_bankacctinfo_p.h \
 g_bankmsgsrsv1_l.h g_bankmsgsrsv1_p.h \
 g_banktranlist_l.h g_banktranlist_p.h \
 g_buymf_l.h g_buymf_p.h \
 g_buystock_l.h g_buystock_p.h \
 g_creditcardmsgsrsv1_l.h g_creditcardmsgsrsv1_p.h \
 g_document_l.h g_document_p.h \
 g_generic_l.h g_generic_p.h \
 g_ignore_l.h g_ignore_p.h \
 g_income_l.h g_income_p.h \
 g_invacc_l.h g_invacc_p.h \
 g_invbuy_l.h g_invbuy_p.h \
 g_invpos_l.h g_invpos_p.h \
 g_invposlist_l.h g_invposlist_p.h \
 g_invstmtmsgsrsv1_l.h g_invstmtmsgsrsv1_p.h \
 g_invstmtrs_l.h g_invstmtrs_p.h \
 g_invstmttrnrs_l.h g_invstmttrnrs_p.h \
 g_invtran_l.h g_invtran_p.h \
 g_invtranlist_l.h g_invtranlist_p.h \
 g_mfinfo_l.h  g_mfinfo_p.h \
 g_ofx_l.h g_ofx_p.h \
 g_posmf_l.h  g_posmf_p.h \
 g_posstock_l.h g_posstock_p.h \
 g_reinvest_l.h g_reinvest_p.h \
 g_secid_l.h g_secid_p.h \
 g_secinfo_l.h g_secinfo_p.h \
 g_seclist_l.h g_seclist_p.h \
 g_seclistmsgsrsv1_l.h g_seclistmsgsrsv1_p.h \
 g_signonmsgsrsv1_l.h g_signonmsgsrsv1_p.h \
 g_signupmsgsrsv1_l.h g_signupmsgsrsv1_p.h \
 g_sonrs_l.h g_sonrs_p.h \
 g_status_l.h g_status_p.h \
 g_stmtrn_l.h g_stmtrn_p.h \
 g_stmtrs_l.h g_stmtrs_p.h \
 g_stmttrnrs_l.h g_stmttrnrs_p.h \
 g_stockinfo_l.h g_stockinfo_p.h

sources:
	for f in $(libofxparser_la_SOURCES); do \
	  echo $(subdir)/$$f >>$(top_srcdir)/i18nsources; \
	done

