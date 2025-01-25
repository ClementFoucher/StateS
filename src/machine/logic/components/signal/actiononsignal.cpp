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
#include "actiononsignal.h"

// Debug
#include <QDebug>

// StateS classes
#include "statestypes.h"
#include "StateS_signal.h"
#include "statesexception.h"
#include "exceptiontypes.h"


ActionOnSignal::ActionOnSignal(shared_ptr<Signal> signal, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR)
{
	this->signal = signal;
	connect(signal.get(), &Signal::signalResizedEvent, this, &ActionOnSignal::signalResizedEventHandler);
	// Renaming a signal doesn't actually changes the action configuration,
	// but it changes the way the action is displayed: trigger an actionChangedEvent
	connect(signal.get(), &Signal::signalRenamedEvent, this, &ActionOnSignal::actionChangedEvent);

	////
	// Affect range parameters

	if (this->checkIfRangeFitsSignal(rangeL, rangeR) == true)
	{
		this->rangeL = rangeL;
		this->rangeR = rangeR;
	}
	else
	{
		this->rangeL = -1;
		this->rangeR = -1;

		qDebug() << "(ActionOnSignal:) Warning! The extraction range requested for action value ([" + QString::number(rangeL) + ( (rangeR>=0) ? (":" + QString::number(rangeR)) : ("") ) + "]) isn't correct (signal size is [" + QString::number(signal->getSize()-1) + ":0]).";
		qDebug() << "(ActionOnSignal:) Range ignored and action set to full signal.";
	}

	////
	// Affect action type

	if ( (this->getActionSize() == 1) && (actionType == ActionOnVariableType_t::assign) )
	{
		this->actionType = ActionOnVariableType_t::set;

		qDebug() << "(ActionOnSignal:) Warning! The action type requested is illegal for 1-bit actions.";
		qDebug() << "(ActionOnSignal:) Action type defaulted to 'set'";
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

			qDebug() << "(ActionOnSignal:) Warning! The action value requested (" + actionValue.toString() + ") didn't match action size (" + QString::number(this->getActionSize()) + ").";
			qDebug() << "(ActionOnSignal:) Value defaulted to " + this->actionValue.toString();
		}
	}
}

void ActionOnSignal::setActionType(ActionOnVariableType_t newType) // Throws StatesException
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
	{
		if ( (this->getActionSize() == 1) && (newType == ActionOnVariableType_t::assign) )
		{
			throw StatesException("ActionOnSignal", ActionOnSignalError_t::illegal_type, "Type can't be applied to this signal");
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

void ActionOnSignal::setActionValue(LogicValue newValue) // Throws StatesException
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
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

				correctedValue.resize(this->getActionSize()); // Throws StatesException - Ignored - should not happen as action size is never 0 when signal in not nullptr
				this->actionValue = correctedValue;
				emit actionChangedEvent();
			}
			else
			{
				throw StatesException("ActionOnSignal", ActionOnSignalError_t::illegal_value, "Requested action value doesn't match action size");
			}
		}
		else
		{
			throw StatesException("ActionOnSignal", ActionOnSignalError_t::action_value_is_read_only, "Can't affect action value as value is implicit for this action");
		}
	}
}

void ActionOnSignal::setActionRange(int newRangeL, int newRangeR, LogicValue newValue) // Throws StatesException
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
	{
		if (this->checkIfRangeFitsSignal(newRangeL, newRangeR) == true)
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
			throw StatesException("ActionOnSignal", ActionOnSignalError_t::illegal_range, "Requested range does not fit signal size");
		}
	}
}

shared_ptr<Signal> ActionOnSignal::getSignalActedOn() const
{
	return this->signal.lock();
}

ActionOnVariableType_t ActionOnSignal::getActionType() const
{
	return this->actionType;
}

LogicValue ActionOnSignal::getActionValue() const
{
	LogicValue publicActionValue = LogicValue::getNullValue();

	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
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
			case ActionOnVariableType_t::activeOnState: // May be implicit on one-bit signals
				publicActionValue = LogicValue::getValue1(this->getActionSize());
				break;
			case ActionOnVariableType_t::pulse: // May be implicit on one-bit signals
				publicActionValue = LogicValue::getValue1(this->getActionSize());
				break;
			case ActionOnVariableType_t::increment:
				publicActionValue = l_signal->getCurrentValue();
				publicActionValue.increment();
				break;
			case ActionOnVariableType_t::decrement:
				publicActionValue = l_signal->getCurrentValue();
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

int ActionOnSignal::getActionRangeL() const
{
	return this->rangeL;
}

int ActionOnSignal::getActionRangeR() const
{
	return this->rangeR;
}

uint ActionOnSignal::getActionSize() const
{
	uint actionSize = 0;

	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
	{
		if (l_signal->getSize() == 1)
			actionSize = 1;
		else
		{
			if ( (this->rangeL < 0) && (this->rangeR < 0) )
				actionSize = l_signal->getSize();
			else if ( (this->rangeL >= 0) && (this->rangeR < 0) )
				actionSize = 1;
			else
				actionSize = (uint)(this->rangeL - this->rangeR + 1);
		}
	}

	return actionSize;
}

bool ActionOnSignal::isActionValueEditable() const
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


void ActionOnSignal::beginAction()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if  (l_signal != nullptr)
	{
		l_signal->setCurrentValueSubRange(this->getActionValue(), this->rangeL, this->rangeR);

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

void ActionOnSignal::endAction()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if  (l_signal != nullptr)
	{
		if (this->isActionActing)
		{
			l_signal->setCurrentValueSubRange(LogicValue::getValue0(this->getActionSize()), this->rangeL, this->rangeR);
			this->isActionActing = false;
		}
	}
}

void ActionOnSignal::signalResizedEventHandler()
{
	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
	{
		try
		{
			if (l_signal->getSize() == 1) // We are now acting on a 1-bit signal
			{
				// Clear range
				this->rangeL = -1;
				this->rangeR = -1;

				// Switch to implicit value whatever the signal size was
				this->actionValue = LogicValue::getNullValue();

				if (this->actionType == ActionOnVariableType_t::assign) // Assign is illegal for single bit signals
					this->actionType = ActionOnVariableType_t::set;
			}
			else // We are now acting on a vector signal
			{
				// First check if range is still valid
				if ( (this->rangeL >= 0) && (this->rangeR < 0) ) // Single bit action
				{
					// Check if bit extracted still in range
					if (this->rangeL >= (int)l_signal->getSize())
						this->rangeL = l_signal->getSize()-1;
				}
				else if ( (this->rangeL >= 0) && (this->rangeR >= 0) ) // Sub-range action
				{
					// Check if parameters are still in range
					if (this->rangeL >= (int)l_signal->getSize())
					{
						this->rangeL = l_signal->getSize()-1;

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
				qDebug() << "(ActionOnSignal:) Error! Unable to resize action value. Action is probably broken now.";
			}
			else
				throw;
		}
	}
}

bool ActionOnSignal::checkIfRangeFitsSignal(int rangeL, int rangeR) const
{
	bool valuesMach = false;

	shared_ptr<Signal> l_signal = this->signal.lock();
	if (l_signal != nullptr)
	{
		if ( (rangeL < 0 ) && (rangeR < 0) )
		{
			// Full range action: always allowed
			valuesMach = true;
		}
		else if ( (rangeL >= 0) && (rangeR < 0) )
		{
			// Single-bit action: check if in range
			if (rangeL < (int)l_signal->getSize())
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
				if (rangeL < (int)l_signal->getSize())
				{
					valuesMach = true;
				}
			}
		}
	}

	return valuesMach;
}
