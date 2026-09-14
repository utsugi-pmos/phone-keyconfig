// SPDX-License-Identifier: GPL-2.0-or-later
//
// What the QML sees: the current bindings and timings, the catalogue of
// built-in actions, the installed applications, and a human label for any
// binding string. Every setter writes straight to the user's config file; the
// daemon watches that file and picks the change up on its own.
#pragma once

#include "settings.h"

#include <QObject>
#include <QVariantList>

namespace keyconfig {

class SettingsModel : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString powerRelease READ powerRelease NOTIFY changed)
	Q_PROPERTY(QString powerHold READ powerHold NOTIFY changed)
	Q_PROPERTY(QString powerVolumeUp READ powerVolumeUp NOTIFY changed)
	Q_PROPERTY(QString powerVolumeDown READ powerVolumeDown NOTIFY changed)
	Q_PROPERTY(QString doubleTap READ doubleTap NOTIFY changed)
	Q_PROPERTY(int holdMenuMs READ holdMenuMs WRITE setHoldMenuMs NOTIFY changed)
	Q_PROPERTY(int doubleTapMs READ doubleTapMs WRITE setDoubleTapMs NOTIFY changed)
	Q_PROPERTY(int forgivenessMs READ forgivenessMs WRITE setForgivenessMs NOTIFY changed)
	Q_PROPERTY(int brightnessStepPercent READ brightnessStepPercent WRITE setBrightnessStepPercent NOTIFY changed)
	Q_PROPERTY(int torchBrightness READ torchBrightness WRITE setTorchBrightness NOTIFY changed)

public:
	explicit SettingsModel(QObject *parent = nullptr);

	QString powerRelease() const { return m_values.powerRelease; }
	QString powerHold() const { return m_values.powerHold; }
	QString powerVolumeUp() const { return m_values.powerVolumeUp; }
	QString powerVolumeDown() const { return m_values.powerVolumeDown; }
	QString doubleTap() const { return m_values.doubleTap; }
	int holdMenuMs() const { return int(m_values.timing.holdMenuMs); }
	int doubleTapMs() const { return int(m_values.timing.doubleTapMs); }
	int forgivenessMs() const { return int(m_values.timing.forgivenessMs); }
	int brightnessStepPercent() const { return m_values.brightnessStepPercent; }
	int torchBrightness() const { return m_values.torchBrightness; }

	void setHoldMenuMs(int ms);
	void setDoubleTapMs(int ms);
	void setForgivenessMs(int ms);
	void setBrightnessStepPercent(int percent);
	void setTorchBrightness(int value);

	// slot: one of the names in actionspec.h (power_release, power_hold,
	// power_volume_up, power_volume_down, double_tap). spec: a binding string.
	Q_INVOKABLE QString binding(const QString &slot) const;
	Q_INVOKABLE void setBinding(const QString &slot, const QString &spec);

	// [{id, label}] of the built-in actions, in display order.
	Q_INVOKABLE QVariantList builtins() const;
	// [{id, name, icon}] of every launchable application, sorted by name.
	Q_INVOKABLE QVariantList apps() const;
	// A human label for a binding string.
	Q_INVOKABLE QString labelFor(const QString &spec) const;

Q_SIGNALS:
	void changed();

private:
	void reload();
	void scanApps() const;

	Values m_values;
	mutable QVariantList m_apps;
	mutable bool m_appsScanned = false;
};

} // namespace keyconfig
