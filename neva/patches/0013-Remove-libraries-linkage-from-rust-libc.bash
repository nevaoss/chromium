#!/bin/bash

filename=$(find src/third_party/rust-toolchain/lib/rustlib/src/rust/library/vendor -wholename *src/unix/mod.rs)
path_to_dir=$(dirname "$filename")

if [ -f "${filename}.orig" ]; then
    echo "---------------- Restoring mod.rs from backup ----------------"
    cp "${filename}.orig" "$filename"
else
    echo "---------------- Creating original backup of mod.rs ----------------"
    cp "$filename" "${filename}.orig"
fi

echo "---------------- MACHINE is $MACHINE (please, use new terminal if it's a new MACHINE) ----------------"

if [ "$MACHINE" == "qemux86-64" ]; then
    path_to_patch="$PWD/src/neva/patches/0014-build-webos-Rust-Remove-lib-m-c-rt-from-linked-libra.patch"
elif [ "$MACHINE" == "raspberrypi4-64" ]; then
    path_to_patch="$PWD/src/neva/patches/0013-Remove-dl-linkage-from-rust-libc.patch"
else
    # Do nothing if the machine is not supported or not set. (e.g. PC build)
    exit 0
fi

patch --directory="$path_to_dir" --input="$path_to_patch" 
