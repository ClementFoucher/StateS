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

// StateS classes
#include "statesexception.h"
#include "exceptiontypes.h"


Variable::Variable(const QString &name, uint size) // Throws StatesException
{
	if (size == 0)
		throw StatesException("Variable", VariableError_t::building_zero_sized, "Variable size set to 0");

	this->name = name;
	this->initialValue = LogicValue::getValue0(size);
	this->currentValue = this->initialValue;

	// Link specific events signals to general events
	connect(this, &Variable::variableRenamedEvent,             this, &Variable::variableStaticConfigurationChangedEvent);
	connect(this, &Variable::variableResizedEvent,             this, &Variable::variableStaticConfigurationChangedEvent);
	connect(this, &Variable::variableInitialValueChangedEvent, this, &Variable::variableStaticConfigurationChangedEvent);

	// This event also impacts dynamic values
	connect(this, &Variable::variableResizedEvent, this, &Variable::variableDynamicStateChangedEvent);
}

Variable::Variable(const QString& name) :
    Variable(name, 1) // Size to 1 => no exception to catch - ignored
{

}

Variable::~Variable()
{
	emit variableDeletedEvent();
}

QString Variable::getName() const
{
	return this->name;
}

void Variable::setName(const QString& value)
{
	this->name = value;
	emit variableRenamedEvent();
}

uint Variable::getSize() const
{
	return this->currentValue.getSize();
}

void Variable::resize(uint newSize) // Throws StatesException
{
	if (newSize == 0)
		throw StatesException("Variable", VariableError_t::variable_resized_to_0, "Trying to resize variable with size 0");

	this->currentValue.resize(newSize); // Throws StatesException - size checked - ignored
	this->initialValue.resize(newSize); // Throws StatesException - size checked - ignored

	emit variableResizedEvent();
}

QString Variable::getText() const
{
	return this->name;
}

void Variable::setCurrentValue(const LogicValue& value) // Throws StatesException
{
	// We have to make sure we use this call fuction,
	// and not the overriding ones:
	Variable::setCurrentValueSubRange(value, -1, -1); // Throws StatesException - Propagted
}

void Variable::setCurrentValueSubRange(const LogicValue& value, int rangeL, int rangeR) // Throws StatesException
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
		emit variableDynamicStateChangedEvent();
	}
	else
	{
		throw StatesException("Variable", VariableError_t::size_mismatch, "Trying to set initial value with value whom size does not match variable size or specified rank");
	}
}

LogicValue Variable::getCurrentValue() const
{
	return this->currentValue;
}

LogicValue Variable::getInitialValue() const
{
	return this->initialValue;
}

void Variable::setInitialValue(const LogicValue& newInitialValue) // Throws StatesException
{
	if (this->getSize() == newInitialValue.getSize())
	{
		this->initialValue = newInitialValue;

		emit variableInitialValueChangedEvent();
	}
	else
	{
		throw StatesException("Variable", VariableError_t::size_mismatch, "Trying to set initial value with value whom size does not match variable size");
	}
}

void Variable::reinitialize()
{
	this->currentValue = this->initialValue;
	emit variableDynamicStateChangedEvent();
}

// True concept here only apply to one bit variables
// A larger variable must never be checked for trueness
bool Variable::isTrue() const // Throws StatesException
{
	if (this->getSize() == 1)
	{
		if (this->currentValue == LogicValue::getValue1(1))
			return true;
		else
			return false;
	}
	else
	{
		throw StatesException("Variable", VariableError_t::variable_is_not_bool, "Asking for boolean value on non 1-sized variable");
	}
}
