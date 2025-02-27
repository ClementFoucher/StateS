/*
 * Copyright © 2014-2025 Clément Foucher
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
#include "constantvaluesetter.h"

// Qt classes
#include <QVBoxLayout>
#include <QLabel>

// StateS classes
#include "dynamiclineedit.h"


ConstantValueSetter::ConstantValueSetter(LogicValue initialValue, QWidget* parent) :
    EditableEquation(parent)
{
	this->currentValue = initialValue;
	ConstantValueSetter::setEdited(false);
}

bool ConstantValueSetter::validEdit()
{
	if (this->valueEditor == nullptr) return false;


	this->currentValue = LogicValue::fromString(this->valueEditor->text());
	if (this->currentValue.isNull() == false)
	{
		emit valueChanged(this->currentValue);
		this->setEdited(false);

		return true;
	}
	else // (this->currentValue.isNull() == true)
	{
		return false;
	}
}

bool ConstantValueSetter::cancelEdit()
{
	if (this->valueEditor != nullptr)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ConstantValueSetter::setEdited(bool edited)
{
	delete this->valueEditor;
	delete this->valueText;
	delete this->layout();

	this->valueEditor = nullptr;
	this->valueText   = nullptr;

	QVBoxLayout* layout = new QVBoxLayout(this);

	if (edited)
	{
		QString value = QString();

		if (!this->currentValue.isNull())
			value = this->currentValue.toString();

		this->valueEditor = new DynamicLineEdit(value, true);

		connect(this->valueEditor, &DynamicLineEdit::newTextAvailableEvent, this, &ConstantValueSetter::newValueAvailable);

		layout->addWidget(this->valueEditor);
		this->valueEditor->setFocus();
	}
	else
	{
		this->valueText = new QLabel(this->currentValue.toString());

		layout->addWidget(this->valueText);
	}
}

void ConstantValueSetter::newValueAvailable(const QString& newValue)
{
	LogicValue value = LogicValue::fromString(newValue);

	if (value.isNull() == false)
	{
		emit valueChanged(value);
	}
}
