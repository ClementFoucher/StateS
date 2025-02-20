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

// Debug
#include <QDebug>

// StateS classes
#include "statestypes.h"
#include "variable.h"
#include "statesexception.h"
#include "exceptiontypes.h"


ActionOnVariable::ActionOnVariable(shared_ptr<Variable> variable, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR)
{
	this->variable = variable;
	connect(variable.get(), &Variable::variableResizedEvent, this, &ActionOnVariable::variableResizedEventHandler);
	// Renaming a variable doesn't actually changes the action configuration,
	// but it changes the way the action is displayed: trigger an actionChangedEvent
	connect(variable.get(), &Variable::variableRenamedEvent, this, &ActionOnVariable::actionChangedEvent);

	////
	// Affect range parameters

	if (this->checkIfRangeFitsVariable(rangeL, rangeR) == true)
	{
		this->rangeL = rangeL;
		this->rangeR = rangeR;
	}
	else
	{
		this->rangeL = -1;
		this->rangeR = -1;

		qDebug() << "(ActionOnVariable:) Warning! The extraction range requested for action value ([" + QString::number(rangeL) + ( (rangeR>=0) ? (":" + QString::number(rangeR)) : ("") ) + "]) isn't correct (variable size is [" + QString::number(variable->getSize()-1) + ":0]).";
		qDebug() << "(ActionOnVariable:) Range ignored and action set to full variable.";
	}

	////
	// Affect action type

	if ( (this->getActionSize() == 1) && (actionType == ActionOnVariableType_t::assign) )
	{
		this->actionType = ActionOnVariableType_t::set;

		qDebug() << "(ActionOnVariable:) Warning! The action type requested is illegal for 1-bit actions.";
		qDebug() << "(ActionOnVariable:) Action type defaulted to 'set'";
	}
	else
	{
		this->actionType = actionType;
	}

	////
	// Affect action value

	if (this->isActionValueEditable() == false)
	{
		this->actionValue = LogicValue::getNullValue();
	}
	else
	{
		if (actionValue.getSize() == this->getActionSize())
		{
			this->actionValue = actionValue;
		}
		else if (actionValue.isNull())
		{
			// No value provided, default value used
			this->actionValue = LogicValue::getValue1(this->getActionSize());
		}
		else
		{
			// Incorrect value provided
			this->actionValue = LogicValue::getValue1(this->getActionSize());

			qDebug() << "(ActionOnVariable:) Warning! The action value requested (" + actionValue.toString() + ") didn't match action size (" + QString::number(this->getActionSize()) + ").";
			qDebug() << "(ActionOnVariable:) Value defaulted to " + this->actionValue.toString();
		}
	}
}

void ActionOnVariable::setActionType(ActionOnVariableType_t newType) // Throws StatesException
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if ( (this->getActionSize() == 1) && (newType == ActionOnVariableType_t::assign) )
		{
			throw StatesException("ActionOnVariable", ActionOnVariableError_t::illegal_type, "Type can't be applied to this variable");
		}

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

		emit actionChangedEvent();
	}
}

void ActionOnVariable::setActionValue(LogicValue newValue) // Throws StatesException
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if (this->isActionValueEditable())
		{
			if (this->getActionSize() == newValue.getSize())
			{
				this->actionValue = newValue;
				emit actionChangedEvent();
			}
			else if ( (! newValue.isNull()) && (this->getActionSize() > newValue.getSize()) )
			{
				// Allow for shorter value => fill left with zeros
				LogicValue correctedValue = newValue;

				correctedValue.resize(this->getActionSize()); // Throws StatesException - Ignored - should not happen as action size is never 0 when variable in not nullptr
				this->actionValue = correctedValue;
				emit actionChangedEvent();
			}
			else
			{
				throw StatesException("ActionOnVariable", ActionOnVariableError_t::illegal_value, "Requested action value doesn't match action size");
			}
		}
		else
		{
			throw StatesException("ActionOnVariable", ActionOnVariableError_t::action_value_is_read_only, "Can't affect action value as value is implicit for this action");
		}
	}
}

void ActionOnVariable::setActionRange(int newRangeL, int newRangeR, LogicValue newValue) // Throws StatesException
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if (this->checkIfRangeFitsVariable(newRangeL, newRangeR) == true)
		{
			this->rangeL = newRangeL;
			this->rangeR = newRangeR;

			if (this->isActionValueEditable())
			{
				if (! newValue.isNull())
				{
					// A new value has been provided: store it
					this->setActionValue(newValue); // Throws exception - Propagated
				}
				else
				{
					// No new value provided: resize existing value or create one
					if (! this->actionValue.isNull())
					{
						this->actionValue.resize(this->getActionSize());
					}
					else
					{
						// Default to vector of ones
						this->actionValue = LogicValue::getValue1(this->getActionSize());
					}
				}
			}
			else
			{
				// Value is not editable any more: erase potentially stored value
				this->actionValue = LogicValue::getNullValue();
			}

			emit actionChangedEvent();
		}
		else
		{
			throw StatesException("ActionOnVariable", ActionOnVariableError_t::illegal_range, "Requested range does not fit variable size");
		}
	}
}

