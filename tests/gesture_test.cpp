// SPDX-License-Identifier: GPL-2.0-or-later
//
// One test per rule in gesture.h and per gap the design had to close. No
// framework: plain C++17 and asserts, so it compiles and runs anywhere with a
// compiler in well under a second -- including the laptop that has no Qt.
//
//   g++ -std=c++17 -I../src/core ../src/core/gesture.cpp gesture_test.cpp && ./a.out
#include "gesture.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

using namespace keyconfig;

namespace {

int g_failures = 0;
int g_checks = 0;

#define CHECK(cond) \
	do { \
		++g_checks; \
		if (!(cond)) { \
			++g_failures; \
			std::fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
		} \
	} while (0)

const char *name(Action::Kind k)
{
	switch (k) {
	case Action::ScreenToggle: return "ScreenToggle";
	case Action::PowerMenu: return "PowerMenu";
	case Action::VolumeCombo: return "VolumeCombo";
	case Action::DoubleTap: return "DoubleTap";
	case Action::PassVolume: return "PassVolume";
	}
	return "?";
}

std::string show(const Action &a)
{
	std::string s = name(a.kind);
	if (a.kind == Action::VolumeCombo || a.kind == Action::PassVolume)
		s += a.volume == Volume::Up ? "(up" : "(down";
	if (a.kind == Action::PassVolume)
		s += a.value == 1 ? ",press)" : a.value == 2 ? ",repeat)" : ",release)";
	else if (a.kind == Action::VolumeCombo)
		s += ")";
	return s;
}

// Drives the machine the way the daemon does: time only moves through
// advance(), and tick() is called at every deadline on the way, in order.
struct Sim {
	Gesture g;
	Ms now = 1000;
	std::vector<std::pair<Ms, Action>> log;   // (time, action)

	explicit Sim(Config c = {}) : g(c) {}

	void record(const std::vector<Action> &acts)
	{
		for (const Action &a : acts)
			log.emplace_back(now, a);
	}
	void advance(Ms ms)
	{
		const Ms target = now + ms;
		for (;;) {
			const Ms d = g.nextDeadline();
			if (d < 0 || d > target)
				break;
			now = d;
			record(g.tick(now));
		}
		now = target;
		record(g.tick(now));
	}
	void powerPress()            { record(g.powerPress(now)); }
	void powerRelease()          { record(g.powerRelease(now)); }
	void volPress(Volume v)      { record(g.volumePress(v, now)); }
	void volRepeat(Volume v)     { record(g.volumeRepeat(v, now)); }
	void volRelease(Volume v)    { record(g.volumeRelease(v, now)); }

	int count(Action::Kind k) const
	{
		int n = 0;
		for (const auto &e : log)
			if (e.second.kind == k)
				++n;
		return n;
	}
	int count(Action::Kind k, Volume v) const
	{
		int n = 0;
		for (const auto &e : log)
			if (e.second.kind == k && e.second.volume == v)
				++n;
		return n;
	}
	Ms timeOf(Action::Kind k) const
	{
		for (const auto &e : log)
			if (e.second.kind == k)
				return e.first;
		return -1;
	}
	void dump(const char *title) const
	{
		std::fprintf(stderr, "  -- %s --\n", title);
		for (const auto &e : log)
			std::fprintf(stderr, "     t=%lld %s\n", (long long)e.first, show(e.second).c_str());
	}
};

// ---- rule 1 --------------------------------------------------------------

void quickTapTogglesOnReleasePlusWindow()
{
	Sim s;
	s.powerPress();
	s.advance(80);
	s.powerRelease();
	const Ms released = s.now;
	s.advance(299);
	CHECK(s.count(Action::ScreenToggle) == 0);      // not yet: window still open
	s.advance(1);
	CHECK(s.count(Action::ScreenToggle) == 1);
	CHECK(s.timeOf(Action::ScreenToggle) == released + 300);
	CHECK(s.count(Action::PowerMenu) == 0);
	CHECK(s.count(Action::DoubleTap) == 0);
	CHECK(s.g.nextDeadline() == -1);
	if (g_failures) s.dump("quickTap");
}

void nextDeadlineTracksTheWindow()
{
	Sim s;
	CHECK(s.g.nextDeadline() == -1);
	s.powerPress();
	CHECK(s.g.nextDeadline() == s.now + 1500);      // the menu
	s.advance(50);
	s.powerRelease();
	CHECK(s.g.nextDeadline() == s.now + 300);       // the double-tap window
	s.advance(300);
	CHECK(s.g.nextDeadline() == -1);
}

// ---- rule 2 --------------------------------------------------------------

void holdBringsMenuAndReleaseIsSilent()
{
	Sim s;
	s.powerPress();
	const Ms pressed = s.now;
	s.advance(1499);
	CHECK(s.count(Action::PowerMenu) == 0);
	s.advance(1);
	CHECK(s.count(Action::PowerMenu) == 1);
	CHECK(s.timeOf(Action::PowerMenu) == pressed + 1500);
	s.advance(2000);                                // keep holding
	CHECK(s.count(Action::PowerMenu) == 1);         // only once
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);      // the gap: no blank behind the menu
	if (g_failures) s.dump("holdMenu");
}

