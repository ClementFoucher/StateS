/*
 * Copyright © 2014-2020 Clément Foucher
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

#include <QDebug>

// StateS classes
#include "dynamiclineedit.h"
#include "statesexception.h"


ConstantValueSetter::ConstantValueSetter(LogicValue initialValue, QWidget* parent) :
    EditableEquation(parent)
{
	this->currentValue = initialValue;
	this->setEdited(false);
}

bool ConstantValueSetter::validEdit()
{
	if (this->valueEditor != nullptr)
	{
		try
		{
			this->currentValue = LogicValue::fromString(this->valueEditor->text()); // Throws StatesException
			emit valueChanged(this->currentValue);
			this->setEdited(false);

			return true;
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
			{
				qDebug() << "(ConstantValueSetter:) Info: Wrong input for constant value, change ignored.";
				return false;
			}
			else
				throw;
		}
	}
	else
		return false;
}

bool ConstantValueSetter::cancelEdit()
{
	if (this->valueEditor != nullptr)
		return true;

	else return false;
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
	try
	{
		LogicValue value = LogicValue::fromString(newValue); // Throws StatesException
		emit valueChanged(value);
	}
	catch (const StatesException& e)
	{
		if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValue::LogicValueErrorEnum::unsupported_char) )
		{
			qDebug() << "(ConstantValueSetter:) Info: Wrong input for constant value, change ignored.";
		}
		else
			throw;
	}
}
