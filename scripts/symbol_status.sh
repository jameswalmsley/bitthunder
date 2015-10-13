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

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
SYMBOLS=$(readelf --wide -s $1 | grep " GLOBAL " | grep "[bB][tT]" | rev | cut -d " " -f1 | rev | sort)
EXPORTED=$(git --git-dir=$2/.git --work-tree=$2 grep BT_EXPORT_SYMBOL | cut -d \( -f2 | cut -d \) -f1 | sort)
IGNORED=$(cat $DIR/symbol_status.ignore | cut -d : -f1)

for SYMBOL in $SYMBOLS; do
	FOUND=0
	IGN=0

	for EXPORT in $EXPORTED; do
		if [[ $SYMBOL = $EXPORT ]]; then
			FOUND=1
			break
		fi
	done

	for IGNORE in $IGNORED; do
		if [[ $IGNORE = $SYMBOL ]]; then
			IGN=1
		fi
	done

	if [[ $IGN = "1" ]] ; then
		continue
	fi

 	if [[ $FOUND = "1" ]] ; then
 		echo "EXPORTED:$SYMBOL"
 	else
 		echo "UNEXPORTED:$SYMBOL"
 	fi

done
