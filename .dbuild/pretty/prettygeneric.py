#!/usr/bin/python

import os, sys
import prettyformat
import fileinput

module = sys.argv[1]

line = sys.stdin.readline()
while(line):
    valid=False
    if("libtool:" in line):
        if("libtool: compile:" in line):
            action = "CC"
            description = line.split("-o")[1].strip().split(" ")[0].strip()
            valid = True
        elif("libtool: link:" in line):
            action = "LN"
            splits = line.split("-o")
            if(len(splits) > 1):
                description = splits[1].strip()
                valid = True
            else:
                description = "intermediate step"
                valid = False

        else:
            action = "UKNOWN"
            description = "LIBTOOL STEP"
    elif("libtool" and "--mode=" in line):
        if("--mode=compile"):
            action = "CC"
			#description = line.split("-o")[1].split(" ")[1].strip()
            valid = True
        else:
            print "LIBTOOL HORROR"
    else:
        stripped = line.strip()
        if(stripped.startswith("rm")):
            rm_split = stripped.split("rm")[1].split(" ")
            if(rm_split[1].strip().startswith("-")):
                rm_split = rm_split[2:]

            for split in rm_split:
                if(split.strip() != ""):
                    prettyformat.pretty("RM", module, split, False)

        elif(stripped.split(" ")[0].endswith("gcc")):
            action = "CC"
            splits = stripped.split("-o")
            if(len(splits) > 1):
                description = splits[1].split(" ")[1].strip()
                valid = True
        elif(stripped.split(" ")[0].endswith("c++")):
            action = "CXX"
            description = stripped.split("-o")[1].split(" ")[1].strip()
            valid = True
        elif(stripped.split(" ")[0].endswith("g++")):
            action =  "CXX"
            description = stripped.split("-o")[1].split(" ")[1].strip()
            valid = True
        elif(stripped.split(" ")[0].endswith("ar")):
            action = "AR"
            description = stripped.split(".a")[0] + ".a"
            valid = True
        elif(stripped.split(" ")[0].endswith("nm")):
            action = "SYMS"
            print stripped
            splits = stripped.split("-o")
            if(len(splits) < 2):
                splits = stripped.split("-p")
            description = splits[1].split(" ")[1].strip()
            valid = True
        elif(stripped.startswith("checking")):
            action = "CHK"
            description = stripped.split("checking")[1]
            valid = True
        elif(stripped.startswith("make[") and ("Nothing to be done for" in stripped)):
                 valid = False
        elif(stripped.split(" ")[0].endswith("moc")):
            action = "MOC"
            description = stripped.split("-o")[1].split(" ")[1].strip()
            valid = True
        else:
            action = "MISC"
            description = line.strip()
            valid = True

    if(valid == True):
        prettyformat.pretty(action, module, description, False)

    line = sys.stdin.readline()
