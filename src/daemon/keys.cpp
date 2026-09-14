// SPDX-License-Identifier: GPL-2.0-or-later
#include "keys.h"

#include <QDebug>
#include <QDir>
#include <QSocketNotifier>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace keyconfig {

namespace {

constexpr int bitsPerLong = 8 * sizeof(long);
inline bool testBit(const unsigned long *map, int bit)
{
	return (map[bit / bitsPerLong] >> (bit % bitsPerLong)) & 1UL;
}

// The name of our own virtual device, so discovery never grabs it.
const char *virtualName = "phone-keyconfig passthrough";

} // namespace

Keys::Keys(QObject *parent)
	: QObject(parent)
{
	discover();
	if (m_devices.isEmpty()) {
		qWarning("phone-keyconfig: no input device with a power or volume key is readable."
			" Is the udev rule installed and is this a seat session?");
		return;
	}
	if (openUinput())
		grab(true);
	else
		qWarning("phone-keyconfig: /dev/uinput is not available, so the keys are NOT grabbed"
			" and nothing here will act on them. The phone keeps its native buttons.");
}

Keys::~Keys()
{
	grab(false);
	for (const Device &d : std::as_const(m_devices)) {
		delete d.notifier;
		::close(d.fd);
	}
	if (m_uinput >= 0) {
		::ioctl(m_uinput, UI_DEV_DESTROY);
		::close(m_uinput);
	}
}

void Keys::discover()
{
	const QDir dir(QStringLiteral("/dev/input"));
	QStringList names = dir.entryList({QStringLiteral("event*")}, QDir::System);
	std::sort(names.begin(), names.end(), [](const QString &a, const QString &b) {
		return a.mid(5).toInt() < b.mid(5).toInt();
	});

	for (const QString &entry : std::as_const(names)) {
		const QString path = dir.filePath(entry);
		const int fd = ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		if (fd < 0)
			continue; // not ours to read: the udev rule decides what we get

		unsigned long types[EV_MAX / bitsPerLong + 1] = {};
		unsigned long keys[KEY_MAX / bitsPerLong + 1] = {};
		if (::ioctl(fd, EVIOCGBIT(0, sizeof(types)), types) < 0
			|| ::ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keys)), keys) < 0) {
			::close(fd);
			continue;
		}
		// Anything with absolute axes is a touchscreen or a sensor, never a
		// button; skipping it is also what keeps this from ever being able to
		// read what is typed on screen.
		if (testBit(types, EV_ABS)) {
			::close(fd);
			continue;
		}

		char name[256] = {};
		if (::ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) < 0)
			qstrncpy(name, "?", sizeof(name));
		if (qstrcmp(name, virtualName) == 0) {
			::close(fd);
			continue;
		}

		const bool power = testBit(keys, KEY_POWER);
		const bool volDown = testBit(keys, KEY_VOLUMEDOWN);
		const bool volUp = testBit(keys, KEY_VOLUMEUP);
		if (!power && !volDown && !volUp) {
			::close(fd);
			continue;
		}

		Device d;
		d.fd = fd;
		d.name = QString::fromLocal8Bit(name);
		d.notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
		connect(d.notifier, &QSocketNotifier::activated, this, [this, fd] { read(fd); });
		m_devices.append(d);
		m_hasPower = m_hasPower || power;

		QStringList what;
		if (power) what << QStringLiteral("power");
		if (volDown) what << QStringLiteral("volume-down");
		if (volUp) what << QStringLiteral("volume-up");
		m_watching << QStringLiteral("%1 (%2): %3").arg(entry, d.name, what.join(QStringLiteral(", ")));
	}
}

bool Keys::openUinput()
{
	m_uinput = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (m_uinput < 0) {
		qWarning("phone-keyconfig: cannot open /dev/uinput: %s", strerror(errno));
		return false;
	}

	uinput_setup setup = {};
	setup.id.bustype = BUS_VIRTUAL;
	setup.id.vendor = 0x1d6b;
	setup.id.product = 0x0002;
	setup.id.version = 1;
	qstrncpy(setup.name, virtualName, sizeof(setup.name));

	if (::ioctl(m_uinput, UI_SET_EVBIT, EV_KEY) < 0
		|| ::ioctl(m_uinput, UI_SET_KEYBIT, KEY_POWER) < 0
		|| ::ioctl(m_uinput, UI_SET_KEYBIT, KEY_VOLUMEDOWN) < 0
		|| ::ioctl(m_uinput, UI_SET_KEYBIT, KEY_VOLUMEUP) < 0
		|| ::ioctl(m_uinput, UI_DEV_SETUP, &setup) < 0
		|| ::ioctl(m_uinput, UI_DEV_CREATE) < 0) {
		qWarning("phone-keyconfig: could not create the virtual device: %s", strerror(errno));
		::close(m_uinput);
		m_uinput = -1;
		return false;
	}
	return true;
}

void Keys::grab(bool on)
{
	if (on == m_grabbed)
		return;
	bool any = false;
	for (const Device &d : std::as_const(m_devices)) {
		if (::ioctl(d.fd, EVIOCGRAB, on ? 1 : 0) == 0)
			any = true;
		else if (on)
			qWarning("phone-keyconfig: could not grab %s: %s", qPrintable(d.name), strerror(errno));
	}
	m_grabbed = on && any;
}

void Keys::replay(int code, int value)
{
	if (m_uinput < 0)
		return;
	input_event ev[2] = {};
	ev[0].type = EV_KEY;
	ev[0].code = static_cast<__u16>(code);
	ev[0].value = value;
	ev[1].type = EV_SYN;
	ev[1].code = SYN_REPORT;
	if (::write(m_uinput, ev, sizeof(ev)) < 0)
		qWarning("phone-keyconfig: could not replay key %d: %s", code, strerror(errno));
}

void Keys::replayVolume(Volume v, int value)
{
	replay(v == Volume::Up ? KEY_VOLUMEUP : KEY_VOLUMEDOWN, value);
}

void Keys::replayPowerTap()
{
	replay(KEY_POWER, 1);
	replay(KEY_POWER, 0);
}

void Keys::read(int fd)
{
	input_event events[32];
	for (;;) {
		const ssize_t n = ::read(fd, events, sizeof(events));
		if (n <= 0)
			return;
		for (int i = 0; i < int(n / sizeof(input_event)); ++i) {
			const input_event &e = events[i];
			if (e.type != EV_KEY)
				continue;
			if (e.code == KEY_POWER)
				Q_EMIT power(e.value);
			else if (e.code == KEY_VOLUMEDOWN)
				Q_EMIT volume(Volume::Down, e.value);
			else if (e.code == KEY_VOLUMEUP)
				Q_EMIT volume(Volume::Up, e.value);
		}
	}
}

} // namespace keyconfig
