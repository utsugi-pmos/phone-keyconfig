// SPDX-License-Identifier: GPL-2.0-or-later
#include "powermenubackend.h"

namespace keyconfig {

PowerMenuBackend::PowerMenuBackend(QObject *parent)
	: QObject(parent)
	, m_library(new ActionLibrary(this))
	, m_menu(new MenuModel(m_library, this))
{
	// When a custom action's name or icon changes in the library, the menu
	// rows that use it must redraw.
	connect(m_library, &ActionLibrary::dirtyChanged, this, [this] { m_menu->refresh(); });
	load();
}

void PowerMenuBackend::load()
{
	m_library->load();
	m_menu->load();
}

void PowerMenuBackend::restoreDefaults()
{
	m_menu->restoreDefaults();
}

} // namespace keyconfig
