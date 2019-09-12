#!/bin/bash


FILE=build-number.c
PFILE=../version.txt

if [ -f ${FILE} ]; then
	echo "File '${FILE}' exists"
	
	NUM=$((`cat ${FILE} | awk '{print $4}'`+1))
	
	echo "int NIMROD_BUILD_NUMBER = ${NUM} ;" > ${FILE}

else
	echo "File '${FILE}' does not exist"

	echo "int NIMROD_BUILD_NUMBER = 1 ;" > ${FILE}

fi

echo "${NUM}" > ${PFILE}
	

