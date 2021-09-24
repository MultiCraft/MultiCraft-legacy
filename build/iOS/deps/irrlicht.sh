#!/bin/bash -e

. sdk.sh

[ ! -d irrlicht-src ] && \
	git clone -b ogl-es-monte48 --recurse-submodules --depth 1 https://github.com/kakashidinho/irrlicht irrlicht-src

cd irrlicht-src/source/Irrlicht
xcodebuild build ARCHS=arm64 \
	-project Irrlicht.xcodeproj \
	-configuration Release \
	-scheme Irrlicht_iOS \
	-destination generic/platform=iOS
cd ../..

[ -d ../irrlicht ] && rm -r ../irrlicht
mkdir -p ../irrlicht
cp lib/iOS/libIrrlicht.a ../irrlicht/
cp -r include ../irrlicht/include
cp -r media/Shaders ../irrlicht/shaders

echo "Irrlicht build successful"
