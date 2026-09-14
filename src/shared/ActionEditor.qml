// SPDX-License-Identifier: GPL-2.0-or-later
//
// Create or edit one named action in the shared library: a name, an icon, and
// either an app to open or a command to run. Shared by both KCMs.
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

KCM.SimpleKCM {
    id: editor
    title: editId.length ? i18n("Edit action") : i18n("New action")

    property var library
    property string editId: ""
    property var onSaved: function(id) {}

    // "app" or "command"
    property string mode: "command"

    Component.onCompleted: {
        if (editId.length && library) {
            nameField.text = library.nameOf(editId)
            iconField.text = library.iconOf(editId)
            const spec = library.specOf(editId)
            if (spec.indexOf("app:") === 0) { editor.mode = "app"; appField.text = spec.substring(4) }
            else if (spec.indexOf("command:") === 0) { editor.mode = "command"; commandField.text = spec.substring(8) }
        }
    }

    function buildSpec() {
        if (editor.mode === "app")
            return appField.text.length ? "app:" + appField.text : "none"
        return commandField.text.trim().length ? "command:" + commandField.text.trim() : "none"
    }

    function commit() {
        const spec = buildSpec()
        if (spec === "none")
            return
        if (editor.editId.length) {
            library.editCustom(editor.editId, nameField.text, iconField.text, spec)
            editor.onSaved(editor.editId)
        } else {
            const id = library.addCustom(nameField.text, iconField.text, spec)
            editor.onSaved(id)
        }
        kcm.pop()
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.FormLayout {
            Layout.fillWidth: true

            Controls.TextField {
                id: nameField
                Kirigami.FormData.label: i18n("Name")
                placeholderText: i18n("e.g. My VPN")
                Layout.fillWidth: true
            }
            RowLayout {
                Kirigami.FormData.label: i18n("Icon")
                Kirigami.Icon { source: iconField.text || "system-run-symbolic"; Layout.preferredWidth: Kirigami.Units.iconSizes.medium; Layout.preferredHeight: Kirigami.Units.iconSizes.medium }
                Controls.TextField {
                    id: iconField
                    placeholderText: i18n("icon name, e.g. network-vpn")
                    Layout.fillWidth: true
                }
            }
            Controls.ButtonGroup { id: modeGroup }
            RowLayout {
                Kirigami.FormData.label: i18n("Does")
                Controls.RadioButton { text: i18n("Open an app"); checked: editor.mode === "app"; Controls.ButtonGroup.group: modeGroup; onToggled: if (checked) editor.mode = "app" }
                Controls.RadioButton { text: i18n("Run a command"); checked: editor.mode === "command"; Controls.ButtonGroup.group: modeGroup; onToggled: if (checked) editor.mode = "command" }
            }
        }

        // Command
        Controls.TextField {
            id: commandField
            visible: editor.mode === "command"
            Layout.fillWidth: true
            placeholderText: i18n("Runs with /bin/sh -c, e.g. nmcli con up VPN")
        }

        // App: a searchable list
        ColumnLayout {
            visible: editor.mode === "app"
            Layout.fillWidth: true
            Kirigami.SearchField { id: appSearch; Layout.fillWidth: true; placeholderText: i18n("Filter apps") }
            Controls.TextField { id: appField; visible: false }
            Repeater {
                model: editor.library ? editor.library.apps() : []
                delegate: Controls.RadioDelegate {
                    required property var modelData
                    visible: appSearch.text.length === 0 || modelData.name.toLowerCase().indexOf(appSearch.text.toLowerCase()) !== -1
                    width: parent ? parent.width : implicitWidth
                    Layout.fillWidth: true
                    text: modelData.name
                    icon.name: modelData.icon
                    checked: appField.text === modelData.id
                    onToggled: if (checked) appField.text = modelData.id
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            Controls.Button {
                visible: editor.editId.length > 0
                text: i18n("Delete")
                icon.name: "edit-delete-symbolic"
                onClicked: { editor.library.removeCustom(editor.editId); kcm.pop() }
            }
            Item { Layout.fillWidth: true }
            Controls.Button { text: i18n("Cancel"); onClicked: kcm.pop() }
            Controls.Button {
                text: i18n("Save")
                icon.name: "document-save-symbolic"
                enabled: editor.buildSpec() !== "none"
                onClicked: editor.commit()
            }
        }
    }
}
