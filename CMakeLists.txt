cmake_minimum_required(VERSION 3.20)
project(phone-keyconfig VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
qt_standard_project_setup(REQUIRES 6.5)

# The state machine: pure C++, no Qt, so it is unit-tested anywhere.
add_library(keyconfig-core STATIC
	src/core/gesture.cpp
	src/core/gesture.h
)
target_include_directories(keyconfig-core PUBLIC src/core)

# Shared by the daemon and the app: the config file and the binding grammar.
add_library(keyconfig-common STATIC
	src/common/settings.cpp
	src/common/settings.h
	src/common/actionspec.h
)
target_include_directories(keyconfig-common PUBLIC src/common)
target_link_libraries(keyconfig-common PUBLIC Qt6::Core keyconfig-core)

# The daemon.
qt_add_executable(phone-keyconfigd
	src/daemon/main.cpp
	src/daemon/daemon.cpp
	src/daemon/daemon.h
	src/daemon/keys.cpp
	src/daemon/keys.h
	src/daemon/actions.cpp
	src/daemon/actions.h
)
target_link_libraries(phone-keyconfigd PRIVATE Qt6::Core Qt6::DBus keyconfig-common)
install(TARGETS phone-keyconfigd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# The settings app.
add_subdirectory(src/app)

# Tests: the state machine, one case per rule of the spec.
enable_testing()
add_executable(gesture_test tests/gesture_test.cpp)
target_link_libraries(gesture_test PRIVATE keyconfig-core)
add_test(NAME gesture COMMAND gesture_test)

# System files.
install(FILES data/phone-keyconfig.desktop
	DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/phone-keyconfig.svg
	DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
install(FILES data/phone-keyconfigd.service
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)
install(FILES data/80-phone-keyconfig.preset
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user-preset)
install(FILES data/71-phone-keyconfig.rules
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/udev/rules.d)
# The shipped defaults: the system layer QSettings falls back to.
install(FILES data/phone-keyconfig.conf
	DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/xdg/phone-keyconfig)
