// SPDX-License-Identifier: GPL-2.0-or-later
//
// The configuration, read the same way by the daemon and the app.
//
// It is a plain INI file handled by QSettings, in two layers QSettings merges on
// its own: the system defaults this package ships, and whatever the user has
// changed in the app.
//
//   /etc/xdg/phone-keyconfig/phone-keyconfig.conf       shipped defaults
//   ~/.config/phone-keyconfig/phone-keyconfig.conf      the user's changes
//
// The app only ever writes the second one, and only the keys the user touched,
// so a package update that changes a default reaches everyone who did not
// override that key.
#pragma once

#include "gesture.h"

#include <QSettings>
#include <QString>

namespace keyconfig {

struct Values {
	Config timing;                       // [timing]
	QString powerRelease;                // [bindings]
	QString powerHold;
	QString powerVolumeUp;
	QString powerVolumeDown;
	QString doubleTap;
	QString torchLed;                    // [hardware]
	int torchBrightness = 100;
	int brightnessStepPercent = 10;

	// The binding for a slot name (see actionspec.h slot::), or "none".
	QString binding(const QString &slot) const;
};

class Settings {
public:
	static QString organisation() { return QStringLiteral("phone-keyconfig"); }
	static QString application() { return QStringLiteral("phone-keyconfig"); }

	// Where the user's overrides live (may not exist yet).
	static QString userFile();

	static Values load();

	// Write one key into the user's file. `key` is "group/name", QSettings
	// style, e.g. "bindings/power_volume_up".
	static void set(const QString &key, const QVariant &value);

private:
	static QSettings open();
};

} // namespace keyconfig
