#!/bin/sh -v
CPPFLAGS='-I/usr/local/include/db48/ -I/usr/local/include'
LDFLAGS='-L/usr/local/lib/db48/ -L/usr/local/lib'
CFLAGS=$CPPFLAGS
CXXFLAGS=$CPPFLAGS

export LDFLAGS CPPFLAGS CFLAGS CXXFLAGS

#./configure
./configure --enable-debug
#./configure --enable-debug --with-libs 
