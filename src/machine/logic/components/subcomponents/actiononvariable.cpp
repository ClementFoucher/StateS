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
#include "actiononvariable.h"

// Qt
#include <QIcon>

// StateS
#include "machinemanager.h"
#include "machine.h"
#include "variable.h"
#include "pixmapgenerator.h"


//
// Static members
//

QString ActionOnVariable::getActionTypeText(ActionOnVariable::Type_t type)
{
	switch (type)
	{
	case ActionOnVariable::Type_t::assign:
		return tr("Assign");
		break;
	case ActionOnVariable::Type_t::pulse:
		return tr("Pulse");
		break;
	case ActionOnVariable::Type_t::continuous:
		return tr("Active on state");
		break;
	case ActionOnVariable::Type_t::increment:
		return tr("Increment");
		break;
	case ActionOnVariable::Type_t::decrement:
		return tr("Decrement");
		break;
	case ActionOnVariable::Type_t::reset:
		return tr("Reset");
		break;
	case ActionOnVariable::Type_t::set:
		return tr("Set");
		break;
	case ActionOnVariable::Type_t::none:
		return QString();
		break;
	}
}

QIcon ActionOnVariable::getActionTypeIcon(ActionOnVariable::Type_t type, bool isDown)
{
	switch (type)
	{
	case ActionOnVariable::Type_t::assign:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/assign")));
		break;
	case ActionOnVariable::Type_t::pulse:
		if (isDown == false)
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/pulse")));
		}
		else
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/pulse_down")));
		}
		break;
	case ActionOnVariable::Type_t::continuous:
		if (isDown == false)
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/active_on_state")));
		}
		else
		{
			return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/active_on_state_down")));
		}
		break;
	case ActionOnVariable::Type_t::increment:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/increment")));
		break;
	case ActionOnVariable::Type_t::decrement:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/decrement")));
		break;
	case ActionOnVariable::Type_t::reset:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/falling_edge")));
		break;
	case ActionOnVariable::Type_t::set:
		return QIcon(PixmapGenerator::getPixmapFromSvg(QString(":/icons/rising_edge")));
		break;
	case ActionOnVariable::Type_t::none:
		return QIcon();
		break;
	}
}

//
// Class object definition
//

ActionOnVariable::ActionOnVariable(ComponentId variableId, uint actuatorAllowedActions)
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
	if ((allowedActions & static_cast<uint>(ActionOnVariable::Type_t::continuous)) != 0)
	{
		// Non memorized on state
		this->actionType = ActionOnVariable::Type_t::continuous;
	}
	else if ((allowedActions & static_cast<uint>(ActionOnVariable::Type_t::pulse)) != 0)
	{
		// Non memorized on transition
		this->actionType = ActionOnVariable::Type_t::pulse;
	}
	else if ((allowedActions & static_cast<uint>(ActionOnVariable::Type_t::assign)) != 0)
	{
		// Memorized with size > 1
		this->actionType = ActionOnVariable::Type_t::assign;
	}
	else
	{
		// Memorized with size = 1
		this->actionType = ActionOnVariable::Type_t::set;
	}

	// Provide a default initial action value
	this->checkAndFixActionValue();

	this->connectSignals(variable);
}

ActionOnVariable::ActionOnVariable(std::shared_ptr<Variable> variable, uint actuatorAllowedActions, ActionOnVariable::Type_t actionType, MachineValue actionValue, int rangeL, int rangeR)
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

void ActionOnVariable::setActionType(ActionOnVariable::Type_t newType)
{
	if (newType == this->actionType) return;

	if ( (this->getAllowedActionTypes() & static_cast<uint>(newType)) == 0) return;


	this->actionType = newType;

	this->checkAndFixActionValue();

	emit this->actionChangedEvent();
}

