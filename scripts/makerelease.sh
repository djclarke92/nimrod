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
	echo "Usage: $0 <release_version> [localhost|dest_hostname] [copy]"
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
	echo "-> Compiling nimrod for linux $HOSTTYPE"
	make -f Makefile clean all
	if [ $? != 0 ]; then
		echo "-> Error: compile failed"
		exit 1
	fi

	if [ $HOSTTYPE != "arm" ]; then
		echo ""
		echo "-> Cross compiling nimrod for armv7"
		make -f Makefile.arm clean all
		if [ $? != 0 ]; then
			echo "-> Error: cross compile failed"
			exit 1
		fi
	fi

	cd ..

	echo "";
	echo "-> Packaging Nimrod version $1";

	echo "-> Make scripts executable"
	chmod 0755 scripts/*.sh
	
fi

echo "-> Create tar package file"
tar -cvz --exclude=makerelease.sh --exclude=site_config.php --exclude=site_config.php.save --exclude="*~" --exclude=no.upgrade --exclude=no.tunnel \
		--exclude=releases --exclude=archive --exclude="*.cpp" --exclude="*.h" --exclude="*.o" --exclude="*.d" \
		--exclude="*.c" --exclude="Makefile*" --exclude="readconfig.txt" --exclude="*.svn*" --exclude="*.tgz" --exclude=tmp \
		--exclude=".*" --file=${SRCFILE} *

if [[ $COPY = 1 ]]; then
	echo "-> Skipping compile step"
fi

if [[ ! -f $SRCFILE ]]; then
	echo "-> Error: $SRCFILE is missing"
	exit 1
fi

echo ""
if [[ "$DEST" != 0 ]]; then
	for CC in $DEST; do
		PI=nimrod@$CC
		echo "-> Copying new version to ${PI}"
		if [ $CC = "localhost" -a -d /var/www/html/nimrod ]; then
			scp ${SRCFILE} ${PI}:/var/www/html/nimrod
			if [ $? != 0 ]; then
				echo "-> Error: failed to copy new package to ${PI}"
			fi
		else
			scp ${SRCFILE} ${PI}:/var/www/html
			if [ $? != 0 ]; then
				echo "-> Error: failed to copy new package to ${PI}"
			fi
		fi
	done
else
	echo "-> Pi hostname(s) not set, skipping scp"
fi


echo "";

echo "-> Done.";
