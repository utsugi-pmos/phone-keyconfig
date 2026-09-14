# SPDX-License-Identifier: GPL-2.0-or-later
# ui/ holds main.qml plus the shared ActionPicker.qml / ActionEditor.qml,
# symlinked in from ../../shared so both KCMs use the one source file.
kcmutils_add_qml_kcm(kcm_phone_keyconfig SOURCES
	kcm.cpp
	keyconfigbackend.cpp keyconfigbackend.h)
target_link_libraries(kcm_phone_keyconfig PRIVATE
	keyconfig-common KF6::I18n KF6::KCMUtils KF6::KCMUtilsQuick)
