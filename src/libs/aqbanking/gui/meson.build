
libabgui_includes = []
libabgui_includes += include_directories('../../')
libabgui_includes += libgwenhywfar_inc
libabgui_includes += root_includes

libabgui_defines = ab_defines


libabgui_src= [
  'abgui.c'
]


libabgui = static_library('abgui', libabgui_src,
                          include_directories : libabgui_includes,
                          c_args : libabgui_defines,
                          install : false
)


