/*
 * Copyright © 2016-2026 Clément Foucher
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
 * along with StateS. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "actiontablevaluedelegate.h"

// Qt classes
#include <QRegularExpressionValidator>

// StateS classes
#include "logicvalue.h"
#include "valueeditor.h"


QWidget* ActionTableValueDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	return new ValueEditor(parent);
}

void ActionTableValueDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto valueEditor = dynamic_cast<ValueEditor*>(editor);
	if (valueEditor == nullptr) return;


	auto valueAsString = index.data(Qt::EditRole).toString();

	auto initialValue = LogicValue::fromString(valueAsString);

	if (initialValue.isNull()) return;


	valueEditor->setBitVectorValue(initialValue, initialValue.getSize());

	connect(valueEditor, &ValueEditor::valueChangedEvent, this, &ActionTableValueDelegate::valueChangedEventHandler);
}

void ActionTableValueDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto valueEditor = dynamic_cast<ValueEditor*>(editor);
	if (valueEditor == nullptr) return;


	auto newValue = valueEditor->getBitVectorValue();
	model->setData(index, newValue.toString(), Qt::EditRole);
}

void ActionTableValueDelegate::valueChangedEventHandler(ValueEditor* editor)
{
	// This is required as ValueEditor is not a QLineEdit:
	// we have to manually handle editor finalization
	emit this->commitData(editor);
	emit this->closeEditor(editor);
}
