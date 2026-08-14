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


SimulatedEquation::SimulatedEquation(std::shared_ptr<const Equation> sourceEquation)
{
	if (sourceEquation == nullptr) return;


	this->operatorType = sourceEquation->getOperator();
	if (this->operatorType == Equation::Operator_t::extractOp)
	{
		this->rangeL = sourceEquation->getRangeL();
		this->rangeR = sourceEquation->getRangeR();
	}

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

// True concept here only apply to boolean and one-bit bit vector results
bool SimulatedEquation::isTrue() const
{
	switch (this->currentValue.getType())
	{
	case MachineValue::Type_t::boolean:
		return this->currentValue.getBooleanValue();
		break;
	case MachineValue::Type_t::bitVector:
	{
		auto bitVectorValue = this->currentValue.getBitVectorValue();

		if (bitVectorValue.getSize() == 1)
		{
			if (bitVectorValue == BitVectorValue::allOnes(1))
			{
				return true;
			}
		}
		break;
	}
	case MachineValue::Type_t::nullType:
		break;
	}

	return false;
}

void SimulatedEquation::computeCurrentValue()
{
	MachineValue previousValue{this->currentValue};

	MachineValue computedValue{};
	switch (this->operatorType)
	{
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::identity:
		// PRE: there is one operand
		// PRE: the operand is valid

		computedValue = this->operands[0]->getCurrentValue();
		break;
	case Equation::Operator_t::equalOp:
	{
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(this->operands[0]->getCurrentValue() == this->operands[1]->getCurrentValue())};
		break;
	}
	case Equation::Operator_t::diffOp:
	{
		// PRE: there are two operands
		// PRE: the two operands are valid

		computedValue = BooleanValue{(this->operands[0]->getCurrentValue() != this->operands[1]->getCurrentValue())};
		break;
	}
	case Equation::Operator_t::extractOp:
	{
		// PRE: there is one operand
		// PRE: the operand is valid
		// PRE: the operand is of type Bit Vector
		// PRE: right range is -1 and left range is < to Bit Vector size and > to 0
		//      OR right range is >= to 0 and <= left range AND left range is < to Bit Vector size

		auto operandValue = this->operands[0]->getCurrentValue().getBitVectorValue();
		if (this->rangeR != -1)
		{
			int range = this->rangeL - this->rangeR + 1;
			auto subVector = BitVectorValue::allZeros(range);

			for (int i = 0 ; i < range ; i++)
			{
				subVector.setBit(i, operandValue[this->rangeR + i]);
			}

			computedValue = subVector;
		}
		else
		{
			computedValue = BooleanValue{operandValue[rangeL]};
		}
		break;
	}
	case Equation::Operator_t::concatOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: all operands are either Boolean or Bit Vector

		int sizeCount = 0;
		for (auto& currentOperand : this->operands)
		{
			if (currentOperand->getType() == MachineValue::Type_t::bitVector)
			{
				sizeCount += currentOperand->getCurrentValue().getBitVectorValue().getSize();
			}
			else // (currentOperandValue.getValueType() == MachineValue::ValueType_t::boolean)
			{
				sizeCount++;
			}
		}

		auto concatVector = BitVectorValue::allZeros(sizeCount);

		int currentBit = sizeCount - 1;
		for (auto& currentOperand : this->operands)
		{
			if (currentOperand->getType() == MachineValue::Type_t::bitVector)
			{
				auto currentOperandBitVectorValue = currentOperand->getCurrentValue().getBitVectorValue();
				for (int i = static_cast<int>(currentOperandBitVectorValue.getSize())-1 ; i >= 0 ; i--)
				{
					concatVector.setBit(currentBit, currentOperandBitVectorValue[i]);
					currentBit--;
				}
			}
			else // (currentOperandValue.getValueType() == MachineValue::ValueType_t::boolean)
			{
				auto currentOperandBooleanValue = currentOperand->getCurrentValue().getBooleanValue();
				concatVector.setBit(currentBit, currentOperandBooleanValue);
				currentBit--;
			}
		}

		computedValue = concatVector;
		break;
	}
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::xnorOp:
	{
		// PRE: there are two or more operands
		// PRE: the operands are all valid
		// PRE: operands are either Boolean or Bit Vector
		// PRE: all operands have the same type
		// PRE: Bit Vector operands have the same size

		auto firstOperand = this->operands[0];
		if (firstOperand->getType() == MachineValue::Type_t::bitVector)
		{
			auto firstOperandBitVectorValue = firstOperand->getCurrentValue().getBitVectorValue();
			uint firstOperandsSize = firstOperandBitVectorValue.getSize();

			BitVectorValue partialResult{};

			switch (this->operatorType)
			{
			case Equation::Operator_t::andOp:
			case Equation::Operator_t::nandOp:
				partialResult = BitVectorValue::allOnes(firstOperandsSize);
				break;
			case Equation::Operator_t::orOp:
			case Equation::Operator_t::norOp:
			case Equation::Operator_t::xorOp:
			case Equation::Operator_t::xnorOp:
				partialResult = BitVectorValue::allZeros(firstOperandsSize);
				break;
			default:
				break;
			}

			for (auto& operand : this->operands)
			{
				auto currentOperandValue = operand->getCurrentValue();
				auto bitVectorOperandValue = currentOperandValue.getBitVectorValue();

				switch (this->operatorType)
				{
				case Equation::Operator_t::andOp:
				case Equation::Operator_t::nandOp:
					partialResult &= bitVectorOperandValue;
					break;
				case Equation::Operator_t::orOp:
				case Equation::Operator_t::norOp:
					partialResult |= bitVectorOperandValue;
					break;
				case Equation::Operator_t::xorOp:
				case Equation::Operator_t::xnorOp:
					partialResult ^= bitVectorOperandValue;
					break;
				default:
					break;
				}
			}

			computedValue = partialResult;
		}
		else // (firstOperand->getType() == MachineValue::ValueType_t::boolean)
		{
			BooleanValue partialResult{};

			switch (this->operatorType)
			{
			case Equation::Operator_t::andOp:
			case Equation::Operator_t::nandOp:
				partialResult = BooleanValue::trueValue();
				break;
			case Equation::Operator_t::orOp:
			case Equation::Operator_t::norOp:
			case Equation::Operator_t::xorOp:
			case Equation::Operator_t::xnorOp:
				partialResult = BooleanValue::falseValue();
				break;
			default:
				break;
			}

			for (auto& operand : this->operands)
			{
				auto currentOperandValue = operand->getCurrentValue();
				auto booleanOperandValue = currentOperandValue.getBooleanValue();

				switch (this->operatorType)
				{
				case Equation::Operator_t::andOp:
				case Equation::Operator_t::nandOp:
					partialResult &= booleanOperandValue;
					break;
				case Equation::Operator_t::orOp:
				case Equation::Operator_t::norOp:
					partialResult |= booleanOperandValue;
					break;
				case Equation::Operator_t::xorOp:
				case Equation::Operator_t::xnorOp:
					partialResult ^= booleanOperandValue;
					break;
				default:
					break;
				}
			}

			computedValue = partialResult;
		}

		break;
	}
	}

	if (this->isInverted() == true)
	{
		// PRE: the result is either Boolean or Bit Vector
		if (computedValue.getType() == MachineValue::Type_t::bitVector)
		{
			computedValue = ~(computedValue.getBitVectorValue());
		}
		else // (computedValue.getType() == MachineValue::ValueType_t::boolean)
		{
			computedValue = !(computedValue.getBooleanValue());
		}
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
	case Equation::Operator_t::notOp:
	case Equation::Operator_t::nandOp:
	case Equation::Operator_t::norOp:
	case Equation::Operator_t::xnorOp:
		return true;
		break;
	case Equation::Operator_t::identity:
	case Equation::Operator_t::andOp:
	case Equation::Operator_t::orOp:
	case Equation::Operator_t::xorOp:
	case Equation::Operator_t::equalOp:
	case Equation::Operator_t::diffOp:
	case Equation::Operator_t::extractOp:
	case Equation::Operator_t::concatOp:
		return false;
		break;
	}
}
