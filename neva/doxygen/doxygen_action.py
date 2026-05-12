#!/usr/bin/env python

# Copyright 2021 LG Electronics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# SPDX-License-Identifier: Apache-2.0

"""Wrapper script to run doxygen command as action with gn."""

import subprocess
import sys

EXIT_FAILURE = 1
EXIT_SUCCESS = 0

def main():

  args = sys.argv[1:]
  if len(args) < 1:
    sys.stderr.write('usage: %s DOXYFILEPATH\n' % sys.argv[0])
    sys.exit(EXIT_FAILURE)

  try:
      subprocess.call([ "doxygen" ] + args)
  except OSError:
      return EXIT_SUCCESS

if __name__ == '__main__':
  sys.exit(main())
