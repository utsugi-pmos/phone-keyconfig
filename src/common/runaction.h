// SPDX-License-Identifier: GPL-2.0-or-later
//
// Runs an action through interfaces the desktop already offers -- no privilege
// beyond the session. Shared by the daemon (for gestures) and the power menu
// (for its buttons). It runs an action BY ID, resolving the id to a spec
// (builtin/app/command) through the ActionLibrary, so a gesture and a menu
// button that name the same custom action do the same thing.
//
// ScreenToggle is the exception: it needs a uinput power tap, which only the
// daemon can do, so runById returns false for it and the daemon handles it.
#pragma once

#include <QObject>
#include <QString>

namespace keyconfig {

class ActionLibrary;

class ActionRunner : public QObject
{
	Q_OBJECT

public:
	// The library is not owned; it must outlive the runner and stay loaded.
	explicit ActionRunner(ActionLibrary *library, QObject *parent = nullptr);

	void setHardware(const QString &torchLed, int torchBrightness, int brightnessStepPercent);

	// Run the action with this id. Returns false only when the id resolves to
	// builtin:screen-toggle, which the caller must handle itself; everything
	// else is done here (and logged, not thrown, on failure). An unknown id or
	// "none" is a no-op that returns true.
	Q_INVOKABLE bool runById(const QString &actionId);

	// Run a raw spec directly (used where there is no id, e.g. tests).
	bool runSpec(const QString &spec);

private:
	bool builtin(const QString &id);
	void powerOff();
	void restart();
	void lockScreen();
	void powerMenu();
	void brightness(int direction);
	void torchToggle();
	void screenshot();
	void launchApp(const QString &desktopId);
	void command(const QString &shell);

	ActionLibrary *const m_library;
	QString m_torchLed;
	int m_torchBrightness = 100;
	int m_stepPercent = 10;
};

} // namespace keyconfig
