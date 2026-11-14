/*
 * Copyright © 2026 Clément Foucher
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
#include "actionconditiondelegate.h"

// Qt
#include <QTableView>

// StateS
#include "actionconditioneditor.h"


QWidget* ActionConditionDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto conditionEditor = new ActionConditionEditor(parent);

	connect(conditionEditor, &ActionConditionEditor::contentChangedEvent, this, &ActionConditionDelegate::conditionChangedEventHandler);

	return conditionEditor;
}

void ActionConditionDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto conditionEditor = dynamic_cast<ActionConditionEditor*>(editor);
	if (conditionEditor == nullptr) return;


	auto cellData = index.data(Qt::EditRole);
	auto equation = cellData.value<std::shared_ptr<const Equation>>();
	conditionEditor->setCondition(equation);
}

void ActionConditionDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto conditionEditor = dynamic_cast<ActionConditionEditor*>(editor);
	if (conditionEditor == nullptr) return;


	auto newCondition = conditionEditor->getCondition();
	auto variant = QVariant::fromValue(newCondition);
	model->setData(index, variant, Qt::EditRole);
}

void ActionConditionDelegate::conditionChangedEventHandler()
{
	auto conditionEditor = dynamic_cast<ActionConditionEditor*>(sender());
	if (conditionEditor == nullptr) return;


	emit this->commitData(conditionEditor);

	auto tableView = dynamic_cast<QTableView*>(this->parent());
	if (tableView != nullptr)
	{
		tableView->resizeColumnsToContents();
	}
}
