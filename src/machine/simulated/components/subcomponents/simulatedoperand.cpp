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
#include "simulatedoperand.h"

// StateS
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "simulatedequation.h"
#include "simulatedvariable.h"


SimulatedOperand::SimulatedOperand(std::shared_ptr<const Operand> sourceOperand)
{
	if (sourceOperand == nullptr) return;


	this->source = sourceOperand->getSource();

	switch (this->source)
	{
	case Operand::Source_t::variable:
	{
		this->value = sourceOperand->getVariableId();

		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return;

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(sourceOperand->getVariableId());
		if (simulatedVariable == nullptr) return;


		connect(simulatedVariable.get(), &SimulatedVariable::variableCurrentValueChangedEvent, this, &SimulatedOperand::operandCurrentValueChangedEvent);
		break;
	}
	case Operand::Source_t::equation:
	{
		auto equation = make_shared<SimulatedEquation>(sourceOperand->getEquation());
		this->value = equation;

		connect(equation.get(), &SimulatedEquation::equationCurrentValueChangedEvent, this, &SimulatedOperand::operandCurrentValueChangedEvent);
		break;
	}
	case Operand::Source_t::constant:
		this->value = sourceOperand->getConstant();
		break;
	}
}

MachineValue SimulatedOperand::getCurrentValue() const
{
	switch (this->source)
	{
	case Operand::Source_t::variable:
	{
		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return MachineValue{};

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(std::get<ComponentId>(this->value));
		if (simulatedVariable == nullptr) return MachineValue{};


		return simulatedVariable->getCurrentValue();
		break;
	}
	case Operand::Source_t::equation:
	{
		auto equation = std::get<std::shared_ptr<SimulatedEquation>>(this->value);
		if (equation == nullptr) return MachineValue{};


		return equation->getCurrentValue();
		break;
	}
	case Operand::Source_t::constant:
		return std::get<MachineValue>(this->value);
		break;
	}
}

MachineValue::Type_t SimulatedOperand::getType() const
{
	return this->getCurrentValue().getType();
}
