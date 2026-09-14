// SPDX-License-Identifier: GPL-2.0-or-later
//
// Create or edit one named action in the shared library: open one of the
// installed apps, or run a command. Shared by both KCMs.
//
// Parameters (set by kcm.push):
//   library   the ActionLibrary
//   editId    the id of a custom action to edit, or "" to create
//   onSaved   function(actionId) called after saving
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.ScrollViewKCM {
    id: editor
    title: editId.length ? i18n("Edit action") : i18n("New action")

    property var library
    property string editId: ""
    property var onSaved: function(id) {}

    property string mode: "app"        // "app" or "command"
    property string appId: ""
    // true until the user types a name/icon by hand, so picking an app can fill them
    property bool nameAuto: true
    property bool iconAuto: true

    readonly property var appList: library ? library.apps() : []

    Component.onCompleted: {
        if (editId.length && library) {
            nameField.text = library.nameOf(editId); nameAuto = false
            iconField.text = library.iconOf(editId); iconAuto = false
            const spec = library.specOf(editId)
            if (spec.indexOf("app:") === 0) { editor.mode = "app"; editor.appId = spec.substring(4) }
            else if (spec.indexOf("command:") === 0) { editor.mode = "command"; commandField.text = spec.substring(8) }
        }
    }

    function buildSpec() {
        if (editor.mode === "app")
            return editor.appId.length ? "app:" + editor.appId : "none"
        return commandField.text.trim().length ? "command:" + commandField.text.trim() : "none"
    }

    function pickApp(app) {
        editor.appId = app.id
        if (editor.nameAuto) nameField.text = app.name
        if (editor.iconAuto) iconField.text = app.icon
    }

    function commit() {
        const spec = buildSpec()
        if (spec === "none") return
        if (editor.editId.length) {
            library.editCustom(editor.editId, nameField.text, iconField.text, spec)
            editor.onSaved(editor.editId)
        } else {
            editor.onSaved(library.addCustom(nameField.text, iconField.text, spec))
        }
        kcm.pop()
    }

    header: Controls.Control {
        padding: Kirigami.Units.largeSpacing
        contentItem: ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: i18n("Name"); Layout.preferredWidth: Kirigami.Units.gridUnit * 4 }
                Controls.TextField {
                    id: nameField; Layout.fillWidth: true
                    placeholderText: i18n("Shown on the button")
                    onTextEdited: editor.nameAuto = false
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Controls.Label { text: i18n("Icon"); Layout.preferredWidth: Kirigami.Units.gridUnit * 4 }
                Kirigami.Icon { source: iconField.text || "system-run-symbolic"; Layout.preferredWidth: Kirigami.Units.iconSizes.medium; Layout.preferredHeight: Kirigami.Units.iconSizes.medium }
                Controls.TextField {
                    id: iconField; Layout.fillWidth: true
                    placeholderText: i18n("icon name, optional")
                    onTextEdited: editor.iconAuto = false
                }
            }
            Controls.ButtonGroup { id: modeGroup }
            RowLayout {
                Layout.fillWidth: true
                Controls.RadioButton { text: i18n("Open an app"); checked: editor.mode === "app"; Controls.ButtonGroup.group: modeGroup; onToggled: if (checked) editor.mode = "app" }
                Controls.RadioButton { text: i18n("Run a command"); checked: editor.mode === "command"; Controls.ButtonGroup.group: modeGroup; onToggled: if (checked) editor.mode = "command" }
                Item { Layout.fillWidth: true }
                Controls.Button { text: i18n("Save"); icon.name: "document-save-symbolic"; enabled: editor.buildSpec() !== "none"; onClicked: editor.commit() }
            }
            Kirigami.SearchField {
                id: appSearch
                visible: editor.mode === "app"
                Layout.fillWidth: true
                placeholderText: i18n("Search apps")
            }
            Controls.TextField {
                id: commandField
                visible: editor.mode === "command"
                Layout.fillWidth: true
                placeholderText: i18n("Runs with /bin/sh -c, e.g. nmcli con up VPN")
            }
        }
    }

    // The list of installed apps to open.
    view: ListView {
        model: editor.mode === "app" ? editor.appList : []
        delegate: Controls.ItemDelegate {
            required property var modelData
            width: ListView.view.width
            text: modelData.name
            icon.name: modelData.icon
            highlighted: editor.appId === modelData.id
            visible: appSearch.text.length === 0 || modelData.name.toLowerCase().indexOf(appSearch.text.toLowerCase()) !== -1
            height: visible ? implicitHeight : 0
            onClicked: editor.pickApp(modelData)
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
            visible: editor.mode === "app" && editor.appList.length === 0
            text: i18n("No apps found")
        }
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 4
            visible: editor.mode === "command"
            icon.name: "utilities-terminal-symbolic"
            text: i18n("Type a command above")
        }
    }

    footer: editor.editId.length ? deleteBar : null
    Component {
        id: deleteBar
        Controls.ToolBar {
            contentItem: RowLayout {
                Item { Layout.fillWidth: true }
                Controls.Button {
                    text: i18n("Delete this action")
                    icon.name: "edit-delete-symbolic"
                    onClicked: { editor.library.removeCustom(editor.editId); kcm.pop() }
                }
            }
        }
    }
}
