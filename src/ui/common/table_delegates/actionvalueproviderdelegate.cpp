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
#include "actionvalueproviderdelegate.h"

// Qt
#include <QTableView>

// StateS
#include "actionvalueprovidereditor.h"
#include "actiontablemodel.h"


QWidget* ActionValueProviderDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex& index) const
{
	auto actionValueProviderEditor = new ActionValueProviderEditor(parent);

	auto model = dynamic_cast<const ActionTableModel*>(index.model());
	uint row = index.row();

	auto expectedType = model->getExpectedProvidedValueType(row);
	if (expectedType != MachineValue::Type_t::bitVector)
	{
		actionValueProviderEditor->setExpectedType(expectedType);
	}
	else
	{
		uint expectedBitVectorSize = model->getExpectedProvidedValueBitVectorSize(row);
		actionValueProviderEditor->setExpectedType(expectedType, expectedBitVectorSize);
	}

	connect(actionValueProviderEditor, &ActionValueProviderEditor::contentChangedEvent, this, &ActionValueProviderDelegate::valueProviderChangedEventHandler);

	return actionValueProviderEditor;
}

void ActionValueProviderDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto actionValueProviderEditor = dynamic_cast<ActionValueProviderEditor*>(editor);
	if (actionValueProviderEditor == nullptr) return;


	auto cellData = index.data(Qt::EditRole);
	auto equation = cellData.value<std::shared_ptr<const Equation>>();
	actionValueProviderEditor->setValueProvider(equation);
}

void ActionValueProviderDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto actionValueProviderEditor = dynamic_cast<ActionValueProviderEditor*>(editor);
	if (actionValueProviderEditor == nullptr) return;


	auto newCondition = actionValueProviderEditor->getValueProvider();
	auto variant = QVariant::fromValue(newCondition);
	model->setData(index, variant, Qt::EditRole);
}

bool ActionValueProviderDelegate::eventFilter(QObject* object, QEvent* event)
{
	// This override solves a very tricky issue: QAbstractItemView
	// installs an event filter on editors, that is handled by
	// QAbstractItemDelegate. This is something we especially
	// DON'T WANT for complex editors that contain subwidgets
	// that will use 'enter' for validation, because the key
	// press will be processed by the delegate's event filter
	// while the subwidget is NOT the current editor.
	return false;
}

void ActionValueProviderDelegate::valueProviderChangedEventHandler()
{
	auto actionValueProviderEditor = dynamic_cast<ActionValueProviderEditor*>(sender());
	if (actionValueProviderEditor == nullptr) return;


	emit this->commitData(actionValueProviderEditor);

	auto tableView = dynamic_cast<QTableView*>(this->parent());
	if (tableView != nullptr)
	{
		tableView->resizeColumnsToContents();
	}
}
