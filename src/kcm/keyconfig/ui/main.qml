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
            // Plain number fields: tap, the keypad opens, type. Saved when you
            // finish (focus leaves), clamped to the allowed range.
            NumberField { label: i18n("Hold for the menu (ms)"); min: 500; max: 10000
                value: root.backend.holdMenuMs; onCommitted: v => root.backend.holdMenuMs = v }
            FormCard.FormDelegateSeparator {}
            NumberField { label: i18n("Double-tap window (ms)"); min: 0; max: 1000
                value: root.backend.doubleTapMs; onCommitted: v => root.backend.doubleTapMs = v }
            FormCard.FormDelegateSeparator {}
            NumberField { label: i18n("Volume forgiveness (ms)"); min: 0; max: 500
                value: root.backend.forgivenessMs; onCommitted: v => root.backend.forgivenessMs = v }
            FormCard.FormDelegateSeparator {}
            NumberField { label: i18n("Brightness step (%)"); min: 1; max: 50
                value: root.backend.brightnessStepPercent; onCommitted: v => root.backend.brightnessStepPercent = v }
            FormCard.FormDelegateSeparator {}
            NumberField { label: i18n("Flashlight brightness (1-255)"); min: 1; max: 255
                value: root.backend.torchBrightness; onCommitted: v => root.backend.torchBrightness = v }
        }

        Item { Layout.preferredHeight: Kirigami.Units.largeSpacing }
    }

    // A labelled number field. Commits on editingFinished, clamped.
    component NumberField: FormCard.AbstractFormDelegate {
        id: nf
        property string label: ""
        property int min: 0
        property int max: 1000000
        property int value: 0
        signal committed(int v)
        background: null
        onValueChanged: field.text = value
        contentItem: RowLayout {
            spacing: Kirigami.Units.largeSpacing
            Controls.Label { text: nf.label; Layout.fillWidth: true; wrapMode: Text.WordWrap }
            Controls.TextField {
                id: field
                text: nf.value
                horizontalAlignment: Text.AlignRight
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: nf.min; top: nf.max }
                Layout.preferredWidth: Kirigami.Units.gridUnit * 6
                onEditingFinished: {
                    var v = Math.max(nf.min, Math.min(nf.max, parseInt(text) || nf.min))
                    nf.committed(v)
                    text = v
                }
            }
        }
    }
}
