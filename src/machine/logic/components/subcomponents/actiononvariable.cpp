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

// Qt classes
#include <QIcon>

// StateS classes
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "pixmapgenerator.h"


//
// Static members
//

QString ActionOnVariable::getActionTypeText(ActionOnVariableType_t type)
{
	switch (type)
	{
	case ActionOnVariableType_t::assign:
		return tr("Assign");
		break;
	case ActionOnVariableType_t::pulse:
		return tr("Pulse");
		break;
	case ActionOnVariableType_t::continuous:
		return tr("Active on state");
		break;
	case ActionOnVariableType_t::increment:
		return tr("Increment");
		break;
	case ActionOnVariableType_t::decrement:
		return tr("Decrement");
		break;
	case ActionOnVariableType_t::reset:
		return tr("Reset");
		break;
	case ActionOnVariableType_t::set:
		return tr("Set");
		break;
	case ActionOnVariableType_t::none:
		return QString();
		break;
	}
}

QIcon ActionOnVariable::getActionTypeIcon(ActionOnVariableType_t type, bool isDown)
{
	switch (type)
	{
	case ActionOnVariableType_t::assign:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/assign")));
		break;
	case ActionOnVariableType_t::pulse:
		if (isDown == false)
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/pulse")));
		}
		else
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/pulse_down")));
		}
		break;
	case ActionOnVariableType_t::continuous:
		if (isDown == false)
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/active_on_state")));
		}
		else
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/active_on_state_down")));
		}
		break;
	case ActionOnVariableType_t::increment:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/increment")));
		break;
	case ActionOnVariableType_t::decrement:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/decrement")));
		break;
	case ActionOnVariableType_t::reset:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/falling_edge")));
		break;
	case ActionOnVariableType_t::set:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/rising_edge")));
		break;
	case ActionOnVariableType_t::none:
		return QIcon();
		break;
	}
}

//
// Class object definition
//

ActionOnVariable::ActionOnVariable(componentId_t variableId, uint actuatorAllowedActions)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	this->variableId             = variableId;
	this->actuatorAllowedActions = actuatorAllowedActions;

	// Assign a default action type.
	// These 4 cases *should* cover all possible situations
	uint allowedActions = this->getAllowedActionTypes();
	if ((allowedActions & (uint)ActionOnVariableType_t::continuous) != 0)
	{
		// Non memorized on state
		this->actionType = ActionOnVariableType_t::continuous;
	}
	else if ((allowedActions & (uint)ActionOnVariableType_t::pulse) != 0)
	{
		// Non memorized on transition
		this->actionType = ActionOnVariableType_t::pulse;
	}
	else if ((allowedActions & (uint)ActionOnVariableType_t::assign) != 0)
	{
		// Memorized with size > 1
		this->actionType = ActionOnVariableType_t::assign;
	}
	else
	{
		// Memorized with size = 1
		this->actionType = ActionOnVariableType_t::set;
	}

	// Provide a default initial action value
	this->checkAndFixActionValue();

	this->connectSignals(variable);
}

ActionOnVariable::ActionOnVariable(shared_ptr<Variable> variable, uint actuatorAllowedActions, ActionOnVariableType_t actionType, LogicValue actionValue, int rangeL, int rangeR)
{
	// Perform absolutely no checks on values: we are loading a file,
	// these checks will be performed later.

	this->variableId             = variable->getId();
	this->actuatorAllowedActions = actuatorAllowedActions;
	this->actionType             = actionType;
	this->actionValue            = actionValue;
	this->rangeL                 = rangeL;
	this->rangeR                 = rangeR;

	this->connectSignals(variable);
}

/**
 * @brief ActionOnVariable::checkAndFixAction is only called
 *        after a load from a save file, to make sure loaded
 *        values are coherent.
 */
void ActionOnVariable::checkAndFixAction()
{
	this->checkAndFixActionRange();
	this->checkAndFixActionType();
	this->checkAndFixActionValue();
}

void ActionOnVariable::setActionType(ActionOnVariableType_t newType)
{
	if (newType == this->actionType) return;

	if ( (this->getAllowedActionTypes() & (uint)newType) == 0) return;


	this->actionType = newType;

	this->checkAndFixActionValue();

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

	if (newValue == this->actionValue) return;


	this->actionValue = newValue;

	emit this->actionChangedEvent();
}

