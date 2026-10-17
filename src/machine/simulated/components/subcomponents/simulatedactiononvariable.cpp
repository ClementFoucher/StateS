/*
 * Copyright © 2025-2026 Clément Foucher
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
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */

// Current class header
#include "simulatedactiononvariable.h"

// StateS
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "actiononvariable.h"
#include "simulatedvariable.h"


SimulatedActionOnVariable::SimulatedActionOnVariable(std::shared_ptr<const ActionOnVariable> sourceAction)
{
	this->variableId  = sourceAction->getVariableActedOnId();
	this->actionType  = sourceAction->getActionType();
	this->actionValue = sourceAction->getActionValue();
	this->rangeL      = sourceAction->getActionRangeL();
	this->rangeR      = sourceAction->getActionRangeR();
}

void SimulatedActionOnVariable::doAction()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return;


	if ( (simulatedVariable->getType() == MachineValue::Type_t::bitVector) && (this->rangeL >= 0) )
	{
		// Actions on part of a bit vector can be one-bit (boolean) or act on a sub-range

		auto variableCurrentValue = simulatedVariable->getCurrentValue().getBitVectorValue();

		if (actionValue.getType() == MachineValue::Type_t::boolean)
		{
			variableCurrentValue.setBit(this->rangeL, actionValue.getBooleanValue());
		}
		else // (actionValue.getType() == MachineValue::Type_t::bitVector)
		{
			variableCurrentValue.setSubrange(this->rangeL, this->rangeR, actionValue.getBitVectorValue());
		}

		simulatedVariable->setCurrentValue(variableCurrentValue);
	}
	else
	{
		simulatedVariable->setCurrentValue(this->getActionValue());
	}
}

bool SimulatedActionOnVariable::isActionMemorized() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return false;

	auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
	if (simulatedVariable == nullptr) return false;


	return simulatedVariable->getMemorized();
}

ComponentId SimulatedActionOnVariable::getVariableId() const
{
	return this->variableId;
}

MachineValue SimulatedActionOnVariable::getActionValue() const
{
	switch (this->actionType)
	{
	case ActionOnVariable::Type_t::reset:
	case ActionOnVariable::Type_t::set:
	case ActionOnVariable::Type_t::continuous:
	case ActionOnVariable::Type_t::pulse:
	case ActionOnVariable::Type_t::assign:
		return this->actionValue;
		break;
	case ActionOnVariable::Type_t::increment:
	case ActionOnVariable::Type_t::decrement:
	{
		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return MachineValue{};

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
		if (simulatedVariable == nullptr) return MachineValue{};

		if ( (simulatedVariable->getType() != MachineValue::Type_t::natural) &&
		     (simulatedVariable->getType() != MachineValue::Type_t::relative)
		   )
			return MachineValue{};


		auto simulatedVariableValue = simulatedVariable->getCurrentValue();
		if (this->actionType == ActionOnVariable::Type_t::increment)
		{
			if (simulatedVariable->getType() == MachineValue::Type_t::natural)
			{
				simulatedVariableValue = simulatedVariableValue + NaturalValue{1};
			}
			else // (simulatedVariable->getType() == MachineValue::Type_t::relative)
			{
				simulatedVariableValue = simulatedVariableValue + RelativeValue{1};
			}
		}
		else // (this->actionType == ActionOnVariable::Type_t::decrement)
		{
			if (simulatedVariable->getType() == MachineValue::Type_t::natural)
			{
				simulatedVariableValue = simulatedVariableValue - NaturalValue{1};
			}
			else // (simulatedVariable->getType() == MachineValue::Type_t::relative)
			{
				simulatedVariableValue = simulatedVariableValue - RelativeValue{1};
			}
		}
		return simulatedVariableValue;
		break;
	}
	case ActionOnVariable::Type_t::none:
		return MachineValue{};
		break;
	}
}
