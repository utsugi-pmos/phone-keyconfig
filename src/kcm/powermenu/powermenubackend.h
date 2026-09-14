// SPDX-License-Identifier: GPL-2.0-or-later
//
// Backend for the "Power menu" KCM: the ordered list of buttons (MenuModel) and
// the shared action library, written only on Apply. dirty() -- the menu OR the
// library -- drives the Apply button.
#pragma once

#include "actionlibrary.h"
#include "menumodel.h"

#include <QObject>

namespace keyconfig {

class PowerMenuBackend : public QObject
{
	Q_OBJECT
	Q_PROPERTY(keyconfig::ActionLibrary *library READ library CONSTANT)
	Q_PROPERTY(keyconfig::MenuModel *menu READ menu CONSTANT)
	Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

public:
	explicit PowerMenuBackend(QObject *parent = nullptr);

	ActionLibrary *library() const { return m_library; }
	MenuModel *menu() const { return m_menu; }
	bool dirty() const { return m_menu->dirty() || m_library->dirty(); }

	void load();
	void save();
	void restoreDefaults();

Q_SIGNALS:
	void dirtyChanged();

private:
	ActionLibrary *const m_library;
	MenuModel *const m_menu;
};

} // namespace keyconfig
