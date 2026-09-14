// SPDX-License-Identifier: GPL-2.0-or-later
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page
    title: "Phone keys"

    function pick(slot, heading) {
        applicationWindow().pageStack.push(Qt.resolvedUrl("ActionPicker.qml"),
            { slot: slot, heading: heading })
    }

    ColumnLayout {
        spacing: 0

        Kirigami.Heading {
            level: 2
            text: "Power button"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        BindingRow {
            title: "Short press, released"
            subtitle: "Nothing else pressed meanwhile. Fires once the double-tap window has passed."
            slot: "power_release"
            onClicked: page.pick(slot, title)
        }
        BindingRow {
            title: "Held for " + (KeySettings.holdMenuMs / 1000).toFixed(1) + " s"
            slot: "power_hold"
            onClicked: page.pick(slot, title)
        }
        BindingRow {
            title: "Double tap"
            subtitle: "A second press within the double-tap window."
            slot: "double_tap"
            onClicked: page.pick(slot, title)
        }

        Kirigami.Heading {
            level: 2
            text: "Power held + volume"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        Controls.Label {
            text: "While power is held (before the menu comes), a volume key runs its action instead of changing the volume. Each press runs it again. Holding a volume key first, then power, counts too."
            wrapMode: Text.WordWrap
            font: Kirigami.Theme.smallFont
            opacity: 0.8
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.smallSpacing
        }
        BindingRow {
            title: "Volume up"
            slot: "power_volume_up"
            onClicked: page.pick(slot, title)
        }
        BindingRow {
            title: "Volume down"
            slot: "power_volume_down"
            onClicked: page.pick(slot, title)
        }

        Kirigami.Heading {
            level: 2
            text: "Timing"
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
        }
        TimingRow {
            title: "Hold for the power menu"
            unit: "ms"
            from: 500; to: 10000; stepSize: 100
            value: KeySettings.holdMenuMs
            onCommitted: v => KeySettings.holdMenuMs = v
        }
        TimingRow {
            title: "Double-tap window"
            subtitle: "The screen toggles this long after a short press is released."
            unit: "ms"
            from: 0; to: 1000; stepSize: 25
            value: KeySettings.doubleTapMs
            onCommitted: v => KeySettings.doubleTapMs = v
        }
        TimingRow {
            title: "Volume-before-power forgiveness"
            subtitle: "A volume key pressed this long before power still counts as the chord. Plain volume presses are delayed by the same amount."
            unit: "ms"
            from: 0; to: 500; stepSize: 25
            value: KeySettings.forgivenessMs
            onCommitted: v => KeySettings.forgivenessMs = v
        }
        TimingRow {
            title: "Brightness step"
            unit: "%"
            from: 1; to: 50; stepSize: 1
            value: KeySettings.brightnessStepPercent
            onCommitted: v => KeySettings.brightnessStepPercent = v
        }
        TimingRow {
            title: "Flashlight brightness"
            subtitle: "0-255 on this phone's flash LED."
            unit: ""
            from: 1; to: 255; stepSize: 5
            value: KeySettings.torchBrightness
            onCommitted: v => KeySettings.torchBrightness = v
        }

        Item { Layout.preferredHeight: Kirigami.Units.largeSpacing }
    }
}
