#!/bin/bash

# TODO: this should probably be a makefile
# TODO: install target
(
moongendir=$(pwd)

cd $(dirname "${BASH_SOURCE[0]}")
cd deps/luajit
if [[ ! -e Makefile ]]
then
	echo "ERROR: LuaJIT submodule not initialized"
	echo "Please run git submodule update --init"
	exit 1
fi
#install luajit
#make clean
make -j 8 'CFLAGS=-DLUAJIT_NUMMODE=2 -DLUAJIT_ENABLE_LUA52COMPAT'
make install DESTDIR=$(pwd)
#install dpdk
cd $moongendir/deps/dpdk
make -j 8 install T=x86_64-native-linuxapp-gcc
#../../bind-interfaces.sh
/bin/bash tools/setup_iface_single_process.sh 3
#install mtcp
echo "MAKING MTCP..."
cd  $moongendir/deps/mtcp
./configure --with-dpdk-lib=$moongendir/deps/dpdk/x86_64-native-linuxapp-gcc
cd mtcp/src
make clean
make
#cd ../../util
#make
#MAKE MOONGEN
cd $moongendir/build
make clean
cmake ..
make
)

