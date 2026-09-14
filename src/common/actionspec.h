// SPDX-License-Identifier: GPL-2.0-or-later
//
// The grammar of a binding, shared by the daemon (which runs them) and the app
// (which edits them):
//
//   none                     do nothing
//   builtin:<id>             one of the actions below
//   app:<desktop-id>         launch <desktop-id>.desktop (the name without .desktop)
//   command:<shell>          run <shell> with /bin/sh -c
#pragma once

#include <QString>
#include <QStringList>

namespace keyconfig {

struct Builtin {
	const char *id;
	const char *label;
};

// The order is the order the app lists them in.
inline const Builtin kBuiltins[] = {
	{"screen-toggle", "Screen off / on"},
	{"power-menu", "Power menu"},
	{"screenshot", "Screenshot"},
	{"torch-toggle", "Flashlight on / off"},
	{"brightness-up", "Brightness up"},
	{"brightness-down", "Brightness down"},
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

inline QString builtinLabel(const QString &id)
{
	for (const Builtin &b : kBuiltins) {
		if (id == QLatin1String(b.id))
			return QString::fromLatin1(b.label);
	}
	return QString();
}

// The binding slots, as the config file names them.
namespace slot {
inline const char *PowerRelease = "power_release";
inline const char *PowerHold = "power_hold";
inline const char *PowerVolumeUp = "power_volume_up";
inline const char *PowerVolumeDown = "power_volume_down";
inline const char *DoubleTap = "double_tap";
}

} // namespace keyconfig
