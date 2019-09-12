#!/bin/bash
#
#-----------------------------------------------------------------------
#
# Nimrod release package script for raspberry pi
#
#-----------------------------------------------------------------------
#
# script to roll up a new release of Nimrod

PI=nimrod@nimrod
SRCFILE=./nimrod-$1.tgz
DEST=0
COPY=0

if [[ $1 == "" ]] ; then
	echo "Usage: $0 <release_version> [dest_pi_hostname] [copy]";
	exit;
fi 

if [[ $2 != "" ]]; then
	DEST=$2
fi

if [[ $3 = "copy" ]]; then
	COPY=1
fi


if [[ $COPY = 0 ]]; then
	cd bin

	echo ""
	echo "-> Cross compiling nimrod for linux amd64"
	make -f Makefile clean all
	if [ $? != 0 ]; then
		echo "-> Error: compile failed"
		exit 1
	fi

	echo ""
	echo "-> Cross compiling nimrod for armv7"
	make -f Makefile.arm clean all
	if [ $? != 0 ]; then
		echo "-> Error: cross compile failed"
		exit 1
	fi

	cd ..

	echo "";
	echo "-> Packaging Nimrod version $1";

	echo "-> Make scripts executable"
	chmod 0755 scripts/*.sh

	echo "-> Create tar package file"
	tar -cvz --exclude=makerelease.sh --exclude=site_config.php --exclude=site_config.php.save --exclude="*~" --exclude=no.upgrade --exclude=no.tunnel \
		--exclude=releases --exclude=archive --exclude="*.cpp" --exclude="*.h" --exclude="*.o" --exclude="*.d" \
		--exclude="*.c" --exclude="Makefile*" --exclude="readconfig.txt" --exclude="*.svn*" --exclude="*.tgz" --exclude=tmp \
		--exclude=".*" --file=${SRCFILE} *
	
else
	echo "-> Skipping compile step"
	if [[ ! -f $SRCFILE ]]; then
		echo "-> Error: $SRCFILE is missing"
		exit 1
	fi
fi

echo ""
if [[ "$DEST" != 0 ]]; then
	for CC in $DEST; do
		PI=nimrod@$CC
		echo "-> Copying new version to ${PI}"
		scp ${SRCFILE} ${PI}:/var/www/html
		if [ $? != 0 ]; then
			echo "-> Error: failed to copy new package to ${PI}"
		fi
	done
else
	echo "-> Pi hostname(s) not set, skipping scp"
fi


echo "";

echo "-> Done.";
