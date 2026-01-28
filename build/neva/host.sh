#!/usr/bin/bash
# to prevent siso add this directory to sysroot
# see siso/toolsupport/gccutil/scandepsparams.go
# ExtractScanDepsParams

compiler="$1"
shift
case "$compiler" in
  cxx)
    exec /usr/bin/g++-13 "$@";;
  cc)
    exec /usr/bin/gcc-13 "$@";;
  *)
    exit 1;;
esac
