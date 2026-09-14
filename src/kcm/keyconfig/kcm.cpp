// SPDX-License-Identifier: GPL-2.0-or-later
#include "keyconfigbackend.h"

#include <KPluginFactory>
#include <KQuickConfigModule>

// No Apply button: every change is saved the instant it is made (see the
// backend). So this never calls setNeedsSave, and save() is a no-op.
class KCMPhoneKeyconfig : public KQuickConfigModule
{
	Q_OBJECT
	Q_PROPERTY(keyconfig::KeyconfigBackend *backend READ backend CONSTANT)

public:
	KCMPhoneKeyconfig(QObject *parent, const KPluginMetaData &data)
		: KQuickConfigModule(parent, data)
		, m_backend(new keyconfig::KeyconfigBackend(this))
	{
	}

	keyconfig::KeyconfigBackend *backend() const { return m_backend; }

	void load() override { m_backend->load(); }
	void save() override {}
	void defaults() override { m_backend->restoreDefaults(); }

private:
	keyconfig::KeyconfigBackend *const m_backend;
};

K_PLUGIN_CLASS_WITH_JSON(KCMPhoneKeyconfig, "kcm_phone_keyconfig.json")

#include "kcm.moc"
