
libabgui_includes = []
libabgui_includes += include_directories('../../')
libabgui_includes += libgwenhywfar_inc

libabgui_src= [
  'abgui.c'
]


libabgui = static_library('abgui', libabgui_src,
                          include_directories : libabgui_includes,
                          c_args : ab_defines,
                          install : false
)


