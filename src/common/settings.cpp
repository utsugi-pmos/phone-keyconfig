// SPDX-License-Identifier: GPL-2.0-or-later
#include "settings.h"

#include "actionspec.h"

#include <QStandardPaths>

namespace keyconfig {

QString Values::binding(const QString &slotName) const
{
	if (slotName == QLatin1String(slot::PowerRelease)) return powerRelease;
	if (slotName == QLatin1String(slot::PowerHold)) return powerHold;
	if (slotName == QLatin1String(slot::PowerVolumeUp)) return powerVolumeUp;
	if (slotName == QLatin1String(slot::PowerVolumeDown)) return powerVolumeDown;
	if (slotName == QLatin1String(slot::DoubleTap)) return doubleTap;
	return QStringLiteral("none");
}

QString Settings::userFile()
{
	return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
		+ QStringLiteral("/") + organisation() + QStringLiteral("/") + application()
		+ QStringLiteral(".conf");
}

Values Settings::load()
{
	// IniFormat + UserScope: reads ~/.config/<org>/<app>.conf and falls back to
	// /etc/xdg/<org>/<app>.conf for any key the user file lacks.
	QSettings s(QSettings::IniFormat, QSettings::UserScope, organisation(), application());
	Values v;

	// The fallbacks here are the fallbacks of last resort, for a system with no
	// defaults file at all; the shipped phone-keyconfig.conf is the real default.
	s.beginGroup(QStringLiteral("timing"));
	v.timing.holdMenuMs = s.value(QStringLiteral("hold_menu_ms"), 1500).toLongLong();
	v.timing.doubleTapMs = s.value(QStringLiteral("double_tap_ms"), 300).toLongLong();
	v.timing.forgivenessMs = s.value(QStringLiteral("volume_forgiveness_ms"), 150).toLongLong();
	s.endGroup();

	// A binding value is an ACTION ID now (resolved through ActionLibrary), not
	// an inline spec: a built-in id like "screenshot", a custom id like
	// "custom-1", or "none".
	s.beginGroup(QStringLiteral("bindings"));
	v.powerRelease = s.value(QLatin1String(slot::PowerRelease), QStringLiteral("screen-toggle")).toString();
	v.powerHold = s.value(QLatin1String(slot::PowerHold), QStringLiteral("power-menu")).toString();
	v.powerVolumeUp = s.value(QLatin1String(slot::PowerVolumeUp), QStringLiteral("brightness-up")).toString();
	v.powerVolumeDown = s.value(QLatin1String(slot::PowerVolumeDown), QStringLiteral("screenshot")).toString();
	v.doubleTap = s.value(QLatin1String(slot::DoubleTap), QStringLiteral("torch-toggle")).toString();
	s.endGroup();

	s.beginGroup(QStringLiteral("hardware"));
	v.torchLed = s.value(QStringLiteral("torch_led"),
		QStringLiteral("/sys/class/leds/white:flash/brightness")).toString();
	v.torchBrightness = s.value(QStringLiteral("torch_brightness"), 100).toInt();
	v.brightnessStepPercent = s.value(QStringLiteral("brightness_step_percent"), 10).toInt();
	s.endGroup();

	// Sanity: a zero or negative window would make the machine misbehave in
	// ways no setting should be able to cause.
	if (v.timing.holdMenuMs < 200) v.timing.holdMenuMs = 200;
	if (v.timing.doubleTapMs < 0) v.timing.doubleTapMs = 0;
	if (v.timing.forgivenessMs < 0) v.timing.forgivenessMs = 0;
	if (v.brightnessStepPercent < 1) v.brightnessStepPercent = 1;
	if (v.brightnessStepPercent > 50) v.brightnessStepPercent = 50;
	return v;
}

void Settings::set(const QString &key, const QVariant &value)
{
	// Writes go to the user's file only.
	QSettings s(QSettings::IniFormat, QSettings::UserScope, organisation(), application());
	s.setValue(key, value);
	s.sync();
}

} // namespace keyconfig