void ActionOnVariable::setActionValue(MachineValue newValue)
{
	if (this->isActionValueEditable() == false) return;

	if (newValue == this->actionValue) return;

	if (newValue.getType() != this->getExpectedActionType()) return;


	this->actionValue = newValue;

	this->checkAndFixActionValue();

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

ComponentId ActionOnVariable::getVariableActedOnId() const
{
	return this->variableId;
}

ActionOnVariable::Type_t ActionOnVariable::getActionType() const
{
	return this->actionType;
}

MachineValue ActionOnVariable::getActionValue() const
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

bool ActionOnVariable::isActionValueEditable() const
{
	switch (this->actionType)
	{
	case ActionOnVariable::Type_t::assign:
		return true;
		break;
	case ActionOnVariable::Type_t::continuous:
	case ActionOnVariable::Type_t::pulse:
		if (this->getExpectedBitVectorActionSize() > 1)
		{
			return true;
		}
		else
		{
			return false;
		}
		break;
	case ActionOnVariable::Type_t::set:
	case ActionOnVariable::Type_t::reset:
	case ActionOnVariable::Type_t::increment:
	case ActionOnVariable::Type_t::decrement:
	case ActionOnVariable::Type_t::none:
		return false;
		break;
	}
}

uint ActionOnVariable::getAllowedActionTypes() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return static_cast<uint>(ActionOnVariable::Type_t::none);

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return static_cast<uint>(ActionOnVariable::Type_t::none);


	uint allowedActionTypes = static_cast<uint>(ActionOnVariable::Type_t::none);
	if (variable->getMemorized() == true)
	{
		allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::assign);
		allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::reset);
		if (this->getExpectedActionType() == MachineValue::Type_t::boolean)
		{
			allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::set);
		}
		else // (this->getExpectedActionType() == MachineValue::ValueType_t::bitVector)
		{
			allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::increment);
			allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::decrement);
		}
	}
	else // (variable->getMemorized() == false)
	{
		allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::pulse);
		allowedActionTypes |= static_cast<uint>(ActionOnVariable::Type_t::continuous);
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
	case ActionOnVariable::Type_t::pulse:
	case ActionOnVariable::Type_t::continuous:
		if ( (this->getExpectedActionType() == MachineValue::Type_t::boolean) && (this->actionValue == BooleanValue::falseValue()) )
		{
			// Return inverted icon for 1-bit actions whose action value is false
			return ActionOnVariable::getActionTypeIcon(this->actionType, true);
		}
		else
		{
			return ActionOnVariable::getActionTypeIcon(this->actionType, false);
		}
		break;
	case ActionOnVariable::Type_t::assign:
	case ActionOnVariable::Type_t::increment:
	case ActionOnVariable::Type_t::decrement:
	case ActionOnVariable::Type_t::reset:
	case ActionOnVariable::Type_t::set:
	case ActionOnVariable::Type_t::none:
		return ActionOnVariable::getActionTypeIcon(this->actionType);
		break;
	}
}

void ActionOnVariable::variableTypeChangedEventHandler()
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
	     (previousActionValue != this->actionValue)
	   )
	{
		emit this->actionFixedEvent();
	}
}

void ActionOnVariable::variableMemorizedStateChangedEventHandler()
{
	auto previousActionType  = this->actionType;
	auto previousActionValue = this->actionValue;

	this->checkAndFixActionType();
	this->checkAndFixActionValue();

	if ( (previousActionType  != this->actionType) ||
	     (previousActionValue != this->actionValue)
	   )
	{
		emit this->actionFixedEvent();
	}
}

void ActionOnVariable::variableInitialValueChangedEventHandler()
{
	auto previousActionValue = this->actionValue;

	this->checkAndFixActionValue();

	if (previousActionValue != this->actionValue)
	{
		emit this->actionFixedEvent();
	}
}

void ActionOnVariable::connectSignals(std::shared_ptr<Variable> variable)
{
	connect(variable.get(), &Variable::variableTypeChangedEvent,           this, &ActionOnVariable::variableTypeChangedEventHandler);
	connect(variable.get(), &Variable::variableMemorizedStateChangedEvent, this, &ActionOnVariable::variableMemorizedStateChangedEventHandler);
	connect(variable.get(), &Variable::variableInitialValueChangedEvent,   this, &ActionOnVariable::variableInitialValueChangedEventHandler);

	// This doesn't actually change the action configuration,
	// but it changes the way the action is displayed: trigger an actionChangedEvent
	connect(variable.get(), &Variable::variableRenamedEvent, this, &ActionOnVariable::actionChangedEvent);

	// Action being fixed results in an action change
	connect(this, &ActionOnVariable::actionFixedEvent, this, &ActionOnVariable::actionChangedEvent);
}

