// SPDX-License-Identifier: GPL-2.0-or-later
#include "keyconfigbackend.h"

#include "actionspec.h"

namespace keyconfig {

KeyconfigBackend::KeyconfigBackend(QObject *parent)
	: QObject(parent)
	, m_library(new ActionLibrary(this))
{
	// A custom action edited in the picker should refresh the row that uses it.
	connect(m_library, &ActionLibrary::dirtyChanged, this, &KeyconfigBackend::changed);
	load();
}

QString &KeyconfigBackend::slotRef(const QString &slotName)
{
	if (slotName == QLatin1String(slot::PowerRelease)) return m_v.powerRelease;
	if (slotName == QLatin1String(slot::PowerHold)) return m_v.powerHold;
	if (slotName == QLatin1String(slot::PowerVolumeUp)) return m_v.powerVolumeUp;
	if (slotName == QLatin1String(slot::PowerVolumeDown)) return m_v.powerVolumeDown;
	return m_v.doubleTap;
}

QString KeyconfigBackend::binding(const QString &slotName) const
{
	return const_cast<KeyconfigBackend *>(this)->slotRef(slotName);
}

// Every change is written to the config file at once: this is a phone, there is
// no Apply button. The daemon watches the file and applies it live.
void KeyconfigBackend::setBinding(const QString &slotName, const QString &actionId)
{
	QString &ref = slotRef(slotName);
	if (ref == actionId)
		return;
	ref = actionId;
	Settings::set(QStringLiteral("bindings/") + slotName, actionId);
	Q_EMIT changed();
}

void KeyconfigBackend::setHoldMenuMs(int v) { if (int(m_v.timing.holdMenuMs) == v) return; m_v.timing.holdMenuMs = v; Settings::set(QStringLiteral("timing/hold_menu_ms"), v); Q_EMIT changed(); }
void KeyconfigBackend::setDoubleTapMs(int v) { if (int(m_v.timing.doubleTapMs) == v) return; m_v.timing.doubleTapMs = v; Settings::set(QStringLiteral("timing/double_tap_ms"), v); Q_EMIT changed(); }
void KeyconfigBackend::setForgivenessMs(int v) { if (int(m_v.timing.forgivenessMs) == v) return; m_v.timing.forgivenessMs = v; Settings::set(QStringLiteral("timing/volume_forgiveness_ms"), v); Q_EMIT changed(); }
void KeyconfigBackend::setBrightnessStepPercent(int v) { if (m_v.brightnessStepPercent == v) return; m_v.brightnessStepPercent = v; Settings::set(QStringLiteral("hardware/brightness_step_percent"), v); Q_EMIT changed(); }
void KeyconfigBackend::setTorchBrightness(int v) { if (m_v.torchBrightness == v) return; m_v.torchBrightness = v; Settings::set(QStringLiteral("hardware/torch_brightness"), v); Q_EMIT changed(); }

void KeyconfigBackend::load()
{
	m_library->load();
	m_v = Settings::load();
	Q_EMIT changed();
}

void KeyconfigBackend::restoreDefaults()
{
	setBinding(QStringLiteral("power_release"), QStringLiteral("screen-toggle"));
	setBinding(QStringLiteral("power_hold"), QStringLiteral("power-menu"));
	setBinding(QStringLiteral("power_volume_up"), QStringLiteral("brightness-up"));
	setBinding(QStringLiteral("power_volume_down"), QStringLiteral("screenshot"));
	setBinding(QStringLiteral("double_tap"), QStringLiteral("torch-toggle"));
	setHoldMenuMs(1500);
	setDoubleTapMs(300);
	setForgivenessMs(150);
	setBrightnessStepPercent(10);
	setTorchBrightness(100);
}

} // namespace keyconfig
