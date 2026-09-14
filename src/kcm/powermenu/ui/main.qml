// SPDX-License-Identifier: GPL-2.0-or-later
//
// The "Power menu" KCM: order the buttons, turn them on or off, add or remove
// them, and choose list or grid. The three defaults (Power off, Restart, Lock)
// can be disabled but not removed.
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: root

    readonly property var backend: kcm.backend
    readonly property var menu: kcm.backend.menu

    header: RowLayout {
        Controls.Label { text: i18n("Layout"); Layout.leftMargin: Kirigami.Units.largeSpacing }
        Controls.ButtonGroup { id: layoutGroup }
        Controls.RadioButton { text: i18n("List"); checked: root.menu.layout === "list"; Controls.ButtonGroup.group: layoutGroup; onToggled: if (checked) root.menu.layout = "list" }
        Controls.RadioButton { text: i18n("Grid"); checked: root.menu.layout === "grid"; Controls.ButtonGroup.group: layoutGroup; onToggled: if (checked) root.menu.layout = "grid" }
        Item { Layout.fillWidth: true }
        Controls.Button {
            text: i18n("Add button")
            icon.name: "list-add-symbolic"
            onClicked: kcm.push("ActionPicker.qml", {
                library: root.backend.library,
                context: "menu",
                allowNone: false,
                onPicked: function(id) { root.menu.addAction(id) }
            })
        }
    }

    view: ListView {
        id: list
        model: root.menu
        spacing: 0

        delegate: Controls.ItemDelegate {
            id: d
            width: ListView.view.width
            // model.* rather than "required property", because a role named
            // icon or enabled would shadow ItemDelegate's own FINAL properties.
            required property int index

            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing
                ColumnLayout {
                    spacing: 0
                    Controls.ToolButton { icon.name: "arrow-up"; enabled: d.index > 0; onClicked: root.menu.move(d.index, d.index - 1); implicitHeight: Kirigami.Units.gridUnit * 1.2 }
                    Controls.ToolButton { icon.name: "arrow-down"; enabled: d.index < list.count - 1; onClicked: root.menu.move(d.index, d.index + 1); implicitHeight: Kirigami.Units.gridUnit * 1.2 }
                }
                Kirigami.Icon { source: model.icon; Layout.preferredWidth: Kirigami.Units.iconSizes.medium; Layout.preferredHeight: Kirigami.Units.iconSizes.medium; opacity: model.enabled ? 1 : 0.4 }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Controls.Label { text: model.name; Layout.fillWidth: true; elide: Text.ElideRight; opacity: model.enabled ? 1 : 0.5 }
                    Controls.Label { text: model.summary; font: Kirigami.Theme.smallFont; opacity: 0.6; Layout.fillWidth: true; elide: Text.ElideRight }
                }
                Controls.ToolButton {
                    visible: model.removable
                    icon.name: "edit-delete-symbolic"
                    onClicked: root.menu.remove(d.index)
                }
                Controls.Switch {
                    checked: model.enabled
                    onToggled: root.menu.setEnabled(d.index, checked)
                }
            }
        }

        footer: Controls.Label {
            width: ListView.view.width
            padding: Kirigami.Units.largeSpacing
            text: i18n("Power off, Restart and Lock screen can be turned off but not removed.")
            font: Kirigami.Theme.smallFont
            opacity: 0.7
            wrapMode: Text.WordWrap
        }
    }
}
