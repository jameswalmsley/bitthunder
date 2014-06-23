#!/bin/bash
#
# This script allows all symbols in a compiled kernel to be listed.
# You can filter the results with grep:
#
# List all exported symbols:
#
#  $ ./scripts/symbol_status.sh [kernel_elf_file] | grep -v UNEXPORTED
#
# List all unexported symbols:
#
#  $ ./scripts/symbol_status.sh [kernel_elf_file] | grep UNEXPORTED
#
#  @author James Walmsley <james@fullfat-fs.co.uk>
#

SYMBOLS=$(nm $1 | grep "[bB][tT]" | cut -d " " -f3)
EXPORTED=$(git grep BT_EXPORT_SYMBOL | cut -d \( -f2 | cut -d \) -f1)

for SYMBOL in $SYMBOLS; do
	FOUND=0

	for EXPORT in $EXPORTED; do
		if [[ $SYMBOL = $EXPORT ]]; then
			FOUND=1
			break
		fi
	done

 	if [[ $FOUND = "1" ]] ; then
 		echo "EXPORTED:$SYMBOL"
 	else
 		echo "UNEXPORTED:$SYMBOL"
 	fi

done
