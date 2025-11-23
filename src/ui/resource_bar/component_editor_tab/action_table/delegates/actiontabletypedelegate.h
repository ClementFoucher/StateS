/*
 * Copyright © 2025 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.
 *
 *
 * This file is part of StateS.
 *
 * StateS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * StateS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIONTABLETYPEDELEGATE_H
#define ACTIONTABLETYPEDELEGATE_H

// Parent
#include <QStyledItemDelegate>

// StateS classes
class ActionTypeEditor;


class ActionTableTypeDelegate : public QStyledItemDelegate
{
	Q_OBJECT

	/////
	// Constructors/destructors
public:
	explicit ActionTableTypeDelegate(QWidget* parent = nullptr) : QStyledItemDelegate(parent) {}

	/////
	// Object functions
public:
	virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&)     const override;
	virtual void     setEditorData(QWidget* editor, const QModelIndex& index)                           const override;
	virtual void     setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;

private slots:
	void actionTypeChangedEventHandler(ActionTypeEditor* editor);

};

#endif // ACTIONTABLETYPEDELEGATE_H
