#!/bin/sh
# This script is used on https://travis-ci.org for installing the gwen
# dependency

# Copied from https://docs.travis-ci.com/user/installing-dependencies/

set -ex
GWEN_DOWNLOAD_NUM=${1}
GWEN_VERSION=${2}
GWEN_TGZ=gwenhywfar-${GWEN_VERSION}.tar.gz
wget https://www.aquamaniac.de/rdm/attachments/download/${GWEN_DOWNLOAD_NUM}/${GWEN_TGZ}
tar -xzf ${GWEN_TGZ}
cd gwenhywfar-${GWEN_VERSION} && \
  ./configure --with-guis="qt4 gtk2 gtk3" --prefix=/usr && \
  make && \
  sudo make install
