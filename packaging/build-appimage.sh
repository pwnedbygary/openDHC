#!/usr/bin/env bash
set -euo pipefail

rm -rf build AppDir *.AppImage *.zsync
mkdir -p build

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build -j
cmake --install build --prefix /usr --config Release -- DESTDIR=AppDir

install -Dm755 packaging/AppRun           AppDir/AppRun
install -Dm644 packaging/opendhc.desktop  AppDir/opendhc.desktop
install -Dm644 packaging/opendhc.png      AppDir/opendhc.png

LD=linuxdeploy-x86_64.AppImage
QT=linuxdeploy-plugin-qt-x86_64.AppImage
curl -L -o "$LD" https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
curl -L -o "$QT" https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x "$LD" "$QT"

export VERSION=${VERSION:-"dev"}
export LINUXDEPLOY_OUTPUT_VERSION="$VERSION"
# If CI doesn't override this, use a generic placeholder (will still embed update info).
export LDAI_UPDATE_INFORMATION="${LDAI_UPDATE_INFORMATION:-gh-releases-zsync|owner|repo|latest|*x86_64.AppImage.zsync}"
export QML_SOURCES_PATHS=ui

./"$LD" --appdir AppDir \
  --executable AppDir/usr/bin/OpenDHC \
  --desktop-file AppDir/opendhc.desktop \
  --icon-file packaging/opendhc.png \
  --plugin qt \
  --output appimage

echo "Artifacts:"
ls -1 *.AppImage *.zsync 2>/dev/null || true
