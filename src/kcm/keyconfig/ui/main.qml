// SPDX-License-Identifier: GPL-2.0-or-later
//
// The "Phone keys" KCM. FormCard sections make the groups and the tappable rows
// obvious; there is no Apply button -- every change is saved at once.
import QtQuick
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
            FormCard.FormSpinBoxDelegate {
                label: i18n("Hold for the menu (ms)")
                from: 500; to: 10000; stepSize: 100
                value: root.backend.holdMenuMs
                onValueChanged: root.backend.holdMenuMs = value
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormSpinBoxDelegate {
                label: i18n("Double-tap window (ms)")
                from: 0; to: 1000; stepSize: 25
                value: root.backend.doubleTapMs
                onValueChanged: root.backend.doubleTapMs = value
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormSpinBoxDelegate {
                label: i18n("Volume forgiveness (ms)")
                from: 0; to: 500; stepSize: 25
                value: root.backend.forgivenessMs
                onValueChanged: root.backend.forgivenessMs = value
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormSpinBoxDelegate {
                label: i18n("Brightness step (%)")
                from: 1; to: 50; stepSize: 1
                value: root.backend.brightnessStepPercent
                onValueChanged: root.backend.brightnessStepPercent = value
            }
            FormCard.FormDelegateSeparator {}
            FormCard.FormSpinBoxDelegate {
                label: i18n("Flashlight brightness")
                from: 1; to: 255; stepSize: 5
                value: root.backend.torchBrightness
                onValueChanged: root.backend.torchBrightness = value
            }
        }

        Item { Layout.preferredHeight: Kirigami.Units.largeSpacing }
    }
}
