// SPDX-License-Identifier: GPL-2.0-or-later
#include "daemon.h"

#include "actionspec.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>

namespace keyconfig {

Daemon::Daemon(QObject *parent)
	: QObject(parent)
	, m_actions(&m_keys)
{
	m_clock.start();

	m_timer.setSingleShot(true);
	m_timer.setTimerType(Qt::PreciseTimer);
	connect(&m_timer, &QTimer::timeout, this, &Daemon::onTimer);

	// Editors and QSettings both write by replacing the file, which drops the
	// watch on the old inode; watching the directory too catches the new one.
	// And a burst of change notifications is one reload, not five.
	m_reloadDebounce.setSingleShot(true);
	m_reloadDebounce.setInterval(200);
	connect(&m_reloadDebounce, &QTimer::timeout, this, &Daemon::reload);
	connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] { m_reloadDebounce.start(); watchConfig(); });
	connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] { m_reloadDebounce.start(); watchConfig(); });

	reload();
	watchConfig();

	connect(&m_keys, &Keys::power, this, &Daemon::onPower);
	connect(&m_keys, &Keys::volume, this, &Daemon::onVolume);
}

void Daemon::watchConfig()
{
	const QString file = Settings::userFile();
	const QString dir = QFileInfo(file).absolutePath();
	QDir().mkpath(dir);
	if (!m_watcher.directories().contains(dir))
		m_watcher.addPath(dir);
	if (QFileInfo::exists(file) && !m_watcher.files().contains(file))
		m_watcher.addPath(file);
}

void Daemon::reload()
{
	m_cfg = Settings::load();
	m_gesture.setConfig(m_cfg.timing);
	m_actions.setHardware(m_cfg.torchLed, m_cfg.torchBrightness, m_cfg.brightnessStepPercent);
	qInfo("phone-keyconfig: config: hold %lld ms, double tap %lld ms, forgiveness %lld ms;"
		" power+up=%s power+down=%s double=%s release=%s hold=%s",
		(long long)m_cfg.timing.holdMenuMs, (long long)m_cfg.timing.doubleTapMs,
		(long long)m_cfg.timing.forgivenessMs,
		qPrintable(m_cfg.powerVolumeUp), qPrintable(m_cfg.powerVolumeDown),
		qPrintable(m_cfg.doubleTap), qPrintable(m_cfg.powerRelease), qPrintable(m_cfg.powerHold));
	arm();
}

void Daemon::onPower(int value)
{
	if (!active())
		return;
	if (value == 1)
		execute(m_gesture.powerPress(now()));
	else if (value == 0)
		execute(m_gesture.powerRelease(now()));
	// Autorepeat of power (value 2) is meaningless here: the machine works on
	// how long it has been down, not on the repeats.
	arm();
}

void Daemon::onVolume(Volume v, int value)
{
	if (!active())
		return;
	if (value == 1)
		execute(m_gesture.volumePress(v, now()));
	else if (value == 2)
		execute(m_gesture.volumeRepeat(v, now()));
	else
		execute(m_gesture.volumeRelease(v, now()));
	arm();
}

void Daemon::onTimer()
{
	execute(m_gesture.tick(now()));
	arm();
}

void Daemon::execute(const std::vector<Action> &actions)
{
	// Every gesture is logged with the binding it ran. When a button "does
	// nothing" this is the line that says whether the machine saw the gesture
	// and what it tried to do about it.
	for (const Action &a : actions) {
		switch (a.kind) {
		case Action::ScreenToggle:
			qInfo("phone-keyconfig: power released -> %s", qPrintable(m_cfg.powerRelease));
			m_actions.run(m_cfg.powerRelease);
			break;
		case Action::PowerMenu:
			qInfo("phone-keyconfig: power held -> %s", qPrintable(m_cfg.powerHold));
			m_actions.run(m_cfg.powerHold);
			break;
		case Action::VolumeCombo: {
			const QString &b = a.volume == Volume::Up ? m_cfg.powerVolumeUp : m_cfg.powerVolumeDown;
			qInfo("phone-keyconfig: power + volume-%s -> %s", a.volume == Volume::Up ? "up" : "down", qPrintable(b));
			m_actions.run(b);
			break;
		}
		case Action::DoubleTap:
			qInfo("phone-keyconfig: power double tap -> %s", qPrintable(m_cfg.doubleTap));
			m_actions.run(m_cfg.doubleTap);
			break;
		case Action::PassVolume:
			m_keys.replayVolume(a.volume, a.value);
			break;
		}
	}
}

void Daemon::arm()
{
	const Ms deadline = m_gesture.nextDeadline();
	if (deadline < 0) {
		m_timer.stop();
		return;
	}
	const Ms wait = deadline - now();
	m_timer.start(static_cast<int>(wait < 0 ? 0 : wait));
}

} // namespace keyconfig
