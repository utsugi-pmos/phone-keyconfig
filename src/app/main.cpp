// SPDX-License-Identifier: GPL-2.0-or-later
//
// phone-keyconfig: the settings app for what the power and volume buttons do.
// It only edits ~/.config/phone-keyconfig/phone-keyconfig.conf; the daemon
// (phone-keyconfigd) watches that file and applies changes at once.
#include "settingsmodel.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
	// Wayland on this phone reports the wrong physical DPI, and Kirigami sizes
	// every touch target from it; left alone the rows come out too small to hit.
	// Same workaround as the other apps of the distribution.
	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QGuiApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("phone-keyconfig"));
	QCoreApplication::setOrganizationName(QStringLiteral("phone-keyconfig"));
	// Ties the window to phone-keyconfig.desktop, so the task switcher shows
	// its name and icon instead of a generic entry.
	QGuiApplication::setDesktopFileName(QStringLiteral("phone-keyconfig"));

	// A bare QGuiApplication inherits no icon theme from Plasma, and without
	// one every icon in the app is an empty square.
	if (QIcon::themeName().isEmpty())
		QIcon::setThemeName(QStringLiteral("breeze"));

	keyconfig::SettingsModel model;

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty(QStringLiteral("KeySettings"), &model);
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
		[] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
	engine.loadFromModule("PhoneKeyconfig", "Main");
	return app.exec();
}
