/*
 * Copyright © 2025 Clément Foucher
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

// StateS classes
#include "machinemanager.h"
#include "simulatedmachine.h"
#include "operand.h"
#include "simulatedequation.h"
#include "simulatedvariable.h"


SimulatedOperand::SimulatedOperand(shared_ptr<const Operand> sourceOperand)
{
	if (sourceOperand == nullptr) return;


	this->source = sourceOperand->getSource();

	switch (this->source)
	{
	case OperandSource_t::variable:
	{
		this->variableId = sourceOperand->getVariableId();

		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return;

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
		if (simulatedVariable == nullptr) return;


		connect(simulatedVariable.get(), &SimulatedVariable::variableCurrentValueChangedEvent, this, &SimulatedOperand::operandCurrentValueChangedEvent);
		break;
	}
	case OperandSource_t::equation:
		this->equation = make_shared<SimulatedEquation>(sourceOperand->getEquation());

		connect(equation.get(), &SimulatedEquation::equationCurrentValueChangedEvent, this, &SimulatedOperand::operandCurrentValueChangedEvent);
		break;
	case OperandSource_t::constant:
		this->constant = sourceOperand->getConstant();
		break;
	}
}

LogicValue SimulatedOperand::getCurrentValue() const
{
	switch (this->source)
	{
	case OperandSource_t::variable:
	{
		auto simulatedMachine = machineManager->getSimulatedMachine();
		if (simulatedMachine == nullptr) return LogicValue::getNullValue();

		auto simulatedVariable = simulatedMachine->getSimulatedVariable(this->variableId);
		if (simulatedVariable == nullptr) return LogicValue::getNullValue();


		return simulatedVariable->getCurrentValue();
		break;
	}
	case OperandSource_t::equation:
		if (this->equation == nullptr) return LogicValue::getNullValue();


		return this->equation->getCurrentValue();
		break;
	case OperandSource_t::constant:
		return this->constant;
		break;
	}
}
