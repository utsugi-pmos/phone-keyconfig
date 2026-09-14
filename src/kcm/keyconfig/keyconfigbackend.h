// SPDX-License-Identifier: GPL-2.0-or-later
//
// Backend for the "Phone keys" KCM. There is no Apply button -- this is a phone
// -- so every setter writes the config file at once; the daemon watches it and
// applies the change live. The library exposes its actions to the shared
// ActionPicker.
#pragma once

#include "actionlibrary.h"
#include "settings.h"

#include <QObject>

namespace keyconfig {

class KeyconfigBackend : public QObject
{
	Q_OBJECT
	Q_PROPERTY(keyconfig::ActionLibrary *library READ library CONSTANT)
	Q_PROPERTY(int holdMenuMs READ holdMenuMs WRITE setHoldMenuMs NOTIFY changed)
	Q_PROPERTY(int doubleTapMs READ doubleTapMs WRITE setDoubleTapMs NOTIFY changed)
	Q_PROPERTY(int forgivenessMs READ forgivenessMs WRITE setForgivenessMs NOTIFY changed)
	Q_PROPERTY(int brightnessStepPercent READ brightnessStepPercent WRITE setBrightnessStepPercent NOTIFY changed)
	Q_PROPERTY(int torchBrightness READ torchBrightness WRITE setTorchBrightness NOTIFY changed)

public:
	explicit KeyconfigBackend(QObject *parent = nullptr);

	ActionLibrary *library() const { return m_library; }

	int holdMenuMs() const { return int(m_v.timing.holdMenuMs); }
	int doubleTapMs() const { return int(m_v.timing.doubleTapMs); }
	int forgivenessMs() const { return int(m_v.timing.forgivenessMs); }
	int brightnessStepPercent() const { return m_v.brightnessStepPercent; }
	int torchBrightness() const { return m_v.torchBrightness; }

	void setHoldMenuMs(int v);
	void setDoubleTapMs(int v);
	void setForgivenessMs(int v);
	void setBrightnessStepPercent(int v);
	void setTorchBrightness(int v);

	Q_INVOKABLE QString binding(const QString &slot) const;
	Q_INVOKABLE void setBinding(const QString &slot, const QString &actionId);

	void load();
	void restoreDefaults();

Q_SIGNALS:
	void changed();

private:
	QString &slotRef(const QString &slot);

	ActionLibrary *const m_library;
	Values m_v;
};

} // namespace keyconfig
