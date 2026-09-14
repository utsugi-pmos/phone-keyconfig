cmake_minimum_required(VERSION 3.20)
project(phone-keyconfig VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

set(KF_MIN 6.0)
set(QT_MIN 6.6)

find_package(ECM ${KF_MIN} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt6 ${QT_MIN} REQUIRED COMPONENTS Core Gui Qml Quick DBus)
find_package(KF6 ${KF_MIN} REQUIRED COMPONENTS CoreAddons Config I18n KCMUtils)
qt_standard_project_setup(REQUIRES 6.6)

# --- the state machine: pure C++, no Qt, unit-tested anywhere ---------------
add_library(keyconfig-core STATIC src/core/gesture.cpp src/core/gesture.h)
target_include_directories(keyconfig-core PUBLIC src/core)
set_target_properties(keyconfig-core PROPERTIES POSITION_INDEPENDENT_CODE ON)

# --- shared model layer: library, menu, settings, runner --------------------
add_library(keyconfig-common STATIC
	src/common/settings.cpp src/common/settings.h
	src/common/actionspec.h
	src/common/actionlibrary.cpp src/common/actionlibrary.h
	src/common/menumodel.cpp src/common/menumodel.h
	src/common/runaction.cpp src/common/runaction.h
)
target_include_directories(keyconfig-common PUBLIC src/common)
set_target_properties(keyconfig-common PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(keyconfig-common PUBLIC Qt6::Core Qt6::DBus keyconfig-core)

# --- the daemon -------------------------------------------------------------
qt_add_executable(phone-keyconfigd
	src/daemon/main.cpp
	src/daemon/daemon.cpp src/daemon/daemon.h
	src/daemon/keys.cpp src/daemon/keys.h
)
target_link_libraries(phone-keyconfigd PRIVATE Qt6::Core Qt6::DBus keyconfig-common)
install(TARGETS phone-keyconfigd RUNTIME DESTINATION ${KDE_INSTALL_BINDIR})

# --- the power menu (layer-shell overlay) -----------------------------------
qt_add_executable(phone-keyconfig-menu src/menu/main.cpp)
qt_add_qml_module(phone-keyconfig-menu URI PhoneKeyconfigMenu VERSION 1.0
	QML_FILES src/menu/MenuWindow.qml)
target_link_libraries(phone-keyconfig-menu PRIVATE
	Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::DBus keyconfig-common)
install(TARGETS phone-keyconfig-menu RUNTIME DESTINATION ${KDE_INSTALL_BINDIR})

# --- the two KCMs -----------------------------------------------------------
# In their own directories so kcmutils_add_qml_kcm finds each ui/ (main.qml plus
# the two shared widgets, symlinked in).
add_subdirectory(src/kcm/keyconfig)
add_subdirectory(src/kcm/powermenu)

# --- system files -----------------------------------------------------------
install(FILES data/phone-keyconfigd.service
	DESTINATION ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
install(FILES data/80-phone-keyconfig.preset
	DESTINATION ${KDE_INSTALL_LIBDIR}/systemd/user-preset)
install(FILES data/71-phone-keyconfig.rules
	DESTINATION ${KDE_INSTALL_PREFIX}/lib/udev/rules.d)
install(FILES data/phone-keyconfig.conf
	DESTINATION ${KDE_INSTALL_FULL_SYSCONFDIR}/xdg/phone-keyconfig)

# --- tests ------------------------------------------------------------------
enable_testing()
add_executable(gesture_test tests/gesture_test.cpp)
target_link_libraries(gesture_test PRIVATE keyconfig-core)
add_test(NAME gesture COMMAND gesture_test)
