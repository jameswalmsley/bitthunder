#!/usr/bin/python

import os, sys
import prettyformat
import fileinput

action = sys.argv[1]
module = sys.argv[2]

line = sys.stdin.readline()

while(line):
    valid = True
    if("Compiling " in line):
        action = "CC"
    elif("Linking " in line):
        action = "LN"
    elif("checking " in line):
        action = "CHECK"
    elif("Building " in line):
        action = "BUILD"
    elif("Using FLAGS" in line):
        valid = False
    elif(line.startswith("rm")):
        line = line.split("rm")[1]

        while(line.strip().endswith("\\")):
            rm_splits = line.strip().split(" ")
            if(len(rm_splits) == 0):
                break
            if(rm_splits[0].startswith("-")):
                rm_splits = rm_splits[1:]

            for item in rm_splits:
                if(item.strip() != "" and item.strip() != "\\"):
                    prettyformat.pretty("RM", module, item.strip(), False)

            line = sys.stdin.readline()

        rm_splits = line.strip().split(" ")
        if(len(rm_splits) != 0):
            if(rm_splits[0].startswith("-")):
                rm_splits = rm_splits[1:]

                for item in rm_splits:
                    if(item.strip() != "" and item.strip() != "\\"):
                        prettyformat.pretty("RM", module, item.strip(), False)

        valid = False


    else:
        action = "CONF"

    if(valid == True):
        newline = line.split("\n")[0]
        if(action == "CC"):
            newline = newline.split("Compiling ")[1]
        elif(action == "LN"):
            newline = newline.split("Linking ")[1]
        elif(action == "BUILD"):
            newline = newline.split("Building ")[1]
        elif(action == "CONF"):
            newline = newline.strip()

        prettyformat.pretty(action, module, newline.strip(), False)

    line = sys.stdin.readline()

