#!/usr/bin/env python

import os, sys
import prettyformat
import fileinput
import unicodedata

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
    line = line.rstrip()
    line = line.lstrip()
    line = line.decode('utf8')
    
    if("removed" in line):
        description = line.split(" ")[-1][1:][:-1]
        description = description.split(os.getcwd()).pop()
        if description[0] == '/':
            description = description[1:]
    else:
        description = line

    prettyformat.pretty(opcode, module, description, bCustom)
