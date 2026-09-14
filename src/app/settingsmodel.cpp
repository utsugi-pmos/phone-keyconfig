// SPDX-License-Identifier: GPL-2.0-or-later
#include "settingsmodel.h"

#include "actionspec.h"

#include <QDir>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QVariantMap>

#include <algorithm>

namespace keyconfig {

SettingsModel::SettingsModel(QObject *parent)
	: QObject(parent)
{
	reload();
}

void SettingsModel::reload()
{
	m_values = Settings::load();
	Q_EMIT changed();
}

void SettingsModel::setHoldMenuMs(int ms)
{
	Settings::set(QStringLiteral("timing/hold_menu_ms"), ms);
	reload();
}

void SettingsModel::setDoubleTapMs(int ms)
{
	Settings::set(QStringLiteral("timing/double_tap_ms"), ms);
	reload();
}

void SettingsModel::setForgivenessMs(int ms)
{
	Settings::set(QStringLiteral("timing/volume_forgiveness_ms"), ms);
	reload();
}

void SettingsModel::setBrightnessStepPercent(int percent)
{
	Settings::set(QStringLiteral("hardware/brightness_step_percent"), percent);
	reload();
}

void SettingsModel::setTorchBrightness(int value)
{
	Settings::set(QStringLiteral("hardware/torch_brightness"), value);
	reload();
}

QString SettingsModel::binding(const QString &slot) const
{
	return m_values.binding(slot);
}

void SettingsModel::setBinding(const QString &slot, const QString &spec)
{
	// Normalise through the parser so the file never holds a malformed string.
	Settings::set(QStringLiteral("bindings/") + slot, Spec::parse(spec).toString());
	reload();
}

QVariantList SettingsModel::builtins() const
{
	QVariantList out;
	for (const Builtin &b : kBuiltins) {
		QVariantMap m;
		m.insert(QStringLiteral("id"), QString::fromLatin1(b.id));
		m.insert(QStringLiteral("label"), QString::fromLatin1(b.label));
		out.append(m);
	}
	return out;
}

void SettingsModel::scanApps() const
{
	if (m_appsScanned)
		return;
	m_appsScanned = true;

	// User directories come first in standardLocations, so a user's own
	// .desktop overrides a system one of the same id, as the launcher does.
	QSet<QString> seen;
	QList<QVariantMap> found;
	const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	for (const QString &dirPath : dirs) {
		const QDir dir(dirPath);
		const QStringList files = dir.entryList({QStringLiteral("*.desktop")}, QDir::Files);
		for (const QString &file : files) {
			const QString id = file.chopped(8);   // strip ".desktop"
			if (seen.contains(id))
				continue;
			QSettings entry(dir.filePath(file), QSettings::IniFormat);
			entry.beginGroup(QStringLiteral("Desktop Entry"));
			const bool isApp = entry.value(QStringLiteral("Type")).toString() == QLatin1String("Application");
			const bool hidden = entry.value(QStringLiteral("NoDisplay"), false).toBool()
				|| entry.value(QStringLiteral("Hidden"), false).toBool();
			const QString exec = entry.value(QStringLiteral("Exec")).toString();
			const QString name = entry.value(QStringLiteral("Name")).toString();
			const QString icon = entry.value(QStringLiteral("Icon")).toString();
			entry.endGroup();
			seen.insert(id);
			if (!isApp || hidden || exec.isEmpty() || name.isEmpty())
				continue;
			QVariantMap m;
			m.insert(QStringLiteral("id"), id);
			m.insert(QStringLiteral("name"), name);
			m.insert(QStringLiteral("icon"), icon);
			found.append(m);
		}
	}
	std::sort(found.begin(), found.end(), [](const QVariantMap &a, const QVariantMap &b) {
		return a.value(QStringLiteral("name")).toString().localeAwareCompare(
			b.value(QStringLiteral("name")).toString()) < 0;
	});
	m_apps.clear();
	for (const QVariantMap &m : found)
		m_apps.append(m);
}

QVariantList SettingsModel::apps() const
{
	scanApps();
	return m_apps;
}

QString SettingsModel::labelFor(const QString &specString) const
{
	const Spec spec = Spec::parse(specString);
	switch (spec.kind) {
	case Spec::None:
		return QStringLiteral("Nothing");
	case Spec::Builtin: {
		const QString l = builtinLabel(spec.payload);
		return l.isEmpty() ? spec.payload : l;
	}
	case Spec::App: {
		scanApps();
		for (const QVariant &v : m_apps) {
			const QVariantMap m = v.toMap();
			if (m.value(QStringLiteral("id")).toString() == spec.payload)
				return QStringLiteral("Open ") + m.value(QStringLiteral("name")).toString();
		}
		return QStringLiteral("Open ") + spec.payload;
	}
	case Spec::Command:
		return QStringLiteral("Run: ") + spec.payload;
	}
	return QString();
}

} // namespace keyconfig
