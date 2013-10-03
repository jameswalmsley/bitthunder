#!/bin/bash

depsdir() {
	echo -n "$1.stamp: "; shift
	for d; do
		( cd "$d"; git ls-files | grep -v '[\"#]' | sed "s,^,$d/,"; )
	done | sort | sed 'x; s/ /\\ /g; /./ { s,$, ,; }; s,$,\\,; $ { p; x; }' 
	echo
}

echo
depsdir $@
