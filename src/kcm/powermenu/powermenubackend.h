// SPDX-License-Identifier: GPL-2.0-or-later
//
// Backend for the "Power menu" KCM. No Apply button: the model and the library
// write the config file on every change, and the menu reads it fresh each time
// it opens.
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

public:
	explicit PowerMenuBackend(QObject *parent = nullptr);

	ActionLibrary *library() const { return m_library; }
	MenuModel *menu() const { return m_menu; }

	void load();
	void restoreDefaults();

private:
	ActionLibrary *const m_library;
	MenuModel *const m_menu;
};

} // namespace keyconfig
