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
#include "variabletablevaluedelegate.h"

// StateS classes
#include "logicvalue.h"
#include "valueeditor.h"


VariableTableValueDelegate::VariableTableValueDelegate(QWidget* parent) :
	QStyledItemDelegate(parent)
{

}

QWidget* VariableTableValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	return new ValueEditor(parent);
}

void VariableTableValueDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto valueEditor = dynamic_cast<ValueEditor*>(editor);
	if (valueEditor == nullptr) return;

	auto valueAsString = index.data(Qt::EditRole).toString();
	auto stringBits = valueAsString.split(":");
	if (stringBits.length() < 2) return;


	auto initialValue = LogicValue::fromString(stringBits.at(0));
	int variableSize = stringBits.at(1).toInt();

	valueEditor->setBitVectorValue(initialValue, variableSize);
}

void VariableTableValueDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto valueEditor = dynamic_cast<ValueEditor*>(editor);
	if (valueEditor == nullptr) return;


	auto newValue = valueEditor->getBitVectorValue();
	model->setData(index, newValue.toString(), Qt::EditRole);
}
