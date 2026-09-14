// SPDX-License-Identifier: GPL-2.0-or-later
//
// phone-keyconfigd: the daemon that owns the power and volume buttons. Runs as
// a user service, one per session. See daemon.h and, for the rules, gesture.h.
#include "daemon.h"

#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);
	QCoreApplication::setApplicationName(QStringLiteral("phone-keyconfig"));
	QCoreApplication::setOrganizationName(QStringLiteral("phone-keyconfig"));

	keyconfig::Daemon daemon;
	const QStringList watching = daemon.watching();
	for (const QString &w : watching)
		qInfo("phone-keyconfig: listening on %s", qPrintable(w));
	if (!daemon.active()) {
		// Stay up rather than exit: a restart loop would not fix a missing
		// udev rule, and the log line above already says what is wrong.
		qWarning("phone-keyconfig: inactive -- the buttons keep their native behaviour.");
	}
	return app.exec();
}
