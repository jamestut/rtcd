#!/usr/bin/env zsh

set -e

# run this script on Ubuntu arm64 host
# the following Ubuntu packages have to be installed:
# - build-essential
# - gcc-x86-64-linux-gnu

if [[ $(uname -m) != 'aarch64' ]]
then
    echo "This script needs to be run on Arm64 Linux."
    exit 1
fi

function prefix {
    if [[ -z $1 ]]
    then
        echo $2
    else
        echo "$1-$2"
    fi
}

ARCH_LIST=(aarch64 x86_64)
ARCH_PREFIX=("" x86_64-linux-gnu)

TOOLCHAIN=(gcc strip)
EXE_TO_CHECK=(make zip)
for AP in "${ARCH_PREFIX[@]}"
do
    for TC in "${TOOLCHAIN[@]}"
    do
        EXE_TO_CHECK+=$(prefix "$AP" "$TC")
    done
done

for I in $EXE_TO_CHECK
do
    if ! which $I > /dev/null
    then
        echo "Executable $I not found"
        echo "The following executables are required:"
        for II in $EXE_TO_CHECK
        do
            echo " - $II"
        done
        exit 1
    fi
done

export CFLAGS="-O2 -static -g0"
OUT_EXES=(rtcd rtcctl)

for I in {1..${#ARCH_LIST[@]}}
do
    ARCH=${ARCH_LIST[$I]}
    export CC=$(prefix "${ARCH_PREFIX[$I]}" gcc)
    echo "Building for $ARCH ..."
    make clean
    make -j$(nproc)
    for OE in $OUT_EXES
    do
        $(prefix "${ARCH_PREFIX[$I]}" strip) "$OE"
    done
    zip rtcd-glibc-static-$ARCH.zip $OUT_EXES
done