void releaseExactlyAtThresholdIsMenuNotToggle()
{
	Sim s;
	s.powerPress();
	s.advance(1500);                                // tick at exactly +1500 fires the menu
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::PowerMenu) == 1);
	CHECK(s.count(Action::ScreenToggle) == 0);
}

// ---- rule 3 --------------------------------------------------------------

void powerThenVolumeDownIsChordNotBlank()
{
	Sim s;
	s.powerPress();
	s.advance(500);
	s.volPress(Volume::Down);
	CHECK(s.count(Action::VolumeCombo, Volume::Down) == 1);
	s.advance(100);
	s.volRelease(Volume::Down);
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
	CHECK(s.count(Action::PowerMenu) == 0);
	CHECK(s.count(Action::PassVolume) == 0);        // the panel never showed
	if (g_failures) s.dump("powerThenVolDown");
}

void volumeJustBeforePowerIsForgiven()
{
	Sim s;
	s.volPress(Volume::Down);
	s.advance(60);                                  // inside forgiveness
	s.powerPress();
	CHECK(s.count(Action::VolumeCombo, Volume::Down) == 1);
	CHECK(s.count(Action::PassVolume) == 0);        // never leaked to the system
	s.advance(100);
	s.powerRelease();
	s.volRelease(Volume::Down);
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
	CHECK(s.count(Action::PassVolume) == 0);
	if (g_failures) s.dump("forgiveness");
}

void volumeHeldLongThenPowerIsStillChord()
{
	// The case that kept breaking: volume-down held past forgiveness (so the
	// system already has it) and THEN power. Must fire the action and must
	// not blank.
	Sim s;
	s.volPress(Volume::Down);
	s.advance(1000);
	CHECK(s.count(Action::PassVolume, Volume::Down) == 1);   // forwarded at +150
	s.powerPress();
	CHECK(s.count(Action::VolumeCombo, Volume::Down) == 1);
	s.advance(200);
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
	s.volRelease(Volume::Down);
	CHECK(s.count(Action::PassVolume, Volume::Down) == 2);   // its release still passes
	if (g_failures) s.dump("volHeldThenPower");
}

void volumeUpDuringHoldFiresAndForbidsBlankAndMenu()
{
	Sim s;
	s.powerPress();
	s.advance(200);
	s.volPress(Volume::Up);
	CHECK(s.count(Action::VolumeCombo, Volume::Up) == 1);
	s.volRelease(Volume::Up);
	s.advance(5000);                                // keep holding power past 3 s
	CHECK(s.count(Action::PowerMenu) == 0);         // the gap: the gesture was spent
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
	CHECK(s.count(Action::PassVolume) == 0);
	if (g_failures) s.dump("volUpDuringHold");
}

void repeatedVolumeDuringOneHoldFiresEachTime()
{
	Sim s;
	s.powerPress();
	for (int i = 0; i < 3; ++i) {
		s.advance(100);
		s.volPress(Volume::Up);
		s.advance(50);
		s.volRelease(Volume::Up);
	}
	CHECK(s.count(Action::VolumeCombo, Volume::Up) == 3);
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
}

void volumeAfterMenuIsSwallowed()
{
	Sim s;
	s.powerPress();
	s.advance(3500);                                // menu is up
	s.volPress(Volume::Down);
	s.advance(50);
	s.volRelease(Volume::Down);
	CHECK(s.count(Action::VolumeCombo) == 0);
	CHECK(s.count(Action::PassVolume) == 0);        // never reaches the system while power is held
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);
}

// ---- rule 4 --------------------------------------------------------------

void doubleTapFiresOnSecondPressAndNeverToggles()
{
	Sim s;
	s.powerPress();
	s.advance(60);
	s.powerRelease();
	s.advance(150);                                 // inside the window
	s.powerPress();
	CHECK(s.count(Action::DoubleTap) == 1);
	s.advance(5000);                                // hold the second press long
	CHECK(s.count(Action::PowerMenu) == 0);         // second press is spent: no menu
	s.powerRelease();
	s.advance(1000);
	CHECK(s.count(Action::ScreenToggle) == 0);      // and no toggle, from either tap
	if (g_failures) s.dump("doubleTap");
}

void twoTapsOutsideWindowAreTwoToggles()
{
	Sim s;
	s.powerPress(); s.advance(50); s.powerRelease();
	s.advance(400);                                 // window (300) closes: toggle fires
	s.powerPress(); s.advance(50); s.powerRelease();
	s.advance(400);
	CHECK(s.count(Action::ScreenToggle) == 2);
	CHECK(s.count(Action::DoubleTap) == 0);
}

// ---- solitary volume passes through -------------------------------------

