// SPDX-License-Identifier: GPL-2.0-or-later
#include "actionlibrary.h"

#include "actionspec.h"
#include "settings.h"

#include <QDir>
#include <QSet>
#include <QSettings>
#include <QStandardPaths>
#include <QVariantMap>

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
	return QStringLiteral("action-") + id;
}

} // namespace

ActionLibrary::ActionLibrary(QObject *parent)
	: QAbstractListModel(parent)
{
	m_actions = builtins();
}

QList<LibraryAction> ActionLibrary::builtins()
{
	QList<LibraryAction> out;
	for (const Builtin &b : kBuiltins) {
		LibraryAction a;
		a.id = QString::fromLatin1(b.id);
		a.name = QString::fromLatin1(b.label);
		a.icon = QString::fromLatin1(b.icon);
		a.spec = QStringLiteral("builtin:") + a.id;
		a.builtin = true;
		a.inMenu = b.inMenu;
		a.inGesture = b.inGesture;
		out.append(a);
	}
	return out;
}

QString ActionLibrary::summaryOfSpec(const QString &spec)
{
	const Spec s = Spec::parse(spec);
	switch (s.kind) {
	case Spec::None: return QStringLiteral("Nothing");
	case Spec::Builtin: return builtinLabel(s.payload);
	case Spec::App: return QStringLiteral("Open ") + s.payload;
	case Spec::Command: return QStringLiteral("Run: ") + s.payload;
	}
	return {};
}

const LibraryAction *ActionLibrary::find(const QString &id) const
{
	for (const LibraryAction &a : m_actions) {
		if (a.id == id)
			return &a;
	}
	return nullptr;
}

int ActionLibrary::rowCount(const QModelIndex &parent) const
{
	return parent.isValid() ? 0 : m_actions.size();
}

QVariant ActionLibrary::data(const QModelIndex &index, int role) const
{
	if (index.row() < 0 || index.row() >= m_actions.size())
		return {};
	const LibraryAction &a = m_actions.at(index.row());
	switch (role) {
	case IdRole: return a.id;
	case NameRole: return a.name;
	case IconRole: return a.icon;
	case SpecRole: return a.spec;
	case SummaryRole: return a.builtin ? a.name : summaryOfSpec(a.spec);
	case BuiltinRole: return a.builtin;
	case InMenuRole: return a.inMenu;
	case InGestureRole: return a.inGesture;
	}
	return {};
}

QHash<int, QByteArray> ActionLibrary::roleNames() const
{
	return {
		{IdRole, "actionId"},
		{NameRole, "name"},
		{IconRole, "icon"},
		{SpecRole, "spec"},
		{SummaryRole, "summary"},
		{BuiltinRole, "builtin"},
		{InMenuRole, "inMenu"},
		{InGestureRole, "inGesture"},
	};
}

bool ActionLibrary::exists(const QString &id) const { return find(id) != nullptr; }
QString ActionLibrary::nameOf(const QString &id) const { const LibraryAction *a = find(id); return a ? a->name : QString(); }
QString ActionLibrary::iconOf(const QString &id) const { const LibraryAction *a = find(id); return a ? a->icon : QString(); }
QString ActionLibrary::specOf(const QString &id) const { const LibraryAction *a = find(id); return a ? a->spec : QStringLiteral("none"); }
QString ActionLibrary::summaryOf(const QString &id) const
{
	const LibraryAction *a = find(id);
	if (!a)
		return QStringLiteral("(missing action)");
	return a->builtin ? a->name : summaryOfSpec(a->spec);
}
bool ActionLibrary::isCustom(const QString &id) const { const LibraryAction *a = find(id); return a && !a->builtin; }

QVariantList ActionLibrary::list(const QString &context) const
{
	const bool menu = context == QLatin1String("menu");
	QVariantList out;
	for (const LibraryAction &a : m_actions) {
		if (menu ? !a.inMenu : !a.inGesture)
			continue;
		QVariantMap m;
		m.insert(QStringLiteral("id"), a.id);
		m.insert(QStringLiteral("name"), a.name);
		m.insert(QStringLiteral("icon"), a.icon);
		m.insert(QStringLiteral("builtin"), a.builtin);
		m.insert(QStringLiteral("summary"), a.builtin ? a.name : summaryOfSpec(a.spec));
		out.append(m);
	}
	return out;
}

void ActionLibrary::markDirty()
{
	if (!m_dirty) {
		m_dirty = true;
		Q_EMIT dirtyChanged();
	}
}

QString ActionLibrary::addCustom(const QString &name, const QString &icon, const QString &spec)
{
	int n = 1;
	QString id;
	do {
		id = QStringLiteral("custom-%1").arg(n++);
	} while (find(id));

	LibraryAction a;
	a.id = id;
	a.name = name.trimmed().isEmpty() ? summaryOfSpec(spec) : name.trimmed();
	a.icon = icon.trimmed().isEmpty() ? QStringLiteral("system-run-symbolic") : icon.trimmed();
	a.spec = Spec::parse(spec).toString();
	a.builtin = false;
	a.inMenu = true;
	a.inGesture = true;
	beginInsertRows({}, m_actions.size(), m_actions.size());
	m_actions.append(a);
	endInsertRows();
	save();          // direct config: no Apply button
	return id;
}

