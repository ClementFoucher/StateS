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
#include "simulatedequation.h"

// StateS classes
#include "equation.h"
#include "simulatedoperand.h"


SimulatedEquation::SimulatedEquation(shared_ptr<const Equation> sourceEquation)
{
	if (sourceEquation == nullptr) return;


	this->operatorType = sourceEquation->getOperatorType();
	this->currentValue = LogicValue::getNullValue();
	if (this->operatorType == OperatorType_t::extractOp)
	{
		this->rangeL = sourceEquation->getRangeL();
		this->rangeR = sourceEquation->getRangeR();
	}
	if (sourceEquation->getComputationFailureCause() != EquationComputationFailureCause_t::nofail)
	{
		this->isValid = false;
	}

	for (uint i = 0 ; i < sourceEquation->getOperandCount() ; i++)
	{
		auto logicOperand = sourceEquation->getOperand(i);
		if (logicOperand == nullptr) continue;


		auto simulatedOperand = shared_ptr<SimulatedOperand>(new SimulatedOperand(sourceEquation->getOperand(i)));

		if (this->isValid == true)
		{
			connect(simulatedOperand.get(), &SimulatedOperand::operandCurrentValueChangedEvent, this, &SimulatedEquation::computeCurrentValue);
		}

		this->operands.append(simulatedOperand);
	}

	if (this->isValid == true)
	{
		this->computeCurrentValue();
	}
}

LogicValue SimulatedEquation::getCurrentValue() const
{
	return this->currentValue;
}

// True concept here only apply to one bit results
bool SimulatedEquation::isTrue() const
{
	if (this->currentValue.getSize() == 1)
	{
		if (this->currentValue == LogicValue::getValue1(1))
		{
			return true;
		}
	}

	return false;
}

void SimulatedEquation::computeCurrentValue()
{
	LogicValue previousValue = this->currentValue;

	LogicValue computedValue;
	switch (this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
		computedValue = this->operands[0]->getCurrentValue();
		break;
	case OperatorType_t::equalOp:
	{
		LogicValue oneBitResult(1);
		oneBitResult[0] = ((this->operands[0]->getCurrentValue() == this->operands[1]->getCurrentValue()));

		computedValue = oneBitResult;
	}
	break;
	case OperatorType_t::diffOp:
	{
		LogicValue oneBitResult(1);
		oneBitResult[0] = ((this->operands[0]->getCurrentValue() != this->operands[1]->getCurrentValue()));

		computedValue = oneBitResult;
	}
	break;
	case OperatorType_t::extractOp:
		if (this->rangeR != -1)
		{
			int range = this->rangeL - this->rangeR + 1;
			LogicValue subVector(range);
			LogicValue originalValue = this->operands[0]->getCurrentValue();

			for (int i = 0 ; i < range ; i++)
			{
				subVector[i] = originalValue[this->rangeR + i];
			}

			computedValue = subVector;
		}
		else
		{
			auto operand = this->operands[0];
			LogicValue operandValue = operand->getCurrentValue();

			LogicValue result(1);
			result[0] = operandValue[rangeL];

			computedValue = result;
		}
		break;
	case OperatorType_t::concatOp:
	{
		int sizeCount = 0;
		for (auto& currentOperand : this->operands)
		{
			LogicValue currentOperandValue = currentOperand->getCurrentValue();

			sizeCount += currentOperandValue.getSize();
		}

		LogicValue concatVector(sizeCount);

		int currentBit = sizeCount - 1;
		for (auto& currentOperand : this->operands)
		{
			LogicValue currentOperandValue;
			currentOperandValue = currentOperand->getCurrentValue();

			for (int i = currentOperandValue.getSize()-1 ; i >= 0 ; i--)
			{
				concatVector[currentBit] = currentOperand->getCurrentValue()[i];
				currentBit--;
			}
		}

		computedValue = concatVector;
	}
	break;
	case OperatorType_t::andOp:
	case OperatorType_t::nandOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize = operand->getCurrentValue().getSize();

		LogicValue partialResult(operandsSize, true);
		for (auto& operand : this->operands)
		{
			partialResult &= operand->getCurrentValue();
		}

		computedValue = partialResult;
	}
	break;

	case OperatorType_t::orOp:
	case OperatorType_t::norOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize = operand->getCurrentValue().getSize();

		LogicValue partialResult(operandsSize);
		for (auto& operand : this->operands)
		{
			partialResult |= operand->getCurrentValue();
		}

		computedValue = partialResult;
	}
	break;

	case OperatorType_t::xorOp:
	case OperatorType_t::xnorOp:
	{
		auto operand = this->getOperand(0);
		uint operandsSize = operand->getCurrentValue().getSize();

		LogicValue partialResult(operandsSize);
		for (auto& operand : this->operands)
		{
			partialResult ^= operand->getCurrentValue();
		}

		computedValue = partialResult;
	}
	break;
	}

	if (this->isInverted())
	{
		computedValue = !computedValue;
	}

	this->currentValue = computedValue;

	if (previousValue != this->currentValue)
	{
		emit this->equationCurrentValueChangedEvent();
	}
}

bool SimulatedEquation::isInverted() const
{
	switch (this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
		return true;
		break;
	case OperatorType_t::identity:
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
	case OperatorType_t::extractOp:
	case OperatorType_t::concatOp:
		return false;
		break;
	}
}

shared_ptr<SimulatedOperand> SimulatedEquation::getOperand(uint i) const
{
	if (i < this->getOperandCount())
	{
		return this->operands.at(i);
	}
	else
	{
		return nullptr;
	}
}

uint SimulatedEquation::getOperandCount() const
{
	switch(this->operatorType)
	{
	case OperatorType_t::notOp:
	case OperatorType_t::identity:
	case OperatorType_t::extractOp:
		return 1;
		break;
	case OperatorType_t::equalOp:
	case OperatorType_t::diffOp:
		return 2;
		break;
	case OperatorType_t::andOp:
	case OperatorType_t::orOp:
	case OperatorType_t::xorOp:
	case OperatorType_t::nandOp:
	case OperatorType_t::norOp:
	case OperatorType_t::xnorOp:
	case OperatorType_t::concatOp:
		return this->operands.size();
		break;
	}
}
