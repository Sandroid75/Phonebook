#!/bin/bash

if [ -z "$1" ]
  then
  	echo "No workingFolder specified"
  	exit 1
fi

# create dir structure
mkdir -p $1/po/it/LC_MESSAGES

# generate file list
echo -e "" > $1/po/files.txt
find $1 -type f \( -iname "*.c" -or -iname "*.h" \) -exec readlink -f "{}" \; > $1/po/files.txt

# scan files
xgettext --keyword=_ --force-po --add-comments --from-code=UTF-8 --language=C --package-name=phonebook --package-version=1.0 --msgid-bugs-address=Sandroid75@github.com -o $1/po/phonebook.pot -f $1/po/files.txt

rm $1/po/files.txt

if [ -e "$1/po/it/phonebook.po" ]
	then
		echo Mergeing existing PO files
		find $1 -type f -iname "*.po" -exec bash -c 'msgmerge --update "{}" '$1'/po/phonebook.pot' \;
	else
		echo Initializing new PO files
		msginit --no-translator --input=$1/po/phonebook.pot --locale=it_IT.UTF-8 --output=$1/po/it/phonebook.po
fi

# compile .po to .mo
find $1 -type f -iname "*.po" -exec bash -c 'msgfmt "$( dirname "{}")/phonebook.po" -o "$( dirname "{}")/LC_MESSAGES/phonebook.mo"' \;