void ActionOnVariable::setActionRange(int newRangeL, int newRangeR)
{
	if ( (newRangeL == this->rangeL) && (newRangeR == this->rangeR) ) return;

	if (this->checkIfRangeFitsVariable(newRangeL, newRangeR) == false) return;


	this->rangeL = newRangeL;
	this->rangeR = newRangeR;

	this->checkAndFixActionValue();

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
	return this->actionValue;
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
	switch (this->actionType)
	{
	case ActionOnVariableType_t::assign:
		return true;
		break;
	case ActionOnVariableType_t::continuous:
	case ActionOnVariableType_t::pulse:
		if (this->getActionSize() > 1)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
	case ActionOnVariableType_t::none:
		return false;
		break;
	}
}

uint ActionOnVariable::getAllowedActionTypes() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return (uint)ActionOnVariableType_t::none;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return (uint)ActionOnVariableType_t::none;


	uint allowedActionTypes = (uint)ActionOnVariableType_t::none;
	if (variable->getMemorized() == true)
	{
		if (this->getActionSize() == 1)
		{
			allowedActionTypes |= (uint)ActionOnVariableType_t::set;
			allowedActionTypes |= (uint)ActionOnVariableType_t::reset;
		}
		else // (this->getActionSize() > 1)
		{
			allowedActionTypes |= (uint)ActionOnVariableType_t::assign;
			allowedActionTypes |= (uint)ActionOnVariableType_t::increment;
			allowedActionTypes |= (uint)ActionOnVariableType_t::decrement;
		}
	}
	else // (variable->getMemorized() == false)
	{
		allowedActionTypes |= (uint)ActionOnVariableType_t::pulse;
		allowedActionTypes |= (uint)ActionOnVariableType_t::continuous;
	}

	allowedActionTypes &= this->actuatorAllowedActions;

	return allowedActionTypes;
}

QString ActionOnVariable::getCurrentActionTypeText() const
{
	return ActionOnVariable::getActionTypeText(this->actionType);
}

QIcon ActionOnVariable::getCurrentActionTypeIcon() const
{
	switch (this->actionType)
	{
	case ActionOnVariableType_t::pulse:
	case ActionOnVariableType_t::continuous:
		if ( (this->getActionSize() == 1) && (this->actionValue == LogicValue::getValue0(1)) )
		{
			return ActionOnVariable::getActionTypeIcon(this->actionType, true);
		}
		else
		{
			return ActionOnVariable::getActionTypeIcon(this->actionType, false);
		}
		break;
	case ActionOnVariableType_t::assign:
	case ActionOnVariableType_t::increment:
	case ActionOnVariableType_t::decrement:
	case ActionOnVariableType_t::reset:
	case ActionOnVariableType_t::set:
	case ActionOnVariableType_t::none:
		return ActionOnVariable::getActionTypeIcon(this->actionType);
		break;
	}
}

void ActionOnVariable::variableResizedEventHandler()
{
	auto previousRangeL      = this->rangeL;
	auto previousRangeR      = this->rangeR;
	auto previousActionType  = this->actionType;
	auto previousActionValue = this->actionValue;

	this->checkAndFixActionRange();
	this->checkAndFixActionType();
	this->checkAndFixActionValue();

	if ( (previousRangeL      != this->rangeL)     ||
	     (previousRangeR      != this->rangeR)     ||
	     (previousActionType  != this->actionType) ||
	     (previousActionValue != this->actionValue) )
	{
		emit this->actionChangedEvent();
	}
}

void ActionOnVariable::variableMemorizedStateChangedEventHandler()
{
	auto previousActionType  = this->actionType;
	auto previousActionValue = this->actionValue;

	this->checkAndFixActionType();
	this->checkAndFixActionValue();

	if ( (previousActionType  != this->actionType) ||
	     (previousActionValue != this->actionValue) )
	{
		emit this->actionChangedEvent();
	}
}

void ActionOnVariable::variableInitialValueChangedEventHandler()
{
	auto previousActionValue = this->actionValue;

	this->checkAndFixActionValue();

	if (previousActionValue != this->actionValue)
	{
		emit this->actionChangedEvent();
	}
}

