// SPDX-License-Identifier: GPL-2.0-or-later
//
// Runs a binding. The state machine says WHAT (a slot fired); the config says
// WHICH binding is in that slot; this turns the binding into something that
// happens on the phone. Every built-in goes through an interface the desktop
// already offers -- nothing here needs root:
//
//   screen-toggle    a virtual power tap through uinput: exactly what the shell
//                    does for a native tap (blank, or wake, lock screen and all)
//   power-menu       org.kde.LogoutPrompt.promptAll on the session bus
//   brightness-up/down
//                    org.kde.ScreenBrightness, the internal display, in steps
//                    of brightness_step_percent of its maximum, never below 1%
//   torch-toggle     the flash LED's sysfs brightness file, which Plasma
//                    Mobile's own udev rule leaves world-writable for its
//                    flashlight tile
//   screenshot       org.surya.Screenglaze.shoot -- screenglaze's own sheet
//   app:<id>         the Exec= of <id>.desktop, field codes stripped
//   command:<sh>     /bin/sh -c
#pragma once

#include <QObject>
#include <QString>

namespace keyconfig {

class Keys;

class Actions : public QObject
{
	Q_OBJECT

public:
	explicit Actions(Keys *keys, QObject *parent = nullptr);

	void setHardware(const QString &torchLed, int torchBrightness, int brightnessStepPercent);

	// Execute one binding string (see actionspec.h). Never throws, never
	// blocks: failures are logged and the daemon carries on.
	void run(const QString &spec);

private:
	void builtin(const QString &id);
	void screenToggle();
	void powerMenu();
	void brightness(int direction);
	void torchToggle();
	void screenshot();
	void launchApp(const QString &desktopId);
	void command(const QString &shell);

	Keys *m_keys;
	QString m_torchLed;
	int m_torchBrightness = 100;
	int m_stepPercent = 10;
};

} // namespace keyconfig
