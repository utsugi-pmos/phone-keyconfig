// SPDX-License-Identifier: GPL-2.0-or-later
#include "poweractions.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDebug>

namespace keyconfig {

namespace {

void call(const QString &service, const QString &path, const QString &iface, const QString &method)
{
	QDBusMessage m = QDBusMessage::createMethodCall(service, path, iface, method);
	// Fire and forget: the shutdown/reboot tears the session down anyway, and a
	// blocking call would just hang while it does.
	QDBusConnection::sessionBus().asyncCall(m);
}

} // namespace

PowerActions::PowerActions(QObject *parent)
	: QObject(parent)
{
}

void PowerActions::powerOff()
{
	// org.kde.Shutdown is what the desktop menu's "Shut Down" ends up calling;
	// it still puts up its own confirmation, so our menu is the quick path.
	call(QStringLiteral("org.kde.Shutdown"), QStringLiteral("/Shutdown"),
		QStringLiteral("org.kde.Shutdown"), QStringLiteral("logoutAndShutdown"));
}

void PowerActions::reboot()
{
	call(QStringLiteral("org.kde.Shutdown"), QStringLiteral("/Shutdown"),
		QStringLiteral("org.kde.Shutdown"), QStringLiteral("logoutAndReboot"));
}

void PowerActions::lockScreen()
{
	call(QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("/ScreenSaver"),
		QStringLiteral("org.freedesktop.ScreenSaver"), QStringLiteral("Lock"));
}

} // namespace keyconfig
