// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: "Phone keys"

    // One column on a phone: the bindings page fills the window, and the
    // picker for a slot is pushed on top of it.
    pageStack.initialPage: BindingsPage {}
}
