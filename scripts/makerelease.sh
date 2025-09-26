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
CLEAN=
PKG=Y

if [[ $1 == "-?" ]] ; then
	echo "Usage: $0 <release_version> [localhost|dest_hostname] [copy]"
	exit;
fi 

if [[ $1 == "" ]] ; then
	PKG=N
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
	make -f Makefile all
	if [ $? != 0 ]; then
		echo "-> Error: compile failed"
		exit 1
	fi

	if [ $HOSTTYPE != "arm" ]; then
		echo ""
		echo "-> Cross compiling nimrod for armv7"
		make -f Makefile.arm all
		if [ $? != 0 ]; then
			echo "-> Error: cross compile failed"
			exit 1
		fi
	fi

	echo ""
	echo "-> Compiling on arm64"
	scp *.cpp gateway:~/build/bin/.
	scp *.h gateway:~/build/bin/.
	scp Makefile gateway:~/build/bin
	ssh gateway "cd build/bin; make -f Makefile"
	scp gateway:~/build/bin/nimrod ./nimrod-arm64
	if [ $? != 0 ]; then
		echo "-> Error: arm64 build failed"
		exit 1
	fi
	scp gateway:~/build/bin/nimrod-msg ./nimrod-msg-arm64
	if [ $? != 0 ]; then
		echo "-> Error: arm64 build failed"
		exit 1
	fi
	scp gateway:~/build/bin/set-address ./set-address-arm64
	if [ $? != 0 ]; then
		echo "-> Error: arm64 build failed"
		exit 1
	fi

	cd ..

	if [[  $PKG = "Y" ]]; then
		echo "";
		echo "-> Packaging Nimrod version $1";
	
		echo "-> Make scripts executable"
		chmod 0755 scripts/*.sh
	fi	
fi

if [[ $PKG = "Y" ]]; then
	echo "-> Create tar package file"
	tar -cvz --exclude=makerelease.sh --exclude=site_config.php --exclude=site_config.php.save --exclude="*~" --exclude=no.upgrade --exclude=no.tunnel \
		--exclude=releases --exclude=archive --exclude="*.cpp" --exclude="*.h" --exclude="*.o" --exclude="*.d" \
		--exclude="*.c" --exclude="Makefile*" --exclude="readconfig.txt" --exclude="*.svn*" --exclude="*.tgz" --exclude=tmp --exclude=websocketpp \
		--exclude=".*" --exclude="bin/obj" --exclude="bin/objarm" --file=${SRCFILE} *
fi

if [[ $COPY = 1 ]]; then
	echo "-> Skipping compile step"
fi

if [[ ! -f $SRCFILE && $PKG = "Y" ]]; then
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
		elif [ ${CC} == "geotechpi" ]; then
			scp -P 2222 ${SRCFILE} ${PI}:/var/www/html
			if [ $? != 0 ]; then
				echo "-> Error: failed to copy new package to ${PI}"
			fi
		elif [ ${CC} == "192.168.0.126" ]; then
			scp ${SRCFILE} ${PI}:/var/www/html
			if [ $? != 0 ]; then
				echo "-> Error: failed to copy new package to ${PI}"
			fi
		else
			scp ${SRCFILE} ${PI}:/var/www/html 2>/dev/null
			if [ $? != 0 ]; then
				scp ${SRCFILE} ${PI}:/var/www/flatcatit.co.nz
				if [ $? != 0 ]; then
					echo "-> Error: failed to copy new package to ${PI}"
				fi
			fi
		fi
	done
else
	echo "-> Pi hostname(s) not set, skipping scp"
fi


echo "";

echo "-> Done.";
