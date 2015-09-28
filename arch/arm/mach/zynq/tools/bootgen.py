#
# BootGen in python :)
#

import sys
import os
import subprocess
import bootgen_line as bootgen

if len(sys.argv) != 2:
    print "Usage: bootgen.py [bif-file].bif"
    exit(1)

file = open(sys.argv[1], 'r')

# Parse the file

linenum = 0
image_name = None
brace = 0
late_close_brace = False
bootloader = None
init_data = None

item_list = []

for line in file:
    linenum += 1
    if line.strip().startswith("#"):
        continue

    if image_name == None and ':' in line:
        image_name = line.split(':')[0].strip()

    if '{' in line:
        brace += 1
        line = line.split('{')[1].strip()

    if '}' in line:
        line = line.split('}')[0].strip()
        brace -= 1

    if len(line) > 0 and brace > 0:
        image_item = bootgen.parse_line(line.strip(), linenum)
        item_list.append(image_item)
        if image_item.attribute == "bootloader":
            bootloader = image_item
        if image_item.attribute == "init":
            init_data = image_item

    if late_close_brace == True:
        late_close_brace = False
        brace -= 1


for item in item_list:
    bootgen.process_item(item)
    #print item.binfile.strip()

if bootloader == None:
    print("Error: no image was marked as the bootloader.");
    exit(1)

os.system("gcc bootgen.c -o bootgen")

p = subprocess.Popen(["readelf", bootloader.name, "-h"], stdout=subprocess.PIPE)
output = p.communicate()[0].split('\n')
for line in output:
    if "Entry point" in line:
        entry_address = line.split(':')[1].strip()

if init_data == None:
    os.system("./bootgen %s %s" % (bootloader.binfile, entry_address))
else:
    os.system("./bootgen %s %s %s" % (bootloader.binfile, entry_address, init_data.binfile))
