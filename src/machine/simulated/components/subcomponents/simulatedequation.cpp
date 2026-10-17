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
#include "simulatedequation.h"

// StateS
#include "simulatedoperand.h"
#include "equationcomputer.h"


SimulatedEquation::SimulatedEquation(std::shared_ptr<const Equation> sourceEquation)
{
	if (sourceEquation == nullptr) return;


	this->operatorType = sourceEquation->getOperator();

	bool isValid = (sourceEquation->getComputationFailureCause() == Equation::ComputationFailureCause_t::nofail) ? true : false;

	for (uint i = 0 ; i < sourceEquation->getOperandCount() ; i++)
	{
		auto logicOperand = sourceEquation->getOperand(i);
		if (logicOperand == nullptr) continue;


		auto simulatedOperand = make_shared<SimulatedOperand>(sourceEquation->getOperand(i));

		if (isValid == true)
		{
			connect(simulatedOperand.get(), &SimulatedOperand::operandCurrentValueChangedEvent, this, &SimulatedEquation::computeCurrentValue);
		}

		this->operands.append(simulatedOperand);
	}

	this->currentValue = sourceEquation->getInitialValue();
}

MachineValue SimulatedEquation::getCurrentValue() const
{
	return this->currentValue;
}

bool SimulatedEquation::isTrue() const
{
	if (this->currentValue.getType() == MachineValue::Type_t::boolean)
	{
		return this->currentValue.getBooleanValue();
	}
	else if (this->currentValue.getType() == MachineValue::Type_t::bitVector)
	{
		// Only one-bit vectors can be considered true
		if (this->currentValue.getBitVectorValue() == BitVectorValue::allOnes(1))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else // "true" concept doesn't apply to other types
	{
		return false;
	}
}

void SimulatedEquation::computeCurrentValue()
{
	// PRE: this slot is connected (thus called) only if the equation is valid
	MachineValue previousValue{this->currentValue};

	int operandsCount = this->operands.count();
	std::vector<MachineValue> operandsValues{static_cast<size_t>(operandsCount)};

	for (int i = 0 ; i < operandsCount ; i++)
	{
		operandsValues[i] = this->operands[i]->getCurrentValue();
	}

	this->currentValue = EquationComputer::computeValue(this->operatorType, operandsValues);

	if (previousValue != this->currentValue)
	{
		emit this->equationCurrentValueChangedEvent();
	}
}
