#!/usr/bin/env python
import sys
import shutil
import os

def main(*files):
    dst = files[-1]
    rest = files[:-1]

    if len(rest) > 1 and not os.path.isdir(dst):
        print >> sys.stderr, "%s must be a directory" % dst
        sys.exit(-1)

    for src in rest:
        if os.path.isdir(src):
            continue

        shutil.copy(src, dst)
        os.chmod(os.path.join(dst, src) if os.path.isdir(dst) else dst, 0o755)


if  __name__ == "__main__":
    main(*sys.argv[1:])
