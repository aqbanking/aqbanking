# "make installcheck" builds consumer.c against the installed AqBanking
# and runs it: once with the flags from aqbanking-config, once with the
# flags from pkg-config, and once with find_package(aqbanking) of CMake.
# This checks the installed headers, library, aqbanking-config,
# aqbanking.pc and CMake package files.

EXTRA_DIST = consumer.c CMakeLists.txt

CMAKE = cmake
CTEST = ctest
consumer_cmakedir = \
  $(libdir)/cmake/aqbanking-$(AQBANKING_VERSION_MAJOR).$(AQBANKING_VERSION_MINOR)

# The installed library and aqbanking.pc are not in the default search
# paths. Windows looks for DLLs in PATH.
consumer_env = \
  PKG_CONFIG_PATH="$(libdir)/pkgconfig$${PKG_CONFIG_PATH:+:$$PKG_CONFIG_PATH}" \
  LD_LIBRARY_PATH="$(libdir)$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}" \
  PATH="$(bindir):$$PATH"; \
  export PKG_CONFIG_PATH LD_LIBRARY_PATH PATH

# Builds consumer.c with the flags in $cflags and $libs as
# consumer-build/$name and runs it.
consumer_build = \
  $(CC) $(CFLAGS) $$cflags -o consumer-build/$$name$(EXEEXT) \
    $(srcdir)/consumer.c $(LDFLAGS) $$libs && \
  consumer-build/$$name$(EXEEXT)

# make exports the CC that ./configure writes into the Makefile to
# the commands it runs, and CMake would read that CC. With MSVC, that
# CC is Automake's compile wrapper with flags, which CMake cannot use
# as a compiler, so we must unset CC and let CMake find the compiler
# by itself. However, under MSYS2, CMake typically finds MinGW's cc
# before cl, so we must force CC=cl whenever ./configure detects MSVC.
if IS_MSVC
consumer_cmake_cc = CC=cl
endif

installcheck-local:
	rm -rf consumer-build
	$(MKDIR_P) consumer-build
	@echo "Building against the installed files with aqbanking-config"
	$(consumer_env); name=config; \
	  cflags=`$(bindir)/aqbanking-config --includes`; \
	  libs=`$(bindir)/aqbanking-config --libraries`; \
	  $(consumer_build)
	@echo "Building against the installed files with pkg-config"
	$(consumer_env); name=pkgconfig; \
	  cflags=`$(PKG_CONFIG) --cflags aqbanking`; \
	  libs=`$(PKG_CONFIG) --libs aqbanking`; \
	  $(consumer_build)
	@echo "Building against the installed files with CMake"
# Under "make -j distcheck" on macOS, the make that CMake runs finds
# the -j settings of the outer make in MAKEFLAGS and tries to take
# part in its parallel build, which fails with "read jobs pipe:
# Resource temporarily unavailable". So we must unset MAKEFLAGS,
# MFLAGS and MAKELEVEL for the CMake step.
	$(consumer_env); unset CC MAKEFLAGS MFLAGS MAKELEVEL; \
	  $(consumer_cmake_cc) $(CMAKE) -S $(srcdir) -B consumer-build/cmake \
	    -Daqbanking_DIR="$(consumer_cmakedir)" \
	    -DCMAKE_PREFIX_PATH="$(gwenhywfar_dir)" \
	    -DAQBANKING_EXPECTED_VERSION="$(AQBANKING_VERSION_STRING)" && \
	  $(CMAKE) --build consumer-build/cmake && \
	  cd consumer-build/cmake && \
	  $(CTEST) -C Debug --output-on-failure

clean-local:
	rm -rf consumer-build
