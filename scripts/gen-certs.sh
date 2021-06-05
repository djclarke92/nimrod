#!/bin/bash

IP="192.168.0.1"
EMAIL=djclarke@flatcatit.co.nz
COUNTRY=NZ
STATE=Buller
CITY=Charleston
COMPANY=FlatCatIT


SUBJECT_CA="/C=$COUNTRY/ST=$STATE/L=$CITY/O=$COMPANY/OU=CA/emailAddress=$EMAIL/CN=$IP"
SUBJECT_SERVER="/C=$COUNTRY/ST=$STATE/L=$CITY/O=$COMPANY/OU=Server/emailAddress=$EMAIL/CN=$IP"
SUBJECT_CLIENT="/C=$COUNTRY/ST=$STATE/L=$CITY/O=$COMPANY/OU=Client/emailAddress=$EMAIL/CN=$IP"

function generate_CA () {
   echo "$SUBJECT_CA"
   openssl req -x509 -nodes -sha256 -newkey rsa:2048 -subj "$SUBJECT_CA"  -days 3650 -keyout nimrodCA.key -out nimrodCA.pem
}

function generate_server () {
   echo "$SUBJECT_SERVER"
   openssl req -nodes -sha256 -new -subj "$SUBJECT_SERVER" -keyout nimrod-cert.key -out nimrod-cert.csr
   openssl x509 -req -sha256 -in nimrod-cert.csr -CA nimrodCA.pem -CAkey nimrodCA.key -CAcreateserial -out nimrod-cert.pem -days 3650
}

function generate_client () {
   echo "$SUBJECT_CLIENT"
   openssl req -new -nodes -sha256 -subj "$SUBJECT_CLIENT" -out client.csr -keyout client.key 
   openssl x509 -req -sha256 -in client.csr -CA nimrodCA.pem -CAkey ca.key -CAcreateserial -out client.pem -days 3650
}


generate_CA
generate_server
#generate_client

