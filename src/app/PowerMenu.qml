// SPDX-License-Identifier: GPL-2.0-or-later
//
// The power menu phone-keyconfig draws itself, because Plasma Mobile's logout
// greeter does not render on this shell. A full-screen dimmed overlay with a
// card of big touch targets; tapping outside, Cancel, or the power/back key
// closes it. Launched as `phone-keyconfig --power-menu`.
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQuick.Window
import org.kde.kirigami as Kirigami

Window {
    id: root
    visibility: Window.FullScreen
    flags: Qt.Window | Qt.FramelessWindowHint
    color: "transparent"
    title: "Power"

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

    Control {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 3, Kirigami.Units.gridUnit * 22)
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