shared_ptr<Variable> ActionOnVariable::getVariableActedOn() const
{
	return this->variable.lock();
}

ActionOnVariableType_t ActionOnVariable::getActionType() const
{
	return this->actionType;
}

LogicValue ActionOnVariable::getActionValue() const
{
	LogicValue publicActionValue = LogicValue::getNullValue();

	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if (this->isActionValueEditable())
		{
			publicActionValue = this->actionValue;
		}
		else
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
				publicActionValue = l_variable->getCurrentValue();
				publicActionValue.increment();
				break;
			case ActionOnVariableType_t::decrement:
				publicActionValue = l_variable->getCurrentValue();
				publicActionValue.decrement();
				break;
			case ActionOnVariableType_t::assign:
				// Should not happen: only explicit values here
				break;
			}
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
	uint actionSize = 0;

	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if (l_variable->getSize() == 1)
			actionSize = 1;
		else
		{
			if ( (this->rangeL < 0) && (this->rangeR < 0) )
				actionSize = l_variable->getSize();
			else if ( (this->rangeL >= 0) && (this->rangeR < 0) )
				actionSize = 1;
			else
				actionSize = (uint)(this->rangeL - this->rangeR + 1);
		}
	}

	return actionSize;
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
			return false;
		else
			return true;
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

void ActionOnVariable::beginAction()
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		l_variable->setCurrentValueSubRange(this->getActionValue(), this->rangeL, this->rangeR);

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
}

void ActionOnVariable::endAction()
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if  (l_variable != nullptr)
	{
		if (this->isActionActing)
		{
			l_variable->setCurrentValueSubRange(LogicValue::getValue0(this->getActionSize()), this->rangeL, this->rangeR);
			this->isActionActing = false;
		}
	}
}

void ActionOnVariable::variableResizedEventHandler()
{
	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		try
		{
			if (l_variable->getSize() == 1) // We are now acting on a 1-bit variable
			{
				// Clear range
				this->rangeL = -1;
				this->rangeR = -1;

				// Switch to implicit value whatever the variable size was
				this->actionValue = LogicValue::getNullValue();

				if (this->actionType == ActionOnVariableType_t::assign) // Assign is illegal for single bit variables
					this->actionType = ActionOnVariableType_t::set;
			}
			else // We are now acting on a vector variable
			{
				// First check if range is still valid
				if ( (this->rangeL >= 0) && (this->rangeR < 0) ) // Single bit action
				{
					// Check if bit extracted still in range
					if (this->rangeL >= (int)l_variable->getSize())
						this->rangeL = l_variable->getSize()-1;
				}
				else if ( (this->rangeL >= 0) && (this->rangeR >= 0) ) // Sub-range action
				{
					// Check if parameters are still in range
					if (this->rangeL >= (int)l_variable->getSize())
					{
						this->rangeL = l_variable->getSize()-1;

						// Make sure R param is always lower than L param
						if (this->rangeR >= this->rangeL)
						{
							this->rangeR = this->rangeL-1;
						}
					}
				}

				// Then check value
				if (this->isActionValueEditable())
				{
					if (! this->actionValue.isNull())
					{
						// Old value should be resized
						this->actionValue.resize(this->getActionSize()); // Throws StatesException - Ignored: action size is not 0
					}
					else
					{
						// This used to be an implicit value:
						// Create action value
						this->actionValue = LogicValue::getValue1(this->getActionSize());
					}
				}
				else
				{
					this->actionValue = LogicValue::getNullValue();
				}
			}

			emit actionChangedEvent();
		}
		catch (const StatesException& e)
		{
			if ( (e.getSourceClass() == "LogicValue") && (e.getEnumValue() == LogicValueError_t::unsupported_char) )
			{
				qDebug() << "(ActionOnVariable:) Error! Unable to resize action value. Action is probably broken now.";
			}
			else
				throw;
		}
	}
}

bool ActionOnVariable::checkIfRangeFitsVariable(int rangeL, int rangeR) const
{
	bool valuesMach = false;

	shared_ptr<Variable> l_variable = this->variable.lock();
	if (l_variable != nullptr)
	{
		if ( (rangeL < 0 ) && (rangeR < 0) )
		{
			// Full range action: always allowed
			valuesMach = true;
		}
		else if ( (rangeL >= 0) && (rangeR < 0) )
		{
			// Single-bit action: check if in range
			if (rangeL < (int)l_variable->getSize())
			{
				valuesMach = true;
			}
		}
		else
		{
			// Sub-range action: check if param order correct
			if (rangeL > rangeR)
			{
				// We know both parameters are positive, and their order is correct.
				// Check if left side is in range.
				if (rangeL < (int)l_variable->getSize())
				{
					valuesMach = true;
				}
			}
		}
	}

	return valuesMach;
}
