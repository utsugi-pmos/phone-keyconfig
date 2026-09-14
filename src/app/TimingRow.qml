// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// A number with a name. The value is committed when the spin box settles, not
// on every step, so dragging through the range does not write the file ten
// times.
ColumnLayout {
    id: row
    required property string title
    property string subtitle: ""
    property string unit: ""
    property int from: 0
    property int to: 100
    property int stepSize: 1
    property int value: 0
    signal committed(int value)

    Layout.fillWidth: true
    Layout.leftMargin: Kirigami.Units.largeSpacing
    Layout.rightMargin: Kirigami.Units.largeSpacing
    Layout.bottomMargin: Kirigami.Units.largeSpacing
    spacing: Kirigami.Units.smallSpacing

    RowLayout {
        Layout.fillWidth: true
        Controls.Label {
            text: row.title
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }
        Controls.SpinBox {
            id: spin
            from: row.from
            to: row.to
            stepSize: row.stepSize
            editable: true
            value: row.value
            textFromValue: function(v) { return row.unit.length ? v + " " + row.unit : "" + v }
            valueFromText: function(t) { return parseInt(t) }
            onValueModified: commitTimer.restart()
            Timer {
                id: commitTimer
                interval: 400
                onTriggered: if (spin.value !== row.value) row.committed(spin.value)
            }
        }
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
