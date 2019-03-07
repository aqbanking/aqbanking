
libabdialogs_includes = []
libabdialogs_includes += include_directories('../../')
libabdialogs_includes += libgwenhywfar_inc
libabdialogs_includes += root_includes

libabdialogs_defines = ab_defines


libabdialogs_src= [
  'dlg_importer.c',
  'dlg_selectbankinfo.c',
  'dlg_setup.c',
  'dlg_selectbackend.c',
  'dlg_editaccount.c',
  'dlg_edituser.c',
  'dlg_usertype_page.c',
  'dlg_setup_newuser.c',
  'dlg_usertype_pagedefault.c',
  'dlg_newaccount.c',
  'dlg_newuser.c'
]


libabdialogs = static_library('abdialogs', libabdialogs_src,
                              include_directories : libabdialogs_includes,
                              c_args : libabdialogs_defines,
                              install : false
)