void ActionLibrary::editCustom(const QString &id, const QString &name, const QString &icon, const QString &spec)
{
	for (int i = 0; i < m_actions.size(); ++i) {
		LibraryAction &a = m_actions[i];
		if (a.id != id || a.builtin)
			continue;
		a.name = name.trimmed().isEmpty() ? summaryOfSpec(spec) : name.trimmed();
		a.icon = icon.trimmed().isEmpty() ? QStringLiteral("system-run-symbolic") : icon.trimmed();
		a.spec = Spec::parse(spec).toString();
		Q_EMIT dataChanged(index(i), index(i));
		save();
		return;
	}
}

void ActionLibrary::removeCustom(const QString &id)
{
	for (int i = 0; i < m_actions.size(); ++i) {
		if (m_actions[i].id != id || m_actions[i].builtin)
			continue;
		beginRemoveRows({}, i, i);
		m_actions.removeAt(i);
		endRemoveRows();
		save();
		return;
	}
}

void ActionLibrary::scanApps() const
{
	if (m_appsScanned)
		return;
	m_appsScanned = true;
	QSet<QString> seen;
	QList<QVariantMap> found;
	const QStringList dirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	for (const QString &dirPath : dirs) {
		const QDir dir(dirPath);
		for (const QString &file : dir.entryList({QStringLiteral("*.desktop")}, QDir::Files)) {
			const QString id = file.chopped(8);
			if (seen.contains(id))
				continue;
			seen.insert(id);
			QSettings entry(dir.filePath(file), QSettings::IniFormat);
			entry.beginGroup(QStringLiteral("Desktop Entry"));
			const bool ok = entry.value(QStringLiteral("Type")).toString() == QLatin1String("Application")
				&& !entry.value(QStringLiteral("NoDisplay"), false).toBool()
				&& !entry.value(QStringLiteral("Hidden"), false).toBool()
				&& !entry.value(QStringLiteral("Exec")).toString().isEmpty()
				&& !entry.value(QStringLiteral("Name")).toString().isEmpty();
			const QString name = entry.value(QStringLiteral("Name")).toString();
			const QString icon = entry.value(QStringLiteral("Icon")).toString();
			entry.endGroup();
			if (!ok)
				continue;
			QVariantMap m;
			m.insert(QStringLiteral("id"), id);
			m.insert(QStringLiteral("name"), name);
			m.insert(QStringLiteral("icon"), icon);
			found.append(m);
		}
	}
	std::sort(found.begin(), found.end(), [](const QVariantMap &a, const QVariantMap &b) {
		return a.value(QStringLiteral("name")).toString().localeAwareCompare(
			b.value(QStringLiteral("name")).toString()) < 0;
	});
	m_apps.clear();
	for (const QVariantMap &m : found)
		m_apps.append(m);
}

QVariantList ActionLibrary::apps() const
{
	scanApps();
	return m_apps;
}

void ActionLibrary::load()
{
	QSettings s = openIni();
	beginResetModel();
	m_actions = builtins();

	const QStringList customIds = s.value(QStringLiteral("actions/custom")).toString()
		.split(QLatin1Char(','), Qt::SkipEmptyParts);
	for (const QString &rawId : customIds) {
		const QString id = rawId.trimmed();
		if (id.isEmpty() || find(id))
			continue;
		s.beginGroup(groupFor(id));
		LibraryAction a;
		a.id = id;
		a.name = s.value(QStringLiteral("name")).toString();
		a.icon = s.value(QStringLiteral("icon"), QStringLiteral("system-run-symbolic")).toString();
		a.spec = s.value(QStringLiteral("spec"), QStringLiteral("none")).toString();
		a.builtin = false;
		a.inMenu = true;
		a.inGesture = true;
		s.endGroup();
		if (a.name.isEmpty())
			a.name = summaryOfSpec(a.spec);
		m_actions.append(a);
	}

	endResetModel();
	m_dirty = false;
	Q_EMIT dirtyChanged();
}

void ActionLibrary::save()
{
	QSettings s = openIni();

	// Drop every action group first, so a removed custom action leaves nothing.
	for (const QString &g : s.childGroups()) {
		if (g.startsWith(QLatin1String("action-")))
			s.remove(g);
	}

	QStringList customIds;
	for (const LibraryAction &a : m_actions) {
		if (a.builtin)
			continue;
		customIds << a.id;
		s.beginGroup(groupFor(a.id));
		s.setValue(QStringLiteral("name"), a.name);
		s.setValue(QStringLiteral("icon"), a.icon);
		s.setValue(QStringLiteral("spec"), a.spec);
		s.endGroup();
	}
	s.setValue(QStringLiteral("actions/custom"), customIds.join(QLatin1Char(',')));
	s.sync();

	m_dirty = false;
	Q_EMIT dirtyChanged();
}

} // namespace keyconfig
