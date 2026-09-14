// SPDX-License-Identifier: GPL-2.0-or-later
//
// THE reusable piece: pick an action from the shared library, for a gesture
// slot or a menu button. Built-in actions and the user's named actions are
// listed together; custom ones can be edited or removed; and "Create action"
// opens the editor, which adds to the library and selects the result.
//
// Parameters (set by kcm.push):
//   library   the ActionLibrary
//   context   "gesture" or "menu" (filters what is offered)
//   allowNone whether to offer "Nothing"
//   onPicked  function(actionId) called when a choice is made
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: picker
    title: i18n("Choose an action")

    property var library
    property string context: "gesture"
    property bool allowNone: true
    property var onPicked: function(id) {}

    // Rebuilt whenever the library changes (a new custom action appears).
    property var actionList: library ? library.list(context) : []
    Connections {
        target: picker.library
        function onDirtyChanged() { picker.actionList = picker.library.list(picker.context) }
    }

    function choose(id) {
        picker.onPicked(id)
        kcm.pop()
    }

    header: Controls.ToolBar {
        contentItem: RowLayout {
            Controls.Label {
                text: i18n("Pick what this does, or create your own action.")
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            Controls.Button {
                text: i18n("Create action")
                icon.name: "list-add-symbolic"
                onClicked: kcm.push("ActionEditor.qml", {
                    library: picker.library,
                    onSaved: function(id) { picker.choose(id) }
                })
            }
        }
    }

    view: ListView {
        model: picker.actionList
        header: picker.allowNone ? noneComponent : null

        Component {
            id: noneComponent
            Controls.ItemDelegate {
                width: ListView.view.width
                text: i18n("Nothing")
                icon.name: "edit-none-symbolic"
                onClicked: picker.choose("none")
            }
        }

        delegate: Controls.ItemDelegate {
            id: row
            required property var modelData
            width: ListView.view.width
            contentItem: RowLayout {
                spacing: Kirigami.Units.smallSpacing
                Kirigami.Icon {
                    source: row.modelData.icon
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0
                    Controls.Label { text: row.modelData.name; Layout.fillWidth: true; elide: Text.ElideRight }
                    Controls.Label {
                        visible: !row.modelData.builtin
                        text: row.modelData.summary
                        font: Kirigami.Theme.smallFont
                        opacity: 0.7
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }
                }
                Controls.ToolButton {
                    visible: !row.modelData.builtin
                    icon.name: "entry-edit-symbolic"
                    onClicked: kcm.push("ActionEditor.qml", {
                        library: picker.library,
                        editId: row.modelData.id,
                        onSaved: function(id) {}
                    })
                }
            }
            onClicked: picker.choose(row.modelData.id)
        }
    }
}
