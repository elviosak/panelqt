#!/bin/bash

mkdir -p build && cd build
#rm -r *
qmake .. && make && make install INSTALL_ROOT=appdir
wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
./linuxdeploy-x86_64.AppImage --output appimage --plugin qt --appdir appdir
