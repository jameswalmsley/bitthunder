#!/bin/bash

#
#	This is the pre-commit hook!
#

#
#	Against which head are we commiting?
#
if git-rev-parse --verify HEAD >/dev/null 2>&1 ; then
    against=HEAD
else
	# In case of initial commit, use the magic sha from GIT to identify an empty tree object.
	against=4b825dc642cb6eb9a060e54bf8d69288fbee4904
fi


#
#	Remove trailing whitespaces...
#
#	This prevents commits containing unrelated differences due editors not removing trailing spaces.
#

# Find files with trailing whitespaces :(
for FILE in `exec git diff-index --check --cached $against -- | sed '/^[+-]/d' | sed -r 's/:[0-9]+:.*//' | uniq` ; do
	# Fix those lines!
	sed -i 's/[[:space:]]*$//' "$FILE"
	git add "$FILE"
done


#
#	All modifications complete :: Exit!
#

exit
