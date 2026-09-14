// SPDX-License-Identifier: GPL-2.0-or-later
//
// Backend for the "Phone keys" KCM: the gesture bindings (each an action id
// from the shared library), the timings, and the library itself, exposed so
// the shared ActionPicker can edit it. Everything is held in memory and written
// only on Apply (save()); dirty() -- own edits OR a library edit -- drives the
// Apply button. The daemon watches the file and applies the change once saved.
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
	Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

public:
	explicit KeyconfigBackend(QObject *parent = nullptr);

	ActionLibrary *library() const { return m_library; }

	int holdMenuMs() const { return int(m_v.timing.holdMenuMs); }
	int doubleTapMs() const { return int(m_v.timing.doubleTapMs); }
	int forgivenessMs() const { return int(m_v.timing.forgivenessMs); }
	int brightnessStepPercent() const { return m_v.brightnessStepPercent; }
	int torchBrightness() const { return m_v.torchBrightness; }
	bool dirty() const { return m_dirty || m_library->dirty(); }

	void setHoldMenuMs(int v);
	void setDoubleTapMs(int v);
	void setForgivenessMs(int v);
	void setBrightnessStepPercent(int v);
	void setTorchBrightness(int v);

	// The action id bound to a slot (power_release, power_hold, ...), or "none".
	Q_INVOKABLE QString binding(const QString &slot) const;
	Q_INVOKABLE void setBinding(const QString &slot, const QString &actionId);

	void load();
	void save();
	void restoreDefaults();

Q_SIGNALS:
	void changed();
	void dirtyChanged();

private:
	void markDirty();
	QString &slotRef(const QString &slot);

	ActionLibrary *const m_library;
	Values m_v;
	bool m_dirty = false;
};

} // namespace keyconfig
