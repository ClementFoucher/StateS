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
#include "equation.h"
#include "operand.h"


//
// Static members
//

QString ActionOnVariable::getBehaviorText(ActionOnVariable::Behavior_t behavior)
{
	switch (behavior)
	{
	case ActionOnVariable::Behavior_t::assign:
		return tr("Assign");
		break;
	case ActionOnVariable::Behavior_t::pulse:
		return tr("Pulse");
		break;
	case ActionOnVariable::Behavior_t::continuous:
		return tr("Active on state");
		break;
	case ActionOnVariable::Behavior_t::none:
		return QString();
		break;
	}
}

QIcon ActionOnVariable::getBehaviorIcon(ActionOnVariable::Behavior_t behavior, bool isDown)
{
	switch (behavior)
	{
	case ActionOnVariable::Behavior_t::assign:
		return QIcon(QString(":/icons/assign"));
		break;
	case ActionOnVariable::Behavior_t::pulse:
		if (isDown == false)
		{
			return QIcon(QString(":/icons/pulse"));
		}
		else
		{
			return QIcon(QString(":/icons/pulse_down"));
		}
		break;
	case ActionOnVariable::Behavior_t::continuous:
		if (isDown == false)
		{
			return QIcon(QString(":/icons/active_on_state"));
		}
		else
		{
			return QIcon(QString(":/icons/active_on_state_down"));
		}
		break;
	case ActionOnVariable::Behavior_t::none:
		return QIcon();
		break;
	}
}

//
// Class object definition
//

ActionOnVariable::ActionOnVariable(ComponentId targetVariableId, uint actuatorAllowedBehaviors) :
    targetVariableId         {targetVariableId},
    actuatorAllowedBehaviors {actuatorAllowedBehaviors},
    behavior                 {ActionOnVariable::Behavior_t::none},
    valueProvider            {nullptr},
    condition                {nullptr},
    rangeL                   {-1},
    rangeR                   {-1}
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto targetVariable = machine->getVariable(targetVariableId);
	if (targetVariable == nullptr) return;


	// Assign a default behavior
	this->checkAndFixBehavior();

	// Provide a default value provider
	this->setDefaultValueProvider();

	this->connectTargetVariableSignals(targetVariable);
}

ActionOnVariable::ActionOnVariable(std::shared_ptr<Variable> targetVariable, uint actuatorAllowedBehaviors, ActionOnVariable::Behavior_t behavior, int rangeL, int rangeR) :
    targetVariableId         {targetVariable != nullptr ? targetVariable->getId() : nullId},
    actuatorAllowedBehaviors {actuatorAllowedBehaviors},
    behavior                 {behavior},
    valueProvider            {nullptr},
    condition                {nullptr},
    rangeL                   {rangeL},
    rangeR                   {rangeR}
{
	// Perform absolutely no checks on provided values:
	// we are loading a file, these checks will be performed later.

	this->connectTargetVariableSignals(targetVariable);
}

void ActionOnVariable::setValueProviderRaw(std::shared_ptr<Equation> newValueProvider)
{
	this->valueProvider = newValueProvider;
	this->connectValueProviderSignals();
}

void ActionOnVariable::setConditionRaw(std::shared_ptr<Equation> newCondition)
{
	this->condition = newCondition;
	this->connectConditionSignals();
}

/**
 * @brief ActionOnVariable::checkAndFixAction is only called
 *        after a load from a save file, to make sure loaded
 *        values are coherent.
 */
void ActionOnVariable::checkAndFixAction()
{
	this->checkAndFixRange();
	this->checkAndFixBehavior();
	this->checkAndFixValueProvider();

	this->valueProvider->doFullStackRecomputation();

	if (this->condition != nullptr)
	{
		this->condition->doFullStackRecomputation();
	}
}

void ActionOnVariable::setBehavior(ActionOnVariable::Behavior_t newBehavior)
{
	if (newBehavior == this->behavior) return;

	if ( (this->getAllowedBehaviors() & static_cast<uint>(newBehavior)) == 0) return;


	this->behavior = newBehavior;

	emit this->actionChangedEvent();
}

