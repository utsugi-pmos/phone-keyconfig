# SPDX-License-Identifier: GPL-2.0-or-later
kcmutils_add_qml_kcm(kcm_phone_powermenu SOURCES
	kcm.cpp
	powermenubackend.cpp powermenubackend.h)
target_link_libraries(kcm_phone_powermenu PRIVATE
	keyconfig-common KF6::I18n KF6::KCMUtils KF6::KCMUtilsQuick)
