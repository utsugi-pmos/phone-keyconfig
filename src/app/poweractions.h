// SPDX-License-Identifier: GPL-2.0-or-later
//
// The buttons of the power menu, as D-Bus calls. Plasma Mobile's own logout
// greeter does not render on this shell -- promptAll starts plasma-shutdown and
// nothing appears -- so phone-keyconfig draws the menu itself (PowerMenu.qml)
// and these are what its buttons do. Each is what the desktop's own menu would
// end up calling, so no privilege beyond the session is needed.
#pragma once

#include <QObject>

namespace keyconfig {

class PowerActions : public QObject
{
	Q_OBJECT

public:
	explicit PowerActions(QObject *parent = nullptr);

	Q_INVOKABLE void powerOff();
	Q_INVOKABLE void reboot();
	Q_INVOKABLE void lockScreen();
};

} // namespace keyconfig
