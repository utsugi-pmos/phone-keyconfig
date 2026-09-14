// SPDX-License-Identifier: GPL-2.0-or-later
//
// phone-keyconfig: the settings app for what the power and volume buttons do
// (it only edits ~/.config/phone-keyconfig/phone-keyconfig.conf; the daemon
// watches that file and applies changes at once), and -- with --power-menu --
// the power menu the daemon puts up on a long press, because Plasma Mobile's
// own logout greeter does not render on this shell.
#include "poweractions.h"
#include "settingsmodel.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QStringList>

int main(int argc, char *argv[])
{
	// Scanned from raw argv, because it decides a thing that must be set BEFORE
	// QGuiApplication (which is when the Wayland plugin picks its shell
	// integration): the power menu is a layer-shell overlay so the mobile shell
	// puts it on TOP of the running app -- an ordinary window is not raised and
	// never appears. The settings window stays an ordinary window.
	bool powerMenu = false;
	for (int i = 1; i < argc; ++i)
		if (qstrcmp(argv[i], "--power-menu") == 0)
			powerMenu = true;
	if (powerMenu && qEnvironmentVariableIsEmpty("PHONE_KEYCONFIG_NO_LAYER_SHELL"))
		qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");

	// Wayland on this phone reports the wrong physical DPI, and Kirigami sizes
	// every touch target from it; left alone the rows come out too small to hit.
	// Same workaround as the other apps of the distribution.
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QGuiApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("phone-keyconfig"));
	QCoreApplication::setOrganizationName(QStringLiteral("phone-keyconfig"));
	QGuiApplication::setDesktopFileName(QStringLiteral("phone-keyconfig"));

	// A bare QGuiApplication inherits no icon theme from Plasma, and without one
	// every icon in the app is an empty square.
	if (QIcon::themeName().isEmpty())
		QIcon::setThemeName(QStringLiteral("breeze"));

	QQmlApplicationEngine engine;
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
		[] { QCoreApplication::exit(1); }, Qt::QueuedConnection);

	// Kept for the whole run whichever branch is taken; QML holds a plain
	// pointer to it through the context property.
	keyconfig::SettingsModel model;
	keyconfig::PowerActions power;

	if (powerMenu) {
		engine.rootContext()->setContextProperty(QStringLiteral("PowerMenu"), &power);
		engine.loadFromModule("PhoneKeyconfig", "PowerMenu");
	} else {
		engine.rootContext()->setContextProperty(QStringLiteral("KeySettings"), &model);
		engine.loadFromModule("PhoneKeyconfig", "Main");
	}
	return app.exec();
}
