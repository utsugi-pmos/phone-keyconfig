// SPDX-License-Identifier: GPL-2.0-or-later
#include "menumodel.h"

#include "actionlibrary.h"
#include "settings.h"

#include <QSettings>

#include <algorithm>

namespace keyconfig {

namespace {

QSettings openIni()
{
	return QSettings(QSettings::IniFormat, QSettings::UserScope,
		Settings::organisation(), Settings::application());
}

QString groupFor(const QString &id)
{
	return QStringLiteral("menuitem-") + id;
}

} // namespace

MenuModel::MenuModel(ActionLibrary *library, QObject *parent)
	: QAbstractListModel(parent)
	, m_library(library)
{
	for (const QString &id : defaultOrder())
		m_items.append({id, true});
}

QStringList MenuModel::defaultOrder()
{
	return {QStringLiteral("power-off"), QStringLiteral("restart"), QStringLiteral("lock-screen")};
}

bool MenuModel::isDefault(const QString &actionId)
{
	return defaultOrder().contains(actionId);
}

int MenuModel::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : m_items.size();
}

QVariant MenuModel::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_items.size())
		return {};
	const Item &it = m_items.at(index.row());
	switch (role) {
	case ActionIdRole: return it.actionId;
	case NameRole: return m_library->nameOf(it.actionId);
	case IconRole: return m_library->iconOf(it.actionId);
	case SummaryRole: return m_library->summaryOf(it.actionId);
	case EnabledRole: return it.enabled;
	case RemovableRole: return !isDefault(it.actionId);
	}
	return {};
}

QHash<int, QByteArray> MenuModel::roleNames() const
{
	return {
		{ActionIdRole, "actionId"},
		{NameRole, "name"},
		{IconRole, "icon"},
		{SummaryRole, "summary"},
		{EnabledRole, "enabled"},
		{RemovableRole, "removable"},
	};
}

void MenuModel::markDirty()
{
	if (!m_dirty) {
		m_dirty = true;
		Q_EMIT dirtyChanged();
	}
}

void MenuModel::setLayout(const QString &v)
{
	const QString norm = v == QLatin1String("grid") ? QStringLiteral("grid") : QStringLiteral("list");
	if (norm == m_layout)
		return;
	m_layout = norm;
	Q_EMIT layoutChanged();
	markDirty();
}

void MenuModel::setEnabled(int row, bool enabled)
{
	if (row < 0 || row >= m_items.size() || m_items[row].enabled == enabled)
		return;
	m_items[row].enabled = enabled;
	Q_EMIT dataChanged(index(row), index(row), {EnabledRole});
	markDirty();
}

void MenuModel::move(int from, int to)
{
	if (from < 0 || from >= m_items.size() || to < 0 || to >= m_items.size() || from == to)
		return;
	const int dest = to > from ? to + 1 : to;
	if (!beginMoveRows({}, from, from, {}, dest))
		return;
	m_items.move(from, to);
	endMoveRows();
	markDirty();
}

void MenuModel::addAction(const QString &actionId)
{
	if (actionId.isEmpty() || contains(actionId))
		return;   // no duplicates in the menu
	beginInsertRows({}, m_items.size(), m_items.size());
	m_items.append({actionId, true});
	endInsertRows();
	markDirty();
}

void MenuModel::remove(int row)
{
	if (row < 0 || row >= m_items.size() || isDefault(m_items[row].actionId))
		return;
	beginRemoveRows({}, row, row);
	m_items.removeAt(row);
	endRemoveRows();
	markDirty();
}

bool MenuModel::contains(const QString &actionId) const
{
	return std::any_of(m_items.cbegin(), m_items.cend(),
		[&actionId](const Item &it) { return it.actionId == actionId; });
}

QVariantList MenuModel::enabledItems() const
{
	QVariantList out;
	for (const Item &it : m_items) {
		if (!it.enabled || !m_library->exists(it.actionId))
			continue;
		QVariantMap m;
		m.insert(QStringLiteral("name"), m_library->nameOf(it.actionId));
		m.insert(QStringLiteral("icon"), m_library->iconOf(it.actionId));
		m.insert(QStringLiteral("actionId"), it.actionId);
		out.append(m);
	}
	return out;
}

void MenuModel::refresh()
{
	if (!m_items.isEmpty())
		Q_EMIT dataChanged(index(0), index(m_items.size() - 1));
}

void MenuModel::load()
{
	QSettings s = openIni();
	beginResetModel();
	m_items.clear();

	s.beginGroup(QStringLiteral("powermenu"));
	m_layout = s.value(QStringLiteral("layout"), QStringLiteral("list")).toString() == QLatin1String("grid")
		? QStringLiteral("grid") : QStringLiteral("list");
	const QStringList order = s.value(QStringLiteral("order")).toString()
		.split(QLatin1Char(','), Qt::SkipEmptyParts);
	s.endGroup();

	for (const QString &rawId : order) {
		const QString id = rawId.trimmed();
		if (id.isEmpty() || contains(id))
			continue;
		s.beginGroup(groupFor(id));
		const bool enabled = s.value(QStringLiteral("enabled"), true).toBool();
		s.endGroup();
		m_items.append({id, enabled});
	}

	if (m_items.isEmpty()) {
		for (const QString &id : defaultOrder())
			m_items.append({id, true});
	}
	// A default that went missing is put back, disabled, so it can never be lost.
	for (const QString &id : defaultOrder()) {
		if (!contains(id))
			m_items.append({id, false});
	}

	endResetModel();
	Q_EMIT layoutChanged();
	m_dirty = false;
	Q_EMIT dirtyChanged();
}

void MenuModel::save()
{
	QSettings s = openIni();
	for (const QString &g : s.childGroups()) {
		if (g.startsWith(QLatin1String("menuitem-")))
			s.remove(g);
	}
	QStringList order;
	for (const Item &it : m_items) {
		order << it.actionId;
		s.beginGroup(groupFor(it.actionId));
		s.setValue(QStringLiteral("enabled"), it.enabled);
		s.endGroup();
	}
	s.beginGroup(QStringLiteral("powermenu"));
	s.setValue(QStringLiteral("layout"), m_layout);
	s.setValue(QStringLiteral("order"), order.join(QLatin1Char(',')));
	s.endGroup();
	s.sync();
	m_dirty = false;
	Q_EMIT dirtyChanged();
}

void MenuModel::restoreDefaults()
{
	beginResetModel();
	m_items.clear();
	for (const QString &id : defaultOrder())
		m_items.append({id, true});
	endResetModel();
	setLayout(QStringLiteral("list"));
	markDirty();
}

} // namespace keyconfig