void ActionOnVariable::setValueProvider(std::shared_ptr<Equation> newValueProvider)
{
	if (newValueProvider == nullptr)
	{
		// Setting a null action value resets
		// it to a default constant.
		this->setDefaultValueProvider();
		// No need to connect a default value provider:
		// it only contains a constant.
	}
	else
	{
		this->valueProvider = newValueProvider;

		// Avoid obviously invalid equations, and
		// adapt between types that are compatible
		this->checkAndFixValueProvider();

		this->connectValueProviderSignals();
	}

	emit this->actionChangedEvent();
}

void ActionOnVariable::setCondition(std::shared_ptr<Equation> newCondition)
{
	this->condition = newCondition;
	this->connectConditionSignals();

	emit this->actionChangedEvent();
}

void ActionOnVariable::setRange(int newRangeL, int newRangeR)
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return;

	if (targetVariable->getType() != MachineValue::Type_t::bitVector) return;

	if ( (newRangeL == this->rangeL) && (newRangeR == this->rangeR) ) return;

	if (this->checkIfRangeFitsTargetVariable(newRangeL, newRangeR) == false) return;


	this->rangeL = newRangeL;
	this->rangeR = newRangeR;

	this->checkAndFixValueProvider();

	emit this->actionChangedEvent();
}

ComponentId ActionOnVariable::getTargetVariableId() const
{
	return this->targetVariableId;
}

ActionOnVariable::Behavior_t ActionOnVariable::getBehavior() const
{
	return this->behavior;
}

std::shared_ptr<const Equation> ActionOnVariable::getValueProvider() const
{
	return this->valueProvider;
}

std::shared_ptr<const Equation> ActionOnVariable::getCondition() const
{
	return this->condition;
}

int ActionOnVariable::getRangeL() const
{
	return this->rangeL;
}

int ActionOnVariable::getRangeR() const
{
	return this->rangeR;
}

uint ActionOnVariable::getAllowedBehaviors() const
{
	uint allowedBehaviors = static_cast<uint>(ActionOnVariable::Behavior_t::none);

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return allowedBehaviors;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return allowedBehaviors;


	// First add base behaviors whether they are allowed or not
	if (targetVariable->getMemorized() == true)
	{
		allowedBehaviors |= static_cast<uint>(ActionOnVariable::Behavior_t::assign);
	}
	else // (targetVariable->getMemorized() == false)
	{
		allowedBehaviors |= static_cast<uint>(ActionOnVariable::Behavior_t::pulse);
		allowedBehaviors |= static_cast<uint>(ActionOnVariable::Behavior_t::continuous);
	}

	// Then filter for actually allowed behaviors
	allowedBehaviors &= this->actuatorAllowedBehaviors;

	return allowedBehaviors;
}

bool ActionOnVariable::isValueProviderValid() const
{
	if (this->valueProvider == nullptr) return false;

	auto providedValue = this->valueProvider->getInitialValue();
	if (providedValue.isNull() == true) return false;


	if (providedValue.getType() != this->getExpectedProvidedValueType())
	{
		return false;
	}
	else // (providedValue.getType() == this->getExpectedProvidedValueType())
	{
		if (providedValue.getType() != MachineValue::Type_t::bitVector)
		{
			return true;
		}
		else // (providedValue.getType() == MachineValue::Type_t::bitVector)
		{
			return providedValue.getBitVectorValue().getSize() == this->getExpectedProvidedValueBitVectorSize();
		}
	}
}

MachineValue::Type_t ActionOnVariable::getExpectedProvidedValueType() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return MachineValue::Type_t::nullType;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return MachineValue::Type_t::nullType;


	if (targetVariable->getType() != MachineValue::Type_t::bitVector)
	{
		return targetVariable->getType();
	}
	else // (targetVariable->getType() == MachineValue::Type_t::bitVector)
	{
		// Provided value should be Boolean for 1-bit actions, Bit Vector otherwise
		if (this->getExpectedProvidedValueBitVectorSize() == 1)
		{
			return MachineValue::Type_t::boolean;
		}
		else // (this->getExpectedProvidedValueBitVectorSize() > 1)
		{
			return MachineValue::Type_t::bitVector;
		}
	}
}

