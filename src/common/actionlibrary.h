// SPDX-License-Identifier: GPL-2.0-or-later
//
// The library of named actions -- THE reusable piece. A gesture binding and a
// power-menu button do not store an action inline; they store the id of an
// action in this library, so the same "My VPN" command can drive a double tap
// and a menu button, defined once. It is a QAbstractListModel so the shared
// ActionPicker can list it, and it is edited in place (add / edit / remove a
// custom action) with the change saved by whichever KCM owns it.
//
// Two kinds of action live here:
//   * BUILT-IN  -- power off, restart, lock, screenshot, flashlight,
//                  brightness, screen toggle, power menu (from actionspec.h).
//                  Fixed id, name, icon and spec; never edited or removed.
//   * CUSTOM    -- the user's own: a name, an icon, and an app or a command.
//                  Stored as [action-<id>] in the settings file.
//
// Resolution (id -> name / icon / spec) is what the daemon and the menu use at
// run time; they load the library read-only and never edit it.
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariantList>

namespace keyconfig {

struct Action {
	QString id;
	QString name;
	QString icon;
	QString spec;      // builtin:<id> | app:<desktop-id> | command:<shell>
	bool builtin = false;
	bool inMenu = true;
	bool inGesture = true;
};

class ActionLibrary : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

public:
	enum Role {
		IdRole = Qt::UserRole + 1,
		NameRole,
		IconRole,
		SpecRole,
		SummaryRole,     // human description of the spec ("Run: ...", "Open ...")
		BuiltinRole,
		InMenuRole,
		InGestureRole,
	};

	explicit ActionLibrary(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	bool dirty() const { return m_dirty; }

	// Resolution, for the daemon / the menu / the pickers.
	Q_INVOKABLE bool exists(const QString &id) const;
	Q_INVOKABLE QString nameOf(const QString &id) const;
	Q_INVOKABLE QString iconOf(const QString &id) const;
	Q_INVOKABLE QString specOf(const QString &id) const;
	Q_INVOKABLE QString summaryOf(const QString &id) const;

	// Filtered lists for a picker: context is "menu" or "gesture".
	// Returns [{id, name, icon, builtin, summary}].
	Q_INVOKABLE QVariantList list(const QString &context) const;

	// Editing custom actions. add returns the new id.
	Q_INVOKABLE QString addCustom(const QString &name, const QString &icon, const QString &spec);
	Q_INVOKABLE void editCustom(const QString &id, const QString &name, const QString &icon, const QString &spec);
	Q_INVOKABLE void removeCustom(const QString &id);
	Q_INVOKABLE bool isCustom(const QString &id) const;

	// Installed applications, for the "open an app" branch of the editor.
	// [{id, name, icon}], sorted by name.
	Q_INVOKABLE QVariantList apps() const;

	void load();
	void save();

Q_SIGNALS:
	void dirtyChanged();

private:
	void markDirty();
	const Action *find(const QString &id) const;
	static QList<Action> builtins();
	static QString summaryOfSpec(const QString &spec);

	QList<Action> m_actions;   // built-ins first, then custom in creation order
	bool m_dirty = false;
	mutable QVariantList m_apps;
	mutable bool m_appsScanned = false;
	void scanApps() const;
};

} // namespace keyconfig
