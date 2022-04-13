#!/bin/bash

CXX="${CXX:-gcc}"
SRC_APP="sha256.c "
FLAGS="-Wall -Wextra -pedantic -ggdb"
LIBS=""
OUT=""

set -xe

if [[ $# -lt 1 ]]
then
    echo "Usage: $(basename "$0") [server/client]"
    exit 0
else
    if [[ "$1" == "--server" ]]
    then
	SRC_APP+="server.c"
	OUT="server"
    elif [[ "$1" == "--client" ]]
    then
	SRC_APP+="client.c"
	OUT="client"
    fi
fi  

$CXX $SRC_APP $FLAGS -o $OUT $(pkg-config --cflags --libs $LIBS)

if [[ $? -ne 0 ]]
then
    echo "ERROR: Compile failed!"
    exit 1
else
    echo "Compile succesfully!"
    exit 0
fi
