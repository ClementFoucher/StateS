/*
 * Copyright © 2025-2026 Clément Foucher
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

// Current class header
#include "actiontypedelegate.h"

// Qt
#include <QRegularExpressionValidator>
#include <QTableView>

// StateS
#include "actiontypeeditor.h"


QWidget* ActionTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto actionTypeEditor = new ActionTypeEditor(parent);

	// Ideally, the wheel event should only be ignored if the table
	// has vertical scrollbars, but this is far easier this way.
	actionTypeEditor->setIgnoreWheelEvents(true);

	return actionTypeEditor;
}

void ActionTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto actionTypeEditor = dynamic_cast<ActionTypeEditor*>(editor);
	if (actionTypeEditor == nullptr) return;


	uint32_t valueAsInt = index.data(Qt::EditRole).toUInt();

	uint16_t allowedActionTypes = (valueAsInt & 0xFFFF0000) >> 16;
	ActionOnVariable::Type_t currentActionType = static_cast<ActionOnVariable::Type_t>(valueAsInt & 0xFFFF);

	actionTypeEditor->fillActionList(allowedActionTypes, currentActionType);

	connect(actionTypeEditor, &ActionTypeEditor::actionTypeChangedEvent, this, &ActionTypeDelegate::actionTypeChangedEventHandler);
}

void ActionTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto actionTypeEditor = dynamic_cast<ActionTypeEditor*>(editor);
	if (actionTypeEditor == nullptr) return;


	auto newActionType = actionTypeEditor->getActionType();

	model->setData(index, static_cast<uint>(newActionType), Qt::EditRole);
}

void ActionTypeDelegate::actionTypeChangedEventHandler()
{
	auto actionTypeEditor = dynamic_cast<ActionTypeEditor*>(sender());
	if (actionTypeEditor == nullptr) return;


	emit this->commitData(actionTypeEditor);

	auto tableView = dynamic_cast<QTableView*>(this->parent());
	if (tableView != nullptr)
	{
		tableView->resizeColumnsToContents();
	}
}
