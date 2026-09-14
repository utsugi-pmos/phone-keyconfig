// SPDX-License-Identifier: GPL-2.0-or-later
//
// The power menu overlay: a dimmed full-screen layer-shell surface with a card
// of buttons, laid out as a list or a grid per the config. Each button runs its
// action and closes; tapping outside or the back key closes.
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

    LayerShell.Window.layer: LayerShell.Window.LayerOverlay
    LayerShell.Window.anchors: LayerShell.Window.AnchorTop | LayerShell.Window.AnchorBottom
        | LayerShell.Window.AnchorLeft | LayerShell.Window.AnchorRight
    LayerShell.Window.exclusionZone: -1
    LayerShell.Window.keyboardInteractivity: LayerShell.Window.KeyboardInteractivityOnDemand
    LayerShell.Window.scope: "phone-keyconfig"

    width: Screen.width
    height: Screen.height

    readonly property var items: MenuModel.enabledItems()
    readonly property bool grid: MenuModel.layout === "grid"

    Shortcut { sequences: [StandardKey.Cancel, "Esc", "Back"]; onActivated: Qt.quit() }

    function fire(actionId) { Runner.runById(actionId); Qt.quit() }

    Rectangle {
        anchors.fill: parent
        color: "#000000"
        opacity: 0.55
        MouseArea { anchors.fill: parent; onClicked: Qt.quit() }
    }

    Controls.Control {
        anchors.centerIn: parent
        width: Math.min(root.width - Kirigami.Units.gridUnit * 3, Kirigami.Units.gridUnit * 24)
        padding: Kirigami.Units.largeSpacing
        background: Rectangle {
            radius: Kirigami.Units.cornerRadius > 0 ? Kirigami.Units.cornerRadius : 12
            color: Kirigami.Theme.backgroundColor
        }
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            Kirigami.Heading { text: "Power"; level: 1; Layout.alignment: Qt.AlignHCenter }

            // Grid
            GridLayout {
                visible: root.grid
                Layout.fillWidth: true
                columns: 3
                columnSpacing: Kirigami.Units.largeSpacing
                rowSpacing: Kirigami.Units.largeSpacing
                Repeater {
                    model: root.grid ? root.items : []
                    delegate: Controls.Button {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 4.5
                        display: Controls.AbstractButton.TextUnderIcon
                        icon.name: modelData.icon
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium
                        text: modelData.name
                        onClicked: root.fire(modelData.actionId)
                    }
                }
            }

            // List
            ColumnLayout {
                visible: !root.grid
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing
                Repeater {
                    model: root.grid ? [] : root.items
                    delegate: Controls.Button {
                        required property var modelData
                        Layout.fillWidth: true
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                        display: Controls.AbstractButton.TextBesideIcon
                        icon.name: modelData.icon
                        icon.width: Kirigami.Units.iconSizes.medium
                        icon.height: Kirigami.Units.iconSizes.medium
                        text: modelData.name
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                        onClicked: root.fire(modelData.actionId)
                    }
                }
            }
        }
    }
}
