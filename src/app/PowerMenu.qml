// SPDX-License-Identifier: GPL-2.0-or-later
//
// The power menu phone-keyconfig draws itself, because Plasma Mobile's logout
// greeter does not render on this shell -- and neither does an ordinary window,
// which the mobile shell will not raise over the running app. So this is a
// LAYER-SHELL overlay, the same mechanism screenglaze's sheet uses: the
// compositor puts it on top of everything, panel and task switcher included,
// and it never becomes an entry in the task bar. Launched as
// `phone-keyconfig --power-menu`; main.cpp turns on the layer-shell integration
// before QGuiApplication for exactly this window.
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.layershell as LayerShell

Window {
    id: root
    visible: true
    color: "transparent"
    flags: Qt.FramelessWindowHint

    // Cover the whole output and sit above everything.
    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop
        | LayerShell.Window.AnchorBottom
        | LayerShell.Window.AnchorLeft
        | LayerShell.Window.AnchorRight
    LayerShell.Window.exclusionZone: -1
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityOnDemand
    LayerShell.Window.scope: "phone-keyconfig"

    // A size for the fallback case where layer-shell is off; with it on the
    // compositor overrides both.
    width: Screen.width
    height: Screen.height

    // Close on Escape / the back gesture.
    Shortcut {
        sequences: [StandardKey.Cancel, "Esc", "Back"]
        onActivated: Qt.quit()
    }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.55
        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()   // tap outside the card dismisses
        }
    }

    Controls.Control {
        anchors.centerIn: parent
        width: Math.min(root.width - Kirigami.Units.gridUnit * 3, Kirigami.Units.gridUnit * 22)
        padding: Kirigami.Units.largeSpacing
        background: Rectangle {
            radius: Kirigami.Units.cornerRadius > 0 ? Kirigami.Units.cornerRadius : 12
            color: Kirigami.Theme.backgroundColor
        }
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: "Power"
                level: 1
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: Kirigami.Units.smallSpacing
            }

            MenuButton {
                text: "Power off"
                icon.name: "system-shutdown-symbolic"
                destructive: true
                onClicked: { PowerMenu.powerOff(); Qt.quit() }
            }
            MenuButton {
                text: "Restart"
                icon.name: "system-reboot-symbolic"
                onClicked: { PowerMenu.reboot(); Qt.quit() }
            }
            MenuButton {
                text: "Lock screen"
                icon.name: "system-lock-screen-symbolic"
                onClicked: { PowerMenu.lockScreen(); Qt.quit() }
            }
            MenuButton {
                text: "Cancel"
                icon.name: "dialog-cancel-symbolic"
                onClicked: Qt.quit()
            }
        }
    }

    // A tall, easy-to-hit row.
    component MenuButton: Controls.Button {
        property bool destructive: false
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
        display: Controls.AbstractButton.TextBesideIcon
        icon.width: Kirigami.Units.iconSizes.medium
        icon.height: Kirigami.Units.iconSizes.medium
        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.15
        palette.buttonText: destructive ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
    }
}
