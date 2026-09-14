// SPDX-License-Identifier: GPL-2.0-or-later
#include "keyconfigbackend.h"

#include <KPluginFactory>
#include <KQuickConfigModule>

class KCMPhoneKeyconfig : public KQuickConfigModule
{
	Q_OBJECT
	Q_PROPERTY(keyconfig::KeyconfigBackend *backend READ backend CONSTANT)

public:
	KCMPhoneKeyconfig(QObject *parent, const KPluginMetaData &data)
		: KQuickConfigModule(parent, data)
		, m_backend(new keyconfig::KeyconfigBackend(this))
	{
		connect(m_backend, &keyconfig::KeyconfigBackend::dirtyChanged, this,
			[this] { setNeedsSave(m_backend->dirty()); });
		setNeedsSave(m_backend->dirty());
	}

	keyconfig::KeyconfigBackend *backend() const { return m_backend; }

	void load() override { m_backend->load(); }
	void save() override { m_backend->save(); }
	void defaults() override { m_backend->restoreDefaults(); }

private:
	keyconfig::KeyconfigBackend *const m_backend;
};

K_PLUGIN_CLASS_WITH_JSON(KCMPhoneKeyconfig, "kcm_phone_keyconfig.json")

#include "kcm.moc"
