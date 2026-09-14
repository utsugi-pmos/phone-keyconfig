qt_add_executable(phone-keyconfig
	main.cpp
	settingsmodel.cpp
	settingsmodel.h
	poweractions.cpp
	poweractions.h
)

# The QML lives next to this file so every type is addressed by its bare name
# (loadFromModule("PhoneKeyconfig", "Main")).
qt_add_qml_module(phone-keyconfig
	URI PhoneKeyconfig
	VERSION 1.0
	QML_FILES
		Main.qml
		BindingsPage.qml
		BindingRow.qml
		TimingRow.qml
		ActionPicker.qml
		PowerMenu.qml
)

target_link_libraries(phone-keyconfig PRIVATE
	Qt6::Core
	Qt6::Gui
	Qt6::Qml
	Qt6::Quick
	Qt6::DBus
	keyconfig-common
)

install(TARGETS phone-keyconfig
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
