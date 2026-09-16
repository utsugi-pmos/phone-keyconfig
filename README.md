# phone-keyconfig

What the power and volume buttons of a Plasma Mobile phone do, configured from
KDE System Settings. Built for the POCO X3 NFC (`xiaomi-surya`) on postmarketOS,
as part of [utsugi-pmaports](https://github.com/utsugi-pmos/utsugi-pmaports),
where it is pulled by `utsugi-surya-base`.

## What it does

| Gesture | Default action |
|---|---|
| Power: short press, released, nothing else pressed meanwhile | screen off / on |
| Power held for 1.5 s | the power menu |
| Power held (< 1.5 s) **+ volume-up** | brightness up |
| Power held (< 1.5 s) **+ volume-down** | screenshot (via [screenglaze](https://github.com/utsugi-pmos/screenglaze)) |
| Power pressed twice within 300 ms | flashlight on / off |

Every gesture, and every button of the power menu, runs an **action**. An action
is a built-in (screen toggle, power menu, screenshot, flashlight, brightness ±,
power off, restart, lock), **opening an app**, or **running a command**. Named
custom actions live in one shared library, so "My VPN" defined once drives a
gesture and a menu button alike. All timings are adjustable.

## The rules (the tricky bits, and why)

- **Power never blanks the screen while any volume key is physically held.** The
  test is the plain key state, not a race against a window: that is what makes
  the volume-down + power screenshot reliable regardless of order.
- The screen toggle fires 300 ms **after** release, not on it -- that window is
  what makes the double tap detectable. It is the one cost of the design.
- Releasing power after the 1.5 s menu has come does nothing; the press was spent.
- A volume press while power is held spends the press: no toggle, no menu, and it
  never reaches the system. Each further volume press during the same hold fires
  its action again (hold power, tap volume-up three times = three brightness
  steps).
- A volume key held first, then power, is the same chord; a volume key pressed up
  to 150 ms *before* power is held back until we know. A solitary volume press
  passes through unchanged, 150 ms late.

## Architecture

Four pieces, sharing `src/common` (the config, the action library, the runner)
and `src/core` (the decisions):

- **`phone-keyconfigd`** (`src/daemon`) -- a user service that **grabs** the
  three keys (`EVIOCGRAB`: power `pm8941_pwrkey`, volume-down `pm8941_resin`,
  volume-up `gpio-keys`) so the shell never sees a raw press, and replays through
  a `uinput` device whatever should pass (a normal volume press, the power tap
  that toggles the screen). It runs the bound action for each gesture. **If
  `/dev/uinput` is missing it grabs nothing and stays inert; if it dies, the grab
  dies with it and the buttons fall back to native.** Nothing is ever stuck.
- **`phone-keyconfig-menu`** (`src/menu`) -- the power menu, a **layer-shell
  overlay** it draws itself. Plasma Mobile's own logout greeter
  (`org.kde.LogoutPrompt.promptAll`, `ksmserver-logout-greeter`) starts but does
  **not render** on this shell, and an ordinary window is not raised over the
  running app; only a layer-shell surface appears. It reads the menu from the
  config (list or grid) and runs each button through the shared runner.
- **`kcm_phone_keyconfig`** and **`kcm_phone_powermenu`** (`src/kcm`) -- two KCMs
  in **Settings &rarr; Hardware** (`FormFactors: handset, tablet`; `NoDisplay` so
  they are not in the app drawer): the gestures + timings, and the menu editor
  (reorder, enable/disable, add/remove, list/grid; the three defaults -- power
  off, restart, lock -- can be disabled but not removed). They share one
  `ActionLibrary` and the `ActionPicker`/`ActionEditor` QML (symlinked into each
  KCM's `ui/`, so it is one source file).

The decisions are a pure state machine, `src/core/gesture.{h,cpp}`: plain C++17,
no Qt, no I/O, no clock of its own -- every call carries the time,
`nextDeadline()` says when to `tick()` next. `tests/gesture_test.cpp` drives it
exactly as the daemon does, one case per rule and per gap:

    g++ -std=c++17 -Isrc/core src/core/gesture.cpp tests/gesture_test.cpp -o gesture_test && ./gesture_test

Actions go through interfaces the desktop already offers, none as root: a virtual
power tap for screen toggle; `org.kde.Shutdown` for power off / restart;
`org.freedesktop.ScreenSaver.Lock`; `org.kde.ScreenBrightness` in steps of N %;
the flash LED's world-writable sysfs file; `org.surya.Screenglaze.shoot`; a
`.desktop`'s `Exec=` (field codes stripped); `/bin/sh -c`.

## Configuration

`/etc/xdg/phone-keyconfig/phone-keyconfig.conf` (INI, documented inline) is the
shipped default; the KCMs write `~/.config/phone-keyconfig/phone-keyconfig.conf`,
which wins key by key. **There is no Apply button** -- every change is written
at once, and the daemon's `QFileSystemWatcher` reloads it live. Gestures and menu
buttons store an **action id** (a built-in id, or `custom-N`), resolved through
the library.

## Gotchas for whoever works on this next

- **KCMs are cached in the running Settings process.** Reinstalling the plugin
  does not reload it -- fully close and reopen the Settings app (or clear
  `~/.cache/*qmlcache*`), or you will see the old UI and think nothing changed.
- **A KCM delegate must not declare `required property icon` / `enabled`**: they
  shadow `ItemDelegate`'s FINAL properties. Use `model.*` roles + `index`; any
  required property also switches off the `model`/`index` context objects.
- The standalone menu binary has **no `KLocalizedContext`**, so `i18n()` is not
  available in `MenuWindow.qml` -- keep its few strings plain (the KCMs, loaded
  by kcmutils, do get i18n).
- `struct Action` in `gesture.h` is the state-machine output; the library entry
  is `LibraryAction` -- they must not both be `Action`.

## Building

    cmake -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build && ctest --test-dir build && cmake --install build

Needs ECM + KF6 (CoreAddons, Config, I18n, KCMUtils) and Qt 6.6+ (Core, Gui, Qml,
Quick, DBus); Kirigami, kirigami-addons and layer-shell-qt at run time.

**Porting to another phone:** the udev rule (`data/71-phone-keyconfig.rules`)
names this device's three input devices; change the `ATTRS{name}` values to what
`cat /proc/bus/input/devices` reports for its power and volume keys, and the flash
LED path in the config if different.

## License

GPL-2.0-or-later.
