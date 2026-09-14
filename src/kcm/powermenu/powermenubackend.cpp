// SPDX-License-Identifier: GPL-2.0-or-later
#include "powermenubackend.h"

namespace keyconfig {

PowerMenuBackend::PowerMenuBackend(QObject *parent)
	: QObject(parent)
	, m_library(new ActionLibrary(this))
	, m_menu(new MenuModel(m_library, this))
{
	connect(m_menu, &MenuModel::dirtyChanged, this, &PowerMenuBackend::dirtyChanged);
	connect(m_library, &ActionLibrary::dirtyChanged, this, [this] {
		m_menu->refresh();               // names/icons may have changed
		Q_EMIT dirtyChanged();
	});
	load();
}

void PowerMenuBackend::load()
{
	m_library->load();
	m_menu->load();
	Q_EMIT dirtyChanged();
}

void PowerMenuBackend::save()
{
	m_library->save();
	m_menu->save();
	Q_EMIT dirtyChanged();
}

void PowerMenuBackend::restoreDefaults()
{
	m_menu->restoreDefaults();
	Q_EMIT dirtyChanged();
}

} // namespace keyconfig
