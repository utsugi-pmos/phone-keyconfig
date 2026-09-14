// SPDX-License-Identifier: GPL-2.0-or-later
#include "powermenubackend.h"

#include <KPluginFactory>
#include <KQuickConfigModule>

class KCMPhonePowerMenu : public KQuickConfigModule
{
	Q_OBJECT
	Q_PROPERTY(keyconfig::PowerMenuBackend *backend READ backend CONSTANT)

public:
	KCMPhonePowerMenu(QObject *parent, const KPluginMetaData &data)
		: KQuickConfigModule(parent, data)
		, m_backend(new keyconfig::PowerMenuBackend(this))
	{
		connect(m_backend, &keyconfig::PowerMenuBackend::dirtyChanged, this,
			[this] { setNeedsSave(m_backend->dirty()); });
		setNeedsSave(m_backend->dirty());
	}

	keyconfig::PowerMenuBackend *backend() const { return m_backend; }

	void load() override { m_backend->load(); }
	void save() override { m_backend->save(); }
	void defaults() override { m_backend->restoreDefaults(); }

private:
	keyconfig::PowerMenuBackend *const m_backend;
};

K_PLUGIN_CLASS_WITH_JSON(KCMPhonePowerMenu, "kcm_phone_powermenu.json")

#include "kcm.moc"
