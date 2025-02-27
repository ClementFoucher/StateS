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
#include "variable.h"


Variable::Variable(const QString& name) :
	MachineComponent()
{
	this->name         = name;
	this->initialValue = LogicValue::getValue0(1);
	this->currentValue = this->initialValue;
}

Variable::Variable(componentId_t id, const QString& name) :
	MachineComponent(id)
{
	this->name         = name;
	this->initialValue = LogicValue::getValue0(1);
	this->currentValue = this->initialValue;
}

/**
 * @brief Variable::setName changes the name of the variable.
 * Only the Machine is allowed to call this function.
 * @param newName
 */
void Variable::setName(const QString& newName)
{
	if (newName.isNull() == true) return;

	if (newName == this->name) return;


	this->name = newName;

	emit this->variableRenamedEvent();
}

void Variable::resize(uint newSize)
{
	if (newSize == 0) return;

	if (newSize == this->getSize()) return;


	this->currentValue.resize(newSize);
	this->initialValue.resize(newSize);

	emit this->variableResizedEvent();
	emit this->variableInitialValueChangedEvent();
}

void Variable::setInitialValue(const LogicValue& newInitialValue)
{
	if (newInitialValue.getSize() != this->getSize()) return;

	if (newInitialValue == this->initialValue) return;


	this->initialValue = newInitialValue;

	emit this->variableInitialValueChangedEvent();
}

void Variable::setCurrentValue(const LogicValue& value)
{
	this->setCurrentValueSubRange(value, -1, -1);
}

void Variable::setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR)
{
	bool setOk = false;

	if (rangeL < 0)
	{
		// Full range affectation
		if (this->getSize() == value.getSize())
		{
			this->currentValue = value;
			setOk = true;
		}
	}
	else if (rangeR < 0)
	{
		// Single bit affectation
		if ( (value.getSize() == 1) && ((uint)rangeL < this->getSize()) )
		{
			this->currentValue[rangeL] = value[0];
			setOk = true;
		}
	}
	else
	{
		// Sub-range affectation
		if ( ((uint)rangeL < this->getSize()) && (rangeL > rangeR) && (value.getSize() == (uint)rangeR+rangeL+1) )
		{
			for (int i = rangeR ; i <= rangeL ; i++)
			{
				this->currentValue[i] = value[i-rangeR];
			}
			setOk = true;
		}
	}

	if (setOk == true)
	{
		emit this->variableCurrentValueChangedEvent();
	}
}

void Variable::reinitialize()
{
	this->currentValue = this->initialValue;

	emit this->variableCurrentValueChangedEvent();
}

QString Variable::getName() const
{
	return this->name;
}

uint Variable::getSize() const
{
	return this->currentValue.getSize();
}

LogicValue Variable::getInitialValue() const
{
	return this->initialValue;
}

LogicValue Variable::getCurrentValue() const
{
	return this->currentValue;
}
