
libabtypes_includes = []
libabtypes_includes += include_directories('../../')
libabtypes_includes += libgwenhywfar_inc
libabtypes_includes += root_includes


libabtypes_defines = ab_defines


libabtypes_gensrc = [
  'transaction.c',
  'transactionlimits.c',
  'bankinfoservice.c',
  'bankinfo.c',
  'document.c',
  'message.c',
  'security.c',
  'balance.c',
  'imexporter_accountinfo.c',
  'imexporter_context.c',
  'account_spec.c'
]

libabtypes_src = [ libabtypes_gensrc,
  'value.c'
]


libabtypes = static_library('abtypes', libabtypes_src,
                            include_directories : libabtypes_includes,
                            c_args : libabtypes_defines,
                            install : false
)



