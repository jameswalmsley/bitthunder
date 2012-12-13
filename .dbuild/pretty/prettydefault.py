#!/usr/bin/python

import os, sys
import prettyformat
import fileinput

module = sys.argv[1]

line = sys.stdin.readline()
while(line):
	print module + ": " + line.strip()
	line = sys.stdin.readline()

