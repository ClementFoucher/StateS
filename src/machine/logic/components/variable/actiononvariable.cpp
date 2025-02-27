/*
 * Copyright © 2016-2025 Clément Foucher
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
#include "actiononvariable.h"

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"


ActionOnVariable::ActionOnVariable(componentId_t variableId, ActionOnVariableType_t actionType)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	this->variableId = variableId;
	this->initialize(variable, actionType);
}

ActionOnVariable::ActionOnVariable(shared_ptr<Variable> variable, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR)
{
	// Perform absolutely no checks on values: we are loading a file,
	// these checks will be performe later.

	this->variableId  = variable->getId();
	this->actionValue = actionValue;
	this->rangeL      = rangeL;
	this->rangeR      = rangeR;

	this->initialize(variable, actionType);
}

void ActionOnVariable::setActionType(ActionOnVariableType_t newType)
{
	if ( (this->getActionSize() == 1) && (newType == ActionOnVariableType_t::assign) ) return;


	// Changing type impacts affected value:
	// may switch between implicit and explicit
	if (this->getActionSize() > 1) // Single-bit actions always have an implicit value: ignored
	{
		switch (newType)
		{
		case ActionOnVariableType_t::reset:
		case ActionOnVariableType_t::set:
		case ActionOnVariableType_t::increment:
		case ActionOnVariableType_t::decrement:
			// Switch to implicit
			this->actionValue = LogicValue::getNullValue();
			break;
		case ActionOnVariableType_t::activeOnState:
		case ActionOnVariableType_t::pulse:
		case ActionOnVariableType_t::assign:
			// Check if previous action type had implicit value
			// and switch to explicit if necessary, preserving
			// previous action value.
			if (this->actionType == ActionOnVariableType_t::reset)
			{
				this->actionValue = LogicValue::getValue0(this->getActionSize());
			}
			else if (this->actionType == ActionOnVariableType_t::set)
			{
				this->actionValue = LogicValue::getValue1(this->getActionSize());
			}
			break;
		}
	}

	this->actionType = newType;

	emit this->actionChangedEvent();
}

void ActionOnVariable::setActionValue(LogicValue newValue)
{
	if (this->isActionValueEditable() == false) return;


	uint actionSize = this->getActionSize();
	if (actionSize != newValue.getSize())
	{
		newValue.resize(actionSize);
	}

	this->actionValue = newValue;
	emit this->actionChangedEvent();
}

void ActionOnVariable::setActionRange(int newRangeL, int newRangeR)
{
	if (this->checkIfRangeFitsVariable(newRangeL, newRangeR) == false) return;


	this->rangeL = newRangeL;
	this->rangeR = newRangeR;

	if (this->isActionValueEditable() == true)
	{
		// Resize existing value or create one
		if (this->actionValue.isNull() == false)
		{
			this->actionValue.resize(this->getActionSize());
		}
		else // (this->actionValue.isNull() == true)
		{
			// Default to vector of ones
			this->actionValue = LogicValue::getValue1(this->getActionSize());
		}
	}
	else // (this->isActionValueEditable() == false)
	{
		// Value is not editable any more: erase potentially stored value
		this->actionValue = LogicValue::getNullValue();
	}

	emit this->actionChangedEvent();
}

componentId_t ActionOnVariable::getVariableActedOnId() const
{
	return this->variableId;
}

ActionOnVariableType_t ActionOnVariable::getActionType() const
{
	return this->actionType;
}

LogicValue ActionOnVariable::getActionValue() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return LogicValue::getNullValue();

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return LogicValue::getNullValue();


	LogicValue publicActionValue = LogicValue::getNullValue();
	if (this->isActionValueEditable() == true)
	{
		publicActionValue = this->actionValue;
	}
	else // (this->isActionValueEditable() == false)
	{
		// Implicit values
		switch (this->actionType)
		{
		case ActionOnVariableType_t::reset:
			publicActionValue = LogicValue::getValue0(this->getActionSize());
			break;
		case ActionOnVariableType_t::set:
			publicActionValue = LogicValue::getValue1(this->getActionSize());
			break;
		case ActionOnVariableType_t::activeOnState: // May be implicit on one-bit variables
			publicActionValue = LogicValue::getValue1(this->getActionSize());
			break;
		case ActionOnVariableType_t::pulse: // May be implicit on one-bit variables
			publicActionValue = LogicValue::getValue1(this->getActionSize());
			break;
		case ActionOnVariableType_t::increment:
			publicActionValue = variable->getCurrentValue();
			publicActionValue.increment();
			break;
		case ActionOnVariableType_t::decrement:
			publicActionValue = variable->getCurrentValue();
			publicActionValue.decrement();
			break;
		case ActionOnVariableType_t::assign:
			// Should not happen: only explicit values here
			break;
		}
	}

	return publicActionValue;
}

int ActionOnVariable::getActionRangeL() const
{
	return this->rangeL;
}

int ActionOnVariable::getActionRangeR() const
{
	return this->rangeR;
}

uint ActionOnVariable::getActionSize() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return 0;


	if (variable->getSize() == 1)
	{
		return 1;
	}
	else
	{
		if ( (this->rangeL < 0) && (this->rangeR < 0) )
		{
			return variable->getSize();
		}
		else if ( (this->rangeL >= 0) && (this->rangeR < 0) )
		{
			return 1;
		}
		else
		{
			return (uint)(this->rangeL - this->rangeR + 1);
		}
	}
}

bool ActionOnVariable::isActionValueEditable() const
{
	if (this->getActionSize() == 1)
	{
		return false;
	}
	else
	{
		if ( (this->actionType == ActionOnVariableType_t::set)      ||
		     (this->actionType == ActionOnVariableType_t::reset)    ||
		     (this->actionType == ActionOnVariableType_t::increment)||
		     (this->actionType == ActionOnVariableType_t::decrement) )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}

bool ActionOnVariable::isActionMemorized() const
{
	switch (this->actionType)
	{
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::assign:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
		return true;
		break;
	case ActionOnVariableType_t::activeOnState:
	case ActionOnVariableType_t::pulse:
		return false;
		break;
	}
}

void ActionOnVariable::checkActionValue()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	auto previousActionValue = this->actionValue;
	auto previousRangeL      = this->rangeL;
	auto previousRangeR      = this->rangeR;

	if (variable->getSize() == 1) // We are acting on a 1-bit variable
	{
		// Clear range
		this->rangeL = -1;
		this->rangeR = -1;

		// Switch to implicit value
		this->actionValue = LogicValue::getNullValue();

		if (this->actionType == ActionOnVariableType_t::assign) // Assign is illegal for single bit variables
		{
			this->actionType = ActionOnVariableType_t::set;
		}
	}
	else // We are acting on a vector variable
	{
		// First check if range is valid
		if ( (this->rangeL >= 0) && (this->rangeR < 0) ) // Single bit action
		{
			// Check if bit extracted is in range
			if (this->rangeL >= (int)variable->getSize())
			{
				this->rangeL = variable->getSize()-1;
			}
		}
		else if ( (this->rangeL >= 0) && (this->rangeR >= 0) ) // Sub-range action
		{
			// Check if parameters are in range
			if (this->rangeL >= (int)variable->getSize())
			{
				this->rangeL = variable->getSize()-1;

				// Make sure R param is always lower than L param
				if (this->rangeR >= this->rangeL)
				{
					this->rangeR = this->rangeL-1;
				}
			}
		}

		// Then check value
		if (this->isActionValueEditable() == true)
		{
			if (this->actionValue.isNull() == false)
			{
				// Make sure action value size is correct
				this->actionValue.resize(this->getActionSize());
			}
			else
			{
				// This used to be an implicit value:
				// Create action value
				this->actionValue = LogicValue::getValue1(this->getActionSize());
			}
		}
		else // (this->isActionValueEditable() == false)
		{
			this->actionValue = LogicValue::getNullValue();
		}
	}

	if ( (previousActionValue != this->actionValue) ||
	     (previousRangeL      != this->rangeL)      ||
	     (previousRangeR      != this->rangeR)      )
	{
		emit this->actionChangedEvent();
	}
}

void ActionOnVariable::beginAction()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	variable->setCurrentValueSubRange(this->getActionValue(), this->rangeL, this->rangeR);

	switch (this->actionType)
	{
	case ActionOnVariableType_t::pulse:
	case ActionOnVariableType_t::activeOnState:
		this->isActionActing = true;
		break;
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::assign:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
		// Do not register, value change is definitive
		break;
	}
}

void ActionOnVariable::endAction()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	if (this->isActionActing == true)
	{
		variable->setCurrentValueSubRange(LogicValue::getValue0(this->getActionSize()), this->rangeL, this->rangeR);
		this->isActionActing = false;
	}
}

void ActionOnVariable::variableResizedEventHandler()
{
	this->checkActionValue();
}

bool ActionOnVariable::checkIfRangeFitsVariable(int rangeL, int rangeR) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return false;


	if ( (rangeL < 0 ) && (rangeR < 0) )
	{
		// Full range action: always allowed
		return true;
	}
	else if ( (rangeL >= 0) && (rangeR < 0) )
	{
		// Single-bit action: check if in range
		if (rangeL < (int)variable->getSize())
		{
			return true;
		}
	}
	else
	{
		// Sub-range action: check if param order correct
		if (rangeL > rangeR)
		{
			// We know both parameters are positive, and their order is correct.
			// Check if left side is in range.
			if (rangeL < (int)variable->getSize())
			{
				return true;
			}
		}
	}

	return false;
}

void ActionOnVariable::initialize(shared_ptr<Variable> variable, ActionOnVariableType_t actionType)
{
	connect(variable.get(), &Variable::variableResizedEvent, this, &ActionOnVariable::variableResizedEventHandler);
	// Renaming a variable doesn't actually changes the action configuration,
	// but it changes the way the action is displayed: trigger an actionChangedEvent
	connect(variable.get(), &Variable::variableRenamedEvent, this, &ActionOnVariable::actionChangedEvent);

	////
	// Affect action type

	if ( (variable->getSize() == 1) && (actionType == ActionOnVariableType_t::assign) )
	{
		// Circumvent illegal action type
		this->actionType = ActionOnVariableType_t::set;
	}
	else
	{
		this->actionType = actionType;
	}
}
