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
#include "actionbehaviordelegate.h"

// Qt
#include <QRegularExpressionValidator>
#include <QTableView>

// StateS
#include "actionbehavioreditor.h"


QWidget* ActionBehaviorDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto actionBehaviorEditor = new ActionBehaviorEditor(parent);

	// Ideally, the wheel event should only be ignored if the table
	// has vertical scrollbars, but this is far easier this way.
	actionBehaviorEditor->setIgnoreWheelEvents(true);

	return actionBehaviorEditor;
}

void ActionBehaviorDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto actionBehaviorEditor = dynamic_cast<ActionBehaviorEditor*>(editor);
	if (actionBehaviorEditor == nullptr) return;


	uint32_t valueAsInt = index.data(Qt::EditRole).toUInt();

	uint16_t allowedBehaviors = (valueAsInt & 0xFFFF0000) >> 16;
	ActionOnVariable::Behavior_t currentBehavior = static_cast<ActionOnVariable::Behavior_t>(valueAsInt & 0xFFFF);

	actionBehaviorEditor->fillList(allowedBehaviors, currentBehavior);

	connect(actionBehaviorEditor, &ActionBehaviorEditor::behaviorChangedEvent, this, &ActionBehaviorDelegate::behaviorChangedEventHandler);
}

void ActionBehaviorDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto actionBehaviorEditor = dynamic_cast<ActionBehaviorEditor*>(editor);
	if (actionBehaviorEditor == nullptr) return;


	auto newBehavior = actionBehaviorEditor->getSelectedBehavior();

	model->setData(index, static_cast<uint>(newBehavior), Qt::EditRole);
}

void ActionBehaviorDelegate::behaviorChangedEventHandler()
{
	auto actionBehaviorEditor = dynamic_cast<ActionBehaviorEditor*>(sender());
	if (actionBehaviorEditor == nullptr) return;


	emit this->commitData(actionBehaviorEditor);

	auto tableView = dynamic_cast<QTableView*>(this->parent());
	if (tableView != nullptr)
	{
		tableView->resizeColumnsToContents();
	}
}
