#!/bin/bash

# check if work folder is passed
if [ -z "$1" ]
  then
  	echo "No workingFolder specified"
  	exit 1
fi

# check if package name is passed
if [ -z "$2" ]
  then
  	echo "No package specified"
  	exit 1
fi

# get locale LANG variable
language=$(locale | grep LANG | cut -d= -f2)
lang=$(echo $language | cut -d_ -f1)

# create dir structure
mkdir -p $1/po/$lang/LC_MESSAGES

# generate file list
echo -e "" > $1/po/files.txt
find $1 -type f \( -iname "*.c" -or -iname "*.h" \) -exec readlink -f "{}" \; > $1/po/files.txt

# scan files
xgettext --keyword=_ --force-po --add-comments --from-code=UTF-8 --language=C --package-name=$2 --package-version=1.0 --msgid-bugs-address=Sandroid75@github.com -o $1/po/$2.pot -f $1/po/files.txt

# remove file list
rm $1/po/files.txt

# initialize PO or merge existing

if [ -e "$1/po/$lang/$2.po" ]
	then
		echo Mergeing existing PO files
		find $1 -type f -iname "*.po" -exec bash -c 'msgmerge --update "{}" '$1'/po/'$2'.pot' \;
	else
		echo Initializing new PO files
		msginit --no-translator --input=$1/po/$2.pot --locale=$language --output=$1/po/$lang/$2.po
fi

# compile .po to .mo
find $1 -type f -iname "*.po" -exec bash -c 'msgfmt "$( dirname "{}")/'$2'.po" -o "$( dirname "{}")/LC_MESSAGES/'$2'.mo"' \;

