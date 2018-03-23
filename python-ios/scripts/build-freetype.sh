#!/bin/bash

. $(dirname $0)/environment.sh

if [ ! -f $CACHEROOT/freetype-$FT_VERSION.tar.gz ]; then
	try curl -L http://downloads.sourceforge.net/project/freetype/freetype2/$FT_VERSION/freetype-$FT_VERSION.tar.gz > $CACHEROOT/freetype-$FT_VERSION.tar.gz
fi
if [ ! -d $TMPROOT/freetype-$FT_VERSION ]; then
	try tar xvf $CACHEROOT/freetype-$FT_VERSION.tar.gz
	try mv freetype-$FT_VERSION $TMPROOT
fi

pushd $TMPROOT/freetype-$FT_VERSION
try ./configure --prefix=$DESTROOT \
	--host=arm-apple-darwin \
	--enable-static=yes \
	--enable-shared=no \
	CC="$ARM_CC" AR="$ARM_AR" \
	LDFLAGS="$ARM_LDFLAGS" CFLAGS="$ARM_CFLAGS"
try make clean
try make
try make install

# copy to buildroot
cp objs/.libs/libfreetype.a $BUILDROOT/lib/libfreetype.a
cp -a include $BUILDROOT/include/freetype

popd
