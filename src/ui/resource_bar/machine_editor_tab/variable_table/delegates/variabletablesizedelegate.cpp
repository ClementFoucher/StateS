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
#include "variabletablesizedelegate.h"

// StateS classes
#include "dynamiclineeditor.h"


VariableTableSizeDelegate::VariableTableSizeDelegate(QWidget* parent) :
	QStyledItemDelegate(parent)
{

}

QWidget* VariableTableSizeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto editor = new DynamicLineEditor(parent);
	editor->setValidator(new QIntValidator(0, 64));

	return editor;
}

void VariableTableSizeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto sizeEditor = dynamic_cast<DynamicLineEditor*>(editor);
	if (sizeEditor == nullptr) return;


	sizeEditor->setText(index.data(Qt::EditRole).toString());
}

void VariableTableSizeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto sizeEditor = dynamic_cast<DynamicLineEditor*>(editor);
	if (sizeEditor == nullptr) return;


	model->setData(index, sizeEditor->text(), Qt::EditRole);
}
