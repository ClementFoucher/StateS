/*
 * Copyright © 2025 Clément Foucher
 *
 * Distributed under the GNU GPL v2. For full terms see the file LICENSE.txt.
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
#include "variabletablenamedelegate.h"

// StateS classes
#include "variabletableview.h"
#include "variabletablemodel.h"
#include "dynamiclineeditor.h"


QWidget* VariableTableNameDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	return new DynamicLineEditor(parent);
}

void VariableTableNameDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto nameEditor = dynamic_cast<DynamicLineEditor*>(editor);
	if (nameEditor == nullptr) return;

	auto tableView = dynamic_cast<VariableTableView*>(this->parent());
	if (tableView == nullptr) return;

	auto tableModel = dynamic_cast<VariableTableModel*>(tableView->model());
	if (tableModel == nullptr) return;


	// Check if this is an automatic edit due to a failed rename
	QString failedName = tableModel->getFailedVariableName();

	if (failedName.isNull() == true) // Test isNull and not isEmpty as empty is a cause of failure
	{
		nameEditor->setText(index.data(Qt::EditRole).toString());
	}
	else
	{
		nameEditor->setText(failedName);
		nameEditor->setErroneous(true);
	}
}

void VariableTableNameDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto nameEditor = dynamic_cast<DynamicLineEditor*>(editor);
	if (nameEditor == nullptr) return;


	auto newName = nameEditor->text();
	bool success = model->setData(index, newName, Qt::EditRole);
	nameEditor->setErroneous(!success);
}

void VariableTableNameDelegate::destroyEditor(QWidget* editor, const QModelIndex& index) const
{
	auto nameEditor = dynamic_cast<DynamicLineEditor*>(editor);
	if (nameEditor == nullptr)
	{
		QStyledItemDelegate::destroyEditor(editor, index);
		return;
	}


	// If editing failed, trigger a new edit
	if (nameEditor->getIsErroneous() == true)
	{
		auto tableView = dynamic_cast<VariableTableView*>(this->parent());
		if (tableView != nullptr)
		{
			tableView->edit(index);
		}
	}

	// Destroy the old editor anyway
	QStyledItemDelegate::destroyEditor(nameEditor, index);
}
