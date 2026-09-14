// SPDX-License-Identifier: GPL-2.0-or-later
//
// The "Phone keys" KCM. FormCard sections make the groups and the tappable rows
// obvious; there is no Apply button -- every change is saved at once.
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root
    readonly property var backend: kcm.backend

    // binding()/nameOf() are functions, not properties, so bump this on every
    // change to force the descriptions to re-read.
    property int rev: 0
    Connections { target: root.backend; function onChanged() { root.rev++ } }

    function actionName(slot) { root.rev; return root.backend.library.nameOf(root.backend.binding(slot)) }
    function actionIcon(slot) { root.rev; return root.backend.library.iconOf(root.backend.binding(slot)) }
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

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            visible: (root.rev, root.backend.binding("power_release")) !== "screen-toggle"
            type: Kirigami.MessageType.Warning
            text: i18n("A short press is what locks and wakes the screen. With something else here you may be left with no easy way to turn the screen off or on.")
        }

        FormCard.FormHeader { title: i18n("Power button") }
        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                text: i18n("Short press, released")
                description: root.actionName("power_release")
                icon.name: root.actionIcon("power_release")
                onClicked: root.pickFor("power_release")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormButtonDelegate {
                text: i18n("Held for %1 s", (root.backend.holdMenuMs / 1000).toFixed(1))
                description: root.actionName("power_hold")
                icon.name: root.actionIcon("power_hold")
                onClicked: root.pickFor("power_hold")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormButtonDelegate {
                text: i18n("Double tap")
                description: root.actionName("double_tap")
                icon.name: root.actionIcon("double_tap")
                onClicked: root.pickFor("double_tap")
            }
        }

        FormCard.FormHeader { title: i18n("Power held + volume") }
        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                text: i18n("Volume up")
                description: root.actionName("power_volume_up")
                icon.name: root.actionIcon("power_volume_up")
                onClicked: root.pickFor("power_volume_up")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormButtonDelegate {
                text: i18n("Volume down")
                description: root.actionName("power_volume_down")
                icon.name: root.actionIcon("power_volume_down")
                onClicked: root.pickFor("power_volume_down")
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormTextDelegate {
                text: i18n("While power is held, a volume key runs its action instead of changing the volume. Each press runs it again.")
                textItem.wrapMode: Text.WordWrap
            }
        }

        FormCard.FormHeader { title: i18n("Timing") }
        FormCard.FormCard {
            Stepper { label: i18n("Hold for the menu"); unit: "ms"; from: 500; to: 10000; step: 100
                value: root.backend.holdMenuMs; onChanged: v => root.backend.holdMenuMs = v }
            FormCard.FormDelegateSeparator {}
            Stepper { label: i18n("Double-tap window"); unit: "ms"; from: 0; to: 1000; step: 25
                value: root.backend.doubleTapMs; onChanged: v => root.backend.doubleTapMs = v }
            FormCard.FormDelegateSeparator {}
            Stepper { label: i18n("Volume forgiveness"); unit: "ms"; from: 0; to: 500; step: 25
                value: root.backend.forgivenessMs; onChanged: v => root.backend.forgivenessMs = v }
            FormCard.FormDelegateSeparator {}
            Stepper { label: i18n("Brightness step"); unit: "%"; from: 1; to: 50; step: 1
                value: root.backend.brightnessStepPercent; onChanged: v => root.backend.brightnessStepPercent = v }
            FormCard.FormDelegateSeparator {}
            Stepper { label: i18n("Flashlight brightness"); unit: ""; from: 1; to: 255; step: 5
                value: root.backend.torchBrightness; onChanged: v => root.backend.torchBrightness = v }
        }

        Item { Layout.preferredHeight: Kirigami.Units.largeSpacing }
    }

    // A timing row with big, finger-sized - / + buttons. FormSpinBoxDelegate's
    // own buttons fought the value binding and looked like nothing happened.
    component Stepper: FormCard.AbstractFormDelegate {
        id: st
        property string label: ""
        property string unit: ""
        property int from: 0
        property int to: 100
        property int step: 1
        property int value: 0
        signal changed(int v)
        background: null
        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Controls.Label { text: st.label; Layout.fillWidth: true; wrapMode: Text.WordWrap }
            Controls.Button {
                icon.name: "list-remove-symbolic"; display: Controls.AbstractButton.IconOnly
                enabled: st.value > st.from
                implicitWidth: Kirigami.Units.gridUnit * 2.6; implicitHeight: Kirigami.Units.gridUnit * 2.6
                onClicked: st.changed(Math.max(st.from, st.value - st.step))
            }
            Controls.Label {
                text: st.value + (st.unit.length ? " " + st.unit : "")
                horizontalAlignment: Text.AlignHCenter
                Layout.minimumWidth: Kirigami.Units.gridUnit * 4
                font.bold: true
            }
            Controls.Button {
                icon.name: "list-add-symbolic"; display: Controls.AbstractButton.IconOnly
                enabled: st.value < st.to
                implicitWidth: Kirigami.Units.gridUnit * 2.6; implicitHeight: Kirigami.Units.gridUnit * 2.6
                onClicked: st.changed(Math.min(st.to, st.value + st.step))
            }
        }
    }
}
