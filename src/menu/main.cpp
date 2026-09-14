// SPDX-License-Identifier: GPL-2.0-or-later
//
// phone-keyconfig-menu: the power menu the daemon puts up on a long press. It
// draws itself as a layer-shell overlay, because Plasma Mobile's own logout
// greeter does not render on this shell. The buttons come from the config
// (MenuModel + ActionLibrary); tapping one runs its action through ActionRunner.
#include "actionlibrary.h"
#include "menumodel.h"
#include "runaction.h"
#include "settings.h"

#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>

using namespace keyconfig;

int main(int argc, char *argv[])
{
	// The menu is a layer-shell overlay so the mobile shell puts it ON TOP of
	// the running app; an ordinary window would not be raised. Must be set
	// before QGuiApplication.
	if (qEnvironmentVariableIsEmpty("PHONE_KEYCONFIG_NO_LAYER_SHELL"))
		qputenv("QT_WAYLAND_SHELL_INTEGRATION", "layer-shell");

	QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
		Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
	QGuiApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("phone-keyconfig"));
	QCoreApplication::setOrganizationName(QStringLiteral("phone-keyconfig"));
	if (QIcon::themeName().isEmpty())
		QIcon::setThemeName(QStringLiteral("breeze"));

	const Values cfg = Settings::load();

	ActionLibrary library;
	library.load();
	MenuModel menu(&library);
	menu.load();
	ActionRunner runner(&library);
	runner.setHardware(cfg.torchLed, cfg.torchBrightness, cfg.brightnessStepPercent);

	QQmlApplicationEngine engine;
	engine.rootContext()->setContextProperty(QStringLiteral("MenuModel"), &menu);
	engine.rootContext()->setContextProperty(QStringLiteral("Runner"), &runner);
	QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app,
		[] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
	engine.loadFromModule("PhoneKeyconfigMenu", "MenuWindow");
	return app.exec();
}
