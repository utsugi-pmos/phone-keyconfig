// SPDX-License-Identifier: GPL-2.0-or-later
//
// The glue, and nothing but glue: key events go into the state machine with a
// timestamp, the actions that come back out are executed, and a single timer
// is armed to the machine's next deadline. Configuration is re-read whenever
// the user's file changes, so the app's edits take effect without a restart.
#pragma once

#include "actions.h"
#include "gesture.h"
#include "keys.h"
#include "settings.h"

#include <QElapsedTimer>
#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

namespace keyconfig {

class Daemon : public QObject
{
	Q_OBJECT

public:
	explicit Daemon(QObject *parent = nullptr);

	bool active() const { return m_keys.grabbed(); }
	QStringList watching() const { return m_keys.watching(); }

private:
	Ms now() const { return m_clock.elapsed(); }
	void onPower(int value);
	void onVolume(Volume v, int value);
	void onTimer();
	void execute(const std::vector<Action> &actions);
	void arm();
	void reload();
	void watchConfig();

	Keys m_keys;
	Actions m_actions;
	Gesture m_gesture;
	Values m_cfg;
	QElapsedTimer m_clock;
	QTimer m_timer;
	QFileSystemWatcher m_watcher;
	QTimer m_reloadDebounce;
};

} // namespace keyconfig
