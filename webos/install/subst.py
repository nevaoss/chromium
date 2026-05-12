#!/usr/bin/env python
import sys
import json


def main(subst, src, dst):
    table = tuple(s.split("=") for s in subst.split(":"))

    with open(src) as fsrc, open(dst, "w") as fdst:
        for line in fsrc:
            for k, v in table:
                line = line.replace(k, v)
            fdst.write(line)


if  __name__ == "__main__":
    main(*sys.argv[1:])
