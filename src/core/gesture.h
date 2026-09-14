// SPDX-License-Identifier: GPL-2.0-or-later
//
// The button state machine. Pure C++17, no Qt, no I/O, no clock of its own:
// every call carries the time, and the caller asks nextDeadline() to know when
// to call tick() again. That is what makes it testable to the millisecond on a
// laptop with nothing but g++, and it is the whole reason the daemon around it
// can be trusted: the daemon only moves bytes, this decides.
//
// THE RULES IT IMPLEMENTS (from the spec, with the gaps closed)
// -------------------------------------------------------------
//  1. Power released, with no other key pressed during the press and held for
//     less than holdMenuMs  -> screen toggle (off if on, on if off). It fires
//     doubleTapMs AFTER the release, not on it: that window is what makes a
//     double tap detectable at all (rule 4). It is the one cost in the design.
//  2. Power held for holdMenuMs  -> the power menu. Releasing afterwards does
//     nothing: the press was spent by the menu.
//  3. Volume up or down pressed while power is held (for less than holdMenuMs)
//     -> that volume key's action. The press is spent: no toggle on release, no
//     menu even if the hold goes on. Every further volume press during the same
//     hold fires its action again (hold power, tap volume-up three times =
//     three steps of brightness). Volume keys NEVER reach the system while
//     power is held.
//     A volume key that is physically down when power is pressed counts too --
//     hold volume, then power, is the same chord. And a volume key pressed up to
//     forgivenessMs BEFORE power is held back until we know, so fingers that
//     land "together" but not in order are still a chord.
//  4. Power pressed again within doubleTapMs of a clean release -> the double
//     tap action, fired on that second press. The second press is spent.
//
// A solitary volume press passes through to the system unchanged, only
// forgivenessMs late (a quick tap is replayed whole on release; a hold is
// forwarded when the window closes and then mirrored until release).
#pragma once

#include <cstdint>
#include <vector>

namespace keyconfig {

using Ms = std::int64_t;

enum class Volume { Up, Down };

// What the machine wants done. The daemon executes; the machine never does.
struct Action {
	enum Kind {
		ScreenToggle,   // rule 1
		PowerMenu,      // rule 2
		VolumeCombo,    // rule 3: `volume` says which key
		DoubleTap,      // rule 4
		PassVolume,     // a solitary volume key, replayed: `volume` + `value`
	};
	Kind kind;
	Volume volume = Volume::Down;
	int value = 0;      // PassVolume only: 1 press, 2 autorepeat, 0 release
};

struct Config {
	Ms holdMenuMs = 3000;
	Ms doubleTapMs = 300;
	Ms forgivenessMs = 150;
};

class Gesture {
public:
	explicit Gesture(Config cfg = {}) : m_cfg(cfg) {}

	void setConfig(Config cfg) { m_cfg = cfg; }
	Config config() const { return m_cfg; }

	// Inputs. Each returns the actions to execute, in order.
	std::vector<Action> powerPress(Ms now);
	std::vector<Action> powerRelease(Ms now);
	std::vector<Action> volumePress(Volume v, Ms now);
	std::vector<Action> volumeRepeat(Volume v, Ms now);
	std::vector<Action> volumeRelease(Volume v, Ms now);
	std::vector<Action> tick(Ms now);

	// When tick() must next be called, or -1 if nothing is pending.
	Ms nextDeadline() const;

	// For tests and diagnostics.
	bool powerDown() const { return m_power.down; }
	bool volumeDown(Volume v) const { return vol(v).down; }

private:
	struct Power {
		bool down = false;
		Ms downAt = 0;
		bool spent = false;     // a combo, the menu or a double tap consumed it
	};
	struct TapWait {
		bool active = false;
		Ms until = 0;
	};
	struct Vol {
		bool down = false;
		Ms downAt = 0;
		bool pending = false;   // held back for forgivenessMs, not passed yet
		bool forwarded = false; // passed to the system, still held
		bool swallowed = false; // consumed by a chord: its release is silent too
	};

	Vol &vol(Volume v) { return v == Volume::Up ? m_up : m_down; }
	const Vol &vol(Volume v) const { return v == Volume::Up ? m_up : m_down; }
	static Volume other(Volume v) { return v == Volume::Up ? Volume::Down : Volume::Up; }

	void chordWith(Volume v, std::vector<Action> &out);
	void expireVolume(Volume v, Ms now, std::vector<Action> &out);

	Config m_cfg;
	Power m_power;
	TapWait m_tapWait;
	Vol m_up;
	Vol m_down;
};

} // namespace keyconfig
