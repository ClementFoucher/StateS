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
#include "variabletablememorizeddelegate.h"

// StateS classes
#include "checkboxeditor.h"


QWidget* VariableTableMemorizedDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	return new CheckBoxEditor(parent);
}

void VariableTableMemorizedDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto memorizedEditor = dynamic_cast<CheckBoxEditor*>(editor);
	if (memorizedEditor == nullptr) return;


	memorizedEditor->setChecked(index.data(Qt::EditRole).toBool());

	connect(memorizedEditor, &CheckBoxEditor::checkedStateChangedEvent, this, &VariableTableMemorizedDelegate::memorizedStateChangedEventHandler);
}

void VariableTableMemorizedDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto memorizedEditor = dynamic_cast<CheckBoxEditor*>(editor);
	if (memorizedEditor == nullptr) return;


	model->setData(index, memorizedEditor->getChecked(), Qt::EditRole);
}

void VariableTableMemorizedDelegate::memorizedStateChangedEventHandler(CheckBoxEditor* memorizedEditor)
{
	emit this->commitData(memorizedEditor);
}
