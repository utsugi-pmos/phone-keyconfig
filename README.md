# phone-keyconfig

What the power and volume buttons of a Plasma Mobile phone do -- and an app to
change it. Built for the POCO X3 NFC (`xiaomi-surya`) on postmarketOS, as part
of [utsugi-pmaports](https://github.com/utsugi-pmos/utsugi-pmaports).

## The rules

| Gesture | Default action |
|---|---|
| Power: short press, released, nothing else pressed meanwhile | screen off / on |
| Power held for 3 s | the power menu |
| Power held (less than 3 s) **+ volume-up** | brightness up |
| Power held (less than 3 s) **+ volume-down** | screenshot (via [screenglaze](https://github.com/utsugi-pmos/screenglaze)) |
| Power pressed twice within 300 ms | flashlight on / off |

Every slot can be set to a built-in action, to opening an application, or to a
shell command. All timings are adjustable.

The details that make it hold together:

- The screen toggle fires 300 ms **after** the release, not on it. That window
  is what makes a double tap detectable at all; it is the one cost of the
  design.
- Releasing power after the menu has come does nothing. The press was spent.
- A volume press while power is held spends the press too: no toggle on
  release, no menu even if the hold goes on. Every further volume press during
  the same hold fires again (hold power, tap volume-up three times: three steps
  of brightness). Volume keys never reach the system while power is held.
- Holding a volume key first and then pressing power is the same chord. A
  volume key pressed up to 150 ms *before* power is held back until we know,
  so fingers that land "together" but out of order still make the chord.
- A solitary volume press passes through unchanged, 150 ms late.

## How it works

`phone-keyconfigd` runs as a user service and **grabs** the three input devices
(`EVIOCGRAB`), so the shell never sees a raw press. Everything it decides to let
through -- a normal volume press, the power tap that toggles the screen -- is
replayed through a `uinput` device as an ordinary key event. Actions go through
what the desktop already offers, none of it as root:

| Action | Mechanism |
|---|---|
| screen off / on | a virtual power tap through uinput: exactly what the shell does for a native tap, lock screen and wake-up included |
| power menu | `org.kde.LogoutPrompt.promptAll` on the session bus |
| brightness | `org.kde.ScreenBrightness`, the internal display, in steps of 10 % |
| flashlight | the flash LED's sysfs brightness file, which Plasma Mobile's own udev rule leaves world-writable |
| screenshot | `org.surya.Screenglaze.shoot` |
| an app | the `Exec=` of its `.desktop`, field codes stripped |
| a command | `/bin/sh -c` |

If `/dev/uinput` is not available nothing is grabbed and the daemon stays inert:
a grabbed key it could not replay would be a key the phone lost. If the daemon
dies, the grab dies with it and the buttons fall back to the shell. Nothing is
ever stuck.

The decisions live in one place, `src/core/gesture.{h,cpp}`: a state machine in
plain C++17 with no Qt, no I/O and no clock of its own. Every call carries the
time and `nextDeadline()` says when to call `tick()` next. `tests/gesture_test.cpp`
drives it exactly the way the daemon does, one case per rule above and per gap
the design had to close:

    g++ -std=c++17 -Isrc/core src/core/gesture.cpp tests/gesture_test.cpp -o gesture_test && ./gesture_test

## Configuration

`/etc/xdg/phone-keyconfig/phone-keyconfig.conf` holds the shipped defaults; the
app writes `~/.config/phone-keyconfig/phone-keyconfig.conf`, which wins key by
key. The daemon watches the user file and applies changes at once. The file is
documented inline (`data/phone-keyconfig.conf`).

## Building

    cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build
    ctest --test-dir build
    cmake --install build

Qt 6.5+ (Core, Gui, Qml, Quick, DBus). Kirigami at run time, for the app.

The udev rule (`data/71-phone-keyconfig.rules`) names this phone's input
devices. On another device, change the three `ATTRS{name}` values to what
`cat /proc/bus/input/devices` reports for its power and volume keys.

## License

GPL-2.0-or-later.
