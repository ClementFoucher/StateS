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
#include "statesexception.h"
#include "simulatedmachine.h"
#include "simulatedvariable.h"
#include "simulatedequation.h"


SimulatedActionOnVariable::SimulatedActionOnVariable(std::shared_ptr<const ActionOnVariable> sourceAction) // Throws StatesException
{
	// Do not build a simulated action for an invalid action
	if (sourceAction == nullptr)
	{
		throw StatesException("SimulatedActionOnVariable", StatesException::Cause_t::sub_component_build_skipped, tr("Invalid action."));
	}
	if (sourceAction->isValueProviderValid() == false)
	{
		throw StatesException("SimulatedActionOnVariable", StatesException::Cause_t::sub_component_build_skipped, tr("Invalid action."));
	}

	this->targetVariableId = sourceAction->getTargetVariableId();
	this->behavior         = sourceAction->getBehavior();
	this->valueProvider    = std::make_shared<SimulatedEquation>(sourceAction->getValueProvider()); // Throws StatesException: ignored, we checked that the action is valid
	this->rangeL           = sourceAction->getRangeL();
	this->rangeR           = sourceAction->getRangeR();

	auto sourceCondition = sourceAction->getCondition();
	if (sourceCondition != nullptr)
	{
		try
		{
			this->condition = std::make_shared<SimulatedEquation>(sourceCondition); // Throws StatesException
		}
		catch (const StatesException& e)
		{
			if (e.getCause() != StatesException::Cause_t::ill_formed_equation) throw;

			// Consider ill-formed condition always false
			auto falseCondition = std::make_shared<Equation>(Equation::Operator_t::identity);
			falseCondition->setOperand(0, BooleanValue::falseValue());

			this->condition = std::make_shared<SimulatedEquation>(falseCondition); // Throws StatesException: Ignored, equation is well-formed.
		}
	}
}

void SimulatedActionOnVariable::doAction()
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return;

	auto simulatedTargetVariable = simulatedMachine->getSimulatedVariable(this->targetVariableId);
	if (simulatedTargetVariable == nullptr) return;


	/////
	// If there is a condition, check that it is true

	if (this->condition != nullptr)
	{
		if (this->condition->isTrue() == false)
		{
			return;
		}
	}

	/////
	// Update target variable value

	MachineValue newValue{};

	if ( (simulatedTargetVariable->getType() == MachineValue::Type_t::bitVector) && (this->rangeL >= 0) )
	{
		// Actions on part of a bit vector can be on one bit or on a sub-range
		auto actionProvidedValue = this->valueProvider->getCurrentValue();

		auto bitVectorNewValue = simulatedTargetVariable->getCurrentValue().getBitVectorValue();
		if (actionProvidedValue.getType() == MachineValue::Type_t::boolean)
		{
			bitVectorNewValue.setBit(this->rangeL, actionProvidedValue.getBooleanValue());
		}
		else // (newValue.getType() == MachineValue::Type_t::bitVector)
		{
			bitVectorNewValue.setSubrange(this->rangeL, this->rangeR, actionProvidedValue.getBitVectorValue());
		}

		newValue = bitVectorNewValue;
	}
	else
	{
		newValue = this->valueProvider->getCurrentValue();
	}

	simulatedTargetVariable->setCurrentValue(newValue);
}

bool SimulatedActionOnVariable::isActionMemorized() const
{
	auto simulatedMachine = machineManager->getSimulatedMachine();
	if (simulatedMachine == nullptr) return false;

	auto simulatedTargetVariable = simulatedMachine->getSimulatedVariable(this->targetVariableId);
	if (simulatedTargetVariable == nullptr) return false;


	return simulatedTargetVariable->getMemorized();
}

ComponentId SimulatedActionOnVariable::getVariableId() const
{
	return this->targetVariableId;
}
