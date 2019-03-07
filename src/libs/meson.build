
subdir('aqbanking')
subdir('plugins')


libaqbanking_includes = []
libaqbanking_includes += include_directories('..')
libaqbanking_includes += libgwenhywfar_inc
libaqbanking_includes += root_includes

libaqbanking_defines = ab_defines


libaqbanking_src = 'dummy.c'




libaqbanking = shared_library('aqbanking', libaqbanking_src,
                              version : ab_so_version,
                              include_directories : libaqbanking_includes,
                              c_args : libaqbanking_defines,
                              install : true,
                              link_with: [libabbase, libabplugins],
                              dependencies : [libgwenhywfar, libgmp],
)


