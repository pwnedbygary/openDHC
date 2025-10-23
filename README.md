# OpenDHC (Linux-native chdman frontend)

Qt 6 GUI for `chdman` (create/verify/info/extract CHD), with batch processing, per-job progress, final report, and CSV export.  
Includes AppImage packaging via linuxdeploy + plugin-qt, with zsync auto-updates from GitHub Releases.

## Build (Linux)
```bash
sudo apt-get update
sudo apt-get install -y cmake build-essential \
  qt6-base-dev qt6-declarative-dev qml6-module-qtquick-controls2 qml6-module-qt-labs-platform \
  mame-tools  # provides chdman
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/OpenDHC

