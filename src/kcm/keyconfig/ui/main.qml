// SPDX-License-Identifier: GPL-2.0-or-later
//
// The "Phone keys" KCM: an action per gesture, plus the timings. The Apply
// button is driven by kcm.backend.dirty from kcm.cpp.
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    readonly property var backend: kcm.backend

    function pickFor(slot) {
        kcm.push("ActionPicker.qml", {
            library: root.backend.library,
            context: "gesture",
            allowNone: true,
            onPicked: function(id) { root.backend.setBinding(slot, id) }
        })
    }

    ColumnLayout {
        spacing: 0

        Kirigami.Heading { level: 2; text: i18n("Power button"); Layout.margins: Kirigami.Units.largeSpacing }

        Component {
            id: bindingRow
            Controls.ItemDelegate {
                id: d
                required property string title
                property string subtitle: ""
                required property string slot
                property string current: root.backend.library.nameOf(root.backend.binding(slot))
                Layout.fillWidth: true
                Connections {
                    target: root.backend
                    function onChanged() { d.current = root.backend.library.nameOf(root.backend.binding(d.slot)) }
                }
                contentItem: ColumnLayout {
                    Controls.Label { text: d.title; font.bold: true; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                    Controls.Label { text: d.current; color: Kirigami.Theme.highlightColor; Layout.fillWidth: true }
                    Controls.Label { visible: d.subtitle.length; text: d.subtitle; font: Kirigami.Theme.smallFont; opacity: 0.7; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                }
                onClicked: root.pickFor(slot)
            }
        }

        Loader { Layout.fillWidth: true; sourceComponent: bindingRow; onLoaded: { item.title = i18n("Short press, released"); item.subtitle = i18n("Nothing else pressed. Fires after the double-tap window."); item.slot = "power_release" } }
        Loader { Layout.fillWidth: true; sourceComponent: bindingRow; onLoaded: { item.title = i18n("Held for %1 s", (root.backend.holdMenuMs/1000).toFixed(1)); item.slot = "power_hold" } }
        Loader { Layout.fillWidth: true; sourceComponent: bindingRow; onLoaded: { item.title = i18n("Double tap"); item.slot = "double_tap" } }

        Kirigami.Heading { level: 2; text: i18n("Power held + volume"); Layout.margins: Kirigami.Units.largeSpacing }
        Controls.Label {
            text: i18n("While power is held, a volume key runs its action instead of changing the volume. Each press runs it again.")
            wrapMode: Text.WordWrap; font: Kirigami.Theme.smallFont; opacity: 0.8
            Layout.fillWidth: true; Layout.leftMargin: Kirigami.Units.largeSpacing; Layout.rightMargin: Kirigami.Units.largeSpacing
        }
        Loader { Layout.fillWidth: true; sourceComponent: bindingRow; onLoaded: { item.title = i18n("Volume up"); item.slot = "power_volume_up" } }
        Loader { Layout.fillWidth: true; sourceComponent: bindingRow; onLoaded: { item.title = i18n("Volume down"); item.slot = "power_volume_down" } }

        Kirigami.Heading { level: 2; text: i18n("Timing"); Layout.margins: Kirigami.Units.largeSpacing }

        Kirigami.FormLayout {
            Layout.fillWidth: true
            Controls.SpinBox { Kirigami.FormData.label: i18n("Hold for the menu (ms)"); from: 500; to: 10000; stepSize: 100; value: root.backend.holdMenuMs; onValueModified: root.backend.holdMenuMs = value }
            Controls.SpinBox { Kirigami.FormData.label: i18n("Double-tap window (ms)"); from: 0; to: 1000; stepSize: 25; value: root.backend.doubleTapMs; onValueModified: root.backend.doubleTapMs = value }
            Controls.SpinBox { Kirigami.FormData.label: i18n("Volume forgiveness (ms)"); from: 0; to: 500; stepSize: 25; value: root.backend.forgivenessMs; onValueModified: root.backend.forgivenessMs = value }
            Controls.SpinBox { Kirigami.FormData.label: i18n("Brightness step (%)"); from: 1; to: 50; stepSize: 1; value: root.backend.brightnessStepPercent; onValueModified: root.backend.brightnessStepPercent = value }
            Controls.SpinBox { Kirigami.FormData.label: i18n("Flashlight brightness"); from: 1; to: 255; stepSize: 5; value: root.backend.torchBrightness; onValueModified: root.backend.torchBrightness = value }
        }
    }
}
