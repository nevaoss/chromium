#!/bin/bash

path_to_dir=$(dirname $(find src/third_party/rust-toolchain/lib/rustlib/src/rust/library/vendor -wholename *src/unix/mod.rs))
path_to_patch="$PWD/src/neva/patches/0013-Remove-dl-linkage-from-rust-libc.patch"

patch --forward --reject-file=- --directory="$path_to_dir" --input="$path_to_patch" ||
patch --dry-run --reverse --directory="$path_to_dir" --input="$path_to_patch"