uint ActionOnVariable::getExpectedProvidedValueBitVectorSize() const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return 0;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return 0;

	if (targetVariable->getType() != MachineValue::Type_t::bitVector) return 0;


	uint variableSize = targetVariable->getInitialValue().getBitVectorValue().getSize();

	if (variableSize == 1)
	{
		return 1;
	}
	else // (variableSize != 1)
	{
		if (this->rangeL < 0)
		{
			return variableSize;
		}
		else if (this->rangeR < 0)
		{
			return 1;
		}
		else // ( (this->rangeL >= 0) && (this->rangeR >= 0) )
		{
			return static_cast<uint>(this->rangeL - this->rangeR + 1);
		}
	}
}

void ActionOnVariable::targetVariableTypeChangedEventHandler()
{
	this->checkAndFixRange();
	this->checkAndFixValueProvider();

	// Just always emit the signal as even if values
	// were not fixed, the type change invalidated
	// any value provider that was previously correct
	emit this->actionChangedAsAResultOfAnotherChangeEvent();
}

void ActionOnVariable::targetVariableMemorizedStateChangedEventHandler()
{
	auto previousBehavior = this->behavior;

	this->checkAndFixBehavior();

	if (previousBehavior != this->behavior)
	{
		emit this->actionChangedAsAResultOfAnotherChangeEvent();
	}
}

void ActionOnVariable::targetVariableInitialValueChangedEventHandler()
{
	// Make sure we always have a value
	if (this->valueProvider == nullptr)
	{
		this->setDefaultValueProvider();
		return;
	}

	// In a very specific case, we will want to update
	// the action value:
	// - The target variable is not memorized
	// - The action provides a boolean
	// - The current value provider is a constant
	// Then the provided value should be inverted.

	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return;


	if (targetVariable->getMemorized() == true) return;

	if (this->getExpectedProvidedValueType() != MachineValue::Type_t::boolean) return;

	if (this->isValueProviderAConstant() == false) return;

	// PRE: expected provided value type is boolean,
	// i.e. variable type is either a boolean or a
	// bit vector of which we extract a single bit.

	auto targetVariableValue = targetVariable->getInitialValue();
	if (targetVariableValue.getType() == MachineValue::Type_t::boolean)
	{
		this->valueProvider->setOperand(0, !(targetVariableValue.getBooleanValue()));
	}
	else // (targetVariableValue.getType() == MachineValue::Type_t::bitVector)
	{
		auto bitVectorValue = targetVariableValue.getBitVectorValue();
		this->valueProvider->setOperand(0, !(bitVectorValue[this->rangeL]));
	}

	emit this->actionChangedAsAResultOfAnotherChangeEvent();
}

void ActionOnVariable::valueProviderEquationInvalidatedEventHandler()
{
	this->setDefaultValueProvider();

	emit this->actionChangedAsAResultOfAnotherChangeEvent();
}

void ActionOnVariable::conditionEquationInvalidatedEventHandler()
{
	this->condition = nullptr;

	emit this->actionChangedAsAResultOfAnotherChangeEvent();
}

void ActionOnVariable::connectTargetVariableSignals(std::shared_ptr<Variable> targetVariable)
{
	if (targetVariable == nullptr) return;


	connect(targetVariable.get(), &Variable::variableTypeChangedEvent,           this, &ActionOnVariable::targetVariableTypeChangedEventHandler);
	connect(targetVariable.get(), &Variable::variableMemorizedStateChangedEvent, this, &ActionOnVariable::targetVariableMemorizedStateChangedEventHandler);
	connect(targetVariable.get(), &Variable::variableInitialValueChangedEvent,   this, &ActionOnVariable::targetVariableInitialValueChangedEventHandler);

	// Renaming the variable doesn't actually change the action in any way,
	// but it changes how it is displayed: trigger an actionChangedEvent
	connect(targetVariable.get(), &Variable::variableRenamedEvent, this, &ActionOnVariable::actionChangedEvent);

	// Action being fixed results in an action change
	connect(this, &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent, this, &ActionOnVariable::actionChangedEvent);
}

void ActionOnVariable::connectValueProviderSignals()
{
	if (this->valueProvider == nullptr) return;


	connect(this->valueProvider.get(), &Equation::equationInitialValueChangedEvent, this, &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent);
	connect(this->valueProvider.get(), &Equation::equationTextChangedEvent,         this, &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent);
	connect(this->valueProvider.get(), &Equation::equationInvalidatedEvent,         this, &ActionOnVariable::valueProviderEquationInvalidatedEventHandler);
}

