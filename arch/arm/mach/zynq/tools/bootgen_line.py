from __future__ import print_function

import shutil
import os

attributes = set(['bootloader', 'init', 'alignment', 'offset'])

def syntax_error(line, lineno, column, message):
    print("Syntax error on line %d (%s):" % (lineno, message))
    print(">> %s" % line)
    print(">> %s^" % (" " * column))

class image_item:
    name = "";
    attribute = None
    line = ""
    linenum = 0
    binfile = ""

def parse_line(line, linenum):
    item = image_item()
    item.line = line
    item.linenum = linenum

    if '[' and ']' in line:
        attribute = line.split(']')[0].split('[')[1].strip()
        line = line.split(']')[1].strip()
        if attribute in attributes:
            item.attribute = attribute
        else:
            syntax_error(item.line, linenum, item.line.find(attribute), "Invalid/unsupported attribute!")
            exit(1)
        

    item.name = line;

    return item

def preprocess_init_data(name):
    try:
        file = open(name, 'r')
        output_file = open("%s.S" % name, 'w')
    except IOError:
        print("Cannot open %s" % name)
        exit(1)

    for line in file:
        if ".set." in line:
            address = line.split('=')[0].split()[1];
            value   = line.split('=')[1].strip()
            print(".word %s" % address, file=output_file)
            print(".word %s" % value, file=output_file)
        else:
            print(line, file=output_file)

    output_file.close
        

def process_init_data(item):
    preprocess_init_data(item.name)         
    os.system("%s-gcc -c %s.S -o %s.o" % (os.environ["TOOLCHAIN"], item.name, item.name))
    os.system("%s-objcopy --only-section=.text -O binary %s.o %s.bin" % (os.environ["TOOLCHAIN"], item.name, item.name))
    item.binfile = "%s.bin" % item.name

def process_elf(item):
    os.system("%s-objcopy -O binary %s %s.bin" % (os.environ["TOOLCHAIN"], item.name, item.name))
    item.binfile = "%s.bin" % item.name

def process_item(item):
    if item.attribute == "init":
        process_init_data(item)
    if ".elf" in item.name:
        process_elf(item)
    if ".bit" in item.name:
        print("WARNING: .bit bitstreams currently unsupported")
        print("WARNING: %d : %s - NOT IN IMAGE" % (item.linenum, item.line))