void ActionOnVariable::connectSignals(shared_ptr<Variable> variable)
{
	connect(variable.get(), &Variable::variableResizedEvent,               this, &ActionOnVariable::variableResizedEventHandler);
	connect(variable.get(), &Variable::variableMemorizedStateChangedEvent, this, &ActionOnVariable::variableMemorizedStateChangedEventHandler);
	connect(variable.get(), &Variable::variableInitialValueChangedEvent,   this, &ActionOnVariable::variableInitialValueChangedEventHandler);

	// This doesn't actually change the action configuration,
	// but it changes the way the action is displayed: trigger an actionChangedEvent
	connect(variable.get(), &Variable::variableRenamedEvent, this, &ActionOnVariable::actionChangedEvent);
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

void ActionOnVariable::checkAndFixActionRange()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	if (variable->getSize() == 1) // We are acting on a 1-bit variable
	{
		// Just clear range
		this->rangeL = -1;
		this->rangeR = -1;
	}
	else // We are acting on a vector variable
	{
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
	}
}

void ActionOnVariable::checkAndFixActionType()
{
	// If current type is allowed, nothing to fix
	if ( ((uint)this->actionType & this->getAllowedActionTypes()) != 0) return;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	if (variable->getMemorized() == false)
	{
		// Always one or the other allowed, but not both
		if ( (this->getAllowedActionTypes() & (uint)ActionOnVariableType_t::pulse) != 0)
		{
			this->actionType = ActionOnVariableType_t::pulse;
		}
		else // if ( (this->getAllowedActionTypes() & (uint)ActionOnVariableType_t::continuous) != 0)
		{
			this->actionType = ActionOnVariableType_t::continuous;
		}
	}
	else // (variable->getMemorized() == true)
	{
		if (this->getActionSize() == 1) // Action is a 1-bit action
		{
			// Try to determine the best action depending on value
			if (this->actionValue == LogicValue::getValue0(this->actionValue.getSize()))
			{
				this->actionType = ActionOnVariableType_t::reset;
			}
			else
			{
				this->actionType = ActionOnVariableType_t::set;
			}
		}
		else // We are acting on a vector variable
		{
			// Default to assign
			this->actionType = ActionOnVariableType_t::assign;
		}
	}
}

void ActionOnVariable::checkAndFixActionValue()
{
	if (this->isActionValueEditable() == true)
	{
		if (this->actionValue.isNull() == false)
		{
			// Make sure action value size is correct
			this->actionValue.resize(this->getActionSize());
		}
		else
		{
			// This used to be an implicit value: create action value
			this->actionValue = LogicValue::getValue0(this->getActionSize());
		}
	}
	else // (this->isActionValueEditable() == false)
	{
		switch (this->actionType)
		{
		case ActionOnVariableType_t::reset:
			this->actionValue = LogicValue::getValue0(this->getActionSize());
			break;
		case ActionOnVariableType_t::set:
			this->actionValue = LogicValue::getValue1(this->getActionSize());
			break;
		case ActionOnVariableType_t::continuous:
		case ActionOnVariableType_t::pulse:
			if (this->getActionSize() == 1)
			{
				auto machine = machineManager->getMachine();
				if (machine == nullptr) return;

				auto variable = machine->getVariable(variableId);
				if (variable == nullptr) return;


				auto variableValue = variable->getInitialValue();
				// Action being size 1 can be the consequence of extracting a single bit
				if (this->rangeL != -1)
				{
					variableValue = variableValue.getSubrange(this->rangeL, this->rangeR);
				}

				this->actionValue = !variableValue;
			}
			// Else value is editable, should not happen
			break;
		case ActionOnVariableType_t::increment:
		case ActionOnVariableType_t::decrement:
			// No value for these action types:
			// the value is determined dymanically during simulation.
			this->actionValue = LogicValue::getNullValue();
			break;
		case ActionOnVariableType_t::none:
			// Just clear value
			this->actionValue = LogicValue::getNullValue();
			break;
		case ActionOnVariableType_t::assign:
			// Value is editable, should not happen
			break;
		}
	}
}
