// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// Choose what a slot does: a built-in action, an application, or a command.
// Picking anything writes it and pops back to the list.
Kirigami.ScrollablePage {
    id: picker
    required property string slot
    property string heading: ""
    title: heading

    readonly property string current: KeySettings.binding(slot)

    function choose(spec) {
        KeySettings.setBinding(slot, spec)
        applicationWindow().pageStack.pop()
    }

    ColumnLayout {
        spacing: 0

        Kirigami.Heading {
            level: 2
            text: "Built-in"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        Controls.ItemDelegate {
            text: "Nothing"
            icon.name: "edit-none"
            highlighted: picker.current === "none"
            Layout.fillWidth: true
            onClicked: picker.choose("none")
        }
        Repeater {
            model: KeySettings.builtins()
            delegate: Controls.ItemDelegate {
                required property var modelData
                text: modelData.label
                highlighted: picker.current === "builtin:" + modelData.id
                Layout.fillWidth: true
                onClicked: picker.choose("builtin:" + modelData.id)
            }
        }

        Kirigami.Heading {
            level: 2
            text: "Open an app"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        Kirigami.SearchField {
            id: search
            placeholderText: "Filter apps"
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }
        Repeater {
            model: KeySettings.apps()
            delegate: Controls.ItemDelegate {
                required property var modelData
                visible: search.text.length === 0
                    || modelData.name.toLowerCase().indexOf(search.text.toLowerCase()) !== -1
                text: modelData.name
                icon.name: modelData.icon
                highlighted: picker.current === "app:" + modelData.id
                Layout.fillWidth: true
                Layout.preferredHeight: visible ? implicitHeight : 0
                onClicked: picker.choose("app:" + modelData.id)
            }
        }

        Kirigami.Heading {
            level: 2
            text: "Run a command"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        Controls.Label {
            text: "Run with /bin/sh -c, as your user. Anything you could type in a terminal."
            font: Kirigami.Theme.smallFont
            opacity: 0.8
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Controls.TextField {
                id: command
                Layout.fillWidth: true
                placeholderText: "e.g. notify-send hello"
                text: picker.current.startsWith("command:") ? picker.current.substring(8) : ""
                onAccepted: if (text.trim().length) picker.choose("command:" + text.trim())
            }
            Controls.Button {
                text: "Use"
                enabled: command.text.trim().length > 0
                onClicked: picker.choose("command:" + command.text.trim())
            }
        }

        Item { Layout.preferredHeight: Kirigami.Units.largeSpacing }
    }
}
