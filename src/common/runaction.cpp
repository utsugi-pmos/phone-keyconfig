// SPDX-License-Identifier: GPL-2.0-or-later
#include "runaction.h"

#include "actionlibrary.h"
#include "actionspec.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>

namespace keyconfig {

namespace {

void sessionCall(const QString &service, const QString &path, const QString &iface, const QString &method)
{
	QDBusMessage m = QDBusMessage::createMethodCall(service, path, iface, method);
	QDBusConnection::sessionBus().asyncCall(m);
}

} // namespace

ActionRunner::ActionRunner(ActionLibrary *library, QObject *parent)
	: QObject(parent)
	, m_library(library)
{
}

void ActionRunner::setHardware(const QString &torchLed, int torchBrightness, int brightnessStepPercent)
{
	m_torchLed = torchLed;
	m_torchBrightness = torchBrightness;
	m_stepPercent = brightnessStepPercent;
}

bool ActionRunner::runById(const QString &actionId)
{
	if (actionId.isEmpty() || actionId == QLatin1String("none"))
		return true;
	if (!m_library->exists(actionId)) {
		qWarning("phone-keyconfig: action '%s' no longer exists", qPrintable(actionId));
		return true;
	}
	return runSpec(m_library->specOf(actionId));
}

bool ActionRunner::runSpec(const QString &specString)
{
	const Spec spec = Spec::parse(specString);
	switch (spec.kind) {
	case Spec::None:
		return true;
	case Spec::Builtin:
		return builtin(spec.payload);
	case Spec::App:
		launchApp(spec.payload);
		return true;
	case Spec::Command:
		command(spec.payload);
		return true;
	}
	return true;
}

bool ActionRunner::builtin(const QString &id)
{
	if (id == QLatin1String("screen-toggle"))
		return false;   // the daemon handles this one
	if (id == QLatin1String("power-off"))
		powerOff();
	else if (id == QLatin1String("restart"))
		restart();
	else if (id == QLatin1String("lock-screen"))
		lockScreen();
	else if (id == QLatin1String("power-menu"))
		powerMenu();
	else if (id == QLatin1String("brightness-up"))
		brightness(+1);
	else if (id == QLatin1String("brightness-down"))
		brightness(-1);
	else if (id == QLatin1String("torch-toggle"))
		torchToggle();
	else if (id == QLatin1String("screenshot"))
		screenshot();
	else
		qWarning("phone-keyconfig: unknown built-in action '%s'", qPrintable(id));
	return true;
}

void ActionRunner::powerOff()
{
	sessionCall(QStringLiteral("org.kde.Shutdown"), QStringLiteral("/Shutdown"),
		QStringLiteral("org.kde.Shutdown"), QStringLiteral("logoutAndShutdown"));
}

void ActionRunner::restart()
{
	sessionCall(QStringLiteral("org.kde.Shutdown"), QStringLiteral("/Shutdown"),
		QStringLiteral("org.kde.Shutdown"), QStringLiteral("logoutAndReboot"));
}

void ActionRunner::lockScreen()
{
	sessionCall(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
		QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("Lock"));
}

void ActionRunner::powerMenu()
{
	if (!QProcess::startDetached(QStringLiteral("phone-keyconfig-menu"), {}))
		qWarning("phone-keyconfig: could not launch the power menu");
}

void ActionRunner::brightness(int direction)
{
	const QString service = QStringLiteral("org.kde.ScreenBrightness");
	const QString root = QStringLiteral("/org/kde/ScreenBrightness");
	QDBusConnection bus = QDBusConnection::sessionBus();

	QDBusInterface rootIface(service, root, service, bus);
	const QStringList names = rootIface.property("DisplaysDBusNames").toStringList();
	if (names.isEmpty()) {
		qWarning("phone-keyconfig: org.kde.ScreenBrightness lists no display");
		return;
	}
	QString chosen = names.first();
	for (const QString &n : names) {
		QDBusInterface probe(service, root + QLatin1Char('/') + n,
			QStringLiteral("org.kde.ScreenBrightness.Display"), bus);
		if (probe.property("IsInternal").toBool()) {
			chosen = n;
			break;
		}
	}

	QDBusInterface display(service, root + QLatin1Char('/') + chosen,
		QStringLiteral("org.kde.ScreenBrightness.Display"), bus);
	const int max = display.property("MaxBrightness").toInt();
	const int cur = display.property("Brightness").toInt();
	if (max <= 0)
		return;
	const int step = qMax(1, max * m_stepPercent / 100);
	const int floor = qMax(1, max / 100);
	int next = cur + direction * step;
	next = qBound(floor, next, max);
	if (next == cur)
		return;
	display.asyncCall(QStringLiteral("SetBrightness"), next, QVariant::fromValue(uint(0)));
}

void ActionRunner::torchToggle()
{
	QFile f(m_torchLed);
	if (!f.open(QIODevice::ReadWrite | QIODevice::Text)) {
		qWarning("phone-keyconfig: cannot open the flashlight LED %s: %s",
			qPrintable(m_torchLed), qPrintable(f.errorString()));
		return;
	}
	const int now = f.readAll().trimmed().toInt();
	const int next = now > 0 ? 0 : m_torchBrightness;
	f.seek(0);
	f.write(QByteArray::number(next));
	f.write("\n");
	f.close();
}

void ActionRunner::screenshot()
{
	sessionCall(QStringLiteral("org.surya.Screenglaze"), QStringLiteral("/"),
		QStringLiteral("org.surya.Screenglaze"), QStringLiteral("shoot"));
}

void ActionRunner::launchApp(const QString &desktopId)
{
	const QString file = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
		desktopId + QStringLiteral(".desktop"));
	if (file.isEmpty()) {
		qWarning("phone-keyconfig: no %s.desktop in any applications directory", qPrintable(desktopId));
		return;
	}
	QSettings entry(file, QSettings::IniFormat);
	entry.beginGroup(QStringLiteral("Desktop Entry"));
	QString exec = entry.value(QStringLiteral("Exec")).toString();
	const QString workDir = entry.value(QStringLiteral("Path")).toString();
	entry.endGroup();
	if (exec.isEmpty()) {
		qWarning("phone-keyconfig: %s has no Exec=", qPrintable(file));
		return;
	}
	static const QRegularExpression fieldCode(QStringLiteral("%[fFuUdDnNickvm]"));
	exec.replace(fieldCode, QString());
	exec.replace(QLatin1String("%%"), QLatin1String("%"));
	QProcess p;
	p.setProgram(QStringLiteral("/bin/sh"));
	p.setArguments({QStringLiteral("-c"), QStringLiteral("exec ") + exec.trimmed()});
	if (!workDir.isEmpty())
		p.setWorkingDirectory(workDir);
	if (!p.startDetached())
		qWarning("phone-keyconfig: could not launch %s", qPrintable(desktopId));
}

void ActionRunner::command(const QString &shell)
{
	if (shell.trimmed().isEmpty())
		return;
	if (!QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), shell}))
		qWarning("phone-keyconfig: could not start the command");
}

} // namespace keyconfig
