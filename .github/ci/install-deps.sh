#!/bin/sh
# Installs the build dependencies of AqBanking on a GitHub-hosted Ubuntu
# or macOS runner.
set -eu

case "$(uname -s)" in
  Linux)
    pkgs="autoconf automake libtool gettext pkg-config
          libgmp-dev libxml2-dev libxmlsec1-dev libxslt1-dev"
    sudo apt-get update -q
    # shellcheck disable=SC2086
    sudo DEBIAN_FRONTEND=noninteractive \
      apt-get install -y -q --no-install-recommends $pkgs
    ;;
  Darwin)
    pkgs="autoconf automake libtool gettext pkgconf
          gmp libxml2 libxmlsec1 libxslt"
    # shellcheck disable=SC2086
    brew install --quiet $pkgs
    # Makefile.cvs calls "libtoolize", which Homebrew installs as
    # "glibtoolize" with the plain name in libexec/gnubin.
    echo "$(brew --prefix libtool)/libexec/gnubin" >>"$GITHUB_PATH"
    # Homebrew's gettext keeps gettext.m4 (AM_GNU_GETTEXT) outside the
    # search path of aclocal.
    echo "ACLOCAL_PATH=$(brew --prefix gettext)/share/gettext/m4${ACLOCAL_PATH:+:$ACLOCAL_PATH}" >>"$GITHUB_ENV"
    # libxml2 and libxslt are keg-only, so pkg-config does not find them
    # by default.
    pc="$(brew --prefix libxml2)/lib/pkgconfig:$(brew --prefix libxslt)/lib/pkgconfig"
    echo "PKG_CONFIG_PATH=$pc${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" >>"$GITHUB_ENV"
    ;;
  *)
    echo "Unsupported system: $(uname -s)" >&2
    exit 1
    ;;
esac
