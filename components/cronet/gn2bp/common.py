# Copyright 2025 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
import os

def is_rust_build_script(script: str) -> bool:
    return script == "//build/rust/gni_impl/run_build_script.py"


def is_supported_source_file(name):
    """Returns True if |name| can appear in a 'srcs' list."""
    return os.path.splitext(name)[1] in [
        '.c', '.cc', '.cpp', '.java', '.proto', '.S', '.aidl', '.rs'
    ]


CPP_VERSION = 'c++17'
tethering_apex = "com.android.tethering"
