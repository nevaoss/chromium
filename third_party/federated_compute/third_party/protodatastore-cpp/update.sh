#!/bin/bash
# Copyright 2026 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

set -e
shopt -s extglob dotglob
SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

# Get the repo URL and hash from  README.chromium
PDS_REPO=$(sed -n 's/^URL: \(.*\)/\1/p' "${SCRIPT_DIR}/README.chromium")
PDS_REV=$(sed -n 's/^Revision: \(.*\)/\1/p' "${SCRIPT_DIR}/README.chromium")

if [[ -z "${PDS_REPO}" || -z "${PDS_REV}" ]]; then
  echo "Error: Could not extract URL or Revision from README.chromium"
  exit 1
fi

echo "Using repo ${PDS_REPO}"
echo "Using revision ${PDS_REV}"

function check_out() {
  git init
  git remote add upstream ${PDS_REPO}
  git fetch --depth 1 upstream "${PDS_REV}"
  git checkout FETCH_HEAD 2>/dev/null
}

pushd "${SCRIPT_DIR}/"

rm -rf .tmp-checkout
mkdir .tmp-checkout
pushd .tmp-checkout

check_out

popd # .tmp-checkout

rm -rf src/
mkdir src/
cp -r .tmp-checkout/!(.git) src/
rm -rf .tmp-checkout/

popd # ${SCRIPT_DIR}/
