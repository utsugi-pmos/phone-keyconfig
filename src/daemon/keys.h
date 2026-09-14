// SPDX-License-Identifier: GPL-2.0-or-later
//
// The hardware side: the evdev devices that carry the power and volume keys,
// grabbed so that the shell never sees a raw press, and a uinput device through
// which whatever the state machine decides to let through is replayed.
//
// WHY GRAB EVERYTHING
// -------------------
// EVIOCGRAB is all-or-nothing per device and it is the only way to stop a key
// from reaching the compositor. Power has to be grabbed or the shell blanks the
// screen the instant it is pressed; the volume keys have to be grabbed or the
// volume panel pops up in the middle of a chord and the chord itself cannot be
// swallowed. Everything the machine wants passed through comes back out of the
// uinput device as an ordinary key event, so the shell still gets normal volume
// presses and a normal power tap -- just from a virtual keyboard, a beat later.
//
// If /dev/uinput cannot be opened nothing is grabbed: a grabbed key that could
// not be replayed would be a key the phone lost, and a phone without a working
// power button is worse than a phone without chords. grabbed() says which world
// we are in; the daemon stays inert in the second one.
#pragma once

#include "gesture.h"

#include <QList>
#include <QObject>
#include <QStringList>

class QSocketNotifier;

namespace keyconfig {

class Keys : public QObject
{
	Q_OBJECT

public:
	explicit Keys(QObject *parent = nullptr);
	~Keys() override;

	bool hasPower() const { return m_hasPower; }
	bool grabbed() const { return m_grabbed; }
	QStringList watching() const { return m_watching; }

	// Put a key event back into the system through the uinput device.
	// value: 1 press, 2 autorepeat, 0 release.
	void replay(int code, int value);
	void replayVolume(Volume v, int value);
	// A complete power press+release. This is how "screen off / on" is done:
	// it is precisely what the shell does with a native tap, lock screen and
	// wake-up included, and it cannot be cancelled later because it is only
	// ever sent once the gesture is decided.
	void replayPowerTap();

Q_SIGNALS:
	void power(int value);
	void volume(keyconfig::Volume v, int value);

private:
	struct Device {
		int fd = -1;
		QSocketNotifier *notifier = nullptr;
		QString name;
	};

	void discover();
	bool openUinput();
	void grab(bool on);
	void read(int fd);

	QList<Device> m_devices;
	QStringList m_watching;
	bool m_hasPower = false;
	int m_uinput = -1;
	bool m_grabbed = false;
};

} // namespace keyconfig
