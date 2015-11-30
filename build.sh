#!/usr/bin/env bash
# ==============================================================================
# This to build project
# ==============================================================================

curdir=$(dirname $(readlink -f $0))
builddir=$curdir/build

function build {
  mkdir -p $builddir  \
    && cd $builddir \
    && cmake -DCMAKE_BUILD_TYPE=Release ..  \
    && make -j4

  return $?
}

function install {
  cd $builddir  \
    && sudo make install -j4

  return $?
}

build

if [[ 0 -eq $? && 'install' == $1 ]]; then
  install
fi

exit $?
