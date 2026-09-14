// SPDX-License-Identifier: GPL-2.0-or-later
//
// The grammar of an action, shared everywhere: the daemon runs one per gesture,
// the power menu runs one per button, and both KCMs edit them.
//
//   none                     do nothing
//   builtin:<id>             one of the built-ins below
//   app:<desktop-id>         launch <desktop-id>.desktop (the name without .desktop)
//   command:<shell>          run <shell> with /bin/sh -c
#pragma once

#include <QString>

namespace keyconfig {

struct Builtin {
	const char *id;
	const char *label;
	const char *icon;
	bool inMenu;     // offered as a power-menu button
	bool inGesture;  // offered as a gesture binding
};

// The order is the order the pickers list them in.
inline const Builtin kBuiltins[] = {
	{"power-off",       "Power off",          "system-shutdown-symbolic",        true,  true},
	{"restart",         "Restart",            "system-reboot-symbolic",          true,  true},
	{"lock-screen",     "Lock screen",        "system-lock-screen-symbolic",     true,  true},
	{"screenshot",      "Screenshot",         "camera-photo-symbolic",           true,  true},
	{"torch-toggle",    "Flashlight",         "flashlight-on-symbolic",          true,  true},
	{"brightness-up",   "Brightness up",      "brightness-high-symbolic",        true,  true},
	{"brightness-down", "Brightness down",    "brightness-low-symbolic",         true,  true},
	// Gesture-only: a power menu button that toggles the screen or opens the
	// power menu would be nonsense (you are already looking at the menu).
	{"screen-toggle",   "Screen off / on",    "system-suspend-symbolic",         false, true},
	{"power-menu",      "Power menu",         "system-shutdown-symbolic",        false, true},
};

struct Spec {
	enum Kind { None, Builtin, App, Command };
	Kind kind = None;
	QString payload;   // builtin id, desktop id, or the shell command

	static Spec parse(const QString &s)
	{
		Spec out;
		const QString t = s.trimmed();
		if (t.isEmpty() || t == QLatin1String("none"))
			return out;
		if (t.startsWith(QLatin1String("builtin:"))) {
			out.kind = Builtin;
			out.payload = t.mid(8);
		} else if (t.startsWith(QLatin1String("app:"))) {
			out.kind = App;
			out.payload = t.mid(4);
		} else if (t.startsWith(QLatin1String("command:"))) {
			out.kind = Command;
			out.payload = t.mid(8);
		}
		return out;
	}

	QString toString() const
	{
		switch (kind) {
		case Builtin: return QStringLiteral("builtin:") + payload;
		case App: return QStringLiteral("app:") + payload;
		case Command: return QStringLiteral("command:") + payload;
		case None: break;
		}
		return QStringLiteral("none");
	}
};

inline const Builtin *builtinById(const QString &id)
{
	for (const Builtin &b : kBuiltins) {
		if (id == QLatin1String(b.id))
			return &b;
	}
	return nullptr;
}

inline QString builtinLabel(const QString &id)
{
	const Builtin *b = builtinById(id);
	return b ? QString::fromLatin1(b->label) : QString();
}

inline QString builtinIcon(const QString &id)
{
	const Builtin *b = builtinById(id);
	return b ? QString::fromLatin1(b->icon) : QString();
}

// The gesture-binding slots, as the config file names them.
namespace slot {
inline const char *PowerRelease = "power_release";
inline const char *PowerHold = "power_hold";
inline const char *PowerVolumeUp = "power_volume_up";
inline const char *PowerVolumeDown = "power_volume_down";
inline const char *DoubleTap = "double_tap";
}

} // namespace keyconfig
