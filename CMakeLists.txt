cmake_minimum_required(VERSION 3.22)
project(OpenDHC LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core Gui Qml Quick QuickControls2 REQUIRED)

qt_standard_project_setup()

qt_add_executable(OpenDHC
    src/main.cpp
    src/app/Job.hpp src/app/Job.cpp
    src/app/JobModel.hpp src/app/JobModel.cpp
    src/app/ChdmanRunner.hpp src/app/ChdmanRunner.cpp
    src/app/Settings.hpp src/app/Settings.cpp
    src/app/BatchScanner.hpp src/app/BatchScanner.cpp
    src/app/Report.hpp src/app/Report.cpp
    src/app/SizeUtil.hpp src/app/SizeUtil.cpp
)

qt_add_qml_module(OpenDHC
    URI OpenDHC
    VERSION 1.0
    QML_FILES ui/qml/Main.qml
    RESOURCE_PREFIX "/"
)

target_link_libraries(OpenDHC
    PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2
)

# Install the app into AppDir/usr/bin when used with DESTDIR during packaging
install(TARGETS OpenDHC RUNTIME DESTINATION usr/bin)
