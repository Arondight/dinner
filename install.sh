#!/usr/bin/env bash
# ==============================================================================
# This to build project and do install
# ==============================================================================

RDIR=$(dirname $(readlink -f $0))
BUILDDIR=${RDIR}/build
BUILD_SH="${RDIR}/build.sh"

function install ()
{
  if [[ -x $BUILD_SH ]]
  then
    command $BUILD_SH $@
  fi

  if [[  0 -eq $? ]]
  then
    cd $BUILDDIR  \
      && sudo make install -j4
  fi

  return $?
}

install $@

exit $?