void ActionOnVariable::connectConditionSignals()
{
	if (this->condition == nullptr) return;


	connect(this->condition.get(), &Equation::equationInitialValueChangedEvent, this, &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent);
	connect(this->condition.get(), &Equation::equationTextChangedEvent,         this, &ActionOnVariable::actionChangedAsAResultOfAnotherChangeEvent);
	connect(this->condition.get(), &Equation::equationInvalidatedEvent,         this, &ActionOnVariable::conditionEquationInvalidatedEventHandler);
}

bool ActionOnVariable::checkIfRangeFitsTargetVariable(int rangeL, int rangeR) const
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return false;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return false;

	if (targetVariable->getType() != MachineValue::Type_t::bitVector) return false;


	if ( (rangeL < 0 ) && (rangeR < 0) )
	{
		// Full range action: always allowed
		return true;
	}
	else if ( (rangeL >= 0) && (rangeR < 0) )
	{
		// Single-bit action: check if in range
		int variableSize = static_cast<int>(targetVariable->getInitialValue().getBitVectorValue().getSize());
		if (rangeL < variableSize)
		{
			return true;
		}
	}
	else // ( (rangeL >= 0) && (rangeR >= 0) )
	{
		// Sub-range action: check if range order is correct
		if (rangeL > rangeR)
		{
			// PRE: both parameters are positive, and their order is correct.
			// Check if left side is in range.
			int variableSize = static_cast<int>(targetVariable->getInitialValue().getBitVectorValue().getSize());
			if (rangeL < variableSize)
			{
				return true;
			}
		}
	}

	return false;
}

void ActionOnVariable::checkAndFixRange()
{
	auto machine = machineManager->getMachine();
	if (machine == nullptr) return;

	auto targetVariable = machine->getVariable(this->targetVariableId);
	if (targetVariable == nullptr) return;

	if (targetVariable->getType() != MachineValue::Type_t::bitVector)
	{
		this->rangeL = -1;
		this->rangeR = -1;
		return;
	}


	int variableSize = static_cast<int>(targetVariable->getInitialValue().getBitVectorValue().getSize());

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

void ActionOnVariable::checkAndFixBehavior()
{
	// If current behavior is allowed, nothing to fix
	if ( (static_cast<uint>(this->behavior) & this->getAllowedBehaviors()) != 0) return;


	uint allowedBehaviors = this->getAllowedBehaviors();
	if ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::continuous)) != 0)
	{
		this->behavior = ActionOnVariable::Behavior_t::continuous;
	}
	else if ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::pulse)) != 0)
	{
		this->behavior = ActionOnVariable::Behavior_t::pulse;
	}
	else // ((allowedBehaviors & static_cast<uint>(ActionOnVariable::Behavior_t::assign)) != 0)
	{
		this->behavior = ActionOnVariable::Behavior_t::assign;
	}
}

