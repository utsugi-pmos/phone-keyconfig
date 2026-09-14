// SPDX-License-Identifier: GPL-2.0-or-later
#include "gesture.h"

#include <algorithm>

namespace keyconfig {

namespace {

Ms minDeadline(Ms a, Ms b)
{
	if (a < 0)
		return b;
	if (b < 0)
		return a;
	return std::min(a, b);
}

} // namespace

// A volume key and the power key are both down: rule 3. Fire the volume key's
// action, spend the power press, and make sure the volume press never reaches
// the system -- unless it already did (held past forgiveness before power
// arrived), in which case the system has its press and will get its release.
void Gesture::chordWith(Volume v, std::vector<Action> &out)
{
	Vol &k = vol(v);
	out.push_back({Action::VolumeCombo, v});
	m_power.spent = true;
	if (k.pending) {
		k.pending = false;
		k.swallowed = true;
	} else if (!k.forwarded) {
		k.swallowed = true;
	}
}

std::vector<Action> Gesture::powerPress(Ms now)
{
	std::vector<Action> out;

	// Rule 4: a second press inside the window is the double tap. This press is
	// spent on the spot: holding it afterwards brings no menu, releasing it no
	// toggle.
	if (m_tapWait.active && now <= m_tapWait.until) {
		m_tapWait.active = false;
		m_power.down = true;
		m_power.downAt = now;
		m_power.spent = true;
		out.push_back({Action::DoubleTap});
		return out;
	}

	m_power.down = true;
	m_power.downAt = now;
	m_power.spent = false;

	// Rule 3, the other order: a volume key already physically down (held back
	// or already forwarded) makes this a chord from the first instant.
	for (Volume v : {Volume::Down, Volume::Up}) {
		if (vol(v).down)
			chordWith(v, out);
	}
	return out;
}

std::vector<Action> Gesture::powerRelease(Ms now)
{
	std::vector<Action> out;
	if (!m_power.down)
		return out;
	m_power.down = false;

	if (m_power.spent)
		return out;                     // combo, menu or double tap took it
	if (now - m_power.downAt >= m_cfg.holdMenuMs)
		return out;                     // the menu fired (or was due): no toggle

	// Rule 1, deferred by the double-tap window (rule 4 needs it).
	m_tapWait.active = true;
	m_tapWait.until = now + m_cfg.doubleTapMs;
	return out;
}

std::vector<Action> Gesture::volumePress(Volume v, Ms now)
{
	std::vector<Action> out;
	Vol &k = vol(v);
	k.down = true;
	k.downAt = now;
	k.pending = false;
	k.forwarded = false;
	k.swallowed = false;

	if (m_power.down) {
		// Rule 3. Past holdMenuMs the menu is up and the press is spent; volume
		// keys still never reach the system while power is held, so this one
		// is simply swallowed.
		if (now - m_power.downAt < m_cfg.holdMenuMs)
			chordWith(v, out);
		else
			k.swallowed = true;
		return out;
	}

	// Solitary so far: hold it back for the forgiveness window, in case power
	// is about to land.
	k.pending = true;
	return out;
}

std::vector<Action> Gesture::volumeRepeat(Volume v, Ms now)
{
	(void)now;
	std::vector<Action> out;
	const Vol &k = vol(v);
	if (k.forwarded)
		out.push_back({Action::PassVolume, v, 2});
	// Pending (still inside forgiveness) or swallowed: nothing goes through.
	return out;
}

std::vector<Action> Gesture::volumeRelease(Volume v, Ms now)
{
	(void)now;
	std::vector<Action> out;
	Vol &k = vol(v);
	if (!k.down)
		return out;
	k.down = false;

	if (k.swallowed) {
		k.swallowed = false;
	} else if (k.pending) {
		// Released before the window closed and no power came: a quick tap.
		// Replay it whole so the system sees a normal volume tap.
		k.pending = false;
		out.push_back({Action::PassVolume, v, 1});
		out.push_back({Action::PassVolume, v, 0});
	} else if (k.forwarded) {
		k.forwarded = false;
		out.push_back({Action::PassVolume, v, 0});
	}
	return out;
}

void Gesture::expireVolume(Volume v, Ms now, std::vector<Action> &out)
{
	Vol &k = vol(v);
	if (!k.pending || now < k.downAt + m_cfg.forgivenessMs)
		return;
	// Forgiveness over, no power: a real, solitary volume press. Forward it and
	// keep mirroring (autorepeat, release) until it is let go.
	k.pending = false;
	k.forwarded = true;
	out.push_back({Action::PassVolume, v, 1});
}

std::vector<Action> Gesture::tick(Ms now)
{
	std::vector<Action> out;

	expireVolume(Volume::Down, now, out);
	expireVolume(Volume::Up, now, out);

	// Rule 2.
	if (m_power.down && !m_power.spent && now - m_power.downAt >= m_cfg.holdMenuMs) {
		m_power.spent = true;
		out.push_back({Action::PowerMenu});
	}

	// Rule 1, once the double-tap window has closed with no second press.
	if (m_tapWait.active && now >= m_tapWait.until) {
		m_tapWait.active = false;
		out.push_back({Action::ScreenToggle});
	}
	return out;
}

Ms Gesture::nextDeadline() const
{
	Ms d = -1;
	for (const Vol *k : {&m_down, &m_up}) {
		if (k->pending)
			d = minDeadline(d, k->downAt + m_cfg.forgivenessMs);
	}
	if (m_power.down && !m_power.spent)
		d = minDeadline(d, m_power.downAt + m_cfg.holdMenuMs);
	if (m_tapWait.active)
		d = minDeadline(d, m_tapWait.until);
	return d;
}

} // namespace keyconfig
