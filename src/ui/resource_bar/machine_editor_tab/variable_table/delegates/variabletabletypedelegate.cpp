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
#include "variabletabletypedelegate.h"

// StateS
#include "typeeditor.h"
#include "variabletableview.h"


VariableTableTypeDelegate::VariableTableTypeDelegate(QWidget* parent) :
	QStyledItemDelegate(parent)
{
	// Dummy editor is only here to provide a size hint
	this->dummyEditor = new TypeEditor(parent);
	this->dummyEditor->setCurrentType(MachineValue::Type_t::bitVector);
	this->dummyEditor->setVisible(false);
}

QWidget* VariableTableTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem&, const QModelIndex&) const
{
	auto typeEditor = new TypeEditor(parent);

	// Ideally, the wheel event should only be ignored if the table
	// has vertical scrollbars, but this is far easier this way.
	typeEditor->setIgnoreWheelEvents(true);

	return typeEditor;
}

void VariableTableTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	auto typeEditor = dynamic_cast<TypeEditor*>(editor);
	if (typeEditor == nullptr) return;


	auto valueAsString = index.data(Qt::EditRole).toString();
	if (valueAsString.startsWith("BOOLEAN"))
	{
		typeEditor->setCurrentType(MachineValue::Type_t::boolean);
	}
	else if (valueAsString.startsWith("BITVECTOR"))
	{
		typeEditor->setCurrentType(MachineValue::Type_t::bitVector);

		auto stringBits = valueAsString.split(":");
		if (stringBits.length() < 2) return;

		typeEditor->setBitVectorSize(stringBits.at(1).toUInt());
	}

	connect(typeEditor, &TypeEditor::typeChangedEvent, this, &VariableTableTypeDelegate::typeChangedEventHandler);
}

void VariableTableTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	auto typeEditor = dynamic_cast<TypeEditor*>(editor);
	if (typeEditor == nullptr) return;


	auto newType = typeEditor->getCurrentType();
	switch (newType)
	{
	case MachineValue::Type_t::boolean:
		model->setData(index, "BOOLEAN", Qt::EditRole);
		break;
	case MachineValue::Type_t::bitVector:
	{
		QString typeText = "BITVECTOR";
		typeText += ":" + QString::number(typeEditor->getBitVectorSize());

		bool accepted = model->setData(index, typeText, Qt::EditRole);
		if (accepted == false)
		{
			typeEditor->setErroneous(true);
		}

		break;
	}
	case MachineValue::Type_t::nullType:
		break;
	}
}

QSize VariableTableTypeDelegate::sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const
{
	return this->dummyEditor->sizeHint();
}

void VariableTableTypeDelegate::typeChangedEventHandler(TypeEditor* editor)
{
	emit this->commitData(editor);
}