void ActionOnVariable::checkAndFixValueProvider()
{
	if (this->isValueProviderValid() == true) return;


	// First check for obviously invalid value providers
	if (this->valueProvider == nullptr)
	{
		this->setDefaultValueProvider();
		return;
	}
	else // (this->valueProvider != nullptr)
	{
		if (this->valueProvider->getOperator() == Equation::Operator_t::identity)
		{
			if (this->valueProvider->getOperand(0) == nullptr)
			{
				this->setDefaultValueProvider();
				return;
			}
		}
	}

	// Other than obviously invalid, only try to fix constant value providers
	if (this->isValueProviderAConstant() == false) return;


	// Try to fix value provider if possible
	switch (this->getExpectedProvidedValueType())
	{
	case MachineValue::Type_t::boolean:
	{
		// If the expected type is a Boolean and we
		// have a Bit Vector constant as value
		// provider, just resize the constant.
		auto oldValue = this->valueProvider->getInitialValue();
		if (oldValue.getType() == MachineValue::Type_t::bitVector)
		{
			auto bitVectorValue = oldValue.getBitVectorValue();
			bitVectorValue.resize(1);

			this->valueProvider->setOperand(0, bitVectorValue[0]);
		}
		break;
	}
	case MachineValue::Type_t::bitVector:
	{
		// If the expected type is a Bit Vector and we
		// already have a Boolean orBit Vector constant
		// as value provider, just resize the constant.
		auto oldValue = this->valueProvider->getInitialValue();
		if (oldValue.getType() == MachineValue::Type_t::bitVector)
		{
			auto bitVectorValue = oldValue.getBitVectorValue();
			bitVectorValue.resize(this->getExpectedProvidedValueBitVectorSize());

			this->valueProvider->setOperand(0, bitVectorValue);
		}
		else if (oldValue.getType() == MachineValue::Type_t::boolean)
		{
			auto bitVectorValue = BitVectorValue{oldValue.getBooleanValue()};
			bitVectorValue.resize(this->getExpectedProvidedValueBitVectorSize());

			this->valueProvider->setOperand(0, bitVectorValue);
		}
		break;
	}
	case MachineValue::Type_t::natural:
	{
		auto oldValue = this->valueProvider->getInitialValue();
		if (oldValue.getType() == MachineValue::Type_t::relative)
		{
			auto relativeValue = oldValue.getRelativeValue();
			if (relativeValue >= 0)
			{
				auto naturalValue = NaturalValue{static_cast<unsigned int>(relativeValue)};
				this->valueProvider->setOperand(0, naturalValue);
			}
		}
		break;
	}
	case MachineValue::Type_t::relative:
	{
		auto oldValue = this->valueProvider->getInitialValue();
		if (oldValue.getType() == MachineValue::Type_t::natural)
		{
			auto naturalValue = oldValue.getNaturalValue();
			auto relativeValue = RelativeValue{static_cast<int>(naturalValue)};
			this->valueProvider->setOperand(0, relativeValue);
		}
		break;
	}
	case MachineValue::Type_t::nullType:
		break;
	}
}

/**
 * @brief ActionOnVariable::setDefaultValueProvider
 *        overrides any existing value provider with
 *        a default constant-based value provider.
 *
 *        For booleans, we have a quite sensible way of
 *        providing a default value for the action: provide
 *        the opposite of the initial value.
 *        For other types, just use 0.
 */
void ActionOnVariable::setDefaultValueProvider()
{
	MachineValue constantValue{};
	switch (this->getExpectedProvidedValueType())
	{
	case MachineValue::Type_t::boolean:
	{
		// PRE: Expected action type being boolean means
		// that variable type is either a boolean or a
		// bit vector of which we extract a single bit.

		auto machine = machineManager->getMachine();
		if (machine == nullptr) break;

		auto targetVariable = machine->getVariable(this->targetVariableId);
		if (targetVariable == nullptr) break;


		auto variableValue = targetVariable->getInitialValue();
		if (targetVariable->getType() == MachineValue::Type_t::boolean)
		{
			constantValue = !variableValue.getBooleanValue();
		}
		else // (targetVariable->getType() == MachineValue::Type_t::bitVector)
		{
			// Get the opposite of the bit acted on
			auto bitVectorValue = variableValue.getBitVectorValue();
			constantValue = !(bitVectorValue[this->rangeL]);
		}
		break;
	}
	case MachineValue::Type_t::bitVector:
		constantValue = BitVectorValue::allZeros(this->getExpectedProvidedValueBitVectorSize());
		break;
	case MachineValue::Type_t::natural:
		constantValue = NaturalValue{0};
		break;
	case MachineValue::Type_t::relative:
		constantValue = RelativeValue{0};
		break;
	case MachineValue::Type_t::nullType:
		break;
	}

	if (constantValue.isNull() == true)
	{
		// In case something went wrong, default to False
		constantValue = BooleanValue::falseValue();
	}

	this->valueProvider = std::make_shared<Equation>(Equation::Operator_t::identity);
	this->valueProvider->setOperand(0, constantValue);
}

bool ActionOnVariable::isValueProviderAConstant() const
{
	if (this->valueProvider == nullptr) return false;


	if (this->valueProvider->getOperator() == Equation::Operator_t::identity)
	{
		auto operand = this->valueProvider->getOperand(0);
		if (operand == nullptr) return false;


		return (operand->getSource() == Operand::Source_t::constant);
	}
	else // (this->valueProvider->getOperator() != Equation::Operator_t::identity)
	{
		return false;
	}
}
