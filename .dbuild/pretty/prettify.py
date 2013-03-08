#!/usr/bin/env python

import os, sys
import prettyformat
import fileinput

bCustom = True
opcode = "??"
module = "Unknown"
description = "Somebody please fix this!"
offset = 0

if(sys.argv[1] == "--dbuild"):
    bCustom = False
    offset = 1;

if(len(sys.argv) > 1+offset):
    opcode = sys.argv[1+offset]

if(len(sys.argv) > 2+offset):
    module = sys.argv[2+offset]


for line in sys.stdin:
    line = line.strip()
    if("removed" in line):
        description = line.split("`")[1]
        description = description.split("'")[0]
        description = description.split(os.getcwd()).pop()
        #description = os.path.dirname(description)
    else:
        description = line

    prettyformat.pretty(opcode, module, description, bCustom)
