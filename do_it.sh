#! /bin/bash

rm -Rf zbar/
git clone https://salsa.debian.org/debian/zbar
cd zbar/
git checkout 5eab87f59794453d27cfe2116cfb5aff5126aaa4

cp -v ../fb.c ./zbar/window/
cp -v ../Makefile.am.inc ./zbar/

autoreconf -v --install -f

./configure --without-python --without-gtk --without-qt --without-java --without-x --with-gtk=no --with-x=no \
  --enable-doc=no --without-python2 \
  --enable-codes=ean,databar,code128,code93,code39,codabar,i25,qrcode

make V=1 -j $(nproc)

gcc -Wall -Wno-parentheses -g -O2 -o zbarcam/.libs/zbarcam zbarcam/zbarcam.o zbar/.libs/libzbar.a -lv4l2 -ljpeg -lpthread

ls -al ./zbar/.libs/libzbar.so
ldd ./zbar/.libs/libzbar.so
echo "---"
ls -al ./zbarcam/.libs/zbarcam
ls -hal ./zbarcam/.libs/zbarcam
ldd ./zbarcam/.libs/zbarcam
