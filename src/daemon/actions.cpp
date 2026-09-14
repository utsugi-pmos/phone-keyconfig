// SPDX-License-Identifier: GPL-2.0-or-later
#include "actions.h"

#include "actionspec.h"
#include "keys.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>

namespace keyconfig {

Actions::Actions(Keys *keys, QObject *parent)
	: QObject(parent)
	, m_keys(keys)
{
}

void Actions::setHardware(const QString &torchLed, int torchBrightness, int brightnessStepPercent)
{
	m_torchLed = torchLed;
	m_torchBrightness = torchBrightness;
	m_stepPercent = brightnessStepPercent;
}

void Actions::run(const QString &specString)
{
	const Spec spec = Spec::parse(specString);
	switch (spec.kind) {
	case Spec::None:
		return;
	case Spec::Builtin:
		builtin(spec.payload);
		return;
	case Spec::App:
		launchApp(spec.payload);
		return;
	case Spec::Command:
		command(spec.payload);
		return;
	}
}

void Actions::builtin(const QString &id)
{
	if (id == QLatin1String("screen-toggle"))
		screenToggle();
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
}

void Actions::screenToggle()
{
	m_keys->replayPowerTap();
}

void Actions::powerMenu()
{
	QDBusMessage m = QDBusMessage::createMethodCall(
		QStringLiteral("org.kde.LogoutPrompt"), QStringLiteral("/LogoutPrompt"),
		QStringLiteral("org.kde.LogoutPrompt"), QStringLiteral("promptAll"));
	QDBusConnection::sessionBus().asyncCall(m);
}

void Actions::brightness(int direction)
{
	const QString service = QStringLiteral("org.kde.ScreenBrightness");
	const QString root = QStringLiteral("/org/kde/ScreenBrightness");
	QDBusConnection bus = QDBusConnection::sessionBus();

	// Which display: the internal one if it says so, else the first listed.
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
	// Never all the way to 0: a screen you cannot see is a screen you cannot
	// fix from the screen.
	const int floor = qMax(1, max / 100);
	int next = cur + direction * step;
	next = qBound(floor, next, max);
	if (next == cur)
		return;
	// SetBrightness(i value, u flags): flags 0 = let the OSD show.
	display.asyncCall(QStringLiteral("SetBrightness"), next, QVariant::fromValue(uint(0)));
}

void Actions::torchToggle()
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

void Actions::screenshot()
{
	QDBusMessage m = QDBusMessage::createMethodCall(
		QStringLiteral("org.surya.Screenglaze"), QStringLiteral("/"),
		QStringLiteral("org.surya.Screenglaze"), QStringLiteral("shoot"));
	QDBusConnection::sessionBus().asyncCall(m);
}

void Actions::launchApp(const QString &desktopId)
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
	// Field codes (%f %u %i %c ...) stand for arguments we do not have. Drop
	// them; "%%" is a literal percent.
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

void Actions::command(const QString &shell)
{
	if (shell.trimmed().isEmpty())
		return;
	if (!QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), shell}))
		qWarning("phone-keyconfig: could not start the command");
}

} // namespace keyconfig