bool ActionOnVariable::checkIfRangeFitsVariable(int rangeL, int rangeR) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return false;

	auto variableValue = variable->getInitialValue();
	if (variableValue.getType() != MachineValue::Type_t::bitVector) return false;


	int variableSize = static_cast<int>(variableValue.getBitVectorValue().getSize());

	if ( (rangeL < 0 ) && (rangeR < 0) )
	{
		// Full range action: always allowed
		return true;
	}
	else if ( (rangeL >= 0) && (rangeR < 0) )
	{
		// Single-bit action: check if in range
		if (rangeL < variableSize)
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
			if (rangeL < variableSize)
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

	auto variableValue = variable->getInitialValue();
	if (variableValue.getType() != MachineValue::Type_t::bitVector)
	{
		this->rangeL = -1;
		this->rangeR = -1;
		return;
	}


	int variableSize = static_cast<int>(variableValue.getBitVectorValue().getSize());

	if (variableSize == 1) // We are acting on a 1-bit vector
	{
		// Just clear range
		this->rangeL = -1;
		this->rangeR = -1;
	}
	else // We are acting on a multi-bit vector
	{
		if ( (this->rangeL >= 0) && (this->rangeR < 0) ) // Single bit action
		{
			// Check if bit extracted is in range
			if (this->rangeL >= variableSize)
			{
				this->rangeL = variableSize-1;
			}
		}
		else if ( (this->rangeL >= 0) && (this->rangeR >= 0) ) // Sub-range action
		{
			// Check if parameters are in range
			if (this->rangeL >= variableSize)
			{
				this->rangeL = variableSize-1;

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
	if ( (static_cast<uint>(this->actionType )& this->getAllowedActionTypes()) != 0) return;

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;


	if (variable->getMemorized() == false)
	{
		// Always one or the other allowed, but not both
		if ( (this->getAllowedActionTypes() & static_cast<uint>(ActionOnVariable::Type_t::pulse)) != 0)
		{
			this->actionType = ActionOnVariable::Type_t::pulse;
		}
		else // if ( (this->getAllowedActionTypes() & static_cast<uint>(ActionOnVariable::Type_t::continuous)) != 0)
		{
			this->actionType = ActionOnVariable::Type_t::continuous;
		}
	}
	else // (variable->getMemorized() == true)
	{
		if (this->getExpectedActionType() == MachineValue::Type_t::boolean)
		{
			// Try to determine the best action depending on current action value
			if (this->actionValue == BooleanValue::trueValue())
			{
				this->actionType = ActionOnVariable::Type_t::set;
			}
			else
			{
				this->actionType = ActionOnVariable::Type_t::reset;
			}
		}
		else // (this->getExpectedActionType() == MachineValue::ValueType_t::bitVector)
		{
			// Default to assign
			this->actionType = ActionOnVariable::Type_t::assign;
		}
	}
}

void ActionOnVariable::checkAndFixActionValue()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;

	if (variable->getType() == MachineValue::Type_t::nullType) return;


	if (this->isActionValueEditable() == true)
	{
		if (this->actionValue.isNull() == false)
		{
			// There is currently an action value: check if it is still correct
			if (this->getExpectedActionType() != this->actionValue.getType())
			{
				this->setDefaultActionValue();
			}
			else // (this->getExpectedActionType() == this->actionValue.getType())
			{
				// Current action type is correct.
				// Make sure its size is also correct for Bit Vectors
				if (this->actionValue.getType() == MachineValue::Type_t::bitVector)
				{
					uint expectedActionSize = this->getExpectedBitVectorActionSize();
					auto currentBitVectorValue = this->actionValue.getBitVectorValue();

					if (expectedActionSize != currentBitVectorValue.getSize())
					{
						currentBitVectorValue.resize(expectedActionSize);
						this->actionValue = currentBitVectorValue;
					}
				}
			}
		}
		else // (this->actionValue.isNull() == true)
		{
			// This used to be an implicit value: create action value
			this->setDefaultActionValue();
		}
	}
	else // (this->isActionValueEditable() == false)
	{
		switch (this->actionType)
		{
		case ActionOnVariable::Type_t::reset:
			switch (this->getExpectedActionType())
			{
			case MachineValue::Type_t::boolean:
				this->actionValue = BooleanValue::falseValue();
				break;
			case MachineValue::Type_t::bitVector:
				this->actionValue = BitVectorValue::allZeros(this->getExpectedBitVectorActionSize());
				break;
			case MachineValue::Type_t::nullType:
				// Checked previously: should not happen
				break;
			}
			break;
		case ActionOnVariable::Type_t::set:
			switch (this->getExpectedActionType())
			{
			case MachineValue::Type_t::boolean:
				this->actionValue = BooleanValue::trueValue();
				break;
			case MachineValue::Type_t::bitVector:
				// Set not allowed on Bit Vectors
				break;
			case MachineValue::Type_t::nullType:
				// Checked previously: should not happen
				break;
			}
			break;
		case ActionOnVariable::Type_t::continuous:
		case ActionOnVariable::Type_t::pulse:
			// Only one-bit values are non editable: use the default value.
			// actions that are more than one bit are editable thus not handled here.
			this->setDefaultActionValue();
			break;
		case ActionOnVariable::Type_t::increment:
		case ActionOnVariable::Type_t::decrement:
			// No value for these action types:
			// the value is determined dynamically during simulation.
			this->actionValue = MachineValue{};
			break;
		case ActionOnVariable::Type_t::none:
			// Just clear value
			this->actionValue = MachineValue{};
			break;
		case ActionOnVariable::Type_t::assign:
			// Value is editable, should not happen
			break;
		}
	}
}

/**
 * @brief ActionOnVariable::setDefaultActionValue computes
 *        a default value for the action when the current
 *        action value is unfixable. By default, use the
 *        opposite of the initial value, as it is most
 *        likely what the user wanted, plus it is the
 *        value that is used for one-bit continuous and
 *        pulse actions.
 */
void ActionOnVariable::setDefaultActionValue()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return;

	auto variableValue = variable->getInitialValue();
	if (variableValue.isNull() == true) return;


	switch (this->getExpectedActionType())
	{
	case MachineValue::Type_t::boolean:
		switch (variable->getType())
		{
		case MachineValue::Type_t::boolean:
			this->actionValue = !variableValue.getBooleanValue();
			break;
		case MachineValue::Type_t::bitVector:
		{
			// For boolean actions over Bit Vectors, get the opposite of the bit acted on
			auto subRangeValue = variableValue.getBitVectorValue().getSubrange(this->rangeL, -1);
			this->actionValue = BooleanValue{!subRangeValue[0]};
			break;
		}
		case MachineValue::Type_t::nullType:
			// Should not happen
			break;
		}
		break;
	case MachineValue::Type_t::bitVector:
		// For vector values, there is no way of determining the value the user want.
		// Default to a full 0 vector.
		this->actionValue = BitVectorValue::allZeros(this->getExpectedBitVectorActionSize());
		break;
	case MachineValue::Type_t::nullType:
		// Should not happen
		break;
	}
}

MachineValue::Type_t ActionOnVariable::getExpectedActionType() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return MachineValue::Type_t::nullType;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return MachineValue::Type_t::nullType;


	switch (variable->getType())
	{
	case MachineValue::Type_t::boolean:
		return MachineValue::Type_t::boolean;
		break;
	case MachineValue::Type_t::bitVector:
		// Action value is Boolean for 1-bit actions, Bit Vector otherwise
		if (this->getExpectedBitVectorActionSize() == 1)
		{
			return MachineValue::Type_t::boolean;
		}
		else // (this->getExpectedBitVectorActionSize() > 1)
		{
			return MachineValue::Type_t::bitVector;
		}
		break;
	case MachineValue::Type_t::nullType:
		return MachineValue::Type_t::nullType;
		break;
	}
}

uint ActionOnVariable::getExpectedBitVectorActionSize() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto variable = machine->getVariable(variableId);
	if (variable == nullptr) return 0;

	if (variable->getType() != MachineValue::Type_t::bitVector) return 0;


	uint variableSize = variable->getInitialValue().getBitVectorValue().getSize();

	if (variableSize == 1)
	{
		return 1;
	}
	else
	{
		if ( (this->rangeL < 0) && (this->rangeR < 0) )
		{
			return variableSize;
		}
		else if ( (this->rangeL >= 0) && (this->rangeR < 0) )
		{
			return 1;
		}
		else
		{
			return static_cast<uint>(this->rangeL - this->rangeR + 1);
		}
	}
}
