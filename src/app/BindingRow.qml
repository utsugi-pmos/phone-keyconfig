// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// One slot: what triggers it, and what it currently does. Tapping opens the
// picker. binding()/labelFor() are invokables, not properties, so the label
// is refreshed by hand whenever the model reports a change.
Controls.ItemDelegate {
    id: row
    required property string title
    property string subtitle: ""
    required property string slot

    property string current: KeySettings.labelFor(KeySettings.binding(slot))

    Layout.fillWidth: true
    implicitHeight: Math.max(Kirigami.Units.gridUnit * 3, column.implicitHeight + Kirigami.Units.largeSpacing * 2)

    contentItem: ColumnLayout {
        id: column
        spacing: Kirigami.Units.smallSpacing
        Controls.Label {
            text: row.title
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        Controls.Label {
            text: row.current
            color: Kirigami.Theme.highlightColor
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        Controls.Label {
            visible: row.subtitle.length > 0
            text: row.subtitle
            font: Kirigami.Theme.smallFont
            opacity: 0.7
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
    }

    Connections {
        target: KeySettings
        function onChanged() { row.current = KeySettings.labelFor(KeySettings.binding(row.slot)) }
    }
}
