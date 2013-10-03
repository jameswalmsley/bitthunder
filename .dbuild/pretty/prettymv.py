#!/usr/bin/python

import os, sys
import prettyformat
import fileinput

command     = "MV"
module      = "Unknown"
description = "Please fix this somebody!"
bCustom     = True
offset      = 0

if(len(sys.argv) >= 2):
    if(sys.argv[1] == "--dbuild"):
        offset = 1
        bCustom = False

if(len(sys.argv) >= 2+offset):
    command = sys.argv[1+offset]

if(len(sys.argv) >= 3+offset):
    module = sys.argv[2+offset]

for line in sys.stdin:
    line = line.strip()
    line = line.rstrip()
    line = line.lstrip()
    line = line.decode('utf8')

    source = line.split(" ")[0][1:][:-1]
    dest = line.split(" ")[2][1:][:-1]

    prettyformat.pretty(command, module, source + " -> " + dest, bCustom)
