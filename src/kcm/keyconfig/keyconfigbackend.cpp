// SPDX-License-Identifier: GPL-2.0-or-later
#include "keyconfigbackend.h"

#include "actionspec.h"

namespace keyconfig {

KeyconfigBackend::KeyconfigBackend(QObject *parent)
	: QObject(parent)
	, m_library(new ActionLibrary(this))
{
	connect(m_library, &ActionLibrary::dirtyChanged, this, &KeyconfigBackend::dirtyChanged);
	load();
}

void KeyconfigBackend::markDirty()
{
	if (!m_dirty) {
		m_dirty = true;
		Q_EMIT dirtyChanged();
	}
}

QString &KeyconfigBackend::slotRef(const QString &slotName)
{
	if (slotName == QLatin1String(slot::PowerRelease)) return m_v.powerRelease;
	if (slotName == QLatin1String(slot::PowerHold)) return m_v.powerHold;
	if (slotName == QLatin1String(slot::PowerVolumeUp)) return m_v.powerVolumeUp;
	if (slotName == QLatin1String(slot::PowerVolumeDown)) return m_v.powerVolumeDown;
	return m_v.doubleTap;   // PowerDouble / DoubleTap
}

QString KeyconfigBackend::binding(const QString &slotName) const
{
	return const_cast<KeyconfigBackend *>(this)->slotRef(slotName);
}

void KeyconfigBackend::setBinding(const QString &slotName, const QString &actionId)
{
	QString &ref = slotRef(slotName);
	if (ref == actionId)
		return;
	ref = actionId;
	Q_EMIT changed();
	markDirty();
}

void KeyconfigBackend::setHoldMenuMs(int v) { if (int(m_v.timing.holdMenuMs) == v) return; m_v.timing.holdMenuMs = v; Q_EMIT changed(); markDirty(); }
void KeyconfigBackend::setDoubleTapMs(int v) { if (int(m_v.timing.doubleTapMs) == v) return; m_v.timing.doubleTapMs = v; Q_EMIT changed(); markDirty(); }
void KeyconfigBackend::setForgivenessMs(int v) { if (int(m_v.timing.forgivenessMs) == v) return; m_v.timing.forgivenessMs = v; Q_EMIT changed(); markDirty(); }
void KeyconfigBackend::setBrightnessStepPercent(int v) { if (m_v.brightnessStepPercent == v) return; m_v.brightnessStepPercent = v; Q_EMIT changed(); markDirty(); }
void KeyconfigBackend::setTorchBrightness(int v) { if (m_v.torchBrightness == v) return; m_v.torchBrightness = v; Q_EMIT changed(); markDirty(); }

void KeyconfigBackend::load()
{
	m_library->load();
	m_v = Settings::load();
	m_dirty = false;
	Q_EMIT changed();
	Q_EMIT dirtyChanged();
}

void KeyconfigBackend::save()
{
	m_library->save();
	Settings::set(QStringLiteral("bindings/") + QLatin1String(slot::PowerRelease), m_v.powerRelease);
	Settings::set(QStringLiteral("bindings/") + QLatin1String(slot::PowerHold), m_v.powerHold);
	Settings::set(QStringLiteral("bindings/") + QLatin1String(slot::PowerVolumeUp), m_v.powerVolumeUp);
	Settings::set(QStringLiteral("bindings/") + QLatin1String(slot::PowerVolumeDown), m_v.powerVolumeDown);
	Settings::set(QStringLiteral("bindings/") + QLatin1String(slot::DoubleTap), m_v.doubleTap);
	Settings::set(QStringLiteral("timing/hold_menu_ms"), int(m_v.timing.holdMenuMs));
	Settings::set(QStringLiteral("timing/double_tap_ms"), int(m_v.timing.doubleTapMs));
	Settings::set(QStringLiteral("timing/volume_forgiveness_ms"), int(m_v.timing.forgivenessMs));
	Settings::set(QStringLiteral("hardware/brightness_step_percent"), m_v.brightnessStepPercent);
	Settings::set(QStringLiteral("hardware/torch_brightness"), m_v.torchBrightness);
	m_dirty = false;
	Q_EMIT dirtyChanged();
}

void KeyconfigBackend::restoreDefaults()
{
	m_v.powerRelease = QStringLiteral("screen-toggle");
	m_v.powerHold = QStringLiteral("power-menu");
	m_v.powerVolumeUp = QStringLiteral("brightness-up");
	m_v.powerVolumeDown = QStringLiteral("screenshot");
	m_v.doubleTap = QStringLiteral("torch-toggle");
	m_v.timing = Config{};
	m_v.brightnessStepPercent = 10;
	m_v.torchBrightness = 100;
	Q_EMIT changed();
	markDirty();
}

} // namespace keyconfig
