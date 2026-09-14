// SPDX-License-Identifier: GPL-2.0-or-later
//
// The power menu as an ordered, editable list. Each item is a reference to an
// action in the ActionLibrary plus an enabled flag; the name and icon are
// resolved from the library, so editing an action updates every menu button
// and gesture that uses it at once. One model serves both sides: the KCM edits
// it (reorder, enable, add, remove) with an Apply button driven by dirty(), and
// the menu binary loads it read-only to draw the buttons.
//
// Three items always exist -- Power off, Restart, Lock screen -- and can be
// disabled but never removed. Anything else the user added can be reordered or
// removed.
//
//   [powermenu]        layout=list|grid   order=action-id,action-id,...
//   [menuitem-<id>]    enabled=
#pragma once

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include <QVariantList>

namespace keyconfig {

class ActionLibrary;

class MenuModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(QString layout READ layout WRITE setLayout NOTIFY layoutChanged)
	Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

public:
	enum Role {
		ActionIdRole = Qt::UserRole + 1,
		NameRole,        // resolved from the library
		IconRole,        // resolved from the library
		SummaryRole,     // human description of the action
		EnabledRole,
		RemovableRole,   // false for the three defaults
	};

	// The library is not owned; it must outlive the model and stay loaded.
	explicit MenuModel(ActionLibrary *library, QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = {}) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QHash<int, QByteArray> roleNames() const override;

	QString layout() const { return m_layout; }
	void setLayout(const QString &v);
	bool dirty() const { return m_dirty; }

	Q_INVOKABLE void setEnabled(int row, bool enabled);
	Q_INVOKABLE void move(int from, int to);
	Q_INVOKABLE void addAction(const QString &actionId);   // append a reference
	Q_INVOKABLE void remove(int row);                       // non-default only
	Q_INVOKABLE bool contains(const QString &actionId) const;

	// The enabled items, in order, for the menu binary. [{name, icon, actionId}]
	Q_INVOKABLE QVariantList enabledItems() const;

	void load();
	void save();
	void restoreDefaults();

	// Call after the library reloads, so resolved names/icons refresh.
	void refresh();

Q_SIGNALS:
	void layoutChanged();
	void dirtyChanged();

private:
	struct Item {
		QString actionId;
		bool enabled = true;
	};

	void markDirty();
	static QStringList defaultOrder();
	static bool isDefault(const QString &actionId);

	ActionLibrary *const m_library;
	QList<Item> m_items;
	QString m_layout = QStringLiteral("list");
	bool m_dirty = false;
};

} // namespace keyconfig
