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

// Current class header
#include "actiontabletypedelegate.h"

// Qt classes
#include <QRegularExpressionValidator>

// StateS classes
#include "actiontypeeditor.h"


QWidget* ActionTableTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	return new ActionTypeEditor(parent);
}

void ActionTableTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto actionTypeEditor = dynamic_cast<ActionTypeEditor*>(editor);
	if (actionTypeEditor == nullptr) return;


	uint32_t valueAsInt = index.data(Qt::EditRole).toUInt();

	uint16_t allowedActionTypes = (valueAsInt & 0xFFFF0000) >> 16;
	ActionOnVariableType_t currentActionType = (ActionOnVariableType_t)(valueAsInt & 0xFFFF);

	actionTypeEditor->fillActionList(allowedActionTypes, currentActionType);

	connect(actionTypeEditor, &ActionTypeEditor::actionTypeChangedEvent, this, &ActionTableTypeDelegate::actionTypeChangedEventHandler);
}

void ActionTableTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto actionTypeEditor = dynamic_cast<ActionTypeEditor*>(editor);
	if (actionTypeEditor == nullptr) return;


	auto newActionType = actionTypeEditor->getActionType();

	model->setData(index, (uint)newActionType, Qt::EditRole);
}

void ActionTableTypeDelegate::actionTypeChangedEventHandler(ActionTypeEditor* editor)
{
	emit this->commitData(editor);
}