void solitaryVolumeQuickTapPassesWhole()
{
	Sim s;
	s.volPress(Volume::Up);
	s.advance(40);
	s.volRelease(Volume::Up);
	CHECK(s.count(Action::PassVolume, Volume::Up) == 2);    // press + release
	CHECK(s.log[0].second.value == 1 && s.log[1].second.value == 0);
	CHECK(s.count(Action::VolumeCombo) == 0);
	CHECK(s.g.nextDeadline() == -1);
}

void solitaryVolumeHoldIsForwardedThenMirrored()
{
	Sim s;
	s.volPress(Volume::Down);
	s.advance(149);
	CHECK(s.count(Action::PassVolume) == 0);
	s.advance(1);
	CHECK(s.count(Action::PassVolume, Volume::Down) == 1);  // press at +150
	s.advance(300);
	s.volRepeat(Volume::Down);
	CHECK(s.count(Action::PassVolume, Volume::Down) == 2);  // repeat mirrored
	s.volRelease(Volume::Down);
	CHECK(s.count(Action::PassVolume, Volume::Down) == 3);  // release mirrored
	CHECK(s.log.back().second.value == 0);
}

void volumeDuringTapWaitPassesAndToggleStillFires()
{
	Sim s;
	s.powerPress(); s.advance(50); s.powerRelease();
	s.advance(100);
	s.volPress(Volume::Up); s.advance(30); s.volRelease(Volume::Up);
	s.advance(400);
	CHECK(s.count(Action::PassVolume, Volume::Up) == 2);
	CHECK(s.count(Action::ScreenToggle) == 1);      // only keys DURING the press matter
	CHECK(s.count(Action::VolumeCombo) == 0);
}

// ---- configuration is honoured -------------------------------------------

void timingsComeFromConfig()
{
	Config c;
	c.holdMenuMs = 1000;
	c.doubleTapMs = 500;
	c.forgivenessMs = 50;
	Sim s(c);
	s.powerPress();
	s.advance(1000);
	CHECK(s.count(Action::PowerMenu) == 1);
	s.powerRelease();
	s.advance(2000);

	Sim t(c);
	t.powerPress(); t.advance(20); t.powerRelease();
	t.advance(499);
	CHECK(t.count(Action::ScreenToggle) == 0);
	t.advance(1);
	CHECK(t.count(Action::ScreenToggle) == 1);

	Sim u(c);
	u.volPress(Volume::Up);
	u.advance(50);
	CHECK(u.count(Action::PassVolume) == 1);        // forwarded at the shorter window
}

struct Case {
	const char *label;
	void (*fn)();
};

} // namespace

int main()
{
	const Case cases[] = {
		{"rule 1: quick tap toggles on release + window", quickTapTogglesOnReleasePlusWindow},
		{"rule 1: nextDeadline tracks the window", nextDeadlineTracksTheWindow},
		{"rule 2: hold brings the menu, release is silent", holdBringsMenuAndReleaseIsSilent},
		{"rule 2: release exactly at threshold is menu, not toggle", releaseExactlyAtThresholdIsMenuNotToggle},
		{"rule 3: power then volume-down is a chord, no blank", powerThenVolumeDownIsChordNotBlank},
		{"rule 3: volume just before power is forgiven", volumeJustBeforePowerIsForgiven},
		{"rule 3: volume held long, then power, is still a chord", volumeHeldLongThenPowerIsStillChord},
		{"rule 3: volume-up during hold fires, forbids blank and menu", volumeUpDuringHoldFiresAndForbidsBlankAndMenu},
		{"rule 3: repeated volume during one hold fires each time", repeatedVolumeDuringOneHoldFiresEachTime},
		{"rule 3: volume after the menu is swallowed", volumeAfterMenuIsSwallowed},
		{"rule 4: double tap fires on second press, never toggles", doubleTapFiresOnSecondPressAndNeverToggles},
		{"rule 4: two taps outside the window are two toggles", twoTapsOutsideWindowAreTwoToggles},
		{"pass-through: solitary volume quick tap passes whole", solitaryVolumeQuickTapPassesWhole},
		{"pass-through: solitary volume hold is forwarded then mirrored", solitaryVolumeHoldIsForwardedThenMirrored},
		{"pass-through: volume during tap-wait passes, toggle still fires", volumeDuringTapWaitPassesAndToggleStillFires},
		{"config: timings are honoured", timingsComeFromConfig},
	};

	int failedCases = 0;
	for (const Case &c : cases) {
		const int before = g_failures;
		c.fn();
		const bool ok = g_failures == before;
		std::printf("%s  %s\n", ok ? "ok  " : "FAIL", c.label);
		if (!ok)
			++failedCases;
	}
	std::printf("%d checks, %d failures, %d/%zu cases failed\n",
		g_checks, g_failures, failedCases, sizeof(cases) / sizeof(cases[0]));
	return g_failures == 0 ? 0 : 1;
}
